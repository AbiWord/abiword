/*
   Support - Provides some big and little endian abstraction functions.
   Copyright (C) 1998  Caolan McNamara

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */
/*
  Caolan McNamara <Caolan.McNamara@ul.ie>
  Real Life: Caolan McNamara           *  Doing: MSc in HCI
  Work: Caolan.McNamara@ul.ie          *  Phone: +353-61-202699
  URL: http://www.csn.ul.ie/~caolan    *  Sig: an oblique strategy
 */



#define fil_sreadU8 sgetc
#define fil_sreadU16 sread_16ubit
#define fil_sreadU32 sread_32ubit


#define test(t,retval,func) \
	{ \
		if (!(t)) { \
			func; \
			return retval; \
		} \
	}


#ifdef VERBOSE
#define verbose(s) { printf (s); printf ("\n"); }
#else
#define verbose(s)
#endif

#ifdef VERBOSE
#define verboseU32(expr) { printf (#expr " = 0x%08lx\n", expr); }
#else
#define verboseU32(expr)
#endif

#ifdef VERBOSE
#define verboseS(expr) { printf (#expr " = %s\n", expr); }
#else
#define verboseS(expr)
#endif

#ifdef VERBOSE
#define verboseU32Array(array,len) \
	{ \
		U32 temp; \
		for (temp = 0; temp <= len; temp++) \
			printf (#array "[%lu] = 0x%08lx\n", temp, array [temp]); \
	}
#else
#define verboseU32Array(array,len)
#endif

#ifdef VERBOSE
#ifdef SHOWCHAR
#define verboseU8Array(arr,len,sblock) \
        { \
                U32 temp1, temp2; \
                for (temp1 = 0; temp1 < len; temp1++) \
                { \
                printf ("\n***%08lx***\n", temp1); \
                for (temp2 = 0; temp2 < sblock; temp2++) \
		{ \
	                printf ("%02x ", *(arr + (temp1 * 0x0200 + temp2))); \
			if (!((temp2+1) % 0x10)) printf ("\n"); \
		} \
                } \
        }
#else /* show caracters */
#define verboseU8Array(arr,len,sblock) \
        { \
                U32 temp1, temp2; \
                for (temp1 = 0; temp1 < len; temp1++) \
                { \
                printf ("\n***%08lx***\n", temp1); \
                for (temp2 = 0; temp2 < sblock; temp2++) \
		{ \
			if (isprint (*(arr + (temp1 * 0x0200 + temp2)))) \
	                        printf ("%c ", *(arr + (temp1 * 0x0200 + temp2))); \
			else \
				printf ("- "); \
			if (!(temp2 % 0xF)) printf ("\n"); \
		} \
                } \
        }
#endif
#else
#define verboseU8Array(arr,len,sblock)
#endif
