#ifndef STUBS_OS_H
#define STUBS_OS_H

#include "mock_engine.h"
#include "cx.h"

void os_perso_derive_node_bip32(cx_curve_t curve,
                                const unsigned int *path,
                                unsigned int pathLength,
                                unsigned char *privateKey,
                                unsigned char *chain);

#endif  // STUBS_OS_H
