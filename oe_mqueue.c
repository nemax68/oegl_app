/**
  * @file oe_mqueue.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "oe_gui.h"
#include "oe_mqueue.h"

static struct posix_st posix;

extern int json_parser(char *);

int send_posix_event(char *JSON_event)
{
    mqd_t mq;
    int	len;

    len=strlen(JSON_event);
    if (len>POSIX_EVT_BUFFER_SIZE)
    	return ENOMEM;

    /* open the mail queue */
    mq = mq_open(QUEUE_GUI_EVENT, O_WRONLY|O_NONBLOCK );
    if(mq==(mqd_t)-1)
		return -1;

	if(	mq_send(mq, JSON_event, len, 0) )
		return -1;

	printf("\nEVENT %s size %d",JSON_event,len);

	mq_close(mq);

	return 0;
}

static int get_command(void)
{
	mqd_t mqdes=posix.cmd.mq;
	char *buf=posix.cmd.buf;
	int size=posix.cmd.size;
	struct mq_attr attr;
	int ret=0;

	if (mq_getattr(mqdes, &attr) == 0) {
		if(attr.mq_curmsgs){
			if(size<attr.mq_msgsize)
            	return ENOMEM;

            ret=mq_receive(mqdes, buf, attr.mq_msgsize, 0);
        }
    } else {
		return ENODATA;
    }

    return ret;
}

void handle_posix_command(void)
{
    int     ret;

    ret = get_command();

    if (ret>0) {
       posix.cmd.buf[ret]=0;	// be sure to terminate buffer string
       json_parser(posix.cmd.buf);
    }
}

int posix_command_init(void)
{
    struct mq_attr attr;
    char *buf;

    memset(&posix,0,sizeof(posix));

    buf = malloc(POSIX_CMD_BUFFER_SIZE);
    if(buf==NULL)
    	return ENOMEM;

    posix.cmd.buf = buf;
    posix.cmd.size = POSIX_CMD_BUFFER_SIZE;

	/* open the mail queque */
	posix.cmd.mq = mq_open(QUEUE_GUI_CMD, O_RDONLY|O_NONBLOCK );
	if (posix.cmd.mq==(mqd_t)-1) {
		if (errno==ENOENT) {
			attr.mq_flags=O_NONBLOCK;
			attr.mq_maxmsg=10;
			attr.mq_msgsize=posix.cmd.size;
			posix.cmd.mq = mq_open(QUEUE_GUI_CMD, O_RDONLY|O_NONBLOCK|O_CREAT, S_IRUSR|S_IWUSR, &attr );

			if (posix.cmd.mq==(mqd_t)-1)
				return errno;
		} else {
			return errno;
		}
	}

	// flush all queued events
    //while(get_command()!=0);

 	return 0;
}
