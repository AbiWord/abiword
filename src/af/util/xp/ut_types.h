/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


#ifndef UT_TYPES_H
#define UT_TYPES_H

#include <stdlib.h>
#ifdef HAVE_GNOME
#include <glib.h>
#endif

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#ifdef __cplusplus
#define UT_BEGIN_EXTERN_C		extern "C" {
#define UT_END_EXTERN_C			}
#else
#define UT_BEGIN_EXTERN_C
#define UT_END_EXTERN_C
#endif

#ifdef HAVE_LIBXML2
#include <libxml/tree.h>
#define XML_Char xmlChar
#endif

typedef		unsigned char		UT_Byte;
typedef		unsigned short		UT_UCSChar;	/* Unicode */

typedef		unsigned short		UT_uint16;
typedef		unsigned int		UT_uint32;
typedef		signed int			UT_sint32;


/*
	TODO we currently use plain old C 'int' all over the place.
	For many applications, this is inappropriate, and we should change
	them to UT_sint32.  Also, there are places where we are
	using it as a bool, and there are places where we are using it as
	an error code.
*/

typedef		unsigned char		UT_Bool;
#define		UT_TRUE				((UT_Bool) 1)
#define		UT_FALSE			((UT_Bool) 0)

/*
	UT_Error should be used far more than it is.  Any function
	which reasonably could fail at runtime for anything other than
	a coding error or bug should return an error code.  Error codes
	should be propogated properly.

	Addendum: 1-23-99
	If you have any problems with or suggestions for error codes, 
	please send them to Sam Tobin-Hochstadt (sytobinh@uchicago.edu).
	I am the person that has worked the most with them. 
*/
typedef		UT_sint32				UT_Error;
#define		UT_OK					((UT_Error) 0)
#define		UT_ERROR            	((UT_Error) -1) 	/* VERY generic */
#define		UT_OUTOFMEM				((UT_Error) -100)
#define     UT_SAVE_WRITEERROR      ((UT_Error) -201)
#define     UT_SAVE_NAMEERROR       ((UT_Error) -202)
#define     UT_SAVE_EXPORTERROR     ((UT_Error) -203)
#define     UT_EXTENSIONERROR       ((UT_Error) -204)
#define     UT_SAVE_OTHERERROR      ((UT_Error) -200) 	/* This should eventually dissapear. */
#define     UT_IE_FILENOTFOUND      ((UT_Error) -301)
#define     UT_IE_NOMEMORY          ((UT_Error) -302)
#define     UT_IE_UNKNOWNTYPE       ((UT_Error) -303)
#define     UT_IE_BOGUSDOCUMENT     ((UT_Error) -304)
#define     UT_IE_COULDNOTOPEN      ((UT_Error) -305)
#define     UT_IE_COULDNOTWRITE     ((UT_Error) -306)
#define     UT_IE_FAKETYPE          ((UT_Error) -307)
#define     UT_INVALIDFILENAME      ((UT_Error) -308)
#define     UT_NOPIECETABLE         ((UT_Error) -309)
#define	    UT_IE_ADDLISTENERERROR  ((UT_Error) -310)
#define     UT_IE_UNSUPTYPE         ((UT_Error) -311)
#define     UT_IE_IMPORTERROR       ((UT_Error) -300) 	/* The general case */


/* 
	The MSVC debug runtime library can track leaks back to the 
	original allocation via the following black magic.
*/
#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
#include <crtdbg.h>
#define UT_DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new UT_DEBUG_NEW
#endif /* _MSC_VER && _DEBUG && _CRTDBG_MAP_ALLOC */


/* Unicode character constants.  Try to use these rather than
** decimal or hex constants throughout the code.  See also bug
** 512.
*/

#define UCS_TAB				((UT_UCSChar)0x0009)
#define UCS_LF				((UT_UCSChar)0x000a)
#define UCS_VTAB			((UT_UCSChar)0x000b)
#define UCS_FF				((UT_UCSChar)0x000c)
#define UCS_CR				((UT_UCSChar)0x000d)
#define UCS_SPACE			((UT_UCSChar)0x0020)
#define UCS_NBSP			((UT_UCSChar)0x00a0)
#define UCS_FIELDSTART                  ((UT_UCSChar)0xFFFE)
#define UCS_FIELDEND                    ((UT_UCSChar)0xFFFD)

#if 1 /* try to use the unicode values for special chars */
#define UCS_EN_SPACE		((UT_UCSChar)0x2002)
#define UCS_EM_SPACE		((UT_UCSChar)0x2003)
#define UCS_EN_DASH			((UT_UCSChar)0x2013)
#define UCS_EM_DASH			((UT_UCSChar)0x2014)
#define UCS_BULLET			((UT_UCSChar)0x2022)
#define UCS_LQUOTE			((UT_UCSChar)0x2018)
#define UCS_RQUOTE			((UT_UCSChar)0x2019)
#define UCS_LDBLQUOTE		((UT_UCSChar)0x201c)
#define UCS_RDBLQUOTE		((UT_UCSChar)0x201d)
#define UCS_UNKPUNK 		((UT_UCSChar)0xFFFF)  /* "unknown punctuation" used with UT_isWordDelimiter() */

#else /* see bug 512 */

#define UCS_EN_SPACE		((UT_UCSChar)0x0020)
#define UCS_EM_SPACE		((UT_UCSChar)0x0020)
#define UCS_EN_DASH			((UT_UCSChar)0x002d)
#define UCS_EM_DASH			((UT_UCSChar)0x002d)
#define UCS_BULLET			((UT_UCSChar)0x0095)
#define UCS_LQUOTE			((UT_UCSChar)0x0027)
#define UCS_RQUOTE			((UT_UCSChar)0x0027)
#define UCS_LDBLQUOTE		((UT_UCSChar)0x0022)
#define UCS_RDBLQUOTE		((UT_UCSChar)0x0022)
#define UCS_UNKPUNK 		((UT_UCSChar)0x00FF)

#endif

/*
** Some useful macros that we use throughout
*/

#define FREEP(p)		do { if (p) free((void *)p); (p)=NULL; } while (0)
#define DELETEP(p)		do { if (p) delete(p); (p)=NULL; } while (0)
#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define REFP(p)			((p)->ref(), (p))
#define UNREFP(p)		do { if (p) (p)->unref(); (p)=NULL; } while (0)
#define CLONEP(p,q)		do { FREEP(p); if (q && *q) UT_cloneString(p,q); } while (0)

#define NrElements(a)	((sizeof(a)/sizeof(a[0])))
#define MyMax(a,b)		(((a)>(b)) ? (a) : (b))
#define MyMin(a,b)		(((a)<(b)) ? (a) : (b))

#define UT_UNUSED(v)	do { (v)=(v); } while (0)

#define E2B(err)		((err) == UT_OK)


/* UGLY UGLY Iconv hack for win32.  I will suffer in the afterlife for this */

#if defined(WIN32) || defined(__QNXNTO__)
#define ICONV_CONST const
#else
#define ICONV_CONST
#endif

#endif /* UT_TYPES_H */
