/*
 * exmachina.c
 *
 *  Created on: 23 de mai de 2020
 *      Author: milton
 */

#include <unistd.h>
#include <stdint.h>

#include "board.h"
#include "exmachina.h"

const uint8_t demoprog[] = {

        0x00,     /*OUT*/         0xE0,
        0x01,     /*ADD 15*/      0x2F,
        0x02,     /*JC 4*/        0x74,
        0x03,     /*JMP 0*/       0x60,
        0x04,     /*SUB 15*/      0x3F,
        0x05,     /*OUT*/         0xE0,
        0x06,     /*JZ 0*/        0x80,
        0x07,     /*JMP 4*/       0x64,
        0x0F,     /*1*/           0x01,
};

const int LINES_PROG = sizeof(demoprog)/2;

////////////////////////////////////////////////////////////////////////////////
void write_key(project_ctx_t *pctx, int key){

    pctx->remote_key = key;
    usleep(100000);
}

////////////////////////////////////////////////////////////////////////////////
void *exmachina_thread(void *args){

    project_ctx_t *pctx = args;
    usleep(1000000);

    uint8_t addr = 0;
    uint8_t data = 0;

    int i;

    for (i = 0; i < LINES_PROG; i++){

        uint8_t newaddr = demoprog[2*i];
        uint8_t newdata = demoprog[2*i+1];

        uint8_t deltaaddr = addr ^ newaddr;

        if (deltaaddr & 0x01){
            write_key(pctx, RAMADDR_KEY_0);
        }
        if (deltaaddr & 0x02){
            write_key(pctx, RAMADDR_KEY_1);
        }
        if (deltaaddr & 0x04){
            write_key(pctx, RAMADDR_KEY_2);
        }
        if (deltaaddr & 0x08){
            write_key(pctx, RAMADDR_KEY_3);
        }

        uint8_t deltadata = data ^ newdata;

        int j;
        for (j = 0; j < 8; j++){

            if (deltadata & (1 << j)){

                int key = RAMDATA_KEY_0 + j;
                write_key(pctx, key);
            }
        }

        addr = newaddr;
        data = newdata;

        write_key(pctx, RAM_WRITE);
        write_key(pctx, RAM_WRITE);
    }

    write_key(pctx, RAM_PROG_RUN);
    write_key(pctx, CTRU_RESET);
    write_key(pctx, CTRU_RESET);

    for (i = 0; i < 9; i++)
        write_key(pctx, KEY_CLK_FAST);

    return NULL;
}
