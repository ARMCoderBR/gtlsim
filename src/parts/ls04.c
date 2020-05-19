////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

#include <malloc.h>

#include "update.h"
#include "ls04.h"

// HEX INVERTER

////////////////////////////////////////////////////////////////////////////////
static int ls04_update(ls04 *a, int index){

    a->value[index] = a->in[index] ^ 1;

    if (a->oldvalue[index] != a->value[index]){

        a->oldvalue[index] = a->value[index];
        return 1;
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
static void ls04_up(ls04 *a, int timestamp, int index){

    if (ls04_update(a,index)){

        event e;
        e.event_handler_root = a->out_event_handler_root[index];
        e.valueptr = &a->value[index];
        e.timestamp = timestamp+1;
        event_insert(a->ec, &e);
    }
}

////////////////////////////////////////////////////////////////////////////////
static void ls04_update_pin_in(ls04 *a, bitvalue_t *valptr, timevalue_t timestamp, int index){

    bitvalue_t val = update_val_multi(&a->in_rootptr[index], valptr);

    if (val > 1) val = 1;

    if (val == a->in[index]) return;

    a->in[index] = val;
    ls04_up(a, timestamp, index);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
ls04 *ls04_create(event_context_t *ec){

    ls04 *b = malloc(sizeof(ls04));

    if (b == NULL)
        return NULL;

    b->ec = ec;

    int i;
    for (i = 0; i < NUM_ELEM_LS04; i++){

        b->in[i] = 2;
        b->value[i] = b->oldvalue[i] = 0;
        b->in_rootptr[i] = NULL;
        b->out_event_handler_root[i] = NULL;
    }

    b->destroy = (void*)ls04_destroy;

    return b;
}

////////////////////////////////////////////////////////////////////////////////
void ls04_destroy (ls04 **dest){

	if (dest == NULL) return;
	ls04 *b = *dest;
	if (b == NULL) return;

	int i;
    for (i = 0; i < NUM_ELEM_LS04; i++){

        vallist_destroy(&b->in_rootptr[i]);
        ehandler_destroy(&b->out_event_handler_root[i]);
    }

	free(b);
	*dest = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void ls04_connect_y1(ls04 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->out_event_handler_root[0], dest, dest_event_handler);
    dest_event_handler(dest,&source->value[0],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_connect_y2(ls04 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->out_event_handler_root[1], dest, dest_event_handler);
    dest_event_handler(dest,&source->value[1],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_connect_y3(ls04 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->out_event_handler_root[2], dest, dest_event_handler);
    dest_event_handler(dest,&source->value[2],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_connect_y4(ls04 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->out_event_handler_root[3], dest, dest_event_handler);
    dest_event_handler(dest,&source->value[3],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_connect_y5(ls04 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->out_event_handler_root[4], dest, dest_event_handler);
    dest_event_handler(dest,&source->value[4],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_connect_y6(ls04 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->out_event_handler_root[5], dest, dest_event_handler);
    dest_event_handler(dest,&source->value[5],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_in_a1(ls04 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls04_update_pin_in(dest, valptr, timestamp, 0);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_in_a2(ls04 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls04_update_pin_in(dest, valptr, timestamp, 1);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_in_a3(ls04 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls04_update_pin_in(dest, valptr, timestamp, 2);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_in_a4(ls04 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls04_update_pin_in(dest, valptr, timestamp, 3);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_in_a5(ls04 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls04_update_pin_in(dest, valptr, timestamp, 4);
}

////////////////////////////////////////////////////////////////////////////////
void ls04_in_a6(ls04 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls04_update_pin_in(dest, valptr, timestamp, 5);
}
