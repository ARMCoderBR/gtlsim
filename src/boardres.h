/*
 * boardres.h
 *
 *  Created on: 1 de jul de 2020
 *      Author: milton
 */

#ifndef SRC_BOARDRES_H_
#define SRC_BOARDRES_H_

#include <gtk/gtk.h>

extern GInputStream *disp7_0_s;
extern GInputStream *disp7_1_s;
extern GInputStream *disp7_2_s;
extern GInputStream *disp7_3_s;
extern GInputStream *disp7_4_s;
extern GInputStream *disp7_5_s;
extern GInputStream *disp7_6_s;
extern GInputStream *disp7_7_s;
extern GInputStream *disp7_8_s;
extern GInputStream *disp7_9_s;
extern GInputStream *disp7_blank_s;
extern GInputStream *disp7_dash_s;

extern GInputStream *led_green_off_s;
extern GInputStream *led_green_on_s;

extern GInputStream *led_blue_off_s;
extern GInputStream *led_blue_on_s;

extern GInputStream *led_red_off_s;
extern GInputStream *led_red_on_s;

extern GInputStream *led_white_off_s;
extern GInputStream *led_white_on_s;

extern GInputStream *led_yellow_off_s;
extern GInputStream *led_yellow_on_s;

extern GInputStream *switch_off_s;
extern GInputStream *switch_on_s;

void boardres_init_streams(void);

#endif /* SRC_BOARDRES_H_ */
