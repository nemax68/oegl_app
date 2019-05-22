/**
  * @file oe_text.c
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

static void deref_text_desc(struct txt_desc **ref)
{
	struct txt_desc *id=*ref;

	if(id->label)
		lv_obj_del(id->label);
	if(id->box)
		lv_obj_del(id->box);

	if(id)
		free(id);

	*ref=NULL;
}

static int find_text_by_name(int *id, char *name)
{
	int i;

	for (i=0;i<MAX_TEXT_DESCRIPTOR;i++) {
		if (td.id[i]) {
			if (strcmp(td.id[i]->name,name)==0) {
				*id=i;
				return 1;
			}
		}
	}

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
	int i,found;

	found=find_text_by_name(&i,jsond->name);

	if (!found) {

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
	} else {
		p=td.id[i];
	}

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

static int word_len(char *str)
{
	int i=0;
	char *pt=str;

	do{
		if(*pt==0x20)	return i;	// found space
		if(*pt==0x0a)	return i;	// found new line
		if(*pt==0x00)	return i;	// found end of string
		*pt++;
	}while(i++<64);

	return i;
}

int text_box_add(struct json_decoder *jsond)
{
	struct txt_desc	*p;
	int i,found;
	char text[1024],*pti,*pto;
	int l,line;

	found=find_text_by_name(&i,jsond->name);

	printf("add box %d name %s [%s]\n",found,jsond->name, jsond->text);

	if (!found) {

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
		p->label = lv_label_create(p->box, NULL);
	} else {
		p=td.id[i];
		lv_label_set_text(p->label, "");
	}

	set_text_style(&p->text_style, jsond);
	set_body_style(&p->text_style, jsond);

	lv_label_set_style(p->box, &p->text_style);
	lv_obj_set_size(p->box,jsond->size.x,jsond->size.y);

	//lv_cont_set_fit(p->box, true, true);
	lv_cont_set_layout(p->box, LV_LAYOUT_CENTER);
    lv_obj_set_pos(p->box, jsond->pos.x, jsond->pos.y);      /*Set the positions*/

    switch(jsond->font.size){
    		case 10: line=jsond->size.x/10; break;
    		case 20: line=jsond->size.x/20; break;
    		case 30: line=jsond->size.x/16; break;
    		default: line=jsond->size.x/40;
    }

    pti=jsond->text;
    pto=text;
    for(i=0,l=0;i<strlen(jsond->text);i++) {
    	*pto++=*pti;

    	if (*pti==0x0a) {
    		l=0;
    	} else {
    		if ( *pti==0x20 ) {
    			if ( (word_len((pti+1))+l)>line ) {	// break line
    				*pto++=0x0a;
    				l=0;
    			}
    		} else {
    			l++;
    		}
    	}
    	pti++;

    	if(l>line) {
    		*pto++=0x0a;
    		l=0;
    	}
    }
    *pto=0;

	printf("TEXT [%s]\n",text);

	lv_label_set_text(p->label, text);

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
				deref_text_desc(&td.id[i]);
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
			deref_text_desc(&td.id[i]);
		}
	}

    return 0;
}
