#ifndef ADDON_PCH
#define ADDON_PCH

#include "GSRoot.hpp"

namespace std {
    void* GS_realloc (void *userData, size_t newSize);
}

#endif // ADDON_PCH