/*
 * Created by Costa Bushnaq
 *
 * 04-04-2021 @ 21:24:11
*/

#include <stdio.h>
#include "queue.h"
#include "misc.h"

#include "config.h"

#include <sys/socket.h>

static void
usage()
{
	printf("usage\n");
}

int main(int argc, char* argv[])
{
	int qfd = queue_mk();
	int s = socket(AF_INET, SOCK_STREAM, 0);

	if (s == -1)
	{
		warn("socket:");
		return 1;
	}

	int res;

	if ((res = queue_add_fd(qfd, s, e_in, 0x0, 0)) < 0)
		return 1;

	ssize_t ev;
	queue_event e[1024];

	if ((ev = queue_wait(qfd, e, 1024)) < 0)
		return 1;

	printf("%ld\n", ev);

	return 0;
}
