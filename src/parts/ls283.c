////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

#include <malloc.h>
#include <string.h>

#include "ls283.h"

// 4-BIT BINARY ADDER WITH FAST CARRY

////////////////////////////////////////////////////////////////////////////////
static void ls283_up(ls283 *a, timevalue_t timestamp){

    uint8_t suma, sumb;
    int mask = 1;
    int i = 0;

    suma = sumb = 0;

    for (i = 0; i < LS283_NBITS; i++){

        if (a->ina[i])
            suma |= mask;

        if (a->inb[i])
            sumb |= mask;

        mask <<= 1;
    }

    logger(a->ec, "ls283 suma:%d sumb:%d",suma,sumb);

    suma += sumb;

    if (a->cin)
        suma++;

    mask = 1;

    for (i = 0; i < LS283_NBITS; i++){

        if (suma & mask)
            a->y[i] = 1;
        else
            a->y[i] = 0;

        if (a->y[i] != a->oldy[i]){

            a->oldy[i] = a->y[i];
            event e;
            e.event_handler_root = a->y_event_handler_root[i];
            e.valueptr = &a->y[i];
            e.timestamp = timestamp+1;
            event_insert(a->ec, &e);
        }

        mask <<= 1;
    }

    if (suma & mask)
        a->cout = 1;
    else
        a->cout = 0;

    if (a->cout != a->oldcout){

        a->oldcout = a->cout;
        event e;
        e.event_handler_root = a->cout_event_handler_root;
        e.valueptr = &a->cout;
        e.timestamp = timestamp+1;
        event_insert(a->ec, &e);
    }
}

////////////////////////////////////////////////////////////////////////////////
static void ls283_update_pin_ina(ls283 *a, bitvalue_t *valptr, timevalue_t timestamp, int index){

    bitvalue_t val = update_val_multi(&a->ina_rootptr[index], valptr);

    if (val > 1) val = 1;

    if (val == a->ina[index]) return;

    logger(a->ec, "ls283_update_pin_ina%d [%s] *valptr:%d val:%d TS:%d",index, a->name,*valptr,val,timestamp);

    a->ina[index] = val;
    ls283_up(a, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
static void ls283_update_pin_inb(ls283 *a, bitvalue_t *valptr, timevalue_t timestamp, int index){

    bitvalue_t val = update_val_multi(&a->inb_rootptr[index], valptr);

    if (val > 1) val = 1;

    if (val == a->inb[index]) return;

    logger(a->ec, "ls283_update_pin_inb%d [%s] *valptr:%d val:%d TS:%d",index, a->name,*valptr,val,timestamp);

    a->inb[index] = val;
    ls283_up(a, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
static void ls283_update_pin_cin(ls283 *a, bitvalue_t *valptr, timevalue_t timestamp){

    bitvalue_t val = update_val_multi(&a->cin_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == a->cin) return;

    logger(a->ec, "ls283_update_pin_cin [%s] *valptr:%d val:%d TS:%d",a->name,*valptr,val,timestamp);

    a->cin = val;
    ls283_up(a, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
ls283 *ls283_create(event_context_t *ec, char *name){

    ls283 *b = malloc(sizeof(ls283));

    if (b == NULL)
        return NULL;

    b->ec = ec;

    int i;

    for (i = 0; i < LS283_NBITS; i++){

        b->ina[i] = b->inb[i] = 2;
        b->y[i] = 0;
        b->ina_rootptr[i] = b->inb_rootptr[i] = NULL;
        b->oldy[i] = 2;
        b->y_event_handler_root[i] = NULL;
    }

    b->cin = 2;
    b->cout = 0;
    b->cin_rootptr = NULL;
    b->oldcout = 2;
    b->cout_event_handler_root = NULL;

    if (name)
        strncpy(b->name,name,sizeof(b->name));
    else
        b->name[0] = 0;

    b->destroy = (void*)ls283_destroy;

    return b;
}

////////////////////////////////////////////////////////////////////////////////
void ls283_destroy (ls283 **dest){

    if (dest == NULL) return;
    ls283 *b = *dest;
    if (b == NULL) return;

    int i;
    for (i = 0; i < LS283_NBITS; i++){

        vallist_destroy(&b->ina_rootptr[i]);
        vallist_destroy(&b->inb_rootptr[i]);
        ehandler_destroy(&b->y_event_handler_root[i]);
    }

    vallist_destroy(&b->cin_rootptr);
    ehandler_destroy(&b->cout_event_handler_root);

    free(b);
    *dest = NULL;
}

////////////////////////////////////////////////////////////////////////////////
void ls283_connect_y1(ls283 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->y_event_handler_root[0], dest, dest_event_handler);
    dest_event_handler(dest,&source->y[0],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_connect_y2(ls283 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->y_event_handler_root[1], dest, dest_event_handler);
    dest_event_handler(dest,&source->y[1],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_connect_y3(ls283 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->y_event_handler_root[2], dest, dest_event_handler);
    dest_event_handler(dest,&source->y[2],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_connect_y4(ls283 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->y_event_handler_root[3], dest, dest_event_handler);
    dest_event_handler(dest,&source->y[3],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_connect_cout(ls283 *source, void *dest, event_function_t dest_event_handler){

    new_ehandler(&source->cout_event_handler_root, dest, dest_event_handler);
    dest_event_handler(dest,&source->cout,0);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_a1(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_ina(dest, valptr, timestamp, 0);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_a2(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_ina(dest, valptr, timestamp, 1);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_a3(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_ina(dest, valptr, timestamp, 2);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_a4(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_ina(dest, valptr, timestamp, 3);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_b1(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_inb(dest, valptr, timestamp, 0);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_b2(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_inb(dest, valptr, timestamp, 1);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_b3(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_inb(dest, valptr, timestamp, 2);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_b4(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_inb(dest, valptr, timestamp, 3);
}

////////////////////////////////////////////////////////////////////////////////
void ls283_in_cin(ls283 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls283_update_pin_cin(dest, valptr, timestamp);
}
