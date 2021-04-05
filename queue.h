/*
 * Created by Costa Bushnaq
 *
 * 04-04-2021 @ 22:32:12
 *
 * see LICENSE
*/

#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

enum queue_event_type {
	e_in  = 1,
	e_out = 1 << 1
};

#ifdef __linux__
  #include <sys/epoll.h>

  typedef struct epoll_event queue_event;
#else
  #include <sys/event.h>

  typedef struct kevent queue_event;
#endif

int queue_mk();
int queue_add_fd(int, int, enum queue_event_type, void const*, int);

#endif /* QUEUE_H */
