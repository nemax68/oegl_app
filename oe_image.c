/**
  * @file oe_image.c
  *
  * Copyright 2019 OPEN-EYES S.r.l.
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  **/


/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "oe_mqueue.h"
#include "oe_gui.h"
#include "oe_json.h"
#include "lv_drivers/indev/evdev.h"

/*********************
 *      DEFINES
 *********************/

#define MAX_IMAGE_DESCRIPTOR	8

/**********************
 *      TYPEDEFS
 **********************/

struct bmp_file {
	size_t width;
	size_t height;
	size_t size;
	uint16_t type;
	uint16_t bitcolor;
	uint8_t *img;
};

struct image_desc {
	char name[64];
	lv_img_dsc_t img_screen;
	lv_obj_t *scr_wp;
	struct bmp_file desc;
};

struct screen_desc {
	lv_style_t scr_style;
	struct image_desc *id[MAX_IMAGE_DESCRIPTOR];
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static struct screen_desc sd;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

extern void GuiLog(const char *fmt, ...);


int init_image(void)
{
	memset(&sd,0,sizeof(sd));
	return 0;
}

static void deref_image_desc(struct image_desc **ref)
{
	struct image_desc *id=*ref;

	if(id->scr_wp)
		lv_obj_del(id->scr_wp);
	if(id->desc.img)
		free(id->desc.img);
	if(id)
		free(id);
	*ref=NULL;
}

static int read_bitmap ( char const *file_name, struct bmp_file *desc  )
{
	FILE *my_file;

	uint8_t bmp_header_buffer[54];
	uint8_t bmp_line_buffer[ LV_HOR_RES * 3 ];

	uint16_t bfType;
	uint32_t bfOffBits;

	uint32_t biSize;

	int32_t biWidth;
	int32_t biHeight;
	uint16_t biBitCount;

	uint32_t y,x;

	size_t img_size;
	uint8_t red, green, blue;

	lv_color_t	*ppt;

	// check for file
	if ( ( my_file = fopen( file_name, "rb" ) ) == NULL )
		return( -ENOENT );

	// read header
	fread( &bmp_header_buffer, 1, 54, my_file );

	// check for "BM"
	bfType = bmp_header_buffer[1];
	bfType = (bfType << 8) | bmp_header_buffer[0];
	desc->type=bfType;

	if ( bfType != 0x4D42) {
		fclose( my_file );
		return( -EINVAL );
	}

	biSize = bmp_header_buffer[17];
	biSize = (biSize << 8) | bmp_header_buffer[16];
	biSize = (biSize << 8) | bmp_header_buffer[15];
	biSize = (biSize << 8) | bmp_header_buffer[14];
	desc->size=biSize;

	biWidth = bmp_header_buffer[21];
	biWidth = (biWidth << 8) | bmp_header_buffer[20];
	biWidth = (biWidth << 8) | bmp_header_buffer[19];
	biWidth = (biWidth << 8) | bmp_header_buffer[18];
	desc->width=biWidth;

	biHeight = bmp_header_buffer[25];
	biHeight = (biHeight << 8) | bmp_header_buffer[24];
	biHeight = (biHeight << 8) | bmp_header_buffer[23];
	biHeight = (biHeight << 8) | bmp_header_buffer[22];
	desc->height=biHeight;

	biBitCount = bmp_header_buffer[29];
	biBitCount = (biBitCount << 8) | bmp_header_buffer[28];
	desc->bitcolor=biBitCount;

	if ( (biWidth > LV_HOR_RES) || (biHeight > LV_VER_RES) || (biBitCount != 24) ) {
		fclose( my_file );
		return( -EINVAL );
	}

	bfOffBits = bmp_header_buffer[13];
	bfOffBits = (bfOffBits << 8) | bmp_header_buffer[12];
	bfOffBits = (bfOffBits << 8) | bmp_header_buffer[11];
	bfOffBits = (bfOffBits << 8) | bmp_header_buffer[10];

	img_size=biWidth*biHeight*sizeof(lv_color_t);//(biBitCount/8);

	desc->img=malloc(img_size);
	if (!desc->img) {
		fclose( my_file );
		return( -ENOMEM );
	}

	// point to the end of buffer
	img_size=biWidth*biHeight;

	ppt=(lv_color_t *)desc->img+img_size;

	fseek( my_file, bfOffBits, SEEK_SET );

	for (y=biHeight; y>0; y--)
	{
		fread( &bmp_line_buffer[0], biWidth*3, 1, my_file );
		for (x=biWidth; x>0; x--)
		{
			blue =  bmp_line_buffer[(x-1)*3 +0];
			green = bmp_line_buffer[(x-1)*3 +1];
			red =   bmp_line_buffer[(x-1)*3 +2];

			*ppt-- = LV_COLOR_MAKE(red,green,blue);
		}
	}

	fclose( my_file );

	return ( 0 );
}

/**
 * Create some objects
 *
 * JSON format:
 * {
 *  "type"  :  "image",
 *  "path"  :  "/../..",
 *  "position"  :  { "x" : "", "y" : "" },
 *
 */

int image_add(struct json_decoder *jsond)
{
	struct image_desc	*p;
	int err=0;
	int i;

	for (i=0;i<MAX_IMAGE_DESCRIPTOR;i++) {
		if(sd.id[i]==NULL)
			break;
	}

	if(i==MAX_IMAGE_DESCRIPTOR)
		return(ENOMEM);

	p=malloc(sizeof(struct image_desc));
	if(!p)
		return(ENOMEM);
	memset(p,0,sizeof(struct image_desc));

	sd.id[i]=p;

	strcpy(p->name,jsond->name);

	err=read_bitmap( jsond->path, &p->desc );
	if(err!=0){
		GuiLog("Failed to load bitmap error=%d",err);
		deref_image_desc(&sd.id[i]);
		return err;
	}

	p->img_screen.header.always_zero=0;
	p->img_screen.data = p->desc.img;
	p->img_screen.data_size= p->desc.height * p->desc.width * LV_COLOR_SIZE / 8;
	p->img_screen.header.cf=LV_IMG_CF_TRUE_COLOR;
	p->img_screen.header.w=p->desc.width;
	p->img_screen.header.h=p->desc.height;

	p->scr_wp = lv_img_create(lv_scr_act(), NULL);

    lv_img_set_src(p->scr_wp, &p->img_screen);
    lv_obj_set_pos(p->scr_wp, jsond->pos.x, jsond->pos.y);      /*Set the positions*/

    return 0;
}

/**
 * delete image
 *
 * JSON format:
 * {
 *  "type"  :  "delimage",
 *  "name" :  "image_name"
 * }
 *
 * Desc: remove all instance with matching name
 *
 */

int image_del(struct json_decoder *jsond)
{
	int i;

	for (i=0;i<MAX_IMAGE_DESCRIPTOR;i++) {
		if (sd.id[i]!=NULL) {
			if (strcmp(sd.id[i]->name,jsond->name)==0) {
				deref_image_desc(&sd.id[i]);
			}
		}
	}

    return 0;
}

/**
 * Clear screen
 *
 * JSON format:
 * {
 *  "type"  :  "image",
 *  "color" :  { "main" : "#000000", "gradient" : "#000000" }
 *
 */

int clear_screen(struct json_decoder *jsond)
{
	int i;
	int rgb;

	/*Delete all images */
	for (i=0;i<MAX_IMAGE_DESCRIPTOR;i++) {
		if (sd.id[i]!=NULL) {
			deref_image_desc(&sd.id[i]);
		}
	}
	/*Delete all text */
	text_del_all();
	/*Delete all button */
	button_del_all();
	/*Delete all keypad */
	keypad_del_all();

	/* CUSTOM SCREEN STYLE */
	lv_style_copy(&sd.scr_style, &lv_style_pretty_color);

	sd.scr_style.text.color = LV_COLOR_WHITE;
	sd.scr_style.text.font = &lv_font_dejavu_30;   /*Unicode and symbol fonts already assigned by the library*/
	rgb=color_conv(jsond->color.main);
	sd.scr_style.body.main_color = RGB_2_16bit(rgb);
	rgb=color_conv(jsond->color.grad);
	sd.scr_style.body.grad_color = RGB_2_16bit(rgb);


	/********************
     * CREATE A SCREEN
     *******************/
    /* Create a new screen and load it
     * Screen can be created from any type object type
     * Now a Page is used which is an objects with scrollable content*/
    lv_obj_t *scr = lv_scr_act();
    lv_page_set_style(scr,LV_PAGE_STYLE_BG,&sd.scr_style);
    lv_scr_load(scr);

    return 0;
}
