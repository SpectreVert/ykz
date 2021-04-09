/*
 * Created by Costa Bushnaq
 *
 * 07-04-2021 @ 01:47:19
 *
 * see LICENSE
*/

#ifndef SERVER_H
#define SERVER_H

#include <stddef.h>

struct vhost {
	char *h;
};

struct server {
	char *host;
	char *port;
	struct vhost *vhost;
	size_t vhosts_nb;
};

#endif /* SERVER_H */
