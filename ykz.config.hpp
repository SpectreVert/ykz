#ifndef YKZ_CONFIG_HPP_
#define YKZ_CONFIG_HPP_

//! Type : bool
//! Usage: Flag to indicate whether to close connection after each response
//!
#define CLOSE_ON_RESPONSE false 

//! Type : u64
//! Usage: Size in bytes of the header buffer
//!
#define HEADER_SIZE 4096

//! Type : u64
//! Usage: Number of connection worker threads
//!
#define NB_WORKERS 4

//! Type : u64
//! Usage: Number of max open connections for a given worker
//!
#define NB_GUESTS 64

#endif /* YKZ_CONFIG_HPP_ */
