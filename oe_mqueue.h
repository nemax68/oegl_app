/**
  * @file oe_mqueue.h
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


#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

//#include "openeyes_gui.h"
#include <mqueue.h>

/*********************
 *      DEFINES
 *********************/

#define QUEUE_GUI_EVENT			"/gui_event"
#define QUEUE_GUI_CMD			"/gui_cmd"

#define POSIX_CMD_BUFFER_SIZE       1024
#define POSIX_EVT_BUFFER_SIZE       1024

/**********************
 *      TYPEDEFS
 **********************/

struct posix_queue {
 	size_t size;
  	char *buf;
 	mqd_t mq;
};

struct posix_st {
    struct posix_queue	cmd;
 	struct posix_queue	evt;
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

void handle_posix_command(void);
int posix_command_init(void);
int send_posix_event(char *);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* IPC_CLIENT_H */
