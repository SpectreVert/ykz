/*
 * Created by Costa Bushnaq
 *
 * 02-07-2021 @ 00:11:13
 *
 * see LICENSE
*/

#ifndef _CONNECTION_HPP
#define _CONNECTION_HPP

#include <string>

namespace ykz {

struct Connection {	
	enum State {
		e_blank,
		e_listen,
		e_send,
		e_close
	} state;

}; // struct Connnection

} // namespace ykz

#endif /* _CONNECTION_HPP */
