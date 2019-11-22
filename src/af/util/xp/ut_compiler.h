

// a bunch of macro for compiler control.
// TODO FIXME, currently tied to gcc.

#pragma once

#if defined(__clang__)
#define ABI_W_PUSH \
_Pragma("clang diagnostic pop")
#else
#define ABI_W_PUSH \
_Pragma("GCC diagnostic pop")
#endif

#define ABI_W_NO_CONST_QUAL \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")

#define ABI_W_NO_DEPRECATED \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#if defined(__clang__)
#define ABI_W_POP \
_Pragma("clang diagnostic pop")
#else
#define ABI_W_POP \
_Pragma("GCC diagnostic pop")
#endif
