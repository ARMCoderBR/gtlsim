////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define LINHAS_JANELA2 3
#define LINHAS_JANELA2B (LINHAS_JANELA2+2)

////////////////////////////////////////////////////////////////////////////////
project_ctx_t *project_init(void){

    project_ctx_t *pctx = malloc(sizeof(project_ctx_t));

    if (!pctx) exit(-1);//return NULL;

    memset(pctx, 0, sizeof(project_ctx_t));

//    bctx->TERM_LINES = 0;
//    bctx->TERM_COLS = 0;

    pthread_mutex_init(&pctx->setrefmutex, NULL);
    pctx->reader_ok = 0;

    pthread_mutex_init(&pctx->ncursesmutex, NULL);

    pctx->boardclk = NULL;
    pctx->refresh_run = 0;

    pctx->focustable_done = 0;
    pctx->num_focuseable_boards = 0;
    pctx->current_board_on_focus = 0;

    pctx->clock_pausing = 0;
    pctx->iclk = 0;

    pctx->clock_faster_req = pctx->clock_slower_req = pctx->clock_pause_req = false;
    pctx->switch_to_toggle = NULL;

    return pctx;
}

////////////////////////////////////////////////////////////////////////////////
void combine_7seg(int segmap, int C[]){

    int i;
    for (i = 0; i < 10; i++)
        C[i] = ' ';

/*
   +----+----+----+----+
   |                   |
   | C0   C1   C2      |
   |                   |
   +----+----+----+----+
   |                   |
   | C3   C4   C5      |
   |                   |
   +----+----+----+----+
   |                   |
   | C6   C7   C8   C9 |
   |                   |
   +----+----+----+----+

   A F    C0
   0 0    SPACE
   0 1    ACS_ULCORNER
   1 0    ACS_HLINE
   1 1    ACS_ULCORNER
*/

    switch(segmap & (MSK_A|MSK_F)){

    case MSK_F:
        C[0] = ACS_URCORNER;
        break;
    case MSK_A|MSK_F:
        C[0] = ACS_ULCORNER;
        break;
    case MSK_A:
        C[0] = ACS_HLINE;
        break;
    }

/*
   A      C1
   0      SPACE
   1      ACS_HLINE
*/

    if (segmap & MSK_A)
        C[1] = ACS_HLINE;

/*
   A B    C2
   0 0    SPACE
   0 1    ACS_URCORNER
   1 0    ACS_HLINE
   1 1    ACS_URCORNER
*/

    switch(segmap & (MSK_A|MSK_B)){

    case MSK_B:
    case MSK_A|MSK_B:
        C[2] = ACS_URCORNER;
        break;
    case MSK_A:
        C[2] = ACS_HLINE;
        break;
    }

/*
   E F G  C3
   0 0 0  SPACE
   0 0 1  ACS_HLINE
   0 1 0  ACS_LLCORNER
   0 1 1  ACS_LLCORNER
   1 0 0  ACS_ULCORNER
   1 0 1  ACS_ULCORNER
   1 1 0  ACS_VLINE
   1 1 1  ACS_LTEE
*/

    switch(segmap & (MSK_E|MSK_F|MSK_G)){

    case MSK_F:
    case MSK_F|MSK_G:
        C[3] = ACS_LLCORNER;
        break;
    case MSK_E:
    case MSK_E|MSK_G:
        C[3] = ACS_ULCORNER;
        break;
    case MSK_G:
        C[3] = ACS_HLINE;
        break;
    case MSK_E|MSK_F:
        C[3] = ACS_VLINE;
        break;
    case MSK_E|MSK_F|MSK_G:
        C[3] = ACS_LTEE;
        break;
    }

/*
   G      C4
   0      SPACE
   1      ACS_HLINE
*/

    if (segmap & MSK_G)
        C[4] = ACS_HLINE;

/*
   B C G  C5
   0 0 0  SPACE
   0 0 1  ACS_HLINE
   0 1 0  ACS_URCORNER
   0 1 1  ACS_URCORNER
   1 0 0  ACS_LRCORNER
   1 0 1  ACS_LRCORNER
   1 1 0  ACS_VLINE
   1 1 1  ACS_RTEE
*/

    switch(segmap & (MSK_B|MSK_C|MSK_G)){

    case MSK_G:
        C[5] = ACS_HLINE;
        break;
    case MSK_C:
    case MSK_C|MSK_G:
        C[5] = ACS_URCORNER;
        break;
    case MSK_B:
    case MSK_B|MSK_G:
        C[5] = ACS_LRCORNER;
        break;
    case MSK_B|MSK_C:
        C[5] = ACS_VLINE;
        break;
    case MSK_B|MSK_C|MSK_G:
        C[5] = ACS_RTEE;
        break;
    }

/*
   D E    C6
   0 0    SPACE
   0 1    ACS_LLCORNER
   1 0    ACS_HLINE
   1 1    ACS_LLCORNER
*/

    switch(segmap & (MSK_D|MSK_E)){

    case MSK_E:
    case MSK_D|MSK_E:
        C[6] = ACS_LLCORNER;
        break;
    case MSK_D:
        C[6] = ACS_HLINE;
        break;
    }

/*
   D      C7
   0      SPACE
   1      ACS_HLINE
*/

    if (segmap & MSK_D)
        C[7] = ACS_HLINE;

/*
   C D    C8
   0 0    SPACE
   0 1    ACS_HLINE
   1 0    ACS_LRCORNER
   1 1    ACS_LRCORNER
*/

    switch(segmap & (MSK_C|MSK_D)){

    case MSK_C:
        C[8] = ACS_LLCORNER;
        break;
    case MSK_C|MSK_D:
        C[8] = ACS_LRCORNER;
        break;
    case MSK_D:
        C[8] = ACS_HLINE;
        break;
    }

/*
   DP     C9
   0      SPACE
   1      o
*/

    if (segmap & MSK_DP)
        C[9] = ACS_DIAMOND;//'o';
}

////////////////////////////////////////////////////////////////////////////////
void display_7seg(WINDOW *wnd, int segmap, int common, int pos_w, int pos_h){

/*
       A
    +-----+
   F|     |B
    +--G--+
   E|     |C
    +-----+
       D
*/

    int C[10];

    if (!common)
        segmap = 0;

    combine_7seg(segmap, C);

    wattron(wnd,COLOR_PAIR(LED_RED));

    wmove(wnd, pos_h, pos_w);
    //waddch(janela1,' ');
    waddch(wnd,C[0]);
    waddch(wnd,C[1]);
    waddch(wnd,C[1]);
    waddch(wnd,C[2]);
    waddch(wnd,' ');

    wmove(wnd, pos_h+1, pos_w);
    //waddch(janela1,' ');
    waddch(wnd,C[3]);
    waddch(wnd,C[4]);
    waddch(wnd,C[4]);
    waddch(wnd,C[5]);
    waddch(wnd,' ');

    wmove(wnd, pos_h+2, pos_w);
    //waddch(janela1,' ');
    waddch(wnd,C[6]);
    waddch(wnd,C[7]);
    waddch(wnd,C[7]);
    waddch(wnd,C[8]);
    waddch(wnd,C[9]);

    wattroff(wnd,COLOR_PAIR(LED_WHITE));
}

////////////////////////////////////////////////////////////////////////////////
void desenha_janelas(project_ctx_t *pctx)
{
//    wbkgd(bctx->janela0,COLOR_PAIR(10));
//    wbkgd(bctx->janela1,COLOR_PAIR(10));
//    wbkgd(bctx->janela2,COLOR_PAIR(10));
//    wbkgd(bctx->janela3,COLOR_PAIR(10));
//
//    /*box(janela0, 0 , 0);  */      /* 0, 0 gives default characters
//                                 * for the vertical and horizontal
//                                 * lines            */
//    wrefresh(bctx->janela0);
//
//    wclear(bctx->janela1);
//    wrefresh(bctx->janela1);
//
//    box(bctx->janela2, 0 , 0);        /* 0, 0 gives default characters
//                                 * for the vertical and horizontal
//                                 * lines            */
//    wmove(bctx->janela1, bctx->TERM_LINES-LINHAS_JANELA2B-2, 2);
//    waddstr(bctx->janela1,"F2,F3:Change focused Panel");
//
//    wmove(bctx->janela1, bctx->TERM_LINES-LINHAS_JANELA2B-2, bctx->TERM_COLS-37);
//    waddstr(bctx->janela1,"https://github.com/ARMCoderBR/tlsim");
//
//    wrefresh(bctx->janela2);
}

////////////////////////////////////////////////////////////////////////////////
void restart_handlers(project_ctx_t *pctx)
{
    struct timeval tv;

    FD_ZERO (&pctx->readfds);
    FD_SET(0,&pctx->readfds);
    FD_SET(pctx->pipekeys[0],&pctx->readfds);

    tv.tv_sec = 0;
    tv.tv_usec = 100;    // 100 us

    select (1+pctx->pipekeys[0],&pctx->readfds,NULL,NULL,&tv);
}

////////////////////////////////////////////////////////////////////////////////
int received_key(project_ctx_t *pctx)
{
    return (FD_ISSET(0,&pctx->readfds));
}

////////////////////////////////////////////////////////////////////////////////
int received_key_simu(project_ctx_t *pctx)
{
    return (FD_ISSET(pctx->pipekeys[0],&pctx->readfds));
}

////////////////////////////////////////////////////////////////////////////////
int read_key(project_ctx_t *pctx)
{
//    pthread_mutex_lock(&pctx->ncursesmutex);
//    int key = wgetch(pctx->janela1);
//    pthread_mutex_unlock(&pctx->ncursesmutex);
//    return key;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int read_key_simu(project_ctx_t *pctx)
{
    int key;
    read(pctx->pipekeys[0], &key, sizeof(int));
    return key;
}

////////////////////////////////////////////////////////////////////////////////
void board_write_key(project_ctx_t *pctx, int key)
{
    write(pctx->pipekeys[1], &key, sizeof(int));
}

////////////////////////////////////////////////////////////////////////////////
void sigterm_handler(int sig){

    endwin();
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void board_set_clk(project_ctx_t *ctx, clkgen *clk){

    if (!ctx) return;

    ctx->boardclk = clk;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
void board_set_refresh(project_ctx_t *ctx){

    if (!ctx) return;

    if (!ctx->reader_ok) return;

    pthread_mutex_lock(&ctx->setrefmutex);

    char buf[] = "1";
    write(ctx->piperefresh[1],buf,1);

    pthread_mutex_unlock(&ctx->setrefmutex);
}

////////////////////////////////////////////////////////////////////////////////
void rectangle(WINDOW *wnd, int y1, int x1, int y2, int x2)
{
    mvwhline(wnd, y1, x1, 0, x2-x1);
    mvwhline(wnd, y2, x1, 0, x2-x1);
    mvwvline(wnd, y1, x1, 0, y2-y1);
    mvwvline(wnd, y1, x2, 0, y2-y1);
    mvwaddch(wnd, y1, x1, ACS_ULCORNER);
    mvwaddch(wnd, y2, x1, ACS_LLCORNER);
    mvwaddch(wnd, y1, x2, ACS_URCORNER);
    mvwaddch(wnd, y2, x2, ACS_LRCORNER);
}

////////////////////////////////////////////////////////////////////////////////
void board_refresh_a(project_ctx_t *pctx, board_object *b, int new_h, int new_w){

//    if (b->type != BOARD) return;   // Erro interno - nunca deve acontecer.
//
//    wattrset(bctx->janela1,A_NORMAL);
//    rectangle(bctx->janela1, new_h, new_w, new_h+b->w_height-1, new_w+b->w_width-1);
//
//    if (b->name[0]){
//
//        wmove(bctx->janela1, new_h, 1 + new_w);
//        waddch(bctx->janela1,'[');
//        waddstr(bctx->janela1,b->name);
//        waddch(bctx->janela1,']');
//    }
//
//    int has_key = 0;
//    board_object *thisboard = b;
//
//    b = b->objptr_root;
//
//    while (b){
//
//        wmove(bctx->janela1, new_h + b->pos_h, new_w + b->pos_w);
//
//        switch (b->type){
//
//        case MANUAL_SWITCH:
//            {
//                bitswitch* bs = b->objptr;
//                if (bs->value){
//                    wattron(bctx->janela1,COLOR_PAIR(6));
//                    waddstr(bctx->janela1,"[ "); waddch(bctx->janela1,ACS_DIAMOND); waddstr(bctx->janela1,"1]");
//                    wattroff(bctx->janela1,COLOR_PAIR(6));
//                }
//                else{
//                    wattron(bctx->janela1,COLOR_PAIR(LED_WHITE));
//                    waddstr(bctx->janela1,"["); waddch(bctx->janela1,ACS_DIAMOND); waddstr(bctx->janela1," 1]");
//                    wattroff(bctx->janela1,COLOR_PAIR(LED_WHITE));
//                }
//            }
//            break;
//
//        case LED:
//            {
//                indicator* out = b->objptr;
//                wattron(bctx->janela1,COLOR_PAIR(b->color));
//                if (out->value){
//                    wattron(bctx->janela1,A_STANDOUT);
//                    waddstr(bctx->janela1,"[#]");
//                    wattroff(bctx->janela1,A_STANDOUT);
//                }else
//                    waddstr(bctx->janela1,"[.]");
//                wattroff(bctx->janela1,COLOR_PAIR(b->color));
//            }
//            break;
//
//        case DIS7SEG:
//            {
//                dis7seg *dis = b->objptr;
//                display_7seg(bctx->janela1, dis->segmap, dis->common_val|(dis->count_persist?1:0), new_w + b->pos_w, new_h + b->pos_h);
//            }
//            break;
//
//        case XDIGIT:
//            {
//                indicator* out = b->objptr;
//                char s[10];
//                sprintf(s,"%X",out->value & 0x0F);
//                waddstr(bctx->janela1,s);
//            }
//            break;
//
//        case BOARD:
//
//            board_refresh_a(bctx, b/*->objptr_root*/,b->pos_h, b->pos_w);
//            break;
//        }
//
//        if ((b->type != BOARD)&&(b->type != DIS7SEG)){
//
//            if (b->type == MANUAL_SWITCH){
//
//                if (thisboard == bctx->board_on_focus[bctx->current_board_on_focus]){
//
//                    wattrset(bctx->janela1,A_STANDOUT);
//                }
//            }
//            wmove(bctx->janela1,1 + new_h + b->pos_h, new_w + b->pos_w);
//            waddstr(bctx->janela1,b->name);
//            wattrset(bctx->janela1,A_NORMAL);
//        }
//
//        if (b->type == MANUAL_SWITCH){
//
//            has_key = 1;
//            if (thisboard == bctx->board_on_focus[bctx->current_board_on_focus]){
//
//                wattrset(bctx->janela1,A_STANDOUT);
//            }
//
//            int key = b->key;
//            char s[10];
//            if ((key >= KEY_F(1)) && (key <= KEY_F(12))){
//
//                sprintf(s,"[F%d]",1+key-KEY_F(1));
//            }
//            else
//                sprintf(s,"[%c]",key);
//
//            waddstr(bctx->janela1,s);
//            wattrset(bctx->janela1,A_NORMAL);
//        }
//
//        b = b->objptr_next;
//    }
//
//    if (!bctx->focustable_done)
//        if (has_key)
//            if (bctx->num_focuseable_boards < MAX_FOCUSEABLES_BOARDS)
//                bctx->board_on_focus[bctx->num_focuseable_boards++] = thisboard;
}

////////////////////////////////////////////////////////////////////////////////
void board_refresh(project_ctx_t *pctx, board_object *b){

    pthread_mutex_lock(&pctx->ncursesmutex);

    board_refresh_a(pctx, b,0,0);

    pctx->focustable_done = 1;

//    wrefresh(bctx->janela1);
//    wrefresh(bctx->janela0);

    pthread_mutex_unlock(&pctx->ncursesmutex);
}

////////////////////////////////////////////////////////////////////////////////
void *refresh_thread(void *args){

    project_ctx_t * pctx = (project_ctx_t *)args;
    board_object * refboard = pctx->board;

    pctx->refresh_run = 1;
    char buf[16];

    fd_set rreadfds;
    struct timeval rtv;

    pipe(pctx->piperefresh);
    bool_t ref_pending = 0;

    struct timespec lastspec, nowspec;
    clock_gettime(CLOCK_REALTIME, &lastspec);

    pctx->reader_ok = 1;

    while (pctx->refresh_run){

        FD_ZERO(&rreadfds);
        FD_SET(pctx->piperefresh[0],&rreadfds);

        rtv.tv_sec = 0;
        rtv.tv_usec = 100000;

        select(1+pctx->piperefresh[0],&rreadfds,NULL,NULL,&rtv);

        if (FD_ISSET(pctx->piperefresh[0],&rreadfds)){

            read(pctx->piperefresh[0], buf, sizeof(buf));
            ref_pending = 1;
        }

        if (!ref_pending) continue;

        clock_gettime(CLOCK_REALTIME, &nowspec);

        int deltams = 1000 * (nowspec.tv_sec - lastspec.tv_sec);
        deltams += (nowspec.tv_nsec - lastspec.tv_nsec) / 1000000;

        if (deltams >= 40){

            board_refresh(pctx, refboard);
            lastspec = nowspec;
            ref_pending = 0;
        }
        else
            ref_pending = 1;
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void refresh_thread_stop(project_ctx_t *pctx){

    pctx->refresh_run = 0;
    //pthread_join(pctx->refthread,NULL);

    pctx->reader_ok = 0;

    close(pctx->piperefresh[0]);
    close(pctx->piperefresh[1]);
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
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

board_object *board_create(int width, int height, int key, char *name){

    board_object *b = malloc(sizeof(board_object));

    if (!b) return b;

    b->pos_w = b->pos_h = 0;
    b->type = BOARD;
    b->objptr = NULL;
    b->key = key;
    if (name)
        strncpy(b->name, name, NAMESIZE);
    else
        b->name[0] = 0;
    b->w_width = width;
    b->w_height = height;
    b->objptr_root = NULL;
    b->objptr_next = NULL;
    //b->board_on_focus = b;  // Focada nela própria no início.


    b->board_frame = (GtkFrame*)gtk_frame_new(name);
    b->board_grid = (GtkGrid*)gtk_grid_new();
    gtk_container_add (GTK_CONTAINER (b->board_frame), (GtkWidget*)b->board_grid);

    b->destroy = (void*)board_destroy;
    return b;
}

////////////////////////////////////////////////////////////////////////////////

//char stypes[5][20]= {
//
//    "MANUAL_SWITCH",
//    "LED",
//    "XDIGIT",
//    "BOARD",
//    "DIS7SEG"
//};

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

        switch_on = gdk_pixbuf_new_from_file("../switch-on.png",NULL);
        switch_off = gdk_pixbuf_new_from_file("../switch-off.png",NULL);
        switch_pixbuf_initted = true;
    }

    if (value)
        gtk_image_set_from_pixbuf ((GtkImage *)ptarget, switch_on);
    else
        gtk_image_set_from_pixbuf ((GtkImage *)ptarget, switch_off);
}

////////////////////////////////////////////////////////////////////////////////
static void
bitswitch_toggle (GtkWidget *widget, GtkWidget *otherwidget,
             gpointer   ptr) {

    //printf("bs toggle(%p)\n",ptr);

    board_object *bo = ptr;

    bitswitch* bs = bo->objptr;//(bitswitch*)ptr;
    ((project_ctx_t*)bo->parent_pctx)->switch_to_toggle = bs;
    //bitswitch_setval(bs, bs->value ?0:1);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_manual_switch(board_object *b, bitswitch *bs, int pos_w, int pos_h, int key, char *name){

/*
  F1[0 >1]
  UP/DOWN

  F1[0< 1]
  UP/DOWN
*/
    //printf("board_add_manual_switch(%s, %p)\n",name, bs);

    if (!b) return -2;
    if (!bs) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    obja->pos_w  = pos_w;
    obja->pos_h  = pos_h;
    obja->type   = MANUAL_SWITCH;
    obja->objptr = bs;
    obja->key    = key;
    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    GtkImage *newimg = (GtkImage *)gtk_image_new();

    gtk_manual_switch_set_img(newimg, /*0,*/ bs->value);

    bs->callback = gtk_manual_switch_set_img;
    bs->cb_target = newimg;

    GtkLabel *newlbl = (GtkLabel *)gtk_label_new(name);

    GtkEventBox *ebox = (GtkEventBox *)gtk_event_box_new();
    gtk_container_add (GTK_CONTAINER (ebox), (GtkWidget*)newimg);
    gtk_grid_attach (b->board_grid, (GtkWidget*)ebox, pos_w, pos_h, 1, 1);
    gtk_grid_attach (b->board_grid, (GtkWidget*)newlbl, pos_w, 1+pos_h, 1, 1);

    //gtk_widget_set_events (ebox, GDK_BUTTON_PRESS_MASK);
    //printf("g_signal_connect bs:%p\n",bs);
    g_signal_connect (ebox, "button_press_event", G_CALLBACK (bitswitch_toggle), obja/*bs*/);

    /* Yet one more thing you need an X window for ... */

    //gtk_widget_realize ((GtkWidget*)ebox);
    //gdk_window_set_cursor (gtk_widget_get_window(ebox), gdk_cursor_new_for_display (gdk_display_get_default(), GDK_HAND1));

    return board_add_object(b, obja);
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

        led_green_on  = gdk_pixbuf_new_from_file("../led-green-on.png",NULL);
        led_green_off = gdk_pixbuf_new_from_file("../led-green-off.png",NULL);
        led_yellow_on  = gdk_pixbuf_new_from_file("../led-yellow-on.png",NULL);
        led_yellow_off = gdk_pixbuf_new_from_file("../led-yellow-off.png",NULL);
        led_blue_on  = gdk_pixbuf_new_from_file("../led-blue-on.png",NULL);
        led_blue_off = gdk_pixbuf_new_from_file("../led-blue-off.png",NULL);
        led_white_on  = gdk_pixbuf_new_from_file("../led-white-on.png",NULL);
        led_white_off = gdk_pixbuf_new_from_file("../led-white-off.png",NULL);
        led_red_on  = gdk_pixbuf_new_from_file("../led-red-on.png",NULL);
        led_red_off = gdk_pixbuf_new_from_file("../led-red-off.png",NULL);

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

/*
  [.]
  STATUS

  [#]
  STATUS
*/
    if (!b) return -2;
    if (!out) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    //out->callback = (indicator_refresh_t)board_set_refresh;
    obja->pos_w  = pos_w;
    obja->pos_h  = pos_h;
    obja->type   = LED;
    obja->color = color;
    obja->objptr = out;
    obja->key    = 0;
    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    GtkImage *newimg = (GtkImage *)gtk_image_new();
    gtk_led_set_img(newimg, color, out->value);

    GtkLabel *newlbl = (GtkLabel *)gtk_label_new(name);

    switch(color){

    case LED_GREEN:
        out->callback = (indicator_refresh_t)gtk_led_img_update_green;
        break;
    case LED_YELLOW:
        out->callback = (indicator_refresh_t)gtk_led_img_update_yellow;
        break;
    case LED_BLUE:
        out->callback = (indicator_refresh_t)gtk_led_img_update_blue;
        break;
    case LED_WHITE:
        out->callback = (indicator_refresh_t)gtk_led_img_update_white;
        break;
    default://case LED_RED:
        out->callback = (indicator_refresh_t)gtk_led_img_update_red;
        break;
    }

    out->cb_target = newimg;

    gtk_grid_attach (b->board_grid, (GtkWidget*)newimg, pos_w, pos_h, 1, 1);
    gtk_grid_attach (b->board_grid, (GtkWidget*)newlbl, pos_w, 1+pos_h, 1, 1);

    return board_add_object(b, obja);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_display_7seg(board_object *b, dis7seg *out, int pos_w, int pos_h, char *name, led_color_t color){

    if (!b) return -2;
    if (!out) return -2;

    board_object *obja = malloc(sizeof(board_object));
    if (!obja) return -1;

    out->callback = (indicator_refresh_t)board_set_refresh;
    obja->pos_w  = pos_w;
    obja->pos_h  = pos_h;
    obja->type   = DIS7SEG;
    obja->color = color;
    obja->objptr = out;
    obja->key    = 0;
    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

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

    obja->pos_w  = pos_w;
    obja->pos_h  = pos_h;
    obja->type   = XDIGIT;
    obja->objptr = out;
    obja->key    = 0;
    if (name)
        strncpy(obja->name, name, NAMESIZE);
    else
        obja->name[0] = 0;
    obja->objptr_root = NULL;
    obja->objptr_next = NULL;

    return board_add_object(b, obja);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_board(board_object *b, board_object *board, int pos_w, int pos_h){

    if (!b) return -2;
    if (!board) return -2;

    board->pos_w = pos_w;
    board->pos_h = pos_h;

    gtk_grid_attach (b->board_grid, (GtkWidget*)board->board_frame, pos_w, pos_h, 1, 1);

    return board_add_object(b, board);
}

////////////////////////////////////////////////////////////////////////////////
int board_add_boardWH(board_object *b, board_object *board, int pos_w, int pos_h, int width, int heigth){

    if (!b) return -2;
    if (!board) return -2;

    board->pos_w = pos_w;
    board->pos_h = pos_h;

    gtk_grid_attach (b->board_grid, (GtkWidget*)board->board_frame, pos_w, pos_h, width, heigth);

    return board_add_object(b, board);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void board_initialize(void){

//    ctx = board_init();
}

////////////////////////////////////////////////////////////////////////////////
int board_run(project_ctx_t *ctx, event_context_t *ec, board_object *board){

    if (!ctx) exit(-3);

    ctx->board = board;

    pipe(ctx->pipekeys);

    if (!board) return -2;

    if (board->type != BOARD)
        return -10;

    signal (SIGHUP,SIG_IGN);
    signal (SIGINT,SIG_IGN);
    signal (SIGTERM,sigterm_handler);
    signal (SIGTSTP,SIG_IGN);
    signal (SIGWINCH,SIG_IGN);

//    if (initscr() == NULL){
//
//        printf("initscr()\n");
//        exit(-1);
//    }

//    cbreak(); noecho();            /* Inicia o NCURSES */
//    ESCDELAY=200;
//    ctx->TERM_LINES = LINES;
//    ctx->TERM_COLS = COLS;

//    start_color();
//    init_pair(1, 8|COLOR_RED, 16|COLOR_BLACK);
//    init_pair(2, 8|COLOR_GREEN, 16|COLOR_BLACK);
//    init_pair(3, 8|COLOR_YELLOW, 16|COLOR_BLACK);
//    init_pair(4, 8|COLOR_BLUE, 16|COLOR_BLACK);
//    init_pair(5, COLOR_WHITE, 16|COLOR_BLACK);
//    init_pair(6, COLOR_WHITE, COLOR_RED);
//
//    init_pair(10, COLOR_WHITE, 8|COLOR_BLACK);
//
//    ctx->janela0 = newwin(ctx->TERM_LINES,ctx->TERM_COLS,0,0);
//    ctx->janela1 = newwin(ctx->TERM_LINES-LINHAS_JANELA2B,ctx->TERM_COLS,0,0);
//    ctx->janela2 = newwin(LINHAS_JANELA2B,ctx->TERM_COLS,ctx->TERM_LINES-LINHAS_JANELA2B,0);
//    ctx->janela3 = newwin(LINHAS_JANELA2,ctx->TERM_COLS-2,1+ctx->TERM_LINES-LINHAS_JANELA2B,1);

//    if (!board->w_width)
//        board->w_width = ctx->TERM_COLS;
//
//    if (!board->w_height)
//        board->w_height = ctx->TERM_LINES-LINHAS_JANELA2B;

    desenha_janelas(ctx);

    clock_reinit(ctx);

//    keypad(ctx->janela1,TRUE);

    //pthread_create(&ctx->refthread, NULL, refresh_thread, ctx);

    //board_set_refresh(ctx);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int board_run_b(project_ctx_t *pctx, event_context_t *ec, board_object *board){

//    bool_t resize = 0;

    //bool_t stoprun = 0;

    /*while (!stoprun)*/{

        event_mutex_lock(ec);
        while (event_process(ec));
        event_mutex_unlock(ec);

        if (pctx->clock_faster_req){

            pctx->clock_faster_req = false;
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

#if 0
//        if (resize){
//
//            pthread_mutex_lock(&ctx->ncursesmutex);
//
//            getmaxyx(stdscr, ctx->TERM_LINES, ctx->TERM_COLS);
//
//            wresize(ctx->janela0,ctx->TERM_LINES,ctx->TERM_COLS);
//            wresize(ctx->janela1,ctx->TERM_LINES-LINHAS_JANELA2B,ctx->TERM_COLS);
//            wresize(ctx->janela2,LINHAS_JANELA2B,ctx->TERM_COLS);
//            mvwin(ctx->janela2,ctx->TERM_LINES-LINHAS_JANELA2B,0);
//            wresize(ctx->janela3,LINHAS_JANELA2,ctx->TERM_COLS-2);
//            mvwin(ctx->janela3,1+ctx->TERM_LINES-LINHAS_JANELA2B,1);
//
//            board->w_width = ctx->TERM_COLS;
//            board->w_height = ctx->TERM_LINES-LINHAS_JANELA2B;
//            desenha_janelas(ctx);
//            resize = 0;
//
//            pthread_mutex_unlock(&ctx->ncursesmutex);
//            board_set_refresh(ctx);
//            clock_redraw(ctx);
//        }

        restart_handlers(pctx);

        bool_t has_key = 0;
        int key;

        if (received_key(pctx)){
            key = read_key(pctx);
            has_key = 1;
        }

        if (received_key_simu(pctx)){
            key = read_key_simu(pctx);
            has_key = 1;
        }

        if (has_key){

            switch(key){

            case KEY_RESIZE:
                //resize = 1;
                break;
            case 27:
                //stoprun = 1;
                board_set_refresh(pctx);
                break;
            case KEY_F(2):
                if (pctx->num_focuseable_boards > 1){
                    if (pctx->current_board_on_focus)
                        pctx->current_board_on_focus--;
                    else
                        pctx->current_board_on_focus = pctx->num_focuseable_boards - 1;
                    board_set_refresh(pctx);
                }
                break;
            case KEY_F(3):
                if (pctx->num_focuseable_boards > 1){

                    pctx->current_board_on_focus = (pctx->current_board_on_focus+1) % pctx->num_focuseable_boards;
                    board_set_refresh(pctx);
                }
                break;
            case KEY_F(12):
                clock_faster(pctx);
                board_set_refresh(pctx);
                break;
            case KEY_F(11):
                clock_slower(pctx);
                board_set_refresh(pctx);
                break;
            case KEY_F(10):
                clock_pause(pctx);
                board_set_refresh(pctx);
                break;
            }

            board_object *pboardfocused = NULL;

            if (pctx->num_focuseable_boards)
                pboardfocused = pctx->board_on_focus[pctx->current_board_on_focus];

            board_object *p;

            if (pboardfocused)
                p = pboardfocused->objptr_root;
            else
                p = NULL;

            while(p){

                if (p->type == MANUAL_SWITCH){

                    if (p->key == key){

                        bitswitch *bs = p->objptr;
                        bitswitch_setval(bs, 1 ^ bs->value);
                        board_set_refresh(pctx);
                    }
                }
                p = p->objptr_next;
            }
        }
#endif
    }
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
int board_run_c(project_ctx_t *ctx, event_context_t *ec, board_object *board){

    refresh_thread_stop(ctx);

    if (!ctx->clock_pausing)
    	clock_pause(ctx);

    pthread_mutex_destroy(&ctx->setrefmutex);

//    endwin();

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
