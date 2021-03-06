////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

// Based on Ben Eater's project (https://www.youtube.com/watch?v=S-3fXU3FZQc)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "alu_8bit.h"
#include "bitconst.h"

////////////////////////////////////////////////////////////////////////////////
alu_8bit *alu_8bit_create(event_context_t *ec, char *name){

    alu_8bit *alu = malloc (sizeof(alu_8bit));
    if (!alu) return alu;

    alu->ec = ec;

    char lshi[60];
    char lslo[60];

    lshi[0] = lslo[0] = 0;
    if (name){

        strncpy(lshi,name,sizeof(lshi));
        strncpy(lslo,name,sizeof(lslo));
    }

    strncat(lshi,"-hiWord",sizeof(lshi)/2);
    strncat(lslo,"-loWord",sizeof(lslo)/2);

    alu->ls86_hi = ls86_create(ec);
    alu->ls86_lo = ls86_create(ec);
    alu->ls283_hi = ls283_create(ec, lshi);
    alu->ls283_lo = ls283_create(ec, lslo);
    alu->ls245_1  = ls245_create(ec);

    int i;
    for (i = 0; i < 8; i++){

        strncpy(lshi,name,sizeof(lshi));
        sprintf(lslo,"-D%d",i);
        strncat(lshi,lslo,sizeof(lshi)/2);
        alu->led[i] = indicator_create(ec, lshi);
    }

    ls283_connect_y1(alu->ls283_lo, alu->led[0], (void*)&indicator_in_d0);
    ls283_connect_y2(alu->ls283_lo, alu->led[1], (void*)&indicator_in_d0);
    ls283_connect_y3(alu->ls283_lo, alu->led[2], (void*)&indicator_in_d0);
    ls283_connect_y4(alu->ls283_lo, alu->led[3], (void*)&indicator_in_d0);

    ls283_connect_y1(alu->ls283_hi, alu->led[4], (void*)&indicator_in_d0);
    ls283_connect_y2(alu->ls283_hi, alu->led[5], (void*)&indicator_in_d0);
    ls283_connect_y3(alu->ls283_hi, alu->led[6], (void*)&indicator_in_d0);
    ls283_connect_y4(alu->ls283_hi, alu->led[7], (void*)&indicator_in_d0);

    ls283_connect_y1(alu->ls283_lo, alu->ls245_1, (void*)&ls245_in_a1);
    ls283_connect_y2(alu->ls283_lo, alu->ls245_1, (void*)&ls245_in_a2);
    ls283_connect_y3(alu->ls283_lo, alu->ls245_1, (void*)&ls245_in_a3);
    ls283_connect_y4(alu->ls283_lo, alu->ls245_1, (void*)&ls245_in_a4);

    ls283_connect_y1(alu->ls283_hi, alu->ls245_1, (void*)&ls245_in_a5);
    ls283_connect_y2(alu->ls283_hi, alu->ls245_1, (void*)&ls245_in_a6);
    ls283_connect_y3(alu->ls283_hi, alu->ls245_1, (void*)&ls245_in_a7);
    ls283_connect_y4(alu->ls283_hi, alu->ls245_1, (void*)&ls245_in_a8);

    ls86_connect_y1(alu->ls86_lo, alu->ls283_lo, (void*)&ls283_in_b1);
    ls86_connect_y2(alu->ls86_lo, alu->ls283_lo, (void*)&ls283_in_b2);
    ls86_connect_y3(alu->ls86_lo, alu->ls283_lo, (void*)&ls283_in_b3);
    ls86_connect_y4(alu->ls86_lo, alu->ls283_lo, (void*)&ls283_in_b4);

    ls86_connect_y1(alu->ls86_hi, alu->ls283_hi, (void*)&ls283_in_b1);
    ls86_connect_y2(alu->ls86_hi, alu->ls283_hi, (void*)&ls283_in_b2);
    ls86_connect_y3(alu->ls86_hi, alu->ls283_hi, (void*)&ls283_in_b3);
    ls86_connect_y4(alu->ls86_hi, alu->ls283_hi, (void*)&ls283_in_b4);

    bitconst_connect_one(alu->ls245_1, (void*)&ls245_in_dir);

    ls283_connect_cout(alu->ls283_lo, alu->ls283_hi, (void*)&ls283_in_cin);

    if (name)
        strncpy(alu->name,name,sizeof(alu->name));
    else
        alu->name[0] = 0;

    alu->ls02 = ls02_create(ec);
    alu->ls08 = ls08_create(ec);

    ls283_connect_y1(alu->ls283_lo, alu->ls02, (void*)&ls02_in_a1);
    ls283_connect_y2(alu->ls283_lo, alu->ls02, (void*)&ls02_in_b1);
    ls283_connect_y3(alu->ls283_lo, alu->ls02, (void*)&ls02_in_a2);
    ls283_connect_y4(alu->ls283_lo, alu->ls02, (void*)&ls02_in_b2);

    ls283_connect_y1(alu->ls283_hi, alu->ls02, (void*)&ls02_in_a3);
    ls283_connect_y2(alu->ls283_hi, alu->ls02, (void*)&ls02_in_b3);
    ls283_connect_y3(alu->ls283_hi, alu->ls02, (void*)&ls02_in_a4);
    ls283_connect_y4(alu->ls283_hi, alu->ls02, (void*)&ls02_in_b4);

    ls02_connect_y1(alu->ls02, alu->ls08, (void*)&ls08_in_a1);
    ls02_connect_y2(alu->ls02, alu->ls08, (void*)&ls08_in_b1);
    ls02_connect_y3(alu->ls02, alu->ls08, (void*)&ls08_in_a2);
    ls02_connect_y4(alu->ls02, alu->ls08, (void*)&ls08_in_b2);

    ls08_connect_y1(alu->ls08, alu->ls08, (void*)&ls08_in_a3);
    ls08_connect_y2(alu->ls08, alu->ls08, (void*)&ls08_in_b3);

    alu->ledz = indicator_create(ec, "Z");
    alu->ledc = indicator_create(ec, "C");

    ls08_connect_y3(alu->ls08, alu->ledz, (void*)&indicator_in_d0);
    ls283_connect_cout(alu->ls283_hi, alu->ledc, (void*)&indicator_in_d0);

    alu->destroy = (void*)alu_8bit_destroy;

    return alu;
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_destroy (alu_8bit **dest){

    if (dest == NULL) return;
    alu_8bit *b = *dest;
    if (b == NULL) return;

    DESTROY(b->ls86_hi);
    DESTROY(b->ls86_lo);
    DESTROY(b->ls283_hi);
    DESTROY(b->ls283_lo);
    DESTROY(b->ls245_1);
    DESTROY(b->ls02);
    DESTROY(b->ls08);

    int i;
    for (i = 0; i < 8; i++)
        indicator_destroy(&b->led[i]);

    DESTROY(b->ledz);
    DESTROY(b->ledc);

    free(b);
    *dest = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_connect_carry_out (alu_8bit *source, void *dest, event_function_t dest_event_handler){

    ls283_connect_cout(source->ls283_hi, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_connect_zero_out (alu_8bit *source, void *dest, event_function_t dest_event_handler){

    ls08_connect_y3(source->ls08, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_connect_bit_out (alu_8bit *source, int index, void *dest, event_function_t dest_event_handler){

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
void alu_8bit_in_dataA_from(void (*connect_fn)(void *source, void *dest, event_function_t dest_event_handler),
                        void *from, alu_8bit *dest, int index){

    switch(index){
        case 0: connect_fn(from,dest->ls283_lo,(void*)&ls283_in_a1); break;
        case 1: connect_fn(from,dest->ls283_lo,(void*)&ls283_in_a2); break;
        case 2: connect_fn(from,dest->ls283_lo,(void*)&ls283_in_a3); break;
        case 3: connect_fn(from,dest->ls283_lo,(void*)&ls283_in_a4); break;
        case 4: connect_fn(from,dest->ls283_hi,(void*)&ls283_in_a1); break;
        case 5: connect_fn(from,dest->ls283_hi,(void*)&ls283_in_a2); break;
        case 6: connect_fn(from,dest->ls283_hi,(void*)&ls283_in_a3); break;
        case 7: connect_fn(from,dest->ls283_hi,(void*)&ls283_in_a4); break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_in_dataB_from(void (*connect_fn)(void *source, void *dest, event_function_t dest_event_handler),
                        void *from, alu_8bit *dest, int index){

    switch(index){
        case 0: connect_fn(from,dest->ls86_lo,(void*)&ls86_in_a1); break;
        case 1: connect_fn(from,dest->ls86_lo,(void*)&ls86_in_a2); break;
        case 2: connect_fn(from,dest->ls86_lo,(void*)&ls86_in_a3); break;
        case 3: connect_fn(from,dest->ls86_lo,(void*)&ls86_in_a4); break;
        case 4: connect_fn(from,dest->ls86_hi,(void*)&ls86_in_a1); break;
        case 5: connect_fn(from,dest->ls86_hi,(void*)&ls86_in_a2); break;
        case 6: connect_fn(from,dest->ls86_hi,(void*)&ls86_in_a3); break;
        case 7: connect_fn(from,dest->ls86_hi,(void*)&ls86_in_a4); break;
    }
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_in_sub_from(void (*connect_fn)(void *source, void *dest, event_function_t dest_event_handler),
                        void *from, alu_8bit *dest){

    connect_fn(from,dest->ls86_lo,(void*)&ls86_in_b1);
    connect_fn(from,dest->ls86_lo,(void*)&ls86_in_b2);
    connect_fn(from,dest->ls86_lo,(void*)&ls86_in_b3);
    connect_fn(from,dest->ls86_lo,(void*)&ls86_in_b4);
    connect_fn(from,dest->ls86_hi,(void*)&ls86_in_b1);
    connect_fn(from,dest->ls86_hi,(void*)&ls86_in_b2);
    connect_fn(from,dest->ls86_hi,(void*)&ls86_in_b3);
    connect_fn(from,dest->ls86_hi,(void*)&ls86_in_b4);

    connect_fn(from,dest->ls283_lo,(void*)&ls283_in_cin);
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_in_enable_from(void (*connect_fn)(void *source, void *dest, event_function_t dest_event_handler),
                        void *from, alu_8bit *dest){

    connect_fn(from,dest->ls245_1,(void*)&ls245_in_oe);
}

////////////////////////////////////////////////////////////////////////////////
void alu_8bit_in_dataA0(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a1(dest->ls283_lo, valptr, timestamp);
}

void alu_8bit_in_dataA1(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a2(dest->ls283_lo, valptr, timestamp);
}

void alu_8bit_in_dataA2(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a3(dest->ls283_lo, valptr, timestamp);
}

void alu_8bit_in_dataA3(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a4(dest->ls283_lo, valptr, timestamp);
}

void alu_8bit_in_dataA4(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a1(dest->ls283_hi, valptr, timestamp);
}

void alu_8bit_in_dataA5(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a2(dest->ls283_hi, valptr, timestamp);
}

void alu_8bit_in_dataA6(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a3(dest->ls283_hi, valptr, timestamp);
}

void alu_8bit_in_dataA7(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_in_a4(dest->ls283_hi, valptr, timestamp);
}

event_function_t alu_8bit_in_dataAN[] = {

        (void*)alu_8bit_in_dataA0,
        (void*)alu_8bit_in_dataA1,
        (void*)alu_8bit_in_dataA2,
        (void*)alu_8bit_in_dataA3,
        (void*)alu_8bit_in_dataA4,
        (void*)alu_8bit_in_dataA5,
        (void*)alu_8bit_in_dataA6,
        (void*)alu_8bit_in_dataA7,
};


////////////////////////////////////////////////////////////////////////////////
void alu_8bit_in_dataB0(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a1(dest->ls86_lo, valptr, timestamp);
}

void alu_8bit_in_dataB1(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a2(dest->ls86_lo, valptr, timestamp);
}

void alu_8bit_in_dataB2(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a3(dest->ls86_lo, valptr, timestamp);
}

void alu_8bit_in_dataB3(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a4(dest->ls86_lo, valptr, timestamp);
}

void alu_8bit_in_dataB4(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a1(dest->ls86_hi, valptr, timestamp);
}

void alu_8bit_in_dataB5(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a2(dest->ls86_hi, valptr, timestamp);
}

void alu_8bit_in_dataB6(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a3(dest->ls86_hi, valptr, timestamp);
}

void alu_8bit_in_dataB7(alu_8bit *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls86_in_a4(dest->ls86_hi, valptr, timestamp);
}

event_function_t alu_8bit_in_dataBN[] = {

        (void*)alu_8bit_in_dataB0,
        (void*)alu_8bit_in_dataB1,
        (void*)alu_8bit_in_dataB2,
        (void*)alu_8bit_in_dataB3,
        (void*)alu_8bit_in_dataB4,
        (void*)alu_8bit_in_dataB5,
        (void*)alu_8bit_in_dataB6,
        (void*)alu_8bit_in_dataB7,
};

////////////////////////////////////////////////////////////////////////////////
board_object *alu_8bit_board_create(alu_8bit *alu, int key, char *name){

    board_object *board = board_create(40, 4, key, name);

    if (!board) return board;

    char s[32];
    int i,j;

    for (i = 0; i < 8; i++){

        j = 7-i;

        sprintf(s,"D%d",i);
        board_add_led(board, alu->led[i],1+4*j,1,s, LED_RED);
    }

    board_add_spacer(board, 33,1);

    board_add_led(board, alu->ledz,34,1," Z", LED_WHITE);
    board_add_led(board, alu->ledc,36,1," C", LED_WHITE);

    return board;
}
