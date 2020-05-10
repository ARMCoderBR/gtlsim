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

#include "update.h"
#include "ls173.h"

// 4-BIT D-TYPE REGISTERS WITH 3-STATE OUTPUTS

////////////////////////////////////////////////////////////////////////////////
static void ls173_update (ls173 *a, timevalue_t timestamp){

    int i;

    if (a->in_clr){

        for (i = 0; i < NUM_ELEM_LS173; i++)
            a->outq[i] = a->latd[i] = 0;

        goto ls173_end;
    }

    ////////////////////////////////
    if (a->clk_o != a->clk){

        a->clk_o = a->clk;

        if ((a->clk) && (!a->in_g1) && (!a->in_g2))
            for (i = 0; i < NUM_ELEM_LS173; i++)
                a->latd[i] = a->inpd[i];
    }

    ////////////////////////////////
    if (a->in_m | a->in_n){

        for (i = 0; i < NUM_ELEM_LS173; i++)
            a->outq[i] = 2; //HiZ
    }
    else{
        for (i = 0; i < NUM_ELEM_LS173; i++)
            a->outq[i] = a->latd[i];
    }

    ////////////////////////////////
    event e;

ls173_end:
    e.timestamp = timestamp+1;

    for (i = 0; i < NUM_ELEM_LS173; i++){

        if (a->outq_o[i] != a->outq[i]){

            a->outq_o[i] = a->outq[i];
            e.event_handler_root = a->outq_event_handler_root[i];
            e.valueptr = &a->outq[i];
            event_insert(&e);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
static void ls173_update_d(ls173 *a, bitvalue_t *valptr, timevalue_t timestamp, int index){

    bitvalue_t val = update_val_multi(&a->inpd_rootptr[index], valptr);

    if (val > 1) val = 1;

    if (a->inpd[index] == val) return;

    logger("ls173_update_d%d [%s] *valptr:%d val:%d TS:%d",index,a->name,*valptr,val,timestamp);

    a->inpd[index] = val;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
ls173 *ls173_create(char *name){

    ls173 *b = malloc(sizeof(ls173));

    if (b == NULL)
        return NULL;

    int i;

    b->clk = b->clk_o = 1;
    b->in_m = b->in_n = 1;
    b->in_g1 = b->in_g2 = 1;

    b->in_clr_rootptr = b->in_m_rootptr = b->in_n_rootptr = b->in_g1_rootptr = b->in_g2_rootptr = b->clk_rootptr = NULL;

    for (i = 0; i < NUM_ELEM_LS173; i++){

        b->inpd_rootptr[i] = NULL;
        b->outq[i] = b->outq_o[i] = 2;
        b->latd[i] = 0;
        b->outq_event_handler_root[i] = NULL;
    }

    if (name)
        strncpy(b->name,name,sizeof(b->name));
    else
        b->name[0] = 0;

    b->destroy = (void*)ls173_destroy;

    return b;
}

////////////////////////////////////////////////////////////////////////////////
void ls173_destroy (ls173 **dest){

    if (dest == NULL) return;
    ls173 *b = *dest;
    if (b == NULL) return;

    vallist_destroy(&b->in_clr_rootptr);
    vallist_destroy(&b->in_m_rootptr);
    vallist_destroy(&b->in_n_rootptr);
    vallist_destroy(&b->in_g1_rootptr);
    vallist_destroy(&b->in_g2_rootptr);
    vallist_destroy(&b->clk_rootptr);

    int i;
    for (i = 0; i < NUM_ELEM_LS173; i++){

        vallist_destroy(&b->inpd_rootptr[i]);
        ehandler_destroy(&b->outq_event_handler_root[i]);
    }

    free(b);
    *dest = NULL;
}

////////////////////////////////////////////////////////////////////////////////
static void ls173_connect_q(ls173 *source, void *dest, event_function_t dest_event_handler, int index){

    new_ehandler(&source->outq_event_handler_root[index], dest, dest_event_handler);
    dest_event_handler(dest,&source->outq[index],0);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_connect_1q(ls173 *source, void *dest, event_function_t dest_event_handler){

    ls173_connect_q(source, dest, dest_event_handler, 0);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_connect_2q(ls173 *source, void *dest, event_function_t dest_event_handler){

    ls173_connect_q(source, dest, dest_event_handler, 1);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_connect_3q(ls173 *source, void *dest, event_function_t dest_event_handler){

    ls173_connect_q(source, dest, dest_event_handler, 2);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_connect_4q(ls173 *source, void *dest, event_function_t dest_event_handler){

    ls173_connect_q(source, dest, dest_event_handler, 3);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_1d(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls173_update_d(dest, valptr, timestamp, 0);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_2d(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls173_update_d(dest, valptr, timestamp, 1);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_3d(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls173_update_d(dest, valptr, timestamp, 2);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_4d(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    ls173_update_d(dest, valptr, timestamp, 3);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_clk(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    bitvalue_t val = update_val_multi(&dest->clk_rootptr, valptr);

    if (val > 1) val = 1;

    if (dest->clk == val) return;

    logger("ls173_in_clk [%s] valptr:%p *valptr:%d val:%d TS:%d",dest->name,valptr,*valptr,val,timestamp);

    dest->clk = val;
    ls173_update(dest, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_clr(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    bitvalue_t val = update_val_multi(&dest->in_clr_rootptr, valptr);

    if (val > 1) val = 1;

    if (dest->in_clr == val) return;

    logger("ls173_in_clr [%s] *valptr:%d val:%d TS:%d",dest->name,*valptr,val,timestamp);

    dest->in_clr = val;
    ls173_update(dest, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_m(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    bitvalue_t val = update_val_multi(&dest->in_m_rootptr, valptr);

    if (val > 1) val = 1;

    if (dest->in_m == val) return;

    logger("ls173_in_m [%s] *valptr:%d val:%d TS:%d",dest->name,*valptr,val,timestamp);

    dest->in_m = val;
    ls173_update(dest, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_n(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    bitvalue_t val = update_val_multi(&dest->in_n_rootptr, valptr);

    if (val > 1) val = 1;

    if (dest->in_n == val) return;

    logger("ls173_in_n [%s] *valptr:%d val:%d TS:%d",dest->name,*valptr,val,timestamp);

    dest->in_n = val;
    ls173_update(dest, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_g1(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    bitvalue_t val = update_val_multi(&dest->in_g1_rootptr, valptr);

    if (val > 1) val = 1;

    if (dest->in_g1 == val) return;

    logger("ls173_in_g1 [%s] *valptr:%d val:%d TS:%d",dest->name,*valptr,val,timestamp);

    dest->in_g1 = val;
    ls173_update(dest, timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls173_in_g2(ls173 *dest, bitvalue_t *valptr, timevalue_t timestamp){

    bitvalue_t val = update_val_multi(&dest->in_g2_rootptr, valptr);

    if (val > 1) val = 1;

    if (dest->in_g2 == val) return;

    logger("ls173_in_g2 [%s] *valptr:%d val:%d TS:%d",dest->name,*valptr,val,timestamp);

    dest->in_g2 = val;
    ls173_update(dest, timestamp);
}

