/* AbiSource Application Framework
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#ifndef XAP_PANGO_FONT_MANAGER_H
#define XAP_PANGO_FONT_MANAGER_H

#ifndef WITH_PANGO
#error Pango specific module included in non-Pango build !!!
#endif

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_abi-pango.h"
#include "ut_vector.h"

/*
   This is partly virtual class, an implementation is required for
   each Pango font backend, see ../ft2/xap_FT2PangoFontManager.[h,cpp]
*/

class XAP_PangoFontManager
{
 public:
	XAP_PangoFontManager(PangoContext *  pContext);
	virtual ~XAP_PangoFontManager();

	virtual PangoFont *    findFont(PangoFontDescription * pDescription);

	virtual PangoFont *    findFont(const char* pszFontFamily,
									const char* pszFontStyle,
									const char* pszFontVariant,
									const char* pszFontWeight,
									const char* pszFontStretch,
									const char* pszFontSize);

	virtual UT_uint32      getAvailableFontFamiliesCount() const;
	virtual const char*    getNthAvailableFontFamily(UT_uint32 n) const;
	virtual const UT_Vector * getAllFonts() const
		{
			return const_cast<const UT_Vector *>(&m_vAllocatedFonts);
		};


	virtual const PangoContext * getPangoContext() const
		{
			return const_cast<const PangoContext *>(m_pPangoContext);
		};

 private:
	UT_Vector       m_vAllocatedFonts;
    UT_Vector       m_vFontFamilies;
	PangoContext *  m_pPangoContext; // must not free this, it belongs to the graphics class
};


#endif
