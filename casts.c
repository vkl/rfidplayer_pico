#include "casts.h"


struct ChromeCast casts[2] = {
    {"Living Room", "192.168.1.106", 8009},
    {"Bedroom Speaker", "192.168.1.102", 8009}
};

struct ChromeCast* findChromecastByName(struct ChromeCast *casts, int size, const char* name) {
    for (int i = 0; i < size; i++) {
        if (strcmp(casts[i].Name, name) == 0) {
            return &casts[i];
        }
    }
    return NULL; // Chromecast not found
}
