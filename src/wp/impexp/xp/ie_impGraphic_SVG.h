/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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


#ifndef IE_IMPGRAPHIC_SVG_H
#define IE_IMPGRAPHIC_SVG_H

#include "ie_impGraphic.h"

class IE_ImpGraphic_SVG : public IE_ImpGraphic
{
public:
	static bool		RecognizeSuffix(const char * szSuffix);
	static bool		RecognizeContents(const char * szBuf, UT_uint32 iNumbytes);
   	static bool		GetDlgLabels(const char ** pszDesc,
					     const char ** pszSuffixList,
					     IEGraphicFileType * ft);
	static bool 		SupportsFileType(IEGraphicFileType ft);
	static UT_Error		StaticConstructor(IE_ImpGraphic **ppieg);

        virtual UT_Error	importGraphic(UT_ByteBuf* pBB, 
					      FG_Graphic ** ppfg);
        virtual UT_Error	convertGraphic(UT_ByteBuf* pBB, 
					       UT_ByteBuf** ppBB);
};

#endif /* IE_IMPGRAPHIC_SVG_H */
