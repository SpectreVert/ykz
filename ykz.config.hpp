#ifndef YKZ_CONFIG_HPP_
#define YKZ_CONFIG_HPP_

//! Type : bool
//! Usage: Flag to indicate whether to close connection after each response
//!
#define CLOSE_ON_RESPONSE false 

//! Type : u64
//! Usage: Size in bytes of the message buffer
//!
#define BUFFER_SIZE 4096

//! Type : u64
//! Usage: Number of poll workers
//!
#define NB_WORKERS 4

//! Type : u64
//! Usage: Number of client slots for each worker
//!
#define GUESTS_MAX 64

#endif /* YKZ_CONFIG_HPP_ */
