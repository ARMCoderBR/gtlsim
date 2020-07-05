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

#ifndef BOARD_H_
#define BOARD_H_

#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

#include <gtk/gtk.h>

#include "update.h"
#include "bitswitch.h"
#include "indicator.h"
#include "dis7seg.h"
#include "clkgen.h"

typedef enum {

    MANUAL_SWITCH,
    PUSHBUTTON,
    LED,
    XDIGIT,
    BOARD,
    DIS7SEG
} control_type;

typedef enum {

    COLOR_NONE = 0,
    LED_RED = 1,
    LED_GREEN,
    LED_YELLOW,
    LED_BLUE,
    LED_WHITE
} led_color_t;

#define NAMESIZE 64

typedef struct {

    void (*destroy)(void **dest);
    control_type type;
    void *objptr;
    int key;
    char name[NAMESIZE];
    int color;
    void/*board_object*/ *objptr_root;
    void/*board_object*/ *objptr_next;

    /// For GTK
    GtkFrame *board_frame;
    GtkGrid *board_grid;
    GtkWidget *gtk_widget;
    int indicator_value;
    void *parent_pctx;
} board_object;

#define MAX_FOCUSEABLES_BOARDS 50
typedef struct {

    clkgen *boardclk;
    bool_t clock_pausing;
    int iclk;

    bool clock_pause_req;
    bool clock_slower_req;
    bool clock_faster_req;

    // For GTK
    bitswitch *switch_to_toggle;
    board_object * main_board;

    int remote_key;

} project_ctx_t;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int board_add_manual_switch(board_object *b, bitswitch *bs, int pos_w, int pow_h, int key, char *name);

int board_add_pushbutton(board_object *b, bitswitch *bs, int pos_w, int pos_h, int key, char *name);

int board_add_led(board_object *b, indicator *out, int pos_w, int pos_h, char *name, led_color_t color);

int board_add_display_7seg(board_object *b, dis7seg *out, int pos_w, int pos_h, char *name, led_color_t color);

int board_add_xdigit(board_object *b, indicator *out, int pos_w, int pos_h, char *name, led_color_t color);

int board_add_spacer(board_object *b, int pos_w, int pos_h);

int board_add_board(board_object *b, board_object *board, int pos_w, int pos_h);
int board_add_boardWH(board_object *b, board_object *board, int pos_w, int pos_h, int width, int heigth);

int board_run(project_ctx_t *ctx, event_context_t *ec, board_object *board);
int board_run_b(project_ctx_t *ctx, event_context_t *ec, board_object *board);
int board_run_c(project_ctx_t *ctx, event_context_t *ec, board_object *board);

board_object *board_create(int width, int height, int key, char *name);

void board_destroy(board_object **dest);

board_object *mainboard_create(char *name);

void board_set_clk(project_ctx_t *ctx, clkgen *clk);

void board_set_refresh(project_ctx_t *ctx);

project_ctx_t *project_init(void);

void board_write_key(project_ctx_t *pctx, int key);

void board_add_clock_buttons(GtkGrid *maingrid, project_ctx_t *pctx);

void board_refresh(project_ctx_t *pctx, board_object *b);

#endif /* BOARD_H_ */
