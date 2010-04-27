/* gc-pbkdf2-sha1.h --- Password-Based Key Derivation Function a'la PKCS#5
   Copyright (C) 2002, 2003, 2004, 2005, 2006, 2009, 2010 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/* Written by Simon Josefsson. */

#ifndef __PBKDF2_SHA1__
#define __PBKDF2_SHA1__

# ifdef __cplusplus
extern "C" {
# endif

extern int pbkdf2_sha1 (const char *P, size_t Plen,
                const char *S, size_t Slen,
                unsigned int c,
                char *DK, size_t dkLen);

# ifdef __cplusplus
}
# endif


#endif /* __PBKDF2_SHA1__ */