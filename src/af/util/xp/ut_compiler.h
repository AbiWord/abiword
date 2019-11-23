

// a bunch of macro for compiler control.
// TODO FIXME, currently tied to gcc and clang.

#pragma once

#if defined(__clang__)
# define ABI_W_PUSH \
_Pragma("clang diagnostic push")
#else
# define ABI_W_PUSH \
_Pragma("GCC diagnostic push")
#endif

#define ABI_W_NO_CONST_QUAL \
ABI_W_PUSH \
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")

#define ABI_W_NO_DEPRECATED \
ABI_W_PUSH \
_Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")

#if defined(__clang__)
# define ABI_W_POP \
_Pragma("clang diagnostic pop")
#else
# define ABI_W_POP \
_Pragma("GCC diagnostic pop")
#endif
