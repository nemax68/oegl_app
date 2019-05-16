/**
 * @file screen.c
 *
 */

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

#define MAX_TEXT_DESCRIPTOR	8

/**********************
 *      TYPEDEFS
 **********************/

struct txt_desc {
	char name[64];
	lv_obj_t *label;
	lv_obj_t *box;
	lv_style_t text_style;
};

struct text_desc {
	lv_style_t scr_style;
	struct txt_desc *id[MAX_TEXT_DESCRIPTOR];
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static struct text_desc td;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/


int init_text(void)
{
	memset(&td,0,sizeof(td));
	return 0;
}



/**
 * Create some objects
 *
 * JSON format:
 * {
 *  "type"  :  "addtext",
 *  "path"  :  "/../..",
 *  "position"  :  { "x" : "", "y" : "" },
 *
 */

int text_add(struct json_decoder *jsond)
{
	struct txt_desc	*p;
	int i;

	for (i=0;i<MAX_TEXT_DESCRIPTOR;i++) {
		if(td.id[i]==NULL)
			break;
	}

	if(i==MAX_TEXT_DESCRIPTOR)
		return(ENOMEM);

	p=malloc(sizeof(struct txt_desc));
	if(!p)
		return(ENOMEM);

	td.id[i]=p;

	memset(p,0,sizeof(struct txt_desc));
	strcpy(p->name,jsond->name);

	lv_style_copy(&p->text_style, &lv_style_plain);		/*Create style */
	set_text_style(&p->text_style, jsond);

	p->label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_style(p->label, &p->text_style);
	lv_label_set_text(p->label, jsond->text);
    lv_obj_set_pos(p->label, jsond->pos.x, jsond->pos.y);      /*Set the positions*/

    return 0;
}

/**
 * Create some objects
 *
 * JSON format:
 * {
 *  "type"  :  "addtext",
 *  "path"  :  "/../..",
 *  "position"  :  { "x" : "", "y" : "" },
 *
 */

int text_box_add(struct json_decoder *jsond)
{
	struct txt_desc	*p;
	int i;

	for (i=0;i<MAX_TEXT_DESCRIPTOR;i++) {
		if(td.id[i]==NULL)
			break;
	}

	if(i==MAX_TEXT_DESCRIPTOR)
		return(ENOMEM);

	p=malloc(sizeof(struct txt_desc));
	if(!p)
		return(ENOMEM);

	td.id[i]=p;
	memset(p,0,sizeof(struct txt_desc));
	strcpy(p->name,jsond->name);

	lv_style_copy(&p->text_style, &lv_style_plain);		/*Create style */

	p->box = lv_cont_create(lv_scr_act(), NULL);
	set_text_style(&p->text_style, jsond);
	set_body_style(&p->text_style, jsond);
	lv_label_set_style(p->box, &p->text_style);
	//lv_cont_set_fit(p->box, true, true);
    lv_obj_set_pos(p->box, jsond->pos.x, jsond->pos.y);      /*Set the positions*/

	p->label = lv_label_create(p->box, NULL);
	lv_label_set_text(p->label, jsond->text);

    return 0;
}

/**
 * Create some objects
 *
 * JSON format:
 * {
 *  "type"  :  "addspecialtext",
 *  "position"  :  { "x" : "", "y" : "" },
 *
 */
int text_special_add(struct json_decoder *jsond)
{
	struct txt_desc	*p;
	int err=0;
	int i;

	for (i=0;i<MAX_TEXT_DESCRIPTOR;i++) {
		if(td.id[i]==NULL)
			break;
	}

	if(i==MAX_TEXT_DESCRIPTOR)
		return(ENOMEM);

	p=malloc(sizeof(struct txt_desc));
	if(!p)
		return(ENOMEM);

	td.id[i]=p;

	strcpy(p->name,jsond->name);

	lv_style_copy(&p->text_style, &lv_style_plain);		/*Create style */
	set_text_style(&p->text_style, jsond);

	p->label = lv_label_create(lv_scr_act(), NULL);
	lv_label_set_style(p->label, &p->text_style);
	lv_label_set_text(p->label, _SYMBOL_VALUE3(EF,A0,80));
    lv_obj_set_pos(p->label, jsond->pos.x, jsond->pos.y);      /*Set the positions*/

    return 0;
}

/**
 * delete image
 *
 * JSON format:
 * {
 *  "type"  :  "deltext",
 *  "name" :  "image_name"
 * }
 *
 * Desc: remove all instance with matching name
 *
 */

int text_del(struct json_decoder *jsond)
{
	int i;

	for (i=0;i<MAX_TEXT_DESCRIPTOR;i++) {
		if (td.id[i]!=NULL) {
			if (strcmp(td.id[i]->name,jsond->name)==0) {
				if(td.id[i]->label)
					lv_obj_del(td.id[i]->label);
				if(td.id[i]->box)
					lv_obj_del(td.id[i]->box);
				free(td.id[i]);
				td.id[i]=NULL;
				return 0;
			}
		}
	}

    return 0;
}

int text_del_all(void)
{
	int i;

	for (i=0;i<MAX_TEXT_DESCRIPTOR;i++) {
		if (td.id[i]!=NULL) {
			if(td.id[i]->label)
				lv_obj_del(td.id[i]->label);
			if(td.id[i]->box)
				lv_obj_del(td.id[i]->box);
			free(td.id[i]);
			td.id[i]=NULL;
		}
	}

    return 0;
}

