/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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


#ifndef IE_IMPGRAPHIC_WMF_H
#define IE_IMPGRAPHIC_WMF_H

#include "ut_bytebuf.h"

#include "ie_impGraphic.h"

class IE_ImpGraphicWMF_Sniffer : public IE_ImpGraphicSniffer
{
 public:
	virtual bool recognizeContents (const char * szBuf, 
					UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
				   const char ** szSuffixList,
				   IEGraphicFileType * ft);
	virtual UT_Error constructImporter (IE_ImpGraphic ** ppieg);
};

class IE_ImpGraphic_WMF : public IE_ImpGraphic
{
public:
    	virtual UT_Error	importGraphic(UT_ByteBuf* pBB, 
					      FG_Graphic ** ppfg);
    	virtual UT_Error	convertGraphic(UT_ByteBuf* pBB, 
					       UT_ByteBuf** ppBB);

private:
};

#endif /* IE_IMPGRAPHIC_WMF_H */
