/* AbiSource Program Utilities
 * Copyright (C) 2001 Tomas Frydrych
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

#ifndef UTCONTEXTGLYPH_H
#define UTCONTEXTGLYPH_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_misc.h"

/* #include "xap_App.h" */
/* #include "xap_Prefs.h" */

class UT_StringPtrMap;

class XAP_App;
class XAP_Prefs;

#define CONTEXT_BUFF_SIZE 5

struct Letter
{
	UT_UCSChar code_low;
	UT_UCSChar code_high;
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

class UT_contextGlyph
{
public:
	UT_contextGlyph();
	UT_UCSChar getGlyph(const UT_UCSChar * code,
						const UT_UCSChar * prev,
						const UT_UCSChar * next,
						const XML_Char   * pLang) const;
		
	void renderString(const UT_UCSChar * src,
					  UT_UCSChar *dest,
					  UT_uint32 len,
					  const UT_UCSChar * prev,
					  const UT_UCSChar * next,
					  const XML_Char   * pLang) const;
		
	const Letter * smartQuote(UT_UCS4Char c,
				  const XML_Char * pLang) const;
	
		
		private:
	GlyphContext _evalGlyphContext( const UT_UCSChar * code,
									const UT_UCSChar * prev,
									const UT_UCSChar * next) const;

	static void _prefsListener(	XAP_App *pApp,
								XAP_Prefs *pPrefs,
								UT_StringPtrMap *phChanges,
								void *data);
										
	static Letter		*s_pGlyphTable;
	static UCSRange 	*s_pIgnore;
	static Letter		*s_pLigature;
	static Letter		*s_pLigRev;
	static bool 		s_bInit;
	static UT_uint32	s_iGlyphTableSize;
	static bool         s_bSmartQuotes;
	static const XML_Char * s_pEN_US;
};
#endif
