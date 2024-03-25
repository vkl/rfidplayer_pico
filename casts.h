#ifndef CASTS_H
#define CASTS_H

#include <stdlib.h>
#include "cast_controllers.h"

extern struct ChromeCast casts[2];

struct ChromeCast* findChromecastByName(struct ChromeCast *casts, int size, const char* name);

#endif // CASTS_H