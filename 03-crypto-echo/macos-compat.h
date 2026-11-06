#ifndef MACOS_COMPAT_H
#define MACOS_COMPAT_H

#define encrypt macos_system_encrypt
#include <unistd.h>
#undef encrypt

#endif
