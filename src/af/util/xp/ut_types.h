/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef UT_TYPES_H
#define UT_TYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <glib.h>

/*
 *  This macro allow using GNUC extension with -pedantic
 */
#if     __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 8)
#  define GNUC_EXTENSION __extension__
#else
#  define GNUC_EXTENSION
#endif

typedef guint8		UT_Byte;

typedef gunichar        UT_UCS4Char;
typedef guint16      UT_UCS2Char;
typedef gint32          UT_GrowBufElement;

/* NOTA BENE: UT_UCSChar is deprecated; all new code must use
   UT_UCS4Char instead !!! */
typedef UT_UCS4Char		UT_UCSChar;	/* Unicode */

typedef guint8       UT_uint8;
typedef gint8        UT_sint8;

typedef guint16		UT_uint16;
typedef gint16       UT_sint16;

typedef guint32		UT_uint32;
typedef gint32		    UT_sint32;

typedef guint64 UT_uint64;
typedef gint64 UT_sint64;

#ifdef _WIN64
typedef guint64 	UT_uintptr;
typedef gint64 	UT_sintptr;
#else
typedef unsigned long UT_uintptr;
typedef long        UT_sintptr;
#endif

/** use to mark variable as unused */
#define UT_UNUSED(x) (void)(x);

/** use to mark an argument as used in debug only
 *  otherwise equivalent to UT_UNUSED.
 *  If it is a local variable use UT_DebugOnly<> instead.
 */
#ifdef DEBUG
#define UT_DEBUG_ONLY_ARG(x)
#else
#define UT_DEBUG_ONLY_ARG(x) (void)(x);
#endif

/*!
 * Confidence heuristic datatype normalized to the range
 * [0,255] with 0 being least confident and 255 being the most confident
 */
typedef UT_uint8 UT_Confidence_t;

#define UT_CONFIDENCE_PERFECT 255
#define UT_CONFIDENCE_GOOD    170
#define UT_CONFIDENCE_SOSO    127
#define UT_CONFIDENCE_POOR     85
#define UT_CONFIDENCE_ZILCH     0

#include "ut_export.h" // ABI_EXPORT is defined in there.

#if __GNUC__
  #define ABI_NORETURN __attribute__((noreturn))
  #define ABI_PRINTF_FORMAT(f,a) __attribute__ ((format (printf, f, a)))
#else
  #define ABI_NORETURN
  #define ABI_PRINTF_FORMAT(f,a)
#endif

/* ABI_FAR_CALL: C function that we want to expose across plugin boundaries */
#define ABI_FAR_CALL extern "C" ABI_PLUGIN_EXPORT

#define _abi_callonce /* only call me once! */

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
typedef	UT_sint32		UT_Error;
#define	UT_OK			((UT_Error) 0)
#define	UT_ERROR            	((UT_Error) -1) 	/* VERY generic */
#define UT_OUTOFMEM		((UT_Error) -100)
#define UT_SAVE_WRITEERROR      ((UT_Error) -201)
#define UT_SAVE_NAMEERROR       ((UT_Error) -202)
#define UT_SAVE_EXPORTERROR     ((UT_Error) -203)
#define UT_EXTENSIONERROR       ((UT_Error) -204)
#define UT_SAVE_CANCELLED       ((UT_Error) -205)
#define UT_SAVE_OTHERERROR      ((UT_Error) -200) 	/* This should eventually dissapear. */
#define UT_IE_FILENOTFOUND      ((UT_Error) -301)
#define UT_IE_NOMEMORY          ((UT_Error) -302)
#define UT_IE_UNKNOWNTYPE       ((UT_Error) -303)
#define UT_IE_BOGUSDOCUMENT     ((UT_Error) -304)
#define UT_IE_COULDNOTOPEN      ((UT_Error) -305)
#define UT_IE_COULDNOTWRITE     ((UT_Error) -306)
#define UT_IE_FAKETYPE          ((UT_Error) -307)
#define UT_INVALIDFILENAME      ((UT_Error) -308)
#define UT_NOPIECETABLE         ((UT_Error) -309)
#define UT_IE_ADDLISTENERERROR  ((UT_Error) -310)
#define UT_IE_UNSUPTYPE         ((UT_Error) -311)
#define UT_IE_PROTECTED         ((UT_Error) -312)       // (pass) protected doc
#define UT_IE_SKIPINVALID       ((UT_Error) -313)       // (pass) protected doc
#define UT_IE_IMPORTERROR       ((UT_Error) -300) 	/* The general case */
#define UT_IE_IMPSTYLEUNSUPPORTED  ((UT_Error) -314)
#define UT_IE_XMLNOANGLEBRACKET    ((UT_Error) -360)
#define UT_IE_TRY_RECOVER          ((UT_Error) -350)    // try recovering the document. ie, we have
                                                        // imported something

#define UT_IS_IE_SUCCESS(x) (((x) == UT_OK) || ((x) == UT_IE_TRY_RECOVER))

ABI_EXPORT UT_Error UT_errnoToUTError (void);

/* defined in ut_misc.cpp */
ABI_EXPORT void * UT_calloc ( UT_uint32 nmemb, UT_uint32 size );

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

/* When objects (fields, etc) must be represented in unicode, use the
   BELL code and let UT_isWordDelimiter recognize it as a word
   character. See bug 223.  */
#define UCS_ABI_OBJECT	((UT_UCSChar)0x0007)

#define UCS_TAB			((UT_UCSChar)0x0009)
#define UCS_LF			((UT_UCSChar)0x000a)
#define UCS_VTAB		((UT_UCSChar)0x000b)
#define UCS_FF			((UT_UCSChar)0x000c)
#define UCS_CR			((UT_UCSChar)0x000d)
#define UCS_SPACE		((UT_UCSChar)0x0020)
#define UCS_NBSP		((UT_UCSChar)0x00a0)
#define UCS_PILCROW		((UT_UCSChar)0x00b6)
#define UCS_LINESEP		((UT_UCSChar)0x2028)			/* Unicode line separator */
#define UCS_PARASEP		((UT_UCSChar)0x2029)			/* Unicode paragraph separator */
#define UCS_BOM			((UT_UCSChar)0xFEFF)			/* Byte order mark */
#define UCS_REPLACECHAR	((UT_UCSChar)0xFFFD)
#define UCS_HYPHEN      ((UT_UCSChar)0x2010)
#define UCS_MINUS       ((UT_UCSChar)0x2d)

/* Note: the following are our interpretations, not Unicode's */
/* Note: use Unicode Private Use Area 0xE000 - 0xF8FF         */
/* Note: GB18030 mandates U+E000 - U+E765 for UDAs 1, 2 and 3 */
/* Note: BIG5-HKSCS uses U+E000 - U+F848                      */
/* Note: Please update UCS_ABICONTROL_START/END if more       */
/* Note: special values are added.  We need to watch out for  */
/* Note: them during import                                   */
#define UCS_ABICONTROL_START	(UCS_FIELDSTART)
#define UCS_FIELDSTART		((UT_UCSChar)0xF850)
#define UCS_FIELDEND		((UT_UCSChar)0xF851)
#define UCS_BOOKMARKSTART	((UT_UCSChar)0xF852)
#define UCS_BOOKMARKEND		((UT_UCSChar)0xF853)
#define UCS_LIGATURE_PLACEHOLDER ((UT_UCS4Char)0xF854)
#define UCS_ABICONTROL_END	(UCS_LIGATURE_PLACEHOLDER)


#if 1 /* try to use the unicode values for special chars */
#define UCS_EN_SPACE		((UT_UCSChar)0x2002)
#define UCS_EM_SPACE		((UT_UCSChar)0x2003)
#define UCS_EN_DASH		((UT_UCSChar)0x2013)
#define UCS_EM_DASH		((UT_UCSChar)0x2014)
#define UCS_BULLET		((UT_UCSChar)0x2022)
/* TODO Quote marks need to be localized - not hard-coded */
#define UCS_LQUOTE		((UT_UCSChar)0x2018)
#define UCS_RQUOTE		((UT_UCSChar)0x2019)
#define UCS_LDBLQUOTE		((UT_UCSChar)0x201c)
#define UCS_RDBLQUOTE		((UT_UCSChar)0x201d)

/* Note: the following is our interpretation, not Unicode's */
#define UCS_UNKPUNK 		((UT_UCSChar)0xFFFF)  /* "unknown punctuation" used with UT_isWordDelimiter() */

#else /* see bug 512 */

#define UCS_EN_SPACE		((UT_UCSChar)0x0020)
#define UCS_EM_SPACE		((UT_UCSChar)0x0020)
#define UCS_EN_DASH		((UT_UCSChar)0x002d)
#define UCS_EM_DASH		((UT_UCSChar)0x002d)
#define UCS_BULLET		((UT_UCSChar)0x0095)
#define UCS_LQUOTE		((UT_UCSChar)0x0027)
#define UCS_RQUOTE		((UT_UCSChar)0x0027)
#define UCS_LDBLQUOTE		((UT_UCSChar)0x0022)
#define UCS_RDBLQUOTE		((UT_UCSChar)0x0022)
#define UCS_UNKPUNK 		((UT_UCSChar)0x00FF)

#endif

/* direction markers */
#define UCS_LRM 0x200E
#define UCS_RLM 0x200F
#define UCS_LRE 0x202a
#define UCS_RLE 0x202b
#define UCS_PDF 0x202c
#define UCS_LRO 0x202d
#define UCS_RLO 0x202e

/*
** Some useful macros that we use throughout
*/

#define FREEP(p)		do { if (p) { g_free((void *)p); (p)=NULL; } } while (0)
#define DELETEP(p)		do { if (p) { delete(p); (p)=NULL; } } while (0)
#define DELETEPV(pa)	do { if (pa) { delete [] (pa); (pa)=NULL; } } while (0)
#define REPLACEP(p,q)		do { if (p) delete p; p = q; } while (0)
#define REFP(p)			((p)->ref(), (p))
#define UNREFP(p)		do { if (p) { (p)->unref(); (p)=NULL; } } while (0)
#define CLONEP(p,q)		do { FREEP(p); if (q && *q) p = g_strdup(q); } while (0)

#define E2B(err)		((err) == UT_OK)

/* This is a value from the private-use space of FriBidi */
#define FRIBIDI_TYPE_UNSET -1
#define FRIBIDI_TYPE_IGNORE -2

// this is maximum revision level; it is intentionally not defined as
// 0xffffffff to avoid problems with bad 64-bit compilers

#define PD_MAX_REVISION 0x0fffffff

#endif /* UT_TYPES_H */
