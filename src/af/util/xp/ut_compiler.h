

// a bunch of macro for compiler control.
// TODO FIXME, currently tied to gcc.

#pragma once

#define ABI_W_NO_CONST_QUAL \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wcast-qual\"")

#define ABI_W_POP
_Pragma("GCC diagnostic pop")
