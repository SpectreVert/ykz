/*
 * Created by Costa Bushnaq
 *
 * 04-04-2021 @ 21:24:11
*/

#include "queue.h"
#include "misc.h"

#include "config.h"

#include <stdio.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

static void
sighandler(int sig)
{
	warn("\nstopping server...");
	kill(0, sig);
	_exit(0);
}

static void
set_sighandler(void (*handler)(int))
{
	struct sigaction ac = { .sa_handler = handler };

	setpgid(0, 0);
	sigemptyset(&ac.sa_mask);
	sigaction(SIGINT, &ac, 0x0);
	sigaction(SIGQUIT, &ac, 0x0);
	sigaction(SIGTERM, &ac, 0x0);
	sigaction(SIGHUP, &ac, 0x0);
}

static void
usage()
{
	printf("usage\n");
}

int main(int argc, char* argv[])
{
	int ssock, res;
	struct rlimit rlim;

	if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("socket:");

	rlim.rlim_max = rlim.rlim_cur = 1 + 10 + nthreads + nthreads * nslots;

	if ((res = setrlimit(RLIMIT_NOFILE, &rlim)) < 0)
	{
		if (res == EPERM)
		{
			die("You need to have CAP_SYS_RESOURCE set, run as root, "
			    "or the system cannot offer enough file descriptors");
		}
		die("setrlimit:");
	}

	set_sighandler(&sighandler);

	return 0;
}
