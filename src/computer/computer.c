////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#include "reg_8bit.h"
#include "alu_8bit.h"
#include "ram_8bit.h"
#include "progctr.h"
#include "bitconst.h"
#include "ctrunit.h"
#include "reg_out.h"
#include "computer.h"
#include "board.h"
#include "indicator.h"
#include "clkgen.h"
#include "pulldown.h"

////////////////////////////////////////////////////////////////////////////////
void computer_sim(){

    board_initialize();

    char name[64];

    sprintf(name,"BEN EATER'S COMPUTER SIM BY ARMCODER V%d.%d.%d",SW_VERSION, SW_REVISION, SW_MINOR);
    board_object *mainboard = board_create(0,0,0,name);

    event_context_t ec;

    event_init(&ec);

    logger_init(&ec);

    clkgen *mainclk = clkgen_create(&ec, "",1000000);

    board_set_clk(mainclk);

    //////// REGS //////////////////////////////////////////////////////////////

    reg_8bit *regA = reg_8bit_create(&ec, REG8BIT_NORMAL, "regA");
    reg_8bit *regB = reg_8bit_create(&ec, REG8BIT_NORMAL, "regB");
    reg_8bit *regIN = reg_8bit_create(&ec, REG8BIT_IR, "regIN");

    if ((!regA)||(!regB)||(!regIN)){

        perror("reg_x create");
        exit(0);
    }

    board_object *regA_board = reg_8bit_board_create(regA, KEY_F(1), "Reg A"); // Requer NCURSES
    board_object *regB_board = reg_8bit_board_create(regB, KEY_F(2), "Reg B"); // Requer NCURSES
    board_object *regIN_board = reg_8bit_board_create(regIN, KEY_F(3), "Reg IN"); // Requer NCURSES

    if ((!regA_board)||(!regB_board)||(!regIN_board)){

        perror("regx_board create");
        exit(0);
    }

    clkgen_connect_out(mainclk, regA, (void*)&reg_8bit_in_clock);
    clkgen_connect_out(mainclk, regB, (void*)&reg_8bit_in_clock);
    clkgen_connect_out(mainclk, regIN, (void*)&reg_8bit_in_clock);

    board_add_board(mainboard,regA_board,1,1);
    board_add_board(mainboard,regB_board,1,9);
    board_add_board(mainboard,regIN_board,1,13);

    //////// ALU ///////////////////////////////////////////////////////////////

    alu_8bit *alu = alu_8bit_create(&ec, "ALU");
    board_object *alu_board = alu_8bit_board_create(alu, KEY_F(4), "ALU"); // Requer NCURSES
    board_add_board(mainboard,alu_board,1,5);

    ls173_connect_1q(regA->ls173_lo, alu, alu_8bit_in_dataAN[0]);
    ls173_connect_2q(regA->ls173_lo, alu, alu_8bit_in_dataAN[1]);
    ls173_connect_3q(regA->ls173_lo, alu, alu_8bit_in_dataAN[2]);
    ls173_connect_4q(regA->ls173_lo, alu, alu_8bit_in_dataAN[3]);
    ls173_connect_1q(regA->ls173_hi, alu, alu_8bit_in_dataAN[4]);
    ls173_connect_2q(regA->ls173_hi, alu, alu_8bit_in_dataAN[5]);
    ls173_connect_3q(regA->ls173_hi, alu, alu_8bit_in_dataAN[6]);
    ls173_connect_4q(regA->ls173_hi, alu, alu_8bit_in_dataAN[7]);

    ls173_connect_1q(regB->ls173_lo, alu, alu_8bit_in_dataBN[0]);
    ls173_connect_2q(regB->ls173_lo, alu, alu_8bit_in_dataBN[1]);
    ls173_connect_3q(regB->ls173_lo, alu, alu_8bit_in_dataBN[2]);
    ls173_connect_4q(regB->ls173_lo, alu, alu_8bit_in_dataBN[3]);
    ls173_connect_1q(regB->ls173_hi, alu, alu_8bit_in_dataBN[4]);
    ls173_connect_2q(regB->ls173_hi, alu, alu_8bit_in_dataBN[5]);
    ls173_connect_3q(regB->ls173_hi, alu, alu_8bit_in_dataBN[6]);
    ls173_connect_4q(regB->ls173_hi, alu, alu_8bit_in_dataBN[7]);

    //////// RAM ///////////////////////////////////////////////////////////////

    ram_8bit *ram = ram_8bit_create(&ec, "RAM");
    board_object *ram_board = ram_8bit_board_create(ram, KEY_F(5), "RAM"); // Requer NCURSES
    board_add_board(mainboard,ram_board,42,1);
    clkgen_connect_out(mainclk, ram, (void*)&ram_8bit_in_clk);

    //////// PROGRAM COUNTER ///////////////////////////////////////////////////

    progctr *pctr = progctr_create(&ec, "PC");
    board_object *pctr_board = progctr_board_create(pctr, KEY_F(6), "PC");
    board_add_board(mainboard,pctr_board,67,21);
    clkgen_connect_out(mainclk, pctr, (void*)&progctr_in_clock);

    //////// REG OUT ///////////////////////////////////////////////////////////

    reg_out *regout = reg_out_create(&ec, "RO");
    board_object *regout_board = reg_out_board_create(regout, KEY_F(6), "RO");
    board_add_board(mainboard,regout_board,60,14);
    clkgen_connect_out(mainclk, regout, (void*)&reg_out_in_clock);

    //////// BUS ///////////////////////////////////////////////////////////////

    indicator *ledbus[8];
    board_object *bus_board = board_create(40, 4, 0, "BUS");

    int i;
    for (i = 0; i < 8; i++){

        ledbus[i] = indicator_create("Data");

        char dname[10];
        sprintf(dname,"D%d",i);

        /// BUS PULL DOWNS
        pulldown_connect(ledbus[i], (void*)indicator_in_d0);
        pulldown_connect(regA, reg_8bit_in_dataN[i]);
        pulldown_connect(regB, reg_8bit_in_dataN[i]);
        pulldown_connect(regIN, reg_8bit_in_dataN[i]);
        pulldown_connect(ram, ram_8bit_in_dataN[i]);
        pulldown_connect(regout, reg_out_in_dataN[i]);

        /// REGA OUTPUT
		reg_8bit_connect_bit_out (regA, i, ledbus[i], (void*)indicator_in_d0);
        reg_8bit_connect_bit_out (regA, i, regB, reg_8bit_in_dataN[i]);
        reg_8bit_connect_bit_out (regA, i, regIN, reg_8bit_in_dataN[i]);
        reg_8bit_connect_bit_out (regA, i, ram, ram_8bit_in_dataN[i]);
        reg_8bit_connect_bit_out (regA, i, regout, reg_out_in_dataN[i]);

        /// REGB OUTPUT
        reg_8bit_connect_bit_out (regB, i, ledbus[i], (void*)indicator_in_d0);
        reg_8bit_connect_bit_out (regB, i, regA, reg_8bit_in_dataN[i]);
        reg_8bit_connect_bit_out (regB, i, regIN, reg_8bit_in_dataN[i]);
        reg_8bit_connect_bit_out (regB, i, ram, ram_8bit_in_dataN[i]);
        reg_8bit_connect_bit_out (regB, i, regout, reg_out_in_dataN[i]);

        /// REGIN OUTPUT
        if (i < 4){

            reg_8bit_connect_bit_out (regIN, i, ledbus[i], (void*)indicator_in_d0);
            reg_8bit_connect_bit_out (regIN, i, regA, reg_8bit_in_dataN[i]);
            reg_8bit_connect_bit_out (regIN, i, regB, reg_8bit_in_dataN[i]);
            reg_8bit_connect_bit_out (regIN, i, ram, ram_8bit_in_dataN[i]);
            reg_8bit_connect_bit_out (regIN, i, regout, reg_out_in_dataN[i]);
        }

        /// ALU OUTPUT
        alu_8bit_connect_bit_out (alu, i, ledbus[i], (void*)indicator_in_d0);
        alu_8bit_connect_bit_out (alu, i, regA, reg_8bit_in_dataN[i]);
        alu_8bit_connect_bit_out (alu, i, regB, reg_8bit_in_dataN[i]);
        alu_8bit_connect_bit_out (alu, i, regIN, reg_8bit_in_dataN[i]);
        alu_8bit_connect_bit_out (alu, i, ram, ram_8bit_in_dataN[i]);
        alu_8bit_connect_bit_out (alu, i, regout, reg_out_in_dataN[i]);

        /// RAM OUTPUT
        ram_8bit_connect_bit_out(ram, i, ledbus[i], (void*)indicator_in_d0);
        ram_8bit_connect_bit_out (ram, i, regA, reg_8bit_in_dataN[i]);
        ram_8bit_connect_bit_out (ram, i, regB, reg_8bit_in_dataN[i]);
        ram_8bit_connect_bit_out (ram, i, regIN, reg_8bit_in_dataN[i]);
        ram_8bit_connect_bit_out (ram, i, regout, reg_out_in_dataN[i]);

        int j = 7-i;

        board_add_led(bus_board,ledbus[i],1+4*j, 1, dname, LED_RED);
    }

    board_add_board(mainboard,bus_board,1,17);

    for (i = 0; i < 4; i++){

        reg_8bit_connect_bit_out (regA, i, pctr, progctr_in_dataN[i]);
        reg_8bit_connect_bit_out (regB, i, pctr, progctr_in_dataN[i]);
        reg_8bit_connect_bit_out (regIN, i, pctr, progctr_in_dataN[i]);
        alu_8bit_connect_bit_out (alu, i, pctr, progctr_in_dataN[i]);
        ram_8bit_connect_bit_out (ram, i, pctr, progctr_in_dataN[i]);

        progctr_connect_bit_out (pctr, i, ledbus[i], (void*)indicator_in_d0);
        progctr_connect_bit_out (pctr, i, regA, reg_8bit_in_dataN[i]);
        progctr_connect_bit_out (pctr, i, regB, reg_8bit_in_dataN[i]);
        progctr_connect_bit_out (pctr, i, regIN, reg_8bit_in_dataN[i]);
        progctr_connect_bit_out (pctr, i, ram, ram_8bit_in_dataN[i]);
        progctr_connect_bit_out (pctr, i, regout, reg_out_in_dataN[i]);
    }

    //////// CONTROL UNIT //////////////////////////////////////////////////////

    ctrunit *ctru =ctrunit_create(&ec, "CONTROL UNIT");
    board_object *ctru_board = ctrunit_board_create(ctru, '*', "CONTROL UNIT");
    board_add_board(mainboard,ctru_board,1,21);

    board_object *ctru_flags_board = ctrunit_board_flags_create(ctru, '*', "FLAGS");
    board_add_board(mainboard,ctru_flags_board,44,15);

    clkgen_connect_out(mainclk, ctru, (void*)&ctrunit_in_clk);
    clkgen_connect_outn(mainclk, ctru, (void*)&ctrunit_in_clkn);

    alu_8bit_connect_zero_out (alu, ctru, (void*)&ctrunit_in_zero);
    alu_8bit_connect_carry_out (alu, ctru, (void*)&ctrunit_in_carry);

    //// Reset controls
    reg_8bit_in_clear_from((void*)&ctrunit_connect_out_reset,ctru,regA);
    reg_8bit_in_clear_from((void*)&ctrunit_connect_out_reset,ctru,regB);
    reg_8bit_in_clear_from((void*)&ctrunit_connect_out_reset,ctru,regIN);
    ctrunit_connect_out_reset(ctru, ram, (void*)&ram_8bit_in_rst);
    ctrunit_connect_out_nreset(ctru, pctr, (void*)&progctr_in_clear);
    reg_out_in_clear_from((void*)&ctrunit_connect_out_reset,ctru,regout);

    //// Instruction Register Bits -> Control Unit
    reg_8bit_connect_bit_out (regIN, 4, ctru, (void*)&ctrunit_in_instr0);
    reg_8bit_connect_bit_out (regIN, 5, ctru, (void*)&ctrunit_in_instr1);
    reg_8bit_connect_bit_out (regIN, 6, ctru, (void*)&ctrunit_in_instr2);
    reg_8bit_connect_bit_out (regIN, 7, ctru, (void*)&ctrunit_in_instr3);

    //// Controls to the registers
    ctrunit_connect_out_hlt(ctru, mainclk, (void*)&clkgen_in_halt);
    ctrunit_connect_out_mi(ctru, ram, (void*)&ram_8bit_in_waddr);
    ctrunit_connect_out_ri(ctru, ram, (void*)&ram_8bit_in_wdata);
    ctrunit_connect_out_ro(ctru, ram, (void*)&ram_8bit_in_oe);
    reg_8bit_in_enable_from((void*)&ctrunit_connect_out_io,ctru,regIN);
    reg_8bit_in_load_from((void*)&ctrunit_connect_out_ii,ctru,regIN);
    reg_8bit_in_load_from((void*)&ctrunit_connect_out_ai,ctru,regA);
    reg_8bit_in_enable_from((void*)&ctrunit_connect_out_ao,ctru,regA);
    alu_8bit_in_enable_from((void*)&ctrunit_connect_out_so, ctru, alu);
    alu_8bit_in_sub_from((void*)&ctrunit_connect_out_su, ctru, alu);
    reg_8bit_in_load_from((void*)&ctrunit_connect_out_bi,ctru,regB);
    reg_out_in_load_from((void*)&ctrunit_connect_out_oi,ctru,regout);
    ctrunit_connect_out_ce(ctru, pctr, (void*)&progctr_in_ct_enable);
    ctrunit_connect_out_co(ctru, pctr, (void*)&progctr_in_oenable);
    ctrunit_connect_out_j(ctru, pctr, (void*)&progctr_in_load);

    ////////////////

    board_run(&ec, mainboard);

    logger_end(&ec);

    ////////////////

    DESTROY(regA);
    DESTROY(regB);
    DESTROY(regIN);

    for (i = 0; i < 8; i++){

        DESTROY(ledbus[i]);
    }

    DESTROY(alu);
    DESTROY(ram);
    DESTROY(pctr);
    DESTROY(regout);

    DESTROY(mainboard);
}
