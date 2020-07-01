/*
 * exmachina.h
 *
 *  Created on: 23 de mai de 2020
 *      Author: milton
 */

#ifndef SRC_COMPUTER_EXMACHINA_H_
#define SRC_COMPUTER_EXMACHINA_H_

void *exmachina_thread(void *args);

#define RAMADDR_KEY_3 1003
#define RAMADDR_KEY_2 1002
#define RAMADDR_KEY_1 1001
#define RAMADDR_KEY_0 1000

#define RAMDATA_KEY_7 1017
#define RAMDATA_KEY_6 1016
#define RAMDATA_KEY_5 1015
#define RAMDATA_KEY_4 1014
#define RAMDATA_KEY_3 1013
#define RAMDATA_KEY_2 1012
#define RAMDATA_KEY_1 1011
#define RAMDATA_KEY_0 1010

#define RAM_PROG_RUN  1020
#define RAM_WRITE     1021

#define CTRU_RESET    1100

#define KEY_CLK_PAUSE 1200
#define KEY_CLK_SLOW  1201
#define KEY_CLK_FAST  1202


#endif /* SRC_COMPUTER_EXMACHINA_H_ */
