/* AbiSource Program Utilities
 * Copyright (C) 2001, 2003 Tomas Frydrych
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

#ifndef GRCONTEXTGLYPH_H
#define GRCONTEXTGLYPH_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_hash.h"
#include "ut_misc.h"
#include "ut_TextIterator.h"

class XAP_App;
class XAP_Prefs;

//#define GR_CG_GENERATION2

struct LigatureData
{
	UT_UCSChar code_low;
	UT_UCSChar code_high;
	UT_UCSChar initial;
	UT_UCSChar medial;
	UT_UCSChar final;
	UT_UCSChar alone;
};

struct LetterData
{
	UT_UCSChar code;
	UT_UCSChar initial;
	UT_UCSChar medial;
	UT_UCSChar final;
	UT_UCSChar alone;
};

struct UCSRange
{
	UT_UCSChar low;
	UT_UCSChar high;
};

enum GlyphContext {GC_ISOLATE,GC_INITIAL,GC_MEDIAL,GC_FINAL,GC_NOT_SET};

// the following enum defines return values for renderString(); the
// values are also used by our run classes to indicated what kind of
// processing needs to be done in light of recent operations on the runs
// 
// the values are set so as to allow us to cast to UT_uint32 and
// manipulate them using OR and AND, and their meaning is as follows:
//
// GRSR_BufferClean : used in run classes to reset dirtiness flags;
//                  NB: THIS VALUES MUST NOT BE RETURNED BY renderString();
//
// GRSR_None: the text contains characters that are neither context
//          sensitive nor susceptible to ligating
//
// GRSR_ContextSensitive: contains characters the appearance of which
//                      depends on their context
//
// GRSR_Ligatures: contains characters that are susceptible to ligating
//               NB: renderString() should return this value even if
//               no ligature replacement was carried out, but the text
//               contains a character that opens ligature sequence, e.g.,
//               if f+l is set to ligate to 'fl', all strings
//               containing 'f' are susceptible to ligating;
//               subsequent characters in ligature sequences should be
//               ignored.
//               
// GRSR_ContextSensitiveAndLigatures: GRSR_ContextSensitive | GRSR_Ligatures
//
// GRSR_Unknown: initial value for text in our runs, equivalent to
//             or-ing all possible text types
//
// GRSR_Error: an error returned during processing

enum GRShapingResult
{
	GRSR_BufferClean = 0x00,                  // clear all bits; see notes above !!!
	GRSR_None = 0x01,                         // bit 0 set
	GRSR_ContextSensitive = 0x02,             // bit 1 set
	GRSR_Ligatures = 0x04,                    // bit 2 set
	GRSR_ContextSensitiveAndLigatures = 0x06, // bit 1, 2 set
	GRSR_Unknown = 0xef,                      // bits 0-6 set, initial value for text in our runs
	GRSR_Error = 0xff                         // bits 0-7 set
};


class GR_ContextGlyph
{
public:
	GR_ContextGlyph();
	static void static_destructor();

	// at the moment we could make all the public functions static, but then we
	// would need to worry about the static data being initalised

#ifndef NO_BIDI_SUPPORT
	GRShapingResult renderString(UT_TextIterator & text,
								 UT_UCSChar      * dest,
								 UT_uint32         len,
								 const XML_Char  * pLang,
								 UT_BidiCharType   iDirection,
								 bool (*isGlyphAvailable)(UT_UCS4Char g, void * custom) = NULL,
								 const void * custom_param = NULL) const;
#else
#define renderString copyString
#endif
	
	GRShapingResult copyString(UT_TextIterator & text,
							   UT_UCSChar      * dest,
							   UT_uint32         len,
							   const XML_Char  * pLang,
							   UT_BidiCharType   iDirection,
							   bool (*isGlyphAvailable)(UT_UCS4Char g, void * custom) = NULL,
							   const void * custom_param = NULL) const;
	
	const LetterData * smartQuote(UT_UCS4Char      c,
								  const XML_Char * pLang) const;

	UT_UCS4Char       getSmartQuote(UT_TextIterator &t,
									const XML_Char   * pLang,
									bool (*isGlyphAvailable)(UT_UCS4Char g, void * custom)=NULL,
		                            void * custom_param = NULL) const;
									
#ifndef NO_BIDI_SUPPORT
	bool  isNotFirstInLigature(UT_UCS4Char c) const;
	bool  isNotContextSensitive(UT_UCS4Char c) const;
	bool  isNotJoiningWithNext(UT_UCS4Char c, UT_UCS4Char n, UT_UCS4Char p) const;
	bool  isNotJoiningWithPrev(UT_UCS4Char c, UT_UCS4Char n, UT_UCS4Char p) const;
#endif
	
private:
	GlyphContext _evalGlyphContext( UT_TextIterator & text, UT_sint32 offset = 0) const;
	
	static void _prefsListener(	XAP_App         * pApp,
								XAP_Prefs       * pPrefs,
								UT_StringPtrMap * phChanges,
								void            * data);

#ifndef NO_BIDI_SUPPORT
	static void _generateNoLigatureTable();
	static void _generateNoShapingTable();
#endif
	
	static UT_UCS4Char _remapGlyph(UT_UCS4Char g);

	static bool 		    s_bInit;
	static bool             s_bSmartQuotes;
	static const XML_Char * s_pEN_US;
	static UT_UCS4Char      s_cDefaultGlyph;
	static bool             s_bLatinLigatures;
};
#endif
