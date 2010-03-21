/* gc-pbkdf2-sha1.c --- Password-Based Key Derivation Function a'la PKCS#5
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

// #include <config.h>

// #include "gc.h"

#include "hmac.h"

#include <stdlib.h>
#include <string.h>

/* Implement PKCS#5 PBKDF2 as per RFC 2898.  The PRF to use is hard
   coded to be HMAC-SHA1.  Inputs are the password P of length PLEN,
   the salt S of length SLEN, the iteration counter C (> 0), and the
   desired derived output length DKLEN.  Output buffer is DK which
   must have room for at least DKLEN octets.  The output buffer will
   be filled with the derived data.  */
/*Gc_rc*/ int
/*gc_*/pbkdf2_sha1 (const char *P, size_t Plen,
                const char *S, size_t Slen,
                unsigned int c,
                char *DK, size_t dkLen)
{
  unsigned int hLen = 20;
  char U[20];
  char T[20];
  unsigned int u;
  unsigned int l;
  unsigned int r;
  unsigned int i;
  unsigned int k;
  int rc;
  char *tmp;
  size_t tmplen = Slen + 4;

  if (c == 0)
    return /*GC_PKCS5_INVALID_ITERATION_COUNT*/ -1;

  if (dkLen == 0)
    return /*GC_PKCS5_INVALID_DERIVED_KEY_LENGTH*/ -1;

  if (dkLen > 4294967295U)
    return /*GC_PKCS5_DERIVED_KEY_TOO_LONG*/ -1;

  l = ((dkLen - 1) / hLen) + 1;
  r = dkLen - (l - 1) * hLen;

  tmp = (char*)malloc (tmplen);
  if (tmp == NULL)
    return /*GC_MALLOC_ERROR*/ -1;

  memcpy (tmp, S, Slen);

  for (i = 1; i <= l; i++)
    {
      memset (T, 0, hLen);

      for (u = 1; u <= c; u++)
        {
          if (u == 1)
            {
              tmp[Slen + 0] = (i & 0xff000000) >> 24;
              tmp[Slen + 1] = (i & 0x00ff0000) >> 16;
              tmp[Slen + 2] = (i & 0x0000ff00) >> 8;
              tmp[Slen + 3] = (i & 0x000000ff) >> 0;

              rc = /*gc_*/hmac_sha1 (P, Plen, tmp, tmplen, U);
            }
          else
            rc = /*gc_*/hmac_sha1 (P, Plen, U, hLen, U);

          if (rc != /*GC_OK*/ 0)
            {
              free (tmp);
              return rc;
            }

          for (k = 0; k < hLen; k++)
            T[k] ^= U[k];
        }

      memcpy (DK + (i - 1) * hLen, T, i == l ? r : hLen);
    }

  free (tmp);

  return /*GC_OK*/ 0;
}
