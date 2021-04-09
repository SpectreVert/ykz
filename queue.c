/*
 * Created by Costa Bushnaq
 *
 * 04-04-2021 @ 22:32:05
*/

#include "queue.h"
#include "misc.h"

#include <assert.h>
#include <stdlib.h> // malloc
#include <unistd.h>

int
queue_mk()
{
	int qfd;

#ifdef __linux__
	if ((qfd = epoll_create1(EPOLL_CLOEXEC)) < 0)
	{
		warn("epoll_create1:");
		
		if (errno == ENOSYS || errno == EINVAL)
		{
			if ((qfd = epoll_create(1024)) < 0)
			{
				warn("epoll_create:");
			}
		}
	}
#else
	if ((qfd = kqueue()) < 0)
	{
		warn("kqueue:");
	}
#endif

	return qfd;
}

int
queue_add_fd(int qfd, int fd, enum queue_event_type t, void const* data, int shared)
{
#ifdef __linux__
	struct epoll_event e;

	/* If the fd is shared we want the events to
	 * vbe distributed over the different threads.
	 * Edge-triggered would not work that way.
	*/
	if (shared)
	{
		e.events = EPOLLEXCLUSIVE;
	}
	else
	{
		e.events = EPOLLET;
	}

	if (t & e_in)
		e.events |= EPOLLIN; /* | EPOLLRDHUP; */
	if (t & e_out)
		e.events |= EPOLLOUT;

	e.data.ptr = (void *)data;

	if (epoll_ctl(qfd, EPOLL_CTL_ADD, fd, &e) < 0)
	{
		warn("epoll_ctl:");
		return -1;
	}
#else
	struct kevent e;
	int events = 0;

	if (shared)
		events = EV_CLEAR;

	if (t & e_in)
		events |= EVFILT_READ;
	if (t & e_out)
		events |= EVFILT_WRITE;

	EV_SET(&e, fd, events, EV_ADD, 0, 0, (void*)data);

	if (kevent(qfd, &e, 1, 0x0, 0, 0x0) < 0)
	{
		warn("kevent:");
		return -1;
	}
#endif

	return 0;
}

int queue_mod_fd(int qfd, int fd, enum queue_event_type t, void const* data)
{
#ifdef __linux__
	struct epoll_event e;

	e.events = EPOLLET;

	if (t & e_in)
		e.events |= EPOLLIN;
	if (t & e_out)
		e.events |= EPOLLOUT;

	e.data.ptr = (void*)data;

	if (epoll_ctl(qfd, EPOLL_CTL_MOD, fd, &e) < 0)
	{
		warn("epoll_ctl:");
		return -1;
	}
#else
	struct kevent e;
	int events = 0;

	events = EV_CLEAR;

	if (t & e_in)
		events |= EVFILT_READ;
	if (t & e_out)
		events |= EVFILT_WRITE;

	EV_SET(&e, fd, events, EV_ADD, 0, 0 (void*)data);	

	if (kevent(qfd, &e, 1, 0x0, 0, 0x0) < 0)
	{
		warn("kevent:");
		return -1;
	}
#endif

	return 0;
}

int
queue_rm_fd(int qfd, int fd)
{
#ifdef __linux__
	struct epoll_event e;

	if (epoll_ctl(qfd, EPOLL_CTL_DEL, fd, &e) < 0)
	{
		warn("epoll_ctl:");
		return -1;
	}
#else
	struct kevent e;

	if (kevent(qfd, &e, 1, 0x0, 0, 0x0) < 0)
	{
		warn("kevent:");
		return -1;
	}
#endif

	return 0;
}

/* TODO
 * add timeout variable
*/
int
queue_wait(int qfd, queue_event *ev, size_t ev_size)
{
	ssize_t ev_nb;
#ifdef __linux__
	if ((ev_nb = epoll_wait(qfd, ev, ev_size, -1)) < 0)
	{
		warn("epoll_wait:");
		return -1;
	}
#else
	if ((ev_nb = kevent(qfd, 0x0, 0, ev, ev_size, 0x0)) < 0)
	{
		warn("kevent:");
		return -1;
	}
#endif

	return 0;	
}

int
queue_event_is_error(queue_event const *e)
{
#ifdef __linux__
	return (e->events & ~(EPOLLIN | EPOLLOUT)) ? 1 : 0;
#else
	return (e->flags & EV_EOF) ? 1 : 0;
#endif
}

void*
queue_event_get_data(queue_event const *e)
{
#ifdef __linux__
	return (void*)e->data.ptr;
#else
	return (void*)e->udata;
#endif
}
