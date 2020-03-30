/*
 * indicator.h
 *
 *  Created on: 16 de jul de 2019
 *      Author: milton
 */

#ifndef INDICATOR_H_
#define INDICATOR_H_


typedef struct  {

    char *name;
    int value;
} indicator;

indicator *indicator_create(char *name);

void indicator_in_d0(indicator *dest, int val, int timestamp);
void indicator_in_d1(indicator *dest, int val, int timestamp);
void indicator_in_d2(indicator *dest, int val, int timestamp);
void indicator_in_d3(indicator *dest, int val, int timestamp);

void indicator_print(indicator *o);
void indicator_println(indicator *o);
void indicator_prints(indicator *o);

#endif /* INDICATOR_H_ */