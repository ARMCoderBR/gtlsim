////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

#ifndef AT28C16_H_
#define AT28C16_H_

#include <stdint.h>
#include "update.h"

#define NUM_BITS_28C16 8
#define NUM_WORDS_28C16 2048

typedef struct {

    part_destroy_function_t destroy;
    bitvalue_t inpd[NUM_BITS_28C16];
	vallist *inpd_rootptr[NUM_BITS_28C16];

	bitvalue_t in_addr[11];
    vallist *in_addr_rootptr[11];
    bitvalue_t in_cs, in_we, in_oe;
    bitvalue_t in_we_old, in_cs_old;
    vallist *in_cs_rootptr, *in_we_rootptr, *in_oe_rootptr;
    uint16_t current_addr;

	uint8_t eep_byte[NUM_WORDS_28C16];
	bitvalue_t outq[NUM_BITS_28C16];
	bitvalue_t outq_o[NUM_BITS_28C16];
    ehandler *outq_event_handler_root[NUM_BITS_28C16];
    char name[32];
    event_context_t *ec;
} at28c16;

at28c16 *at28c16_create(event_context_t *ec, char *name, unsigned char *template);
void at28c16_destroy(at28c16 **dest);

void at28c16_connect_o0(at28c16 *source, void *dest, event_function_t dest_event_handler);
void at28c16_connect_o1(at28c16 *source, void *dest, event_function_t dest_event_handler);
void at28c16_connect_o2(at28c16 *source, void *dest, event_function_t dest_event_handler);
void at28c16_connect_o3(at28c16 *source, void *dest, event_function_t dest_event_handler);
void at28c16_connect_o4(at28c16 *source, void *dest, event_function_t dest_event_handler);
void at28c16_connect_o5(at28c16 *source, void *dest, event_function_t dest_event_handler);
void at28c16_connect_o6(at28c16 *source, void *dest, event_function_t dest_event_handler);
void at28c16_connect_o7(at28c16 *source, void *dest, event_function_t dest_event_handler);

void at28c16_in_i0(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_i1(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_i2(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_i3(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_i4(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_i5(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_i6(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_i7(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);

void at28c16_in_a0(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a1(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a2(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a3(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a4(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a5(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a6(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a7(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a8(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a9(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_a10(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);

void at28c16_in_cs(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_we(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);
void at28c16_in_oe(at28c16 *dest, bitvalue_t *valptr, timevalue_t timestamp);

#endif /* at28c16_H_ */
