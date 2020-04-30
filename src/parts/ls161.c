/*
 * ls161.c
 *
 *  Created on: 29 de abr de 2020
 *      Author: milton
 */

#include <malloc.h>

#include "update.h"
#include "ls161.h"

// UP/DOWN BINARY COUNTER

////////////////////////////////////////////////////////////////////////////////
static int ls161_update(ls161 *a){

    if (!a->clear){

        a->qa =
        a->qb =
        a->qc =
        a->qd = 0;
        goto end_ls161;
    }

    if ((!a->clk_o) && (a->clk)){

        if (!a->load){

            a->qa = a->da;
            a->qb = a->db;
            a->qc = a->dc;
            a->qd = a->dd;
            goto end_ls161;
        }

        if ((a->enp) && (a->ent)){

            a->qa++;
            //UP
            if (a->qa == 2) {
                a->qa = 0;
                a->qb++;
                if (a->qb == 2){
                    a->qb = 0;
                    a->qc++;
                    if (a->qc == 2){
                        a->qc = 0;
                        a->qd++;
                        if (a->qd == 2)
                            a->qd = 0;
                    }
                }
            }
        }
    }

    a->ripclk = 0;
    if (a->qa && a->qb && a->qc && a->qd && a->ent)
        a->ripclk = 1;

    int mask = 0;

end_ls161:

    a->clk_o = a->clk;

    if (a->qa_o != a->qa){

        a->qa_o = a->qa;
        mask |= 0x01;
    }

    if (a->qb_o != a->qb){

        a->qb_o = a->qb;
        mask |= 0x02;
    }

    if (a->qc_o != a->qc){

        a->qc_o = a->qc;
        mask |= 0x04;
    }

    if (a->qd_o != a->qd){

        a->qd_o = a->qd;
        mask |= 0x08;
    }

    if (a->ripclk_o != a->ripclk){

        a->ripclk_o = a->ripclk;
        mask |= 0x10;
    }

    return mask;
}

////////////////////////////////////////////////////////////////////////////////
static void ls161_up(ls161 *a, int timestamp){

    event e;
    e.timestamp = timestamp+1;

    int mask = ls161_update(a);
    if (mask & 0x01){

        e.event_handler_root = a->qa_event_handler_root;
        e.valueptr = &a->qa;
        event_insert(&e);
    }

    if (mask & 0x02){

        e.event_handler_root = a->qb_event_handler_root;
        e.valueptr = &a->qb;
        event_insert(&e);
    }

    if (mask & 0x04){

        e.event_handler_root = a->qc_event_handler_root;
        e.valueptr = &a->qc;
        event_insert(&e);
    }

    if (mask & 0x08){

        e.event_handler_root = a->qd_event_handler_root;
        e.valueptr = &a->qd;
        event_insert(&e);
    }

    if (mask & 0x10){

        e.event_handler_root = a->ripclk_event_handler_root;
        e.valueptr = &a->ripclk;
        event_insert(&e);
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
ls161 *ls161_create(){

    ls161 *b = malloc(sizeof(ls161));

    if (b == NULL){
        return NULL;
    }

    b->clk_o = b->da = b->db = b->dc = b->dd = b->load = b->enp = b->ent = b->clk = b->clear = 2;
	b->da_rootptr = b->db_rootptr = b->dc_rootptr = b->dd_rootptr = b->load_rootptr = b->enp_rootptr = b->ent_rootptr = b->clk_rootptr = b->clear_rootptr = NULL;
    b->qa_o = b->qb_o = b->qc_o = b->qd_o = b->ripclk_o = 0;
    b->qa_event_handler_root =
    b->qb_event_handler_root =
    b->qc_event_handler_root =
    b->qd_event_handler_root =
    b->ripclk_event_handler_root = NULL;

    return b;
}

////////////////////////////////////////////////////////////////////////////////
void ls161_connect_qa(ls161 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    new_ehandler(&source->qa_event_handler_root, dest, dest_event_handler);
    dest_event_handler(dest,&source->qa,0);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_connect_qb(ls161 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    new_ehandler(&source->qb_event_handler_root, dest, dest_event_handler);
    dest_event_handler(dest,&source->qb,0);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_connect_qc(ls161 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    new_ehandler(&source->qc_event_handler_root, dest, dest_event_handler);
    dest_event_handler(dest,&source->qc,0);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_connect_qd(ls161 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    new_ehandler(&source->qd_event_handler_root, dest, dest_event_handler);
    dest_event_handler(dest,&source->qd,0);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_connect_ripclk(ls161 *source, void *dest, void (*dest_event_handler)(void *dest, int *valptr, int timestamp)){

    new_ehandler(&source->ripclk_event_handler_root, dest, dest_event_handler);
    dest_event_handler(dest,&source->ripclk,0);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_da(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->da_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->da) return;
    dest->da = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_db(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->db_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->db) return;
    dest->db = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_dc(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->dc_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->dc) return;
    dest->dc = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_dd(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->dd_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->dd) return;
    dest->dd = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_load(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->load_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->load) return;
    dest->load = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_enp(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->enp_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->enp) return;
    dest->enp = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_ent(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->ent_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->ent) return;
    dest->ent = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_clk(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->clk_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->clk) return;
    dest->clk = val;
    ls161_up(dest,timestamp);
}

////////////////////////////////////////////////////////////////////////////////
void ls161_in_clear(ls161 *dest, int *valptr, int timestamp){

    int val = update_val_multi(&dest->clear_rootptr, valptr);

    if (val > 1) val = 1;

    if (val == dest->clear) return;
    dest->clear = val;
    ls161_up(dest,timestamp);
}
