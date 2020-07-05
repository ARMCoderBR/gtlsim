////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

// Based on Ben Eater's project (https://www.youtube.com/watch?v=uYXwCBo40iA)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ram_8bit.h"
#include "bitconst.h"
#include "exmachina.h"

////////////////////////////////////////////////////////////////////////////////
void *difpulse_function(void *args){

    fd_set readfds;
    struct timeval tv;
    char buf[16];

    ram_8bit *o = (ram_8bit *)args;

    pipe(o->pipefd);

    o->running = 1;

    for (;o->running;){

        tv.tv_sec = 10;
        tv.tv_usec = 0;
        FD_ZERO(&readfds);
        FD_SET(o->pipefd[0],&readfds);

        select(1+o->pipefd[0],&readfds,NULL,NULL,&tv);

        if (FD_ISSET(o->pipefd[0],&readfds)){

            read(o->pipefd[0],buf,1);
        }

        if (o->reqpulse){

            event_mutex_lock(o->ec);
            while (event_process(o->ec));
            event_mutex_unlock(o->ec);
            o->reqpulse = 0;
            o->valpulse = 0;
            ls00_in_b1(o->ls00_clk, &o->valpulse, 0);
            usleep(100);
            event_mutex_lock(o->ec);
            while (event_process(o->ec));
            event_mutex_unlock(o->ec);
        }
    }

    close(o->pipefd[0]);
    close(o->pipefd[1]);

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_reqpulse(ram_8bit *dest){

    char buf = '1';

    write(dest->pipefd[1],&buf,1);
}

////////////////////////////////////////////////////////////////////////////////
ram_8bit *ram_8bit_create(event_context_t *ec, char *name){

    ram_8bit *ram = malloc (sizeof(ram_8bit));
    if (!ram) return ram;

    ram->ec = ec;

    char lshi[60];
    char lslo[60];

    lshi[0] = lslo[0] = 0;
    if (name){

        strncpy(lshi,name,sizeof(lshi));
        strncpy(lslo,name,sizeof(lslo));    // Inicializar com 1E 2F E0 00 00 00 00 00 00 00 00 00 00 00 0E 1C
    }

    strncat(lshi,"-hiWord",sizeof(lshi)/2);
    strncat(lslo,"-loWord",sizeof(lslo)/2);

    ram->ls189_hi = ls189_create(ec, lshi);
    ram->ls189_lo = ls189_create(ec, lslo);
    ram->ls245_1  = ls245_create(ec);
    ram->ls04_hi = ls04_create(ec);
    ram->ls04_lo = ls04_create(ec);

    int i;
    for (i = 0; i < 8; i++){

        strncpy(lshi,name,sizeof(lshi));
        sprintf(lslo,"-D%d",i);
        strncat(lshi,lslo,sizeof(lshi)/2);
        ram->leddata[i] = indicator_create(ec, lshi);
    }

    ls189_connect_1q(ram->ls189_lo, ram->ls04_lo, (void*)&ls04_in_a1);
    ls189_connect_2q(ram->ls189_lo, ram->ls04_lo, (void*)&ls04_in_a2);
    ls189_connect_3q(ram->ls189_lo, ram->ls04_lo, (void*)&ls04_in_a3);
    ls189_connect_4q(ram->ls189_lo, ram->ls04_lo, (void*)&ls04_in_a4);

    ls04_connect_y1(ram->ls04_lo, ram->leddata[0], (void*)&indicator_in_d0);
    ls04_connect_y2(ram->ls04_lo, ram->leddata[1], (void*)&indicator_in_d0);
    ls04_connect_y3(ram->ls04_lo, ram->leddata[2], (void*)&indicator_in_d0);
    ls04_connect_y4(ram->ls04_lo, ram->leddata[3], (void*)&indicator_in_d0);

    ls189_connect_1q(ram->ls189_hi, ram->ls04_hi, (void*)&ls04_in_a1);
    ls189_connect_2q(ram->ls189_hi, ram->ls04_hi, (void*)&ls04_in_a2);
    ls189_connect_3q(ram->ls189_hi, ram->ls04_hi, (void*)&ls04_in_a3);
    ls189_connect_4q(ram->ls189_hi, ram->ls04_hi, (void*)&ls04_in_a4);

    ls04_connect_y1(ram->ls04_hi, ram->leddata[4], (void*)&indicator_in_d0);
    ls04_connect_y2(ram->ls04_hi, ram->leddata[5], (void*)&indicator_in_d0);
    ls04_connect_y3(ram->ls04_hi, ram->leddata[6], (void*)&indicator_in_d0);
    ls04_connect_y4(ram->ls04_hi, ram->leddata[7], (void*)&indicator_in_d0);

    ls04_connect_y1(ram->ls04_lo, ram->ls245_1, (void*)&ls245_in_a1);
    ls04_connect_y2(ram->ls04_lo, ram->ls245_1, (void*)&ls245_in_a2);
    ls04_connect_y3(ram->ls04_lo, ram->ls245_1, (void*)&ls245_in_a3);
    ls04_connect_y4(ram->ls04_lo, ram->ls245_1, (void*)&ls245_in_a4);

    ls04_connect_y1(ram->ls04_hi, ram->ls245_1, (void*)&ls245_in_a5);
    ls04_connect_y2(ram->ls04_hi, ram->ls245_1, (void*)&ls245_in_a6);
    ls04_connect_y3(ram->ls04_hi, ram->ls245_1, (void*)&ls245_in_a7);
    ls04_connect_y4(ram->ls04_hi, ram->ls245_1, (void*)&ls245_in_a8);

    bitconst_connect_zero(ram->ls189_lo, (void*)&ls189_in_cs);
    bitconst_connect_zero(ram->ls189_hi, (void*)&ls189_in_cs);

    bitconst_connect_one(ram->ls245_1, (void*)&ls245_in_dir);

    /////////////////////////

    ram->ls157_addr = ls157_create(ec);
    ram->ls173_addreg = ls173_create(ec, "ADDREG");
    ram->ls157_datalo = ls157_create(ec);
    ram->ls157_datahi = ls157_create(ec);
    ram->ls157_write = ls157_create(ec);
    ram->ls00_clk = ls00_create(ec);

    ls00_connect_y1(ram->ls00_clk, ram->ls157_write, (void*)&ls157_in_b1);

    ram->ledaddr[0] = indicator_create(ec, "A0");
    ram->ledaddr[1] = indicator_create(ec, "A1");
    ram->ledaddr[2] = indicator_create(ec, "A2");
    ram->ledaddr[3] = indicator_create(ec, "A3");
    ram->prog_run = bitswitch_create(ec, "ProgRun");
    ram->progaddr[0] = bitswitch_create(ec, "RA0");
    ram->progaddr[1] = bitswitch_create(ec, "RA1");
    ram->progaddr[2] = bitswitch_create(ec, "RA2");
    ram->progaddr[3] = bitswitch_create(ec, "RA3");
    ram->progdata[0] = bitswitch_create(ec, "RD0");
    ram->progdata[1] = bitswitch_create(ec, "RD1");
    ram->progdata[2] = bitswitch_create(ec, "RD2");
    ram->progdata[3] = bitswitch_create(ec, "RD3");
    ram->progdata[4] = bitswitch_create(ec, "RD4");
    ram->progdata[5] = bitswitch_create(ec, "RD5");
    ram->progdata[6] = bitswitch_create(ec, "RD6");
    ram->progdata[7] = bitswitch_create(ec, "RD7");
    ram->progwrite = bitswitch_create(ec, "PRW");

    for (i = 0; i < 4; i++)
        bitswitch_setval(ram->progaddr[i],0);

    for (i = 0; i < 8; i++)
        bitswitch_setval(ram->progdata[i],0);

    bitswitch_setval(ram->progwrite, 1);

    bitswitch_connect_out(ram->prog_run,ram->ls157_addr,(void*)&ls157_in_sel);
    bitswitch_connect_out(ram->prog_run,ram->ls157_datalo,(void*)&ls157_in_sel);
    bitswitch_connect_out(ram->prog_run,ram->ls157_datahi,(void*)&ls157_in_sel);
    bitswitch_connect_out(ram->prog_run,ram->ls157_write,(void*)&ls157_in_sel);

    bitconst_connect_zero(ram->ls157_addr,(void*)&ls157_in_g);
    bitconst_connect_zero(ram->ls157_datalo,(void*)&ls157_in_g);
    bitconst_connect_zero(ram->ls157_datahi,(void*)&ls157_in_g);
    bitconst_connect_zero(ram->ls157_write,(void*)&ls157_in_g);

    bitswitch_connect_out(ram->progaddr[0],ram->ls157_addr,(void*)&ls157_in_a1);
    bitswitch_connect_out(ram->progaddr[1],ram->ls157_addr,(void*)&ls157_in_a2);
    bitswitch_connect_out(ram->progaddr[2],ram->ls157_addr,(void*)&ls157_in_a3);
    bitswitch_connect_out(ram->progaddr[3],ram->ls157_addr,(void*)&ls157_in_a4);

    ls157_connect_y1(ram->ls157_addr,ram->ls189_lo, (void*)&ls189_in_a0);
    ls157_connect_y2(ram->ls157_addr,ram->ls189_lo, (void*)&ls189_in_a1);
    ls157_connect_y3(ram->ls157_addr,ram->ls189_lo, (void*)&ls189_in_a2);
    ls157_connect_y4(ram->ls157_addr,ram->ls189_lo, (void*)&ls189_in_a3);
    ls157_connect_y1(ram->ls157_addr,ram->ls189_hi, (void*)&ls189_in_a0);
    ls157_connect_y2(ram->ls157_addr,ram->ls189_hi, (void*)&ls189_in_a1);
    ls157_connect_y3(ram->ls157_addr,ram->ls189_hi, (void*)&ls189_in_a2);
    ls157_connect_y4(ram->ls157_addr,ram->ls189_hi, (void*)&ls189_in_a3);
    ls157_connect_y1(ram->ls157_addr,ram->ledaddr[0], (void*)&indicator_in_d0);
    ls157_connect_y2(ram->ls157_addr,ram->ledaddr[1], (void*)&indicator_in_d0);
    ls157_connect_y3(ram->ls157_addr,ram->ledaddr[2], (void*)&indicator_in_d0);
    ls157_connect_y4(ram->ls157_addr,ram->ledaddr[3], (void*)&indicator_in_d0);

    bitswitch_connect_out(ram->progdata[0],ram->ls157_datalo,(void*)&ls157_in_a1);
    bitswitch_connect_out(ram->progdata[1],ram->ls157_datalo,(void*)&ls157_in_a2);
    bitswitch_connect_out(ram->progdata[2],ram->ls157_datalo,(void*)&ls157_in_a3);
    bitswitch_connect_out(ram->progdata[3],ram->ls157_datalo,(void*)&ls157_in_a4);
    bitswitch_connect_out(ram->progdata[4],ram->ls157_datahi,(void*)&ls157_in_a1);
    bitswitch_connect_out(ram->progdata[5],ram->ls157_datahi,(void*)&ls157_in_a2);
    bitswitch_connect_out(ram->progdata[6],ram->ls157_datahi,(void*)&ls157_in_a3);
    bitswitch_connect_out(ram->progdata[7],ram->ls157_datahi,(void*)&ls157_in_a4);

    ls157_connect_y1(ram->ls157_datalo,ram->ls189_lo, (void*)&ls189_in_1d);
    ls157_connect_y2(ram->ls157_datalo,ram->ls189_lo, (void*)&ls189_in_2d);
    ls157_connect_y3(ram->ls157_datalo,ram->ls189_lo, (void*)&ls189_in_3d);
    ls157_connect_y4(ram->ls157_datalo,ram->ls189_lo, (void*)&ls189_in_4d);

    ls157_connect_y1(ram->ls157_datahi,ram->ls189_hi, (void*)&ls189_in_1d);
    ls157_connect_y2(ram->ls157_datahi,ram->ls189_hi, (void*)&ls189_in_2d);
    ls157_connect_y3(ram->ls157_datahi,ram->ls189_hi, (void*)&ls189_in_3d);
    ls157_connect_y4(ram->ls157_datahi,ram->ls189_hi, (void*)&ls189_in_4d);

    bitswitch_connect_out(ram->progwrite,ram->ls157_write,(void*)&ls157_in_a1);
    ls157_connect_y1(ram->ls157_write,ram->ls189_lo, (void*)&ls189_in_we);
    ls157_connect_y1(ram->ls157_write,ram->ls189_hi, (void*)&ls189_in_we);

    ls173_connect_1q(ram->ls173_addreg,ram->ls157_addr,(void*)&ls157_in_b1);
    ls173_connect_2q(ram->ls173_addreg,ram->ls157_addr,(void*)&ls157_in_b2);
    ls173_connect_3q(ram->ls173_addreg,ram->ls157_addr,(void*)&ls157_in_b3);
    ls173_connect_4q(ram->ls173_addreg,ram->ls157_addr,(void*)&ls157_in_b4);

    bitconst_connect_zero(ram->ls173_addreg,(void*)&ls173_in_m);
    bitconst_connect_zero(ram->ls173_addreg,(void*)&ls173_in_n);
    bitconst_connect_zero(ram->ls173_addreg,(void*)&ls173_in_g1);
    //bitconst_connect_zero(ram->ls173_addreg,(void*)&ls173_in_clr);

    ram->ledprog = indicator_create(ec, "Ledprog");
    ram->ledrun = indicator_create(ec, "Ledrun");
    bitswitch_connect_out(ram->prog_run,ram->ledrun,(void*)&indicator_in_d0);
    bitswitch_connect_out(ram->prog_run, ram->ls04_lo, (void*)&ls04_in_a6);
    ls04_connect_y6(ram->ls04_lo, ram->ledprog, (void*)&indicator_in_d0);

    if (name)
        strncpy(ram->name,name,sizeof(ram->name));
    else
        ram->name[0] = 0;

    ram->clk = 2;
    ram->oldclk = 1;
    ram->clk_rootptr = NULL;

    pthread_create(&ram->difpulse_thread, NULL, difpulse_function, ram);

    ram->destroy = (void*)ram_8bit_destroy;

    return ram;
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_destroy (ram_8bit **dest){

    if (dest == NULL) return;
    ram_8bit *b = *dest;
    if (b == NULL) return;

    b->running = 0;
    ram_8bit_reqpulse(b);
    pthread_join(b->difpulse_thread, NULL);

    DESTROY(b->ls189_hi);
    DESTROY(b->ls189_lo);
    DESTROY(b->ls245_1);
    DESTROY(b->ls04_hi);
    DESTROY(b->ls04_lo);

    int i;
    for (i = 0; i < 4; i++)
        DESTROY(b->ledaddr[i]);

    DESTROY(b->prog_run);

    for (i = 0; i < 4; i++)
        DESTROY(b->progaddr[i]);

    for (i = 0; i < 8; i++)
        DESTROY(b->progdata[i]);

    DESTROY(b->progwrite);

    DESTROY(b->ledprog);
    DESTROY(b->ledrun);

    vallist_destroy((void*)&b->clk_rootptr);

    free(b);
    *dest = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_connect_bit_out (ram_8bit *source, int index, void *dest, event_function_t dest_event_handler){

    switch(index){
        case 0:    ls245_connect_b1(source->ls245_1,dest,dest_event_handler); break;
        case 1:    ls245_connect_b2(source->ls245_1,dest,dest_event_handler); break;
        case 2:    ls245_connect_b3(source->ls245_1,dest,dest_event_handler); break;
        case 3:    ls245_connect_b4(source->ls245_1,dest,dest_event_handler); break;
        case 4:    ls245_connect_b5(source->ls245_1,dest,dest_event_handler); break;
        case 5:    ls245_connect_b6(source->ls245_1,dest,dest_event_handler); break;
        case 6:    ls245_connect_b7(source->ls245_1,dest,dest_event_handler); break;
        case 7:    ls245_connect_b8(source->ls245_1,dest,dest_event_handler); break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_data_from(void (*connect_fn)(void *source, void *dest, event_function_t dest_event_handler),
                        void *from, ram_8bit *dest, int index){

    switch(index){
        case 0: connect_fn(from,dest->ls157_datalo,(void*)&ls157_in_b1); break;
        case 1: connect_fn(from,dest->ls157_datalo,(void*)&ls157_in_b2); break;
        case 2: connect_fn(from,dest->ls157_datalo,(void*)&ls157_in_b3); break;
        case 3: connect_fn(from,dest->ls157_datalo,(void*)&ls157_in_b4); break;
        case 4: connect_fn(from,dest->ls157_datahi,(void*)&ls157_in_b1); break;
        case 5: connect_fn(from,dest->ls157_datahi,(void*)&ls157_in_b2); break;
        case 6: connect_fn(from,dest->ls157_datahi,(void*)&ls157_in_b3); break;
        case 7: connect_fn(from,dest->ls157_datahi,(void*)&ls157_in_b4); break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_data0(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b1(dest->ls157_datalo, valptr, timestamp);
    ls173_in_1d(dest->ls173_addreg, valptr, timestamp);
}

void ram_8bit_in_data1(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b2(dest->ls157_datalo, valptr, timestamp);
    ls173_in_2d(dest->ls173_addreg, valptr, timestamp);
}

void ram_8bit_in_data2(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b3(dest->ls157_datalo, valptr, timestamp);
    ls173_in_3d(dest->ls173_addreg, valptr, timestamp);
}

void ram_8bit_in_data3(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b4(dest->ls157_datalo, valptr, timestamp);
    ls173_in_4d(dest->ls173_addreg, valptr, timestamp);
}

void ram_8bit_in_data4(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b1(dest->ls157_datahi, valptr, timestamp);
}

void ram_8bit_in_data5(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b2(dest->ls157_datahi, valptr, timestamp);
}

void ram_8bit_in_data6(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b3(dest->ls157_datahi, valptr, timestamp);
}

void ram_8bit_in_data7(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls157_in_b4(dest->ls157_datahi, valptr, timestamp);
}

event_function_t ram_8bit_in_dataN[] = {

        (void*)ram_8bit_in_data0,
        (void*)ram_8bit_in_data1,
        (void*)ram_8bit_in_data2,
        (void*)ram_8bit_in_data3,
        (void*)ram_8bit_in_data4,
        (void*)ram_8bit_in_data5,
        (void*)ram_8bit_in_data6,
        (void*)ram_8bit_in_data7,
};

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_wdata(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

	ls00_in_a1(dest->ls00_clk, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_waddr(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

	ls173_in_g2(dest->ls173_addreg, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_oe(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

	ls245_in_oe(dest->ls245_1, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_clk(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls173_in_clk(dest->ls173_addreg, valptr, timestamp);

#define DIFPULSE 1

#if DIFPULSE

    bitvalue_t val = update_val_multi(&dest->clk_rootptr, valptr);
    if (val > 1) val = 1;

    if (val == dest->oldclk) return;

    dest->oldclk = val;

    if (val){

    	dest->valpulse = 1;
    	ls00_in_b1(dest->ls00_clk, &dest->valpulse, timestamp+10);
        dest->reqpulse = 1;
        ram_8bit_reqpulse(dest);
    }
#else
	ls00_in_b1(dest->ls00_clk, valptr, timestamp);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_rst(ram_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls173_in_clr(dest->ls173_addreg, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
board_object *ram_8bit_board_create(ram_8bit *ram, int key, char *name){

    board_object *board = board_create(58, 13, key, name);

    if (!board) return board;

    char s[32];
    int i,j;

    for (i = 0; i < 8; i++){

        j = 7-i;

        sprintf(s,"D%d",i);
        board_add_led(board, ram->leddata[i],5+j,1,s, LED_RED);

        board_add_manual_switch(board, ram->progdata[i], 5+j, 3, RAMDATA_KEY_0+i, s);
    }

    for (i = 0; i < 4; i++){

        j = 3-i;

        sprintf(s,"A%d",i);
        board_add_led(board, ram->ledaddr[i],9+j,5,s, LED_YELLOW);
        board_add_manual_switch(board, ram->progaddr[i], 9+j, 6, RAMADDR_KEY_0+i, s);
    }

    board_add_pushbutton(board, ram->progwrite, 7, 6, RAM_WRITE,"Write");

    board_add_led(board, ram->ledrun,1,1,"RUN", LED_GREEN);
    board_add_led(board, ram->ledprog,2,1,"PRG", LED_RED);
    board_add_spacer(board,3,1);
    board_add_manual_switch(board, ram->prog_run, 1, 3, RAM_PROG_RUN,"Pr/Run");

    return board;
}
