/*
 * update.c
 *
 *  Created on: 16 de jul de 2019
 *      Author: milton
 */

#include <malloc.h>
#include <string.h>

#include "update.h"

#define EVQUEUELEN 200

event evqueue[EVQUEUELEN];
int evins = 0;
int evget = 0;

void event_insert(event *e);

////////////////////////////////////////////////////////////////////////////////
void event_process(){

#ifdef DEBUG
    printf("event_process BEGIN evins:%d evget:%d\n",evins,evget);
#endif

    ehandler * ehandlerptr;
    while (evins != evget){

        ehandlerptr = evqueue[evget].event_handler_root;
        int value = evqueue[evget].value;
        int timestamp = 1 + evqueue[evget].timestamp;

        while (ehandlerptr != NULL){

            ehandlerptr->objdest_event_handler(ehandlerptr->objdest,value,timestamp);
            ehandlerptr = ehandlerptr->next;
        }

        if (evins == evget) break;

        evget++;
        evget %= EVQUEUELEN;
    }
#ifdef DEBUG
    printf("event_process END\n");
#endif
}

////////////////////////////////////////////////////////////////////////////////
void event_insert(event *e){

#ifdef DEBUG
    printf("event_insert BEGIN\n");
#endif

    memcpy(&evqueue[evins],e,sizeof(event));
    evins++;
    evins %= EVQUEUELEN;

    event_process();

#ifdef DEBUG
    printf("event_insert END evins:%d evget:%d\n",evins,evget);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void new_ehandler(ehandler **ehptr, void *objdest, void (*objdest_event_handler)(void *objdest, int val, int timestamp)){

    ehandler *ept = malloc(sizeof(ehandler));
    ept->objdest_event_handler = objdest_event_handler;
    ept->objdest = objdest;
    ept->next = NULL;

    if (*ehptr == NULL){

        *ehptr = ept;
#ifdef DEBUG
        printf("new_ehandler(1)\n");
#endif
        return;
    }

    ehandler *eptscan = *ehptr;

    while (eptscan->next != NULL)
        eptscan = eptscan->next;

#ifdef DEBUG
    printf("new_ehandler(2)\n");
#endif

    eptscan->next = ept;
}