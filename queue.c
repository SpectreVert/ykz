/*
 * Created by Costa Bushnaq
 *
 * 04-04-2021 @ 22:32:05
*/

#include "queue.h"
#include "utils.h"

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
		e.events |= EPOLLIN | EPOLLRDHUP;
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

	if (kevent(qfd, &e, 1, 0x0, 0, 0x0) < 0)
	{
		warn("kevent:");
		return -1;
	}
#endif

	return 0;
}
