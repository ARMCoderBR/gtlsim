/*
 * ctrunit.c
 *
 *  Created on: 01 de mai de 2020
 *      Author: milton
 */

// Baseado no projeto do Ben Eater (https://www.youtube.com/watch?v=AwUirxi9eBg)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ctrunit.h"
#include "bitconst.h"

char labels[16][4] = {"", "J", "CO", "CE", "OI", "BI", "SU", "SO", "AO", "AI", "II", "IO", "RO", "RI", "MI", "HLT" };

////////////////////////////////////////////////////////////////////////////////
ctrunit *ctrunit_create(char *name){

    ctrunit *ctru = malloc (sizeof(ctrunit));
    if (!ctru) return ctru;

    ctru->ls00 = ls00_create();
    ctru->ls04_1 = ls04_create();
    ctru->ls04_2 = ls04_create();
    ctru->reset_sw = bitswitch_create("Reset");

    bitswitch_connect_out(ctru->reset_sw, ctru->ls00, (void*)&ls00_in_a1);
    bitswitch_connect_out(ctru->reset_sw, ctru->ls00, (void*)&ls00_in_b1);
    ls00_connect_y1(ctru->ls00, ctru->ls00, (void*)&ls00_in_a2);
    ls00_connect_y1(ctru->ls00, ctru->ls00, (void*)&ls00_in_b2);

    int i;
    for (i = 0; i < NSIGNALS_CTRU; i++){

        ctru->led[i] = indicator_create(labels[i]);
        ctru->in_rootptr[i] = NULL; ctru->out_event_handler_root[i] = NULL;
    }

    //// LS161
    ctru->ls161 = ls161_create();
    ctru->ct[0] = indicator_create("C0");
    ctru->ct[1] = indicator_create("C1");
    ctru->ct[2] = indicator_create("C2");
    ls161_connect_qa(ctru->ls161, ctru->ct[0], (void*)&indicator_in_d0);
    ls161_connect_qb(ctru->ls161, ctru->ct[1], (void*)&indicator_in_d0);
    ls161_connect_qc(ctru->ls161, ctru->ct[2], (void*)&indicator_in_d0);

    bitconst_connect_one(ctru->ls161, (void*)&ls161_in_enp);
    bitconst_connect_one(ctru->ls161, (void*)&ls161_in_ent);
    bitconst_connect_one(ctru->ls161, (void*)&ls161_in_load);
    ls00_connect_y1(ctru->ls00, ctru->ls161, (void*)&ls161_in_clear);

    //// LS138
    ctru->ls138 = ls138_create();
    ctru->t[0]  = indicator_create("T0");
    ctru->t[1]  = indicator_create("T1");
    ctru->t[2]  = indicator_create("T2");
    ctru->t[3]  = indicator_create("T3");
    ctru->t[4]  = indicator_create("T4");
    ctru->t[5]  = indicator_create("T5");
    bitconst_connect_zero(ctru->ls138, (void*)&ls138_ing2a);
    bitconst_connect_zero(ctru->ls138, (void*)&ls138_ing2b);
    bitconst_connect_one(ctru->ls138, (void*)&ls138_ing1);
    ls138_connect_y0(ctru->ls138, ctru->t[0], (void*)&indicator_in_d0);
    ls138_connect_y1(ctru->ls138, ctru->t[1], (void*)&indicator_in_d0);
    ls138_connect_y2(ctru->ls138, ctru->t[2], (void*)&indicator_in_d0);
    ls138_connect_y3(ctru->ls138, ctru->t[3], (void*)&indicator_in_d0);
    ls138_connect_y4(ctru->ls138, ctru->t[4], (void*)&indicator_in_d0);
    ls138_connect_y5(ctru->ls138, ctru->t[5], (void*)&indicator_in_d0);

    ls161_connect_qa(ctru->ls161, ctru->ls138, (void*)&ls138_ina);
    ls161_connect_qb(ctru->ls161, ctru->ls138, (void*)&ls138_inb);
    ls161_connect_qc(ctru->ls161, ctru->ls138, (void*)&ls138_inc);

    ctru->ledclk  = indicator_create("CLK");

    ctru->destroy = (void*)ctrunit_destroy;

    return ctru;
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_destroy (ctrunit **dest){

    if (dest == NULL) return;
    ctrunit *b = *dest;
    if (b == NULL) return;

    DESTROY(b->ls00);
    DESTROY(b->ls04_1);
    DESTROY(b->ls04_2);
    DESTROY(b->reset_sw);

    DESTROY(b->ls161);
    DESTROY(b->ct[0]);
    DESTROY(b->ct[1]);
    DESTROY(b->ct[2]);

    DESTROY(b->ls138);
    DESTROY(b->t[0]);
    DESTROY(b->t[1]);
    DESTROY(b->t[2]);
    DESTROY(b->t[3]);
    DESTROY(b->t[4]);
    DESTROY(b->t[5]);

    int i;
    for (i = 0; i < NSIGNALS_CTRU; i++){

        DESTROY(b->led[i]);
        vallist_destroy(&b->in_rootptr[i]);
        ehandler_destroy(&b->out_event_handler_root[i]);
    }

    free(b);
    *dest = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in(ctrunit *dest, int index, int *valptr, int timestamp){

    indicator_in_d0(dest->led[index], valptr, timestamp);

    int val = update_val_multi(&dest->in_rootptr[index], valptr);

    if (val > 1)
        val = 1;

    dest->val[index] = val;

    if (val == dest->o_val[index]) return;

    dest->o_val[index] = val;

    event e;
    e.event_handler_root = dest->out_event_handler_root[index];
    e.valueptr = &dest->val[index];
    e.timestamp = timestamp+1;
    event_insert(&e);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out(ctrunit *source, int index, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    new_ehandler(&source->out_event_handler_root[index], dest, dest_event_handler);
    dest_event_handler(dest,&source->val[index],0);
}

////////////////////////////////////////////////////////////////////////////////
board_object *ctrunit_board_create(ctrunit *reg, int key, char *name){

    board_object *board = board_create(62, 7, key, name);

    if (!board) return board;

    char s[32];
    int i,j;

    for (i = 1; i < NSIGNALS_CTRU; i++){

        j = NSIGNALS_CTRU-i;
        sprintf(s,labels[i],i);
        board_add_led(board, reg->led[i],1+4*(j-1),1,s, LED_BLUE);
    }

    board_add_manual_switch(board, reg->reset_sw, 1, 4, 'r', "RST");

    board_add_led(board, reg->ct[2],9,4,"C2", LED_RED);
    board_add_led(board, reg->ct[1],13,4,"C1", LED_RED);
    board_add_led(board, reg->ct[0],17,4,"C0", LED_RED);

    board_add_led(board, reg->t[0],25,4,"T0", LED_GREEN);
    board_add_led(board, reg->t[1],29,4,"T1", LED_GREEN);
    board_add_led(board, reg->t[2],33,4,"T2", LED_GREEN);
    board_add_led(board, reg->t[3],37,4,"T3", LED_GREEN);
    board_add_led(board, reg->t[4],41,4,"T4", LED_GREEN);
    board_add_led(board, reg->t[5],45,4,"T5", LED_GREEN);

    board_add_led(board, reg->ledclk,57,4,"CLK", LED_BLUE);

    return board;
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_hlt(ctrunit *dest, int *valptr, int timestamp){

    ctrunit_in(dest, HLT, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_hlt(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ctrunit_connect_out(source, HLT, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_mi(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a4(dest->ls04_2, valptr, timestamp);
    //ctrunit_in(dest, MI, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_mi(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y4(source->ls04_2, dest, dest_event_handler);
    //ctrunit_connect_out(source, MI, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_ri(ctrunit *dest, int *valptr, int timestamp){

    ctrunit_in(dest, RI, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_ri(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ctrunit_connect_out(source, RI, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_ro(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a1(dest->ls04_1, valptr, timestamp);
    //ctrunit_in(dest, RO, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_ro(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y1(source->ls04_1, dest, dest_event_handler);
    //ctrunit_connect_out(source, RO, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_io(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a2(dest->ls04_1, valptr, timestamp);
    //ctrunit_in(dest, IO, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_io(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y2(source->ls04_1, dest, dest_event_handler);
    //ctrunit_connect_out(source, IO, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_ii(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a3(dest->ls04_2, valptr, timestamp);
    //ctrunit_in(dest, II, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_ii(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y3(source->ls04_2, dest, dest_event_handler);
    //ctrunit_connect_out(source, II, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_ai(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a1(dest->ls04_2, valptr, timestamp);
    //ctrunit_in(dest, AI, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_ai(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y1(source->ls04_2, dest, dest_event_handler);
    //ctrunit_connect_out(source, AI, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_ao(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a3(dest->ls04_1, valptr, timestamp);
    //ctrunit_in(dest, AO, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_ao(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y3(source->ls04_1, dest, dest_event_handler);
    //ctrunit_connect_out(source, AO, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_so(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a4(dest->ls04_1, valptr, timestamp);
    //ctrunit_in(dest, SO, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_so(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y4(source->ls04_1, dest, dest_event_handler);
    //ctrunit_connect_out(source, SO, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_su(ctrunit *dest, int *valptr, int timestamp){

    ctrunit_in(dest, SU, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_su(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ctrunit_connect_out(source, SU, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_bi(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a2(dest->ls04_2, valptr, timestamp);
    //ctrunit_in(dest, BI, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_bi(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y2(source->ls04_2, dest, dest_event_handler);
    //ctrunit_connect_out(source, BI, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_oi(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a5(dest->ls04_2, valptr, timestamp);
    //ctrunit_in(dest, OI, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_oi(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y5(source->ls04_2, dest, dest_event_handler);
    //ctrunit_connect_out(source, OI, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_ce(ctrunit *dest, int *valptr, int timestamp){

    ctrunit_in(dest, CE, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_ce(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ctrunit_connect_out(source, CE, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_co(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a5(dest->ls04_1, valptr, timestamp);
    //ctrunit_in(dest, CO, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_co(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y5(source->ls04_1, dest, dest_event_handler);
    //ctrunit_connect_out(source, CO, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_j(ctrunit *dest, int *valptr, int timestamp){

    ls04_in_a6(dest->ls04_1, valptr, timestamp);
    //ctrunit_in(dest, J, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_in_clk(ctrunit *dest, int *valptr, int timestamp){

    ls161_in_clk(dest->ls161, valptr, timestamp);
    indicator_in_d0(dest->ledclk, valptr, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_j(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls04_connect_y6(source->ls04_1, dest, dest_event_handler);
    //ctrunit_connect_out(source, J, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_reset(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls00_connect_y2(source->ls00, dest, dest_event_handler);
}

////////////////////////////////////////////////////////////////////////////////
void ctrunit_connect_out_nreset(ctrunit *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    ls00_connect_y1(source->ls00, dest, dest_event_handler);
}
