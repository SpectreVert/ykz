/*
 * Created by Costa Bushnaq
 *
 * 08-04-2021 @ 22:30:18
*/

#include "server.h"

#include <pthread.h>
#include <stdlib.h>

void *
worker(void *data)
{
	return 0x0;
}

int
init_workers(int ssock, struct server *srv, size_t nb_workers, size_t nb_slots)
{
	size_t i, res;
	pthread_t *thread;
	void *data = 0x0;

	if ((thread = calloc(nb_workers, sizeof(*thread))) == 0x0)
		die("calloc:");

	for (i = 0; i != nb_workers; ++i)
	{
		if (pthread_create(&thread[i], 0x0, &worker, (void *)data) < 0)
			die("pthread_create:");
	}

	return 0;
}
