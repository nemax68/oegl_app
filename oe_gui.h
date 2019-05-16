/**
 * @file openeyes.h
 *
 */

#ifndef OPENEYES_H
#define OPENEYES_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "../lv_ex_conf.h"
#include "../lvgl/lvgl.h"
#include "oe_json.h"

/*********************
 *      DEFINES
 *********************/

#define API_VERSION		"1.0"

#define RGB_2_16bit(x) LV_COLOR_MAKE( (x>>16)&0xFF, (x>>8)&0xFF, (x)&0xFF)


/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void init_gui_evdev(void);
void init_gui(void);
uint32_t color_conv(char *);
void set_text_style(lv_style_t *, struct json_decoder *);
void set_body_style(lv_style_t *, struct json_decoder *);


int init_image(void);
int image_add(struct json_decoder *);
int image_del(struct json_decoder *);
int clear_screen(struct json_decoder *);

int init_keypad(void);
int keypad_add(struct json_decoder *);
int keypad_del(struct json_decoder *);
int keypad_del_all(void);

int init_button(void);
int button_add(struct json_decoder *);
int button_del(struct json_decoder *);
int button_del_all(void);

int init_text(void);
int text_add(struct json_decoder *);
int text_box_add(struct json_decoder *);
int text_del(struct json_decoder *);
int text_del_all(void);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OPENEYES_H */
