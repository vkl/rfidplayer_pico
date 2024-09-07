#ifndef _CAST_CONTROLLERS_H
#define _CAST_CONTROLLERS_H

#include "tls_client.h"
#include "casts.h"

#define MAXFAILEDPINGS 3

struct ChromeCast {
    char *Name;
    char *IPAddr;
    int Port;
};

void initCastConnectionState(struct ConnectionState *connState);
void CastConnect(ChromeCastDevices *devices);

#endif
