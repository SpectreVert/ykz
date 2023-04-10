/* Indicates wether to close the connection after each response */
static constexpr bool CLOSE_ON_RESPONSE{ false };

/* Defines the length (bytes) of the request and response header */
static constexpr u32 HEADER_LENGTH{ 4096 };

/* Defines the number of event-loop threads spawned by a single Host */
static constexpr u32 DRIVERS_PER_HOST{ 1 };

/* Defines the limit of connections handled by a single Driver */
static constexpr u32 GUESTS_PER_DRIVER{ 64 };

/* Defines the limit of events per event-loop */
static constexpr u32 EVENTS_PER_BATCH{ 1024 };

/* Defines the limit of simultaneous pending new connections */
static constexpr s32 LISTEN_SOCKET_BACKLOG{ 128 };
