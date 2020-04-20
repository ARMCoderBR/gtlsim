/*
 * update.c
 *
 *  Created on: 16 de jul de 2019
 *      Author: milton
 */

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "update.h"

//#define EVQUEUELEN 2000

//event evqueue[EVQUEUELEN];
//int evinq = 0;
//int evins = 0;
//int evget = 0;

void event_insert(event *e);

event *event_list = NULL;
event *event_last = NULL;
int scanning_timestamp = 0;
int max_timestamp = 0;

////////////////////////////////////////////////////////////////////////////////
void event_flush(){

    if (max_timestamp || event_list)
        logger("== event_flush() max_timestamp:%d",max_timestamp);

    while (event_list){

        event_last = event_list;
        event_list = event_list->next;
        free(event_last);
    }

    event_last = NULL;
    scanning_timestamp = 0;
    max_timestamp = 0;
}

////////////////////////////////////////////////////////////////////////////////
void event_process(){

#ifdef DEBUG
    printf("event_process BEGIN evins:%d evget:%d\n",evins,evget);
#endif

    if (!event_list) return;

    while (scanning_timestamp <= max_timestamp){

        event * eventptr = event_list;

        while (eventptr){

            if (eventptr->timestamp == scanning_timestamp){

                ehandler * ehandlerptr = eventptr->event_handler_root;

                //int *valueptr = eventptr->valueptr;
                //int timestamp = 1 + eventptr->timestamp;

                while (ehandlerptr != NULL){

                    ehandlerptr->objdest_event_handler(ehandlerptr->objdest,eventptr->valueptr,eventptr->timestamp);
                    ehandlerptr = ehandlerptr->next;
                }
            }

            eventptr = eventptr->next;
        }


        scanning_timestamp++;
    }

    event_flush();

#ifdef DEBUG
    printf("event_process END\n");
#endif
}

////////////////////////////////////////////////////////////////////////////////
void event_insert(event *e){

#ifdef DEBUG
    printf("event_insert BEGIN\n");
#endif

    event *newev = malloc(sizeof(event));
    if (!newev){

        perror("event_insert() could not malloc()\n");
        exit(-1);
    }

    memcpy(newev,e,sizeof(event));
    newev->next = NULL;

    if (!event_list){

        event_list = newev;
        scanning_timestamp = max_timestamp = 0;
    }else
        event_last->next = newev;

    if (newev->timestamp > max_timestamp)
        max_timestamp = newev->timestamp;

    event_last = newev;

    event_process();

#ifdef DEBUG
    printf("event_insert END evins:%d evget:%d\n",evins,evget);
#endif
}

////////////////////////////////////////////////////////////////////////////////
void new_ehandler(ehandler **ehptr, void *objdest, void (*objdest_event_handler)(void *objdest, int *valptr, int timestamp)){

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

////////////////////////////////////////////////////////////////////////////////
int update_val_multi(vallist **rootptr, int *valptr){

    vallist *entryptr = *rootptr;

    if (!entryptr){

        entryptr = malloc(sizeof(vallist));
        if (!entryptr){

            perror("update_val_multi cant malloc()\n");
            exit(-1);
        }

        entryptr->valptr = valptr;
        entryptr->next = NULL;
        *rootptr = entryptr;
        return *valptr;
    }

    int val = 2;
    int found = 0;
    vallist *lastptr = NULL;

    while (entryptr){

        if (*entryptr->valptr < val)
            val = *entryptr->valptr;

        if (entryptr->valptr == valptr)
            found = 1;

        lastptr = entryptr;
        entryptr = entryptr->next;
    }

    if (!found){

        if (*valptr < val)
            val = *valptr;

        vallist *newptr = malloc(sizeof(vallist));
        if (!newptr){
            perror ("update_val_multi() cant malloc()\n");
            exit(-1);
        }

        newptr->valptr = valptr;
        newptr->next = NULL;
        lastptr->next = newptr;
    }

    return val;
}

////////////////////////////////////////////////////////////////////////////////
FILE *logfile = NULL;

void logger_init(){

    logfile = fopen ("out.log","w");
    logger("=============================");
    logger("======= SESSION START =======");
    logger("=============================");
}

////////////////////////////////////////////////////////////////////////////////
void logger(const char *fmt, ...) {

    char buftxt[301];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buftxt, 300, fmt, args);
    va_end(args);
    fprintf(logfile,"%s\n",buftxt);
    fflush(logfile);
}

////////////////////////////////////////////////////////////////////////////////
void logger_end(){

    if (logfile) fclose(logfile);
}
