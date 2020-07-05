////////////////////////////////////////////////////////////////////////////////
// GTLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2020 BY ARMCODER - milton@armcoder.com.br
//
// https://github.com/ARMCoderBR/gtlsim
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

#include <ncurses.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <gtk/gtk.h>

#include "update.h"
#include "board.h"
#include "bitswitch.h"
#include "update.h"
#include "dis7seg.h"
#include "clkgen.h"
#include "boardres.h"
#include "exmachina.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void gtk_manual_switch_set_img(void *ptarget, /*int swtype,*/ int value);
void gtk_pushbutton_set_img(void *ptarget, /*int swtype,*/ int value);
void gtk_led_set_img(GtkImage *gtkimg, led_color_t color, int value);
void gtk_7seg_set_img(GtkImage *gtkimg, led_color_t color, int segmap);

////////////////////////////////////////////////////////////////////////////////
project_ctx_t *project_init(void){

    project_ctx_t *pctx = malloc(sizeof(project_ctx_t));

    if (!pctx) exit(-1);//return NULL;

    memset(pctx, 0, sizeof(project_ctx_t));

    pctx->boardclk = NULL;
    pctx->clock_pausing = 0;
    pctx->iclk = 0;

    pctx->clock_faster_req = pctx->clock_slower_req = pctx->clock_pause_req = false;
    pctx->switch_to_toggle = NULL;
    pctx->pushb_timer = 0;
    pctx->remote_key = 0;

    return pctx;
}

////////////////////////////////////////////////////////////////////////////////
void sigterm_handler(int sig){

    endwin();
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////

void board_set_clk(project_ctx_t *ctx, clkgen *clk){

    if (!ctx) return;

    ctx->boardclk = clk;
}

////////////////////////////////////////////////////////////////////////////////
void board_refresh_a(project_ctx_t *pctx, board_object *b, int new_h, int new_w){

    if (b->type != BOARD) return;   // Erro interno - nunca deve acontecer.

    b = b->objptr_root;

    while (b){

        switch (b->type){

        case MANUAL_SWITCH:
        case PUSHBUTTON:
            {
                bitswitch* bs = b->objptr;

                if ((b->key) && (b->key == pctx->remote_key)){

                    pctx->switch_to_toggle = bs;
                    pctx->remote_key = 0;
                }

                if (bs->value != b->indicator_value){

                    if (b->type == MANUAL_SWITCH)
                        gtk_manual_switch_set_img(b->gtk_widget, /*int swtype,*/ bs->value);
                    else
                        gtk_pushbutton_set_img(b->gtk_widget, /*int swtype,*/ bs->value ^ bs->initial);
                    b->indicator_value = bs->value;
                }
            }
            break;

        case LED:
            {
                indicator* out = b->objptr;

                if (out->value != b->indicator_value){

                    gtk_led_set_img((GtkImage*)b->gtk_widget, b->color, out->value);
                    b->indicator_value = out->value;
                }
            }
            break;

        case DIS7SEG:
            {
                dis7seg *dis = b->objptr;

                if (dis->segmap != b->indicator_value){

                    gtk_7seg_set_img((GtkImage*)b->gtk_widget, b->color, dis->segmap);
                    b->indicator_value = dis->segmap;
                }
            }
            break;

        case XDIGIT:        //TODO
//            {
//                indicator* out = b->objptr;
//                char s[10];
//                sprintf(s,"%X",out->value & 0x0F);
//                waddstr(bctx->janela1,s);
//            }
            break;

        case BOARD:

            board_refresh_a(pctx, b, 0/*b->pos_h*/, 0/*b->pos_w*/);
            break;
        }

        b = b->objptr_next;
    }
}

////////////////////////////////////////////////////////////////////////////////
void board_refresh(project_ctx_t *pctx, board_object *b){

    board_refresh_a(pctx, b,0,0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define NCLKS 10

const int CLKS_PERIOD_US[NCLKS] = {
                             2000000,    //2s
                             1000000,    //1s
                             500000,     //500ms
                             250000,     //250ms
                             100000,     //100ms
                             50000,      //50ms
                             25000,      //25ms
                             10000,      //10ms
                             5000,       //5ms
                             2000,       //2ms
};

////////////////////////////////////////////////////////////////////////////////
void clock_redraw(project_ctx_t *pctx){

//    pthread_mutex_lock(&bctx->ncursesmutex);
//
//    int i;
//
//    char s[50] = "Clk:[";
//
//    for (i = 0; i < NCLKS; i++)
//        if (i != bctx->iclk)
//            strcat(s,"-");
//        else
//            strcat(s,"|");
//    strcat(s,"]");
//
//    wmove(bctx->janela3,0,1);
//    waddstr(bctx->janela3,s);
//
//    wmove(bctx->janela3,1,1);
//    if (!bctx->clock_pausing)
//        waddstr(bctx->janela3,"RUNNING");
//    else
//        waddstr(bctx->janela3,"PAUSED ");
//
//    wmove(bctx->janela3,2,1);
//    waddstr(bctx->janela3,"F10:Pause/Pulse  F11:Slower  F12:Faster");
//
//    wrefresh(bctx->janela3);
//
//    pthread_mutex_unlock(&bctx->ncursesmutex);
}

////////////////////////////////////////////////////////////////////////////////
void clock_reinit(project_ctx_t *pctx){

    int clock_period_us = CLKS_PERIOD_US[pctx->iclk];
    clkgen_set_us(pctx->boardclk, clock_period_us);
    clock_redraw(pctx);
}

////////////////////////////////////////////////////////////////////////////////
void clock_faster(project_ctx_t *pctx){

    if (pctx->clock_pausing){

        pctx->clock_pausing = 0;
        clkgen_pause(pctx->boardclk, pctx->clock_pausing);
        clock_redraw(pctx);
        return;
    }

    if (pctx->iclk < (NCLKS-1)){

        pctx->iclk++;
        int clock_period_us = CLKS_PERIOD_US[pctx->iclk];
        clkgen_set_us(pctx->boardclk, clock_period_us);
        clock_redraw(pctx);
    }
}

////////////////////////////////////////////////////////////////////////////////
void clock_slower(project_ctx_t *pctx){

    if (pctx->clock_pausing){

        pctx->clock_pausing = 0;
        clkgen_pause(pctx->boardclk, pctx->clock_pausing);
        clock_redraw(pctx);
        return;
    }

    if (pctx->iclk > 0){

        pctx->iclk--;
        int clock_period_us = CLKS_PERIOD_US[pctx->iclk];
        clkgen_set_us(pctx->boardclk, clock_period_us);
        clock_redraw(pctx);
    }
}

////////////////////////////////////////////////////////////////////////////////
void clock_pause(project_ctx_t *pctx){

    if (!pctx->clock_pausing){

        pctx->clock_pausing = 1;
        clkgen_pause(pctx->boardclk, pctx->clock_pausing);
        clock_redraw(pctx);
        return;
    }
    else{

        clkgen_step(pctx->boardclk);
        usleep(10000);
    }

    clock_redraw(pctx);
}

////////////////////////////////////////////////////////////////////////////////
board_object *board_create(int width, int height, int key, char *name){

    board_object *b = malloc(sizeof(board_object));

    if (!b) return b;

    b->type = BOARD;
    b->objptr = NULL;

    if (name)
        strncpy(b->name, name, NAMESIZE);
    else
        b->name[0] = 0;

    b->objptr_root = NULL;
    b->objptr_next = NULL;

    b->board_frame = (GtkFrame*)gtk_frame_new(name);
    b->board_grid = (GtkGrid*)gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (b->board_frame), (GtkWidget*)b->board_grid);

    b->destroy = (void*)board_destroy;
    return b;
}

////////////////////////////////////////////////////////////////////////////////
void board_destroy_a(board_object *dest){

//	printf("board_destroy()\n");
    if (dest == NULL) return;
    board_object *b = dest;
    if (b == NULL) return;

    if (b->type != BOARD) return;   // Erro interno - nunca deve acontecer.

    board_object *bo = b->objptr_root;
    while (bo){

        if (bo->type == BOARD){

        	//printf("will board_destroy()\n");

            board_destroy_a(bo);
        }else{

        	//printf("will object_destroy()\n");

        }

        board_object *tofree = bo;
        bo = bo->objptr_next;
        free(tofree);
    }
    b->objptr_root = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void board_destroy(board_object **dest){

	board_destroy_a(*dest);
    free(*dest);
    *dest = NULL;
}

////////////////////////////////////////////////////////////////////////////////
board_object *mainboard_create(char *name){

    return board_create(0, 0, 0, name);
}

////////////////////////////////////////////////////////////////////////////////
void propagate_parent_ctx(board_object *b){

    if (!b) return;
    if (b->type != BOARD) return;   // Folha da árvore, não tem filhos, retorna.

    board_object *pb = b->objptr_root;

    while (pb){

        pb->parent_pctx = b->parent_pctx;
        propagate_parent_ctx(pb);
        pb = pb->objptr_next;
    }
}

////////////////////////////////////////////////////////////////////////////////
int board_add_object(board_object *b, board_object *newobject){

    if (!b) return -2;
    if (!newobject) return -2;

    if (b->type != BOARD) return -10;

    newobject->parent_pctx = b->parent_pctx;

    board_object *pb = b->objptr_root;

    if (!pb)
        b->objptr_root = newobject;
    else
    while(b->objptr_next)
        b = b->objptr_next;

    b->objptr_next = newobject;

    newobject->objptr_next = NULL;  // Dupla garantia.

    propagate_parent_ctx(newobject);

    return 0;
}


static GdkPixbuf *switch_on, *switch_off;
static bool switch_pixbuf_initted = false;

////////////////////////////////////////////////////////////////////////////////
void gtk_manual_switch_set_img(void *ptarget, /*int swtype,*/ int value){

    if (!switch_pixbuf_initted){

        boardres_init_streams();

        switch_on = gdk_pixbuf_new_from_stream(switch_on_s, NULL, NULL);
        switch_off = gdk_pixbuf_new_from_stream(switch_off_s, NULL, NULL);

        switch_pixbuf_initted = true;
    }

    if (value)
        gtk_image_set_from_pixbuf ((GtkImage *)ptarget, switch_on);
    else
        gtk_image_set_from_pixbuf ((GtkImage *)ptarget, switch_off);
}

static GdkPixbuf *pushb_on, *pushb_off;
static bool pushb_pixbuf_initted = false;

////////////////////////////////////////////////////////////////////////////////
void gtk_pushbutton_set_img(void *ptarget, /*int swtype,*/ int value){

    if (!pushb_pixbuf_initted){

        //boardres_init_streams();

        pushb_on = gdk_pixbuf_new_from_file("../pushbutton-on.png", NULL);//gdk_pixbuf_new_from_stream(switch_on_s, NULL, NULL);
        pushb_off = gdk_pixbuf_new_from_file("../pushbutton-off.png", NULL);//gdk_pixbuf_new_from_stream(switch_off_s, NULL, NULL);

        pushb_pixbuf_initted = true;
    }

    if (value)
        gtk_image_set_from_pixbuf ((GtkImage *)ptarget, pushb_on);
    else
        gtk_image_set_from_pixbuf ((GtkImage *)ptarget, pushb_off);
}

////////////////////////////////////////////////////////////////////////////////
static void
bitswitch_toggle (GtkWidget *widget, GtkWidget *otherwidget,
             gpointer   ptr) {

    board_object *bo = ptr;

    project_ctx_t *pctx = bo->parent_pctx;

    if (pctx->pushb_timer) return;

    bitswitch* bs = bo->objptr;
    pctx->switch_to_toggle = bs;
}

////////////////////////////////////////////////////////////////////////////////
static void
pushbutton_activate (GtkWidget *widget, GtkWidget *otherwidget,
             gpointer   ptr) {

    board_object *bo = ptr;

    project_ctx_t *pctx = bo->parent_pctx;

    if (pctx->pushb_timer) return;

    bitswitch* bs = bo->objptr;
    pctx->pushb_timer = 100000 / SIM_STEP_TIME_US;  // 100 ms
    pctx->pushbutton_to_release = pctx->switch_to_toggle = bs;
}

////////////////////////////////////////////////////////////////////////////////
int board_add_manual_switch(board_object *b, bitswitch *bs, int pos_w, int pos_h, int key, char *name){

    //printf("board_add_manual_switch(%s, %p)\n",name, bs);

    if (!b) return -2;
    if (!bs) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    obja->type   = MANUAL_SWITCH;
    obja->objptr = bs;
    obja->key = key;

    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    GtkImage *newimg = (GtkImage *)gtk_image_new();

    gtk_manual_switch_set_img(newimg, /*0,*/ bs->value);

    GtkLabel *newlbl = (GtkLabel *)gtk_label_new(name);

    GtkEventBox *ebox = (GtkEventBox *)gtk_event_box_new();
    gtk_container_add (GTK_CONTAINER (ebox), (GtkWidget*)newimg);
    gtk_grid_attach (b->board_grid, (GtkWidget*)ebox, pos_w, pos_h, 1, 1);
    gtk_grid_attach (b->board_grid, (GtkWidget*)newlbl, pos_w, 1+pos_h, 1, 1);

    obja->gtk_widget = (GtkWidget*)newimg;
    obja->indicator_value = bs->value;

    //gtk_widget_set_events (ebox, GDK_BUTTON_PRESS_MASK);
    //printf("g_signal_connect bs:%p\n",bs);
    g_signal_connect (ebox, "button_press_event", G_CALLBACK (bitswitch_toggle), obja/*bs*/);

    /* Yet one more thing you need an X window for ... */

    gtk_widget_realize ((GtkWidget*)ebox);
    gdk_window_set_cursor (gtk_widget_get_window(ebox), gdk_cursor_new_for_display (gdk_display_get_default(), GDK_HAND1));

    return board_add_object(b, obja);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_pushbutton(board_object *b, bitswitch *bs, int pos_w, int pos_h, int key, char *name){

    //printf("board_add_manual_switch(%s, %p)\n",name, bs);

    if (!b) return -2;
    if (!bs) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    obja->type   = PUSHBUTTON;
    obja->objptr = bs;
    obja->key = key;

    bs->initial = bs->value;

    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    GtkImage *newimg = (GtkImage *)gtk_image_new();

    gtk_pushbutton_set_img(newimg, /*0,*/ 0);

    GtkLabel *newlbl = (GtkLabel *)gtk_label_new(name);

    GtkEventBox *ebox = (GtkEventBox *)gtk_event_box_new();
    gtk_container_add (GTK_CONTAINER (ebox), (GtkWidget*)newimg);
    gtk_grid_attach (b->board_grid, (GtkWidget*)ebox, pos_w, pos_h, 2, 1);
    gtk_grid_attach (b->board_grid, (GtkWidget*)newlbl, pos_w, 1+pos_h, 2, 1);

    obja->gtk_widget = (GtkWidget*)newimg;
    obja->indicator_value = bs->value;

    //gtk_widget_set_events (ebox, GDK_BUTTON_PRESS_MASK);
    //printf("g_signal_connect bs:%p\n",bs);
    g_signal_connect (ebox, "button_press_event", G_CALLBACK (pushbutton_activate), obja/*bs*/);

    /* Yet one more thing you need an X window for ... */

    gtk_widget_realize ((GtkWidget*)ebox);
    gdk_window_set_cursor (gtk_widget_get_window(ebox), gdk_cursor_new_for_display (gdk_display_get_default(), GDK_HAND1));

    return board_add_object(b, obja);
}


static GdkPixbuf *disp7_0, *disp7_1, *disp7_2, *disp7_3, *disp7_4, *disp7_5, *disp7_6, *disp7_7, *disp7_8, *disp7_9;
static GdkPixbuf *disp7_blank, *disp7_dash;
static bool disp7_pixbuf_initted = false;

void gtk_7seg_set_img(GtkImage *gtkimg, led_color_t color, int segmap){

    if (!disp7_pixbuf_initted){

        boardres_init_streams();

        disp7_0 = gdk_pixbuf_new_from_stream(disp7_0_s, NULL, NULL);
        disp7_1 = gdk_pixbuf_new_from_stream(disp7_1_s, NULL, NULL);
        disp7_2 = gdk_pixbuf_new_from_stream(disp7_2_s, NULL, NULL);
        disp7_3 = gdk_pixbuf_new_from_stream(disp7_3_s, NULL, NULL);
        disp7_4 = gdk_pixbuf_new_from_stream(disp7_4_s, NULL, NULL);
        disp7_5 = gdk_pixbuf_new_from_stream(disp7_5_s, NULL, NULL);
        disp7_6 = gdk_pixbuf_new_from_stream(disp7_6_s, NULL, NULL);
        disp7_7 = gdk_pixbuf_new_from_stream(disp7_7_s, NULL, NULL);
        disp7_8 = gdk_pixbuf_new_from_stream(disp7_8_s, NULL, NULL);
        disp7_9 = gdk_pixbuf_new_from_stream(disp7_9_s, NULL, NULL);
        disp7_blank = gdk_pixbuf_new_from_stream(disp7_blank_s, NULL, NULL);
        disp7_dash = gdk_pixbuf_new_from_stream(disp7_dash_s, NULL, NULL);
        disp7_pixbuf_initted = true;
    }

    switch(segmap){

    case MSK_A|MSK_B|MSK_C|MSK_D|MSK_E|MSK_F:           //'0'
        gtk_image_set_from_pixbuf (gtkimg, disp7_0);
        break;

    case MSK_B|MSK_C:                                   //'1'
        gtk_image_set_from_pixbuf (gtkimg, disp7_1);
        break;

    case MSK_A|MSK_B|MSK_D|MSK_E|MSK_G:                 //'2'
        gtk_image_set_from_pixbuf (gtkimg, disp7_2);
        break;

    case MSK_A|MSK_B|MSK_C|MSK_D|MSK_G:                 //'3'
        gtk_image_set_from_pixbuf (gtkimg, disp7_3);
        break;

    case MSK_B|MSK_C|MSK_F|MSK_G:                       //'4'
        gtk_image_set_from_pixbuf (gtkimg, disp7_4);
        break;

    case MSK_A|MSK_C|MSK_D|MSK_F|MSK_G:                 //'5'
        gtk_image_set_from_pixbuf (gtkimg, disp7_5);
        break;

    case MSK_A|MSK_C|MSK_D|MSK_E|MSK_F|MSK_G:           //'6'
        gtk_image_set_from_pixbuf (gtkimg, disp7_6);
        break;

    case MSK_A|MSK_B|MSK_C:                             //'7'
        gtk_image_set_from_pixbuf (gtkimg, disp7_7);
        break;

    case MSK_A|MSK_B|MSK_C|MSK_D|MSK_E|MSK_F|MSK_G:     //'8'
        gtk_image_set_from_pixbuf (gtkimg, disp7_8);
        break;

    case MSK_A|MSK_B|MSK_C|MSK_D|MSK_F|MSK_G:           //'9'
        gtk_image_set_from_pixbuf (gtkimg, disp7_9);
        break;

    case MSK_G:                                         //'-'
        gtk_image_set_from_pixbuf (gtkimg, disp7_dash);
        break;

    default:                                            //Blank
        gtk_image_set_from_pixbuf (gtkimg, disp7_blank);
        break;
    }
}

static GdkPixbuf *led_green_on, *led_green_off;
static GdkPixbuf *led_yellow_on, *led_yellow_off;
static GdkPixbuf *led_blue_on, *led_blue_off;
static GdkPixbuf *led_white_on, *led_white_off;
static GdkPixbuf *led_red_on, *led_red_off;
static bool led_pixbuf_initted = false;

////////////////////////////////////////////////////////////////////////////////
void gtk_led_set_img(GtkImage *gtkimg, led_color_t color, int value){

    if (!led_pixbuf_initted){

        boardres_init_streams();

        led_green_on = gdk_pixbuf_new_from_stream(led_green_on_s, NULL, NULL);
        led_green_off = gdk_pixbuf_new_from_stream(led_green_off_s, NULL, NULL);
        led_yellow_on = gdk_pixbuf_new_from_stream(led_yellow_on_s, NULL, NULL);
        led_yellow_off = gdk_pixbuf_new_from_stream(led_yellow_off_s, NULL, NULL);
        led_blue_on = gdk_pixbuf_new_from_stream(led_blue_on_s, NULL, NULL);
        led_blue_off = gdk_pixbuf_new_from_stream(led_blue_off_s, NULL, NULL);
        led_white_on = gdk_pixbuf_new_from_stream(led_white_on_s, NULL, NULL);
        led_white_off = gdk_pixbuf_new_from_stream(led_white_off_s, NULL, NULL);
        led_red_on = gdk_pixbuf_new_from_stream(led_red_on_s, NULL, NULL);
        led_red_off = gdk_pixbuf_new_from_stream(led_red_off_s, NULL, NULL);

        led_pixbuf_initted = true;
    }

    switch(color){

    case LED_GREEN:
        if (value)
            gtk_image_set_from_pixbuf (gtkimg, led_green_on);
        else
            gtk_image_set_from_pixbuf (gtkimg, led_green_off);
        break;
    case LED_YELLOW:
        if (value)
            gtk_image_set_from_pixbuf (gtkimg, led_yellow_on);
        else
            gtk_image_set_from_pixbuf (gtkimg, led_yellow_off);
        break;
    case LED_BLUE:
        if (value)
            gtk_image_set_from_pixbuf (gtkimg, led_blue_on);
        else
            gtk_image_set_from_pixbuf (gtkimg, led_blue_off);
        break;
    case LED_WHITE:
        if (value)
            gtk_image_set_from_pixbuf (gtkimg, led_white_on);
        else
            gtk_image_set_from_pixbuf (gtkimg, led_white_off);
        break;
    default://case LED_RED:
        if (value)
            gtk_image_set_from_pixbuf (gtkimg, led_red_on);
        else
            gtk_image_set_from_pixbuf (gtkimg, led_red_off);
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void gtk_led_img_update_green(void *ptarget, int value){

    gtk_led_set_img((GtkImage *)ptarget, LED_GREEN, value);
}

////////////////////////////////////////////////////////////////////////////////
void gtk_led_img_update_yellow(void *ptarget, int value){

    gtk_led_set_img((GtkImage *)ptarget, LED_YELLOW, value);
}

////////////////////////////////////////////////////////////////////////////////
void gtk_led_img_update_blue(void *ptarget, int value){

    gtk_led_set_img((GtkImage *)ptarget, LED_BLUE, value);
}

////////////////////////////////////////////////////////////////////////////////
void gtk_led_img_update_white(void *ptarget, int value){

    gtk_led_set_img((GtkImage *)ptarget, LED_WHITE, value);
}

////////////////////////////////////////////////////////////////////////////////
void gtk_led_img_update_red(void *ptarget, int value){

    gtk_led_set_img((GtkImage *)ptarget, LED_RED, value);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_led(board_object *b, indicator *out, int pos_w, int pos_h, char *name, led_color_t color){

    if (!b) return -2;
    if (!out) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    obja->type   = LED;
    obja->color = color;
    obja->objptr = out;

    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    GtkImage *newimg = (GtkImage *)gtk_image_new();
    gtk_led_set_img(newimg, color, out->value);

    GtkLabel *newlbl = (GtkLabel *)gtk_label_new(name);

    gtk_grid_attach (b->board_grid, (GtkWidget*)newimg, pos_w, pos_h, 1, 1);
    gtk_grid_attach (b->board_grid, (GtkWidget*)newlbl, pos_w, 1+pos_h, 1, 1);

    obja->gtk_widget = (GtkWidget*)newimg;
    obja->indicator_value = out->value;

    return board_add_object(b, obja);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_display_7seg(board_object *b, dis7seg *out, int pos_w, int pos_h, char *name, led_color_t color){

    if (!b) return -2;
    if (!out) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    obja->type   = DIS7SEG;
    obja->color = color;
    obja->objptr = out;

    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    GtkImage *newimg = (GtkImage *)gtk_image_new();
    gtk_7seg_set_img(newimg, color, out->segmap);

    gtk_grid_attach (b->board_grid, (GtkWidget*)newimg, pos_w, pos_h, 2, 1);

    obja->gtk_widget = (GtkWidget*)newimg;
    obja->indicator_value = out->segmap;

    return board_add_object(b, obja);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_xdigit(board_object *b, indicator *out, int pos_w, int pos_h, char *name, led_color_t color){

/*
  A
  STATUS
*/
    if (!b) return -2;
    if (!out) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    obja->type   = XDIGIT;
    obja->objptr = out;

    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    return board_add_object(b, obja);
}

static GdkPixbuf *transparent;
static bool transp_pixbuf_initted = false;

////////////////////////////////////////////////////////////////////////////////
int board_add_spacer(board_object *b, int pos_w, int pos_h){

    if (!b) return -2;

    if (!transp_pixbuf_initted){

        transparent = gdk_pixbuf_new_from_file("../transparent.png", NULL);
        transp_pixbuf_initted = true;
    }

    GtkImage *transimg = (GtkImage *)gtk_image_new();
    gtk_image_set_from_pixbuf (transimg, transparent);

    gtk_grid_attach (b->board_grid, (GtkWidget*)transimg, pos_w, pos_h, 1, 1);
    return 0;
}



////////////////////////////////////////////////////////////////////////////////
int board_add_board(board_object *b, board_object *board, int pos_w, int pos_h){

    if (!b) return -2;
    if (!board) return -2;

    gtk_grid_attach (b->board_grid, (GtkWidget*)board->board_frame, pos_w, pos_h, 1, 1);

    return board_add_object(b, board);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_boardWH(board_object *b, board_object *board, int pos_w, int pos_h, int width, int heigth){

    if (!b) return -2;
    if (!board) return -2;

    gtk_grid_attach (b->board_grid, (GtkWidget*)board->board_frame, pos_w, pos_h, width, heigth);

    return board_add_object(b, board);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void board_initialize(void){

}

////////////////////////////////////////////////////////////////////////////////
int board_run(project_ctx_t *ctx, event_context_t *ec, board_object *board){

    if (!ctx) exit(-3);

    ctx->main_board = board;

    if (!board) return -2;

    if (board->type != BOARD)
        return -10;

    signal (SIGHUP,SIG_IGN);
    signal (SIGINT,SIG_IGN);
    signal (SIGTERM,sigterm_handler);
    signal (SIGTSTP,SIG_IGN);

    clock_reinit(ctx);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int board_run_b(project_ctx_t *pctx, event_context_t *ec, board_object *board){

    event_mutex_lock(ec);
    while (event_process(ec));
    event_mutex_unlock(ec);

    if (pctx->clock_faster_req || (pctx->remote_key == KEY_CLK_FAST)){

        pctx->clock_faster_req = false;
        pctx->remote_key = 0;
        clock_faster(pctx);
    }

    if (pctx->clock_slower_req){

        pctx->clock_slower_req = false;
        clock_slower(pctx);
    }

    if (pctx->clock_pause_req){

        pctx->clock_pause_req = false;
        clock_pause(pctx);
    }

    bitswitch* bs = pctx->switch_to_toggle;
    if (bs){

        bitswitch_setval(bs, bs->value ?0:1);
        pctx->switch_to_toggle = NULL;
    }

    if (pctx->pushb_timer){

        --pctx->pushb_timer;
        if (!pctx->pushb_timer){

            bitswitch* bs = pctx->pushbutton_to_release;
            if (bs){

                bitswitch_setval(bs, bs->value ?0:1);
                pctx->pushbutton_to_release = NULL;
            }
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int board_run_c(project_ctx_t *ctx, event_context_t *ec, board_object *board){

    if (!ctx->clock_pausing)
    	clock_pause(ctx);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
static void
gtk_clock_pause (GtkWidget *widget,
                 gpointer   pctx) {

    ((project_ctx_t*)pctx)->clock_pause_req = 1;
}

////////////////////////////////////////////////////////////////////////////////
static void
gtk_clock_slower (GtkWidget *widget,
                 gpointer   pctx) {

    ((project_ctx_t*)pctx)->clock_slower_req = 1;
}

////////////////////////////////////////////////////////////////////////////////
static void
gtk_clock_faster (GtkWidget *widget,
                 gpointer   pctx) {

    ((project_ctx_t*)pctx)->clock_faster_req = 1;
}

////////////////////////////////////////////////////////////////////////////////
void board_add_clock_buttons(GtkGrid *maingrid, project_ctx_t *pctx){

    GtkWidget *buttonfaster = gtk_button_new_with_label (">>");
    GtkWidget *buttonslower = gtk_button_new_with_label ("<<");
    GtkWidget *buttonpause  = gtk_button_new_with_label ("||>");

    gtk_grid_attach ((GtkGrid*)maingrid, buttonpause, 1, 3, 1, 1);
    gtk_grid_attach ((GtkGrid*)maingrid, buttonslower, 2, 3, 1, 1);
    gtk_grid_attach ((GtkGrid*)maingrid, buttonfaster, 3, 3, 1, 1);

    g_signal_connect (buttonpause, "clicked", G_CALLBACK (gtk_clock_pause), pctx);
    g_signal_connect (buttonslower, "clicked", G_CALLBACK (gtk_clock_slower), pctx);
    g_signal_connect (buttonfaster, "clicked", G_CALLBACK (gtk_clock_faster), pctx);
}
