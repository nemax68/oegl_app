/**
  * @file oe_json.c
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

#include<stdio.h>
#include<string.h>

#include<json-c/json.h>
#include"oe_json.h"
#include"oe_mqueue.h"
#include"oe_gui.h"

enum typefunc {
	IMAGE_FUNCTION,
};

static const struct json_function jf[] = {
		{"addimage", 	image_add},
		{"delimage", 	image_del},
		{"clrscreen", 	clear_screen},
		{"addbutton", 	button_add},
		{"delbutton", 	button_del},
		{"addkeypad", 	keypad_add},
		{"delkeypad", 	keypad_del},
		{"addtext", 	text_add},
		{"addboxtext", 	text_box_add},
		{"deltext", 	text_del},
		{"delboxtext", 	text_del},
		{"",			NULL}
};


int json_parser(char *buffer)
{
	struct json_object *parsed_json;
	struct json_object *type;
	struct json_object *name;
	struct json_object *path;
	struct json_object *text;
	struct json_object *position;
	struct json_object *size;
	struct json_object *font;
	struct json_object *border;
	struct json_object *param;
	struct json_object *color;

	char				*str,*pt;
	struct json_decoder jd;
	const struct json_function *pjf=jf;

	memset(&jd,0,sizeof(jd));

	parsed_json = json_tokener_parse(buffer);
	if (parsed_json) {

		// extract name
		if( json_object_object_get_ex(parsed_json, "name", &name) ) {
			str=(char *)json_object_get_string(name);
			if(strlen(str))
				strcpy(jd.name,str);
			else
				sprintf(jd.name,"none");
			json_object_put(name);
		}

		// extract position
		if( json_object_object_get_ex(parsed_json, "position", &position) ) {
			if( json_object_object_get_ex(position, "x", &param) ) {
				jd.pos.x=json_object_get_int(param);
				json_object_put(param);
			}
			if( json_object_object_get_ex(position, "y", &param) ) {
				jd.pos.y=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(position);
		}

		// extract size
		if( json_object_object_get_ex(parsed_json, "size", &size) ) {
			if( json_object_object_get_ex(size, "x", &param) ) {
				jd.size.x=json_object_get_int(param);
				json_object_put(param);
			}
			if( json_object_object_get_ex(size, "y", &param) ) {
				jd.size.y=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(size);
		}

		// extract color
		if( json_object_object_get_ex(parsed_json, "color", &color) ) {
			if( json_object_object_get_ex(color, "main", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.color.main,str);
				json_object_put(param);
			}
			if( json_object_object_get_ex(color, "gradient", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.color.grad,str);
				json_object_put(param);
			}
			json_object_put(color);
		}

		// extract font
		if( json_object_object_get_ex(parsed_json, "font", &font) ) {
			if( json_object_object_get_ex(font, "color", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.font.color,str);
				json_object_put(param);
			}
			if( json_object_object_get_ex(font, "size", &param) ) {
				jd.font.size=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(font);
		}

		// extract border
		if( json_object_object_get_ex(parsed_json, "border", &border) ) {
			if( json_object_object_get_ex(border, "color", &param) ) {
				str=(char *)json_object_get_string(param);
				strcpy(jd.border.color,str);
				json_object_put(param);
			}
			if( json_object_object_get_ex(border, "size", &param) ) {
				jd.border.size=json_object_get_int(param);
				json_object_put(param);
			}
			if( json_object_object_get_ex(border, "round", &param) ) {
				jd.border.radius=json_object_get_int(param);
				json_object_put(param);
			}
			json_object_put(border);
		}

		// extract path
		if( json_object_object_get_ex(parsed_json, "path", &path) ) {
			str=(char *)json_object_get_string(path);
			strcpy(jd.path,str);
			json_object_put(path);
		}

		// extract text
		if( json_object_object_get_ex(parsed_json, "text", &text) ) {
			str=(char *)json_object_get_string(text);
			pt=jd.text;
			// sobstitute \n with 0x0a
			while (*str) {
				if( (*str==92) && (*(str+1)==110) ) {
					*pt++=0x0a;
					str+=2;
				}else{
					*pt++=*str++;
				}
			}
			json_object_put(text);
		}

		if (json_object_object_get_ex(parsed_json, "type", &type)) {
			str=(char *)json_object_get_string(type);

			while (strlen(pjf->name)) {
				if(strcmp(str,pjf->name)==0)
					(pjf->jfunc)(&jd);
				pjf++;

			}
		}

		json_object_put(type);
		json_object_put(parsed_json);
	}

	return 0;
}

int json_encoder(struct json_encoder *json_enc)
{
	json_object * jobj = json_object_new_object();
	json_object *jevent = json_object_new_string(json_enc->event);
	json_object *jtype = json_object_new_string(json_enc->type);
	json_object *jname = json_object_new_string(json_enc->name);

	json_object_object_add(jobj,"name", jname);
	json_object_object_add(jobj,"event", jevent);
	json_object_object_add(jobj,"type", jtype);

	send_posix_event((char *)json_object_to_json_string(jobj));

	return 0;
}
