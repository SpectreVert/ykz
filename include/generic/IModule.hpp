/*
 * Created by Costa Bushnaq
 *
 * 04-05-2021 @ 19:54:38
 *
 * see LICENSE
*/

#ifndef _IMODULE_HPP
#define _IMODULE_HPP

#include "Config.hpp"

#define TODO void

namespace ykz {

struct IModule {
	virtual ~IModule() = default;

	//! All the hooks that are called in the message
	//! processing phase. Order is respected.
	virtual void startup(Config const&) = 0;  //! after reading configuration file
	virtual void post_connect(TODO) = 0;      //! after opening a connection
	virtual void post_request(TODO) = 0;      //! after receiving data
	virtual void handle(TODO) = 0;            //! handling of the connection state
	virtual void pre_response(TODO) = 0;	  //! before sending data
	virtual void pre_disconnect(TODO) = 0;    //! before closing a connection
	virtual void cleanup() = 0;               //! before closing the server
};

} // namespace ykz

#endif /* _IMODULE_HPP */
