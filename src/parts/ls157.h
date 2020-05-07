/*
 * ls157.h
 *
 *  Created on: 25 de abr de 2020
 *      Author: milton
 */

#ifndef LS157_H_
#define LS157_H_

#include "update.h"

#define NUM_ELEM_LS157 4

typedef struct {

    void (*destroy)(void **dest);
    int ina[NUM_ELEM_LS157];
    vallist *ina_rootptr[NUM_ELEM_LS157];
    int inb[NUM_ELEM_LS157];
    vallist *inb_rootptr[NUM_ELEM_LS157];
    int in_sel;
    vallist *in_sel_rootptr;
    int in_g;
    vallist *in_g_rootptr;
    int oldvalue[NUM_ELEM_LS157];
    int value[NUM_ELEM_LS157];
    ehandler *out_event_handler_root[NUM_ELEM_LS157];
} ls157;

ls157 *ls157_create();

void ls157_destroy(ls157 **dest);

void ls157_connect_y1(ls157 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp));
void ls157_connect_y2(ls157 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp));
void ls157_connect_y3(ls157 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp));
void ls157_connect_y4(ls157 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp));

void ls157_in_a1(ls157 *dest, int *valptr, int timestamp);
void ls157_in_b1(ls157 *dest, int *valptr, int timestamp);
void ls157_in_a2(ls157 *dest, int *valptr, int timestamp);
void ls157_in_b2(ls157 *dest, int *valptr, int timestamp);
void ls157_in_a3(ls157 *dest, int *valptr, int timestamp);
void ls157_in_b3(ls157 *dest, int *valptr, int timestamp);
void ls157_in_a4(ls157 *dest, int *valptr, int timestamp);
void ls157_in_b4(ls157 *dest, int *valptr, int timestamp);

void ls157_in_sel(ls157 *dest, int *valptr, int timestamp);

void ls157_in_g(ls157 *dest, int *valptr, int timestamp);

#endif /* LS157_H_ */
