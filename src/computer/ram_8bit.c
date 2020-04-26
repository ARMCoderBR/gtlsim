/*
 * ram_8bit.c
 *
 *  Created on: 25 de abr de 2020
 *      Author: milton
 */

// Baseado no projeto do Ben Eater (https://www.youtube.com/watch?v=uYXwCBo40iA)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ram_8bit.h"
#include "bitconst.h"

////////////////////////////////////////////////////////////////////////////////
ram_8bit *ram_8bit_create(char *name){

    ram_8bit *ram = malloc (sizeof(ram_8bit));
    if (!ram) return ram;

    char lshi[60];
    char lslo[60];

    lshi[0] = lslo[0] = 0;
    if (name){

        strncpy(lshi,name,sizeof(lshi));
        strncpy(lslo,name,sizeof(lslo));
    }

    strncat(lshi,"-hiWord",sizeof(lshi)/2);
    strncat(lslo,"-loWord",sizeof(lslo)/2);

    ram->ls189_hi = ls189_create(lshi);
    ram->ls189_lo = ls189_create(lslo);
    ram->ls245_1  = ls245_create();
    ram->ls04_hi = ls04_create();
    ram->ls04_lo = ls04_create();

    int i;
    for (i = 0; i < 8; i++){

        strncpy(lshi,name,sizeof(lshi));
        sprintf(lslo,"-D%d",i);
        strncat(lshi,lslo,sizeof(lshi)/2);
        ram->leddata[i] = indicator_create(lshi);
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

    ram->ls157_addr = ls157_create();
    ram->ls173_addreg = ls173_create("ADDREG");
    ram->ls157_datalo = ls157_create();
    ram->ls157_datahi = ls157_create();
    ram->ls157_write = ls157_create();
    ram->ledaddr[0] = indicator_create("A0");
    ram->ledaddr[1] = indicator_create("A1");
    ram->ledaddr[2] = indicator_create("A2");
    ram->ledaddr[3] = indicator_create("A3");
    ram->prog_run = bitswitch_create("ProgRun");
    ram->progaddr[0] = bitswitch_create("RA0");
    ram->progaddr[1] = bitswitch_create("RA1");
    ram->progaddr[2] = bitswitch_create("RA2");
    ram->progaddr[3] = bitswitch_create("RA3");
    ram->progdata[0] = bitswitch_create("RD0");
    ram->progdata[1] = bitswitch_create("RD1");
    ram->progdata[2] = bitswitch_create("RD2");
    ram->progdata[3] = bitswitch_create("RD3");
    ram->progdata[4] = bitswitch_create("RD4");
    ram->progdata[5] = bitswitch_create("RD5");
    ram->progdata[6] = bitswitch_create("RD6");
    ram->progdata[7] = bitswitch_create("RD7");
    ram->progwrite = bitswitch_create("PRW");

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
    bitconst_connect_zero(ram->ls173_addreg,(void*)&ls173_in_clr);



    if (name)
        strncpy(ram->name,name,sizeof(ram->name));
    else
        ram->name[0] = 0;

    return ram;
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_connect_bit_out (ram_8bit *source, int index, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

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
void ram_8bit_in_data_from(void (*connect_fn)(void *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)),
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
void ram_8bit_in_addr_from(void (*connect_fn)(void *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)),
                        void *from, ram_8bit *dest, int index){

    switch(index){
        case 0:
            connect_fn(from,dest->ls157_addr,(void*)&ls157_in_b1);
            break;
        case 1:
            connect_fn(from,dest->ls157_addr,(void*)&ls157_in_b2);
            break;
        case 2:
            connect_fn(from,dest->ls157_addr,(void*)&ls157_in_b3);
            break;
        case 3:
            connect_fn(from,dest->ls157_addr,(void*)&ls157_in_b4);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_we_from(void (*connect_fn)(void *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)),
                        void *from, ram_8bit *dest){

    //connect_fn(from,dest->ls189_lo,(void*)&ls189_in_we);
    //connect_fn(from,dest->ls189_hi,(void*)&ls189_in_we);
}

////////////////////////////////////////////////////////////////////////////////
void ram_8bit_in_oe_from(void (*connect_fn)(void *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)),
                        void *from, ram_8bit *dest){

    connect_fn(from,dest->ls245_1,(void*)&ls245_in_oe);
}

void ram_8bit_in_data0(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b1(dest->ls157_datalo, valptr, timestamp);
}

void ram_8bit_in_data1(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b2(dest->ls157_datalo, valptr, timestamp);
}

void ram_8bit_in_data2(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b3(dest->ls157_datalo, valptr, timestamp);
}

void ram_8bit_in_data3(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b4(dest->ls157_datalo, valptr, timestamp);
}

void ram_8bit_in_data4(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b1(dest->ls157_datahi, valptr, timestamp);
}

void ram_8bit_in_data5(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b2(dest->ls157_datahi, valptr, timestamp);
}

void ram_8bit_in_data6(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b3(dest->ls157_datahi, valptr, timestamp);
}

void ram_8bit_in_data7(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b4(dest->ls157_datahi, valptr, timestamp);
}


void (*ram_8bit_in_dataN[])(void *dest, int *valptr, int timestamp) = {

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

void ram_8bit_in_addr0(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b1(dest->ls157_addr, valptr, timestamp);
}

void ram_8bit_in_addr1(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b2(dest->ls157_addr, valptr, timestamp);
}

void ram_8bit_in_addr2(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b3(dest->ls157_addr, valptr, timestamp);
}

void ram_8bit_in_addr3(ram_8bit *dest, int *valptr, int timestamp){

    ls157_in_b4(dest->ls157_addr, valptr, timestamp);
}

void (*ram_8bit_in_addrN[])(void *dest, int *valptr, int timestamp) = {

        (void*)ram_8bit_in_addr0,
        (void*)ram_8bit_in_addr1,
        (void*)ram_8bit_in_addr2,
        (void*)ram_8bit_in_addr3,
};

////////////////////////////////////////////////////////////////////////////////
board_object *ram_8bit_board_create(ram_8bit *ram, int key, char *name){

    board_object *board = board_create(40, 9, key, name);

    if (!board) return board;

    char s[32];
    int i,j;

    for (i = 0; i < 8; i++){

        j = 7-i;

        sprintf(s,"D%d",i);
        board_add_led(board, ram->leddata[i],1+4*j,1,s);
    }

    for (i = 0; i < 4; i++){

        j = 3-i;

        sprintf(s,"A%d",i);
        board_add_led(board, ram->ledaddr[i],17+4*j,4,s);
    }


    board_add_manual_switch(board, ram->prog_run, 1, 4, 'r',"Pr/Run");

    return board;
}
