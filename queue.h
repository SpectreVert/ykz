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
int queue_mod_fd(int, int, enum queue_event_type, void const*);
int queue_rm_fd(int, int);
int queue_wait(int, queue_event *, size_t, int);
int queue_event_is_error(queue_event const *);
void *queue_event_get_data(queue_event const *);

#endif /* QUEUE_H */
