/*
 * main.c
 *
 *  Created on: 16 de jul de 2019
 *      Author: milton
 */


#include <stdio.h>
#include <ncurses.h>

#include "board.h"
#include "bitswitch.h"
#include "indicator.h"
#include "ls08.h"
#include "update.h"
#include "ls191.h"

////////////////////////////////////////////////////////////////////////////////
void gatetest(){

    bitswitch *s1,*s2,*s3;
    ls08 *ands;
    indicator *os1,*os2,*os3,*oand1,*oand2;

    s1 = bitswitch_create();
    s2 = bitswitch_create();
    s3 = bitswitch_create();
    ands = ls08_create();
    os1 = indicator_create("S1");
    os2 = indicator_create("S2");
    os3 = indicator_create("S3");
    oand1 = indicator_create("AND1");
    oand2 = indicator_create("AND2");

    bitswitch_connect_out(s1, ands, (void*)&ls08_in_a1);
    bitswitch_connect_out(s2, ands, (void*)&ls08_in_b1);
    bitswitch_connect_out(s3, ands, (void*)&ls08_in_a2);

    ls08_connect_y1(ands, ands, (void*)&ls08_in_b2);
    ls08_connect_y1(ands, oand1, (void*)&indicator_in_d0);

    bitswitch_connect_out(s1, os1, (void*)&indicator_in_d0);
    bitswitch_connect_out(s2, os2, (void*)&indicator_in_d0);
    bitswitch_connect_out(s3, os3, (void*)&indicator_in_d0);
    ls08_connect_y2(ands, oand2, (void*)&indicator_in_d0);

    printf("===================== 0 ==== 0\n");

    bitswitch_setval(s1, 0);
    bitswitch_setval(s2, 0);

    indicator_prints(os1);
    indicator_prints(os2);
    indicator_prints(os3);
    indicator_prints(oand1);
    indicator_println(oand2);

    printf("===================== 1 ==== 0\n");

    bitswitch_setval(s1, 1);
    bitswitch_setval(s2, 0);

    indicator_prints(os1);
    indicator_prints(os2);
    indicator_prints(os3);
    indicator_prints(oand1);
    indicator_println(oand2);

    printf("===================== 0 ==== 1\n");

    bitswitch_setval(s1, 0);
    bitswitch_setval(s2, 1);

    indicator_prints(os1);
    indicator_prints(os2);
    indicator_prints(os3);
    indicator_prints(oand1);
    indicator_println(oand2);

    printf("===================== 1 ==== 1\n");

    bitswitch_setval(s1, 1);
    bitswitch_setval(s2, 1);

    indicator_prints(os1);
    indicator_prints(os2);
    indicator_prints(os3);
    indicator_prints(oand1);
    indicator_println(oand2);

    printf("===================== 1 ==== 1 and 1\n");

    bitswitch_setval(s3, 1);

    indicator_prints(os1);
    indicator_prints(os2);
    indicator_prints(os3);
    indicator_prints(oand1);
    indicator_println(oand2);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[]){

    ls191 *ctr1 = ls191_create();
    indicator *oqa1 = indicator_create(NULL);
    indicator *oqb1 = indicator_create(NULL);
    indicator *oqc1 = indicator_create(NULL);
    indicator *oqd1 = indicator_create(NULL);
    indicator *ohex1 = indicator_create("Hex");

    indicator *omaxmin1 = indicator_create("MAXMIN");
    indicator *oripclk1 = indicator_create("RIPCLK");
    indicator *oclk = indicator_create("CLK");
    indicator *oupdown = indicator_create("-UP/+DOWN");

    bitswitch *clk = bitswitch_create();
    bitswitch *updownsel = bitswitch_create();

    bitswitch *level0 = bitswitch_create();
    bitswitch *level1 = bitswitch_create();
    bitswitch_setval(level0,0);
    bitswitch_setval(level1,1);

    bitswitch_connect_out(level1,ctr1,(void*)&ls191_in_load);

    bitswitch_connect_out(level0,ctr1,(void*)&ls191_in_enable);

    bitswitch_connect_out(updownsel,ctr1,(void*)&ls191_in_updown);
    bitswitch_connect_out(updownsel, oupdown, (void*)&indicator_in_d0);

    bitswitch_connect_out(clk, ctr1, (void*)&ls191_in_clk);

    bitswitch_connect_out(clk, oclk, (void*)&indicator_in_d0);

    ls191_connect_qa(ctr1, oqa1, (void*)&indicator_in_d0);
    ls191_connect_qb(ctr1, oqb1, (void*)&indicator_in_d0);
    ls191_connect_qc(ctr1, oqc1, (void*)&indicator_in_d0);
    ls191_connect_qd(ctr1, oqd1, (void*)&indicator_in_d0);

    ls191_connect_qa(ctr1, ohex1, (void*)&indicator_in_d0);
    ls191_connect_qb(ctr1, ohex1, (void*)&indicator_in_d1);
    ls191_connect_qc(ctr1, ohex1, (void*)&indicator_in_d2);
    ls191_connect_qd(ctr1, ohex1, (void*)&indicator_in_d3);

    ls191_connect_maxmin(ctr1, omaxmin1, (void*)&indicator_in_d0);
    ls191_connect_ripclk(ctr1, oripclk1, (void*)&indicator_in_d0);

    printf("### UP\n");
    bitswitch_setval(updownsel,0);  //UP

#if 0
    int i;

    for (i = 0; i < 16; i++){
        bitswitch_setval(clk,0);
        indicator_prints(oclk); indicator_prints(oupdown); indicator_prints(oripclk1); output_print(oqd1); output_print(oqc1); output_print(oqb1); indicator_prints(oqa1); indicator_prints(ohex1); indicator_println(omaxmin1);
        bitswitch_setval(clk,1);
        indicator_prints(oclk); indicator_prints(oupdown); indicator_prints(oripclk1); output_print(oqd1); output_print(oqc1); output_print(oqb1); indicator_prints(oqa1); indicator_prints(ohex1); indicator_println(omaxmin1);
    }

    printf("### DOWN\n");
    bitswitch_setval(updownsel,1);  //DOWN
    indicator_prints(oclk); indicator_prints(oupdown); indicator_prints(oripclk1); output_print(oqd1); output_print(oqc1); output_print(oqb1); indicator_prints(oqa1); indicator_prints(ohex1); indicator_println(omaxmin1);

    for (i = 0; i < 16; i++){
        bitswitch_setval(clk,0);
        indicator_prints(oclk); indicator_prints(oupdown); indicator_prints(oripclk1); output_print(oqd1); output_print(oqc1); output_print(oqb1); indicator_prints(oqa1); indicator_prints(ohex1); indicator_println(omaxmin1);
        bitswitch_setval(clk,1);
        indicator_prints(oclk); indicator_prints(oupdown); indicator_prints(oripclk1); output_print(oqd1); output_print(oqc1); output_print(oqb1); indicator_prints(oqa1); indicator_prints(ohex1); indicator_println(omaxmin1);
    }
#endif

    board_init(0,0);

    board_assign_clock_to_switch(clk);

    board_add_led(oclk,1,1,"CLK");

    board_add_xdigit(ohex1,10,1,"COUNT");

    board_add_led(omaxmin1,20,1,"MAXMIN");

    board_add_led(oripclk1,30,1,"RIPPLE");

    board_add_manual_switch(updownsel, 1, 4, KEY_F(1), "UP/DN");

    board_run();

    return 0;
}