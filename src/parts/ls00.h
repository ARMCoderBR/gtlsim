/*
 * ls00.h
 *
 *  Created on: 29 de mar de 2020
 *      Author: milton
 */

#ifndef LS00_H_
#define LS00_H_

#include "update.h"

#define NUM_ELEM_LS00 4

typedef struct {

    part_destroy_function_t destroy;
    bitvalue_t ina[NUM_ELEM_LS00];
    vallist *ina_rootptr[NUM_ELEM_LS00];
    bitvalue_t inb[NUM_ELEM_LS00];
    vallist *inb_rootptr[NUM_ELEM_LS00];
    bitvalue_t oldvalue[NUM_ELEM_LS00];
    bitvalue_t value[NUM_ELEM_LS00];
    ehandler *out_event_handler_root[NUM_ELEM_LS00];
} ls00;

ls00 *ls00_create();

void ls00_destroy (ls00 **dest);

void ls00_connect_y1(ls00 *source, void *dest, event_function_t dest_event_handler);
void ls00_connect_y2(ls00 *source, void *dest, event_function_t dest_event_handler);
void ls00_connect_y3(ls00 *source, void *dest, event_function_t dest_event_handler);
void ls00_connect_y4(ls00 *source, void *dest, event_function_t dest_event_handler);

void ls00_in_a1(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void ls00_in_b1(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void ls00_in_a2(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void ls00_in_b2(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void ls00_in_a3(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void ls00_in_b3(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void ls00_in_a4(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void ls00_in_b4(ls00 *dest, bitvalue_t *valptr, timevalue_t timestamp);

#endif /* LS00_H_ */
