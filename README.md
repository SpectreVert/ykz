# ykz

[![MIT licensed][license-badge]][license-url]

A hackable and scalable request/response **TCP** server that you can call yours.

## Features

* define 4 hooks and your server is ready for takeoff
* edge-polled event handling
* no external dependencies
* easy to port to other OSes

## Example

This is how a basic 'echo' server looks like with ykz:

```c++
#include "ykz.hpp"

using namespace ykz;

/* Triggers after host_start() is called and before starting to wait for connections */
void init()
{
}

/* Triggers when some data has been received */
proto_result okay(void *data)
{
    /* Possible return values:
    
    PRS_SWITCH_MODE: switch from receiving mode to sending mode
    PRS_SUGGEST_WAITING: a hint to wait for more data
    PRS_INTERNAL_ERROR: close the connection

    */

    return PRS_SWITCH_MODE;
}

/* Triggers when a client is disconnected from the server */
void goodbye(void *data)
{
}

/* Triggers after host_stop() is called and before it returns */
void cleanup()
{
}

int main(int ac, char *av[])
{
    Host h;
    Protocol echo{
        &init,
        &okay,
        &goodbye,
        &cleanup,
    };
    
    host_start(h, echo, IP_STRING_TO_U32("127.0.0.1"), 8080);
    
    /* ... <do something or wait> ... */
    
    host_stop(h);
    
    return 0;
}
```

### See also

A HTTP GET/HEAD-only implementation is provided in `protocols/http_mini/`. The goal is to
support delivering static content and that's it.

I'm planning to add a game implementation with more complex logic. Be back :soon:

## Configuration

File: [ykz.config.hpp][config-url]

Values:

|Name|Type|Value|
|----|----|-----|
|CLOSE_ON_RESPONSE|bool|Controls if the connection is closed after the response has been sent|
|HEADER_LENGTH|u32|The length in bytes of the message buffer|
|DRIVERS_PER_HOST|u32|The number of event-loop threads spawned by a single Host|
|GUESTS_PER_DRIVER|u32|The limit of simultaneous connections handled by a single event-loop thread|
|EVENTS_PER_BATCH|u32|The limit of events returned per event-loop|
|LISTEN_SOCKET_BACKLOG|s32|A hint to the listen(2) implementation to limit the number of pending connections on the listening socket|

## Compilation

TDB

[license-badge]: https://img.shields.io/badge/license-MIT-blue
[license-url]: https://github.com/SpectreVert/ykz/blob/master/LICENSE
[config-url]: https://github.com/SpectreVert/ykz/blob/master/ykz.config.hpp
