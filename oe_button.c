/**
  * @file oe_button.c
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
#include <stdlib.h>
#include <errno.h>
#include "oe_gui.h"
#include "oe_json.h"

/*********************
 *      DEFINES
 *********************/

#define MAX_BUTTON_DESCRIPTOR		8
#define BUTTON_OPERATION_RUNNING	0x80000000

/**********************
 *      TYPEDEFS
 **********************/

struct btn_desc {
	char name[64];
	lv_task_t * refr_task;
	lv_obj_t *button;
	lv_obj_t *button_label;
	lv_style_t button_style;
};

struct buttons_desc {
	struct btn_desc *id[MAX_BUTTON_DESCRIPTOR];
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static struct buttons_desc bd;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int init_button(void)
{
	memset(&bd,0,sizeof(bd));
	return 0;
}

static void deref_btn_desc(struct btn_desc **ref)
{
	struct btn_desc *id=*ref;

	if(id->button)
		lv_obj_del(id->button);
	if(id->refr_task)
		lv_task_del(id->refr_task);
	if(id)
		free(id);
	*ref=NULL;
}

static int find_button_by_name(int *id, char *name)
{
	int i;
	for (i=0;i<MAX_BUTTON_DESCRIPTOR;i++) {
		if (bd.id[i]) {
			if (strcmp(bd.id[i]->name,name)==0) {
				*id=i;
				return 1;
			}
		}
	}

	return 0;
}

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @return LV_RES_OK because the object is not deleted in this function
 */
static  void btn_close(void * btn)
{
	struct btn_desc 	*p;
	lv_color_t			color;
	lv_style_t			*bstyle;
	lv_obj_t			*lvbtn = (lv_obj_t *)btn;
	uint32_t			id=lvbtn->free_num;
	int					idx=id&0xFF;

	if(	! (id&BUTTON_OPERATION_RUNNING) )
		return;

	if(idx<MAX_BUTTON_DESCRIPTOR){
		p=bd.id[idx];
		/*point to button specific style */
		bstyle = &p->button_style;

		/*Swap the button color*/
		color = bstyle->body.main_color;
		bstyle->body.main_color = bstyle->body.border.color;
		bstyle->body.border.color = color;

		/*Update button color */
		lv_obj_set_style((lv_obj_t *)btn, bstyle);

		/*delete task */
		lvbtn->free_num = id & ~BUTTON_OPERATION_RUNNING;
		if (p->refr_task) {
			lv_task_del(p->refr_task);
			p->refr_task=NULL;
		}
	}
}

/**
 * Called when a button is released
 * @param btn pointer to the released button
 * @return LV_RES_OK because the object is not deleted in this function
 */
static  lv_res_t btn_action(lv_obj_t * btn)
{
	lv_color_t				color;
	lv_style_t				*bstyle;
	uint32_t				id=btn->free_num;
	int						idx=id&0xFF;
	struct btn_desc 		*p;
	struct json_encoder		je;

	if(	id&BUTTON_OPERATION_RUNNING )
		return LV_RES_OK;

	if(idx<MAX_BUTTON_DESCRIPTOR){
		p=bd.id[id];
		/*point to button specific style */
		bstyle = &p->button_style;

		/*Swap the button color*/
		color = bstyle->body.main_color;
		bstyle->body.main_color = bstyle->body.border.color;
		bstyle->body.border.color = color;

		lv_obj_set_style(btn, bstyle);

		/*Call progress here */
		btn->free_num = id | BUTTON_OPERATION_RUNNING;

		printf("press button %s",p->name);
		memset(&je,0,sizeof(struct json_encoder));
		strcpy(je.type,"button");
		strcpy(je.event,"press");
		strcpy(je.name,p->name);
		json_encoder(&je);

		p->refr_task=lv_task_create(btn_close, 1000, LV_TASK_PRIO_MID, btn);
	}
    return LV_RES_OK;
}


/**
 * Create button objects
 *
 * JSON format:
 * {
 *  "type"  :  "addbutton",
 *  "position"  :  { "x" : "", "y" : "" },
 *
 */

int button_add(struct json_decoder *jsond)
{
	struct btn_desc *p;
	int i,found;

	found=find_button_by_name(&i,jsond->name);

	if (!found) {
		for (i=0;i<MAX_BUTTON_DESCRIPTOR;i++) {
			if(bd.id[i]==NULL)
				break;
		}

		if(i==MAX_BUTTON_DESCRIPTOR)
			return(ENOMEM);

		p=malloc(sizeof(struct btn_desc));
		if(p==NULL)
			return(ENOMEM);
		memset(p,0,sizeof(struct btn_desc));

		bd.id[i]=p;

		memset(p,0,sizeof(struct btn_desc));
		strcpy(p->name,jsond->name);

		p->button = lv_btn_create(lv_scr_act(), NULL);          		/*Create a button on the currently loaded screen*/
		p->button_label = lv_label_create(p->button, NULL);				/*Create a label on the current button*/
		lv_style_copy(&p->button_style, &lv_style_btn_rel);		/*Create style */
		p->button->free_num=i;											/*link obj_id */
		/* CUSTOM BUTTON STYLE */
		lv_btn_set_action(p->button, LV_BTN_ACTION_CLICK, btn_action); /*Set function to be called when the button is released*/

	} else {
		p=bd.id[i];
	}

	set_text_style(&p->button_style, jsond);
	set_body_style(&p->button_style, jsond);

	lv_obj_set_size(p->button,jsond->size.x,jsond->size.y);
	lv_obj_set_pos(p->button,jsond->pos.x, jsond->pos.y);
	lv_obj_set_style(p->button, &p->button_style);

	lv_label_set_text(p->button_label, jsond->text);

	return 0;
}

/**
 * Delete button
 *
 * JSON format:
 * {
 *  "type"  :  "delbutton",
 *  "name" :  "button_name"
 * }
 *
 * Desc: remove all instance with matching name
 *
 */

int button_del(struct json_decoder *jsond)
{
	int i;

	for (i=0;i<MAX_BUTTON_DESCRIPTOR;i++) {
		if (bd.id[i]) {
			if (strcmp(bd.id[i]->name,jsond->name)==0) {
				deref_btn_desc(&bd.id[i]);
			}
		}
	}

    return 0;
}

int button_del_all(void)
{
	int i;

	for (i=0;i<MAX_BUTTON_DESCRIPTOR;i++) {
		if (bd.id[i]) {
			deref_btn_desc(&bd.id[i]);
		}
	}

    return 0;
}
