/**
 * @file openeyes_gui.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include "lv_drivers/indev/evdev.h"
#include "oe_gui.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create some objects
 */
void init_gui_evdev(void)
{
	/* EVDEV Touchpanel Initialization */
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);     			/*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;		/*See below.*/
	indev_drv.read = evdev_read;              	/*See below.*/
	lv_indev_drv_register(&indev_drv);     		/*Register the driver in LittlevGL*/
}

void init_gui(void)
{
	struct json_decoder jd;

	init_image();
	init_button();
	init_keypad();
	init_text();

	/*Display logo full screen */
	memset(&jd,0,sizeof(jd));
	strcpy(jd.path,"/usr/share/oedd/img/logo.bmp");
	image_add(&jd);

	memset(&jd,0,sizeof(jd));
	sprintf(jd.text,"OPEN-EYES display driver Ver %s",API_VERSION);
	strcpy(jd.font.color,"#0000ff");

	jd.font.size=10;
	jd.pos.x=5;
	jd.pos.y=200;
	jd.size.x=200;
	jd.size.y=100;
	jd.border.size=2;
	jd.border.radius=3;

	text_add(&jd);

}

uint32_t color_conv(char *c)
{
	int r,g,b;
	char col[3];
	char *pt=c;

	col[0]=pt[1];
	col[1]=pt[2];
	col[2]=0;
	r=strtol(col,NULL,16);

	col[0]=pt[3];
	col[1]=pt[4];
	g=strtol(col,NULL,16);

	col[0]=pt[5];
	col[1]=pt[6];
	b=strtol(col,NULL,16);

	return( (r<<16) | (g<<8) | b );
}

void set_text_style(lv_style_t *style, struct json_decoder *jsond)
{
	int rgb;

	rgb=color_conv(jsond->font.color);
	style->text.color = RGB_2_16bit(rgb);

	switch(jsond->font.size){
		case 10: style->text.font = &lv_font_dejavu_10; break;
		case 20: style->text.font = &lv_font_dejavu_20; break;
		case 30: style->text.font = &lv_font_dejavu_30; break;
		default: style->text.font = &lv_font_dejavu_40;
	}
}

void set_body_style(lv_style_t *style, struct json_decoder *jsond)
{
	int rgb;

	rgb=color_conv(jsond->color.main);

	style->body.main_color = RGB_2_16bit(rgb);
	rgb=color_conv(jsond->color.grad);
	style->body.grad_color = RGB_2_16bit(rgb);
	style->body.radius = jsond->border.radius;
	rgb=color_conv(jsond->border.color);
	style->body.border.color = RGB_2_16bit(rgb);
	style->body.border.opa = LV_OPA_COVER;
	style->body.border.width = jsond->border.size;
}
