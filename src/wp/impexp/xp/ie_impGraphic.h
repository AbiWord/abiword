/* AbiWord
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


#ifndef IE_IMPGRAPHIC_H
#define IE_IMPGRAPHIC_H

#include "ut_types.h"
#include "ie_types.h"

class FG_Graphic;
class UT_ByteBuf;

//
// IE_ImpGraphic defines the abstract base class for graphic file importers.
// 
// Subclasses which load raster files should generally convert to a
// PNG format image and construct a FG_GraphicRaster.  Subclasses
// which load vector files should generally convert to a SVG format
// and construct a FG_GraphicVector.
//
// That way, the only formats that the layout and display code need to know
// about are PNG and SVG.  The code to handle other formats should be isolated
// in the impexp package.
//
class IE_ImpGraphic
{
public:

	// constructs an importer of the right type based upon
	// either the filename or sniffing the file.  caller is
	// responsible for destroying the importer when finished
	// with it.

	static IEGraphicFileType	fileTypeForSuffix(const char * szSuffix);
	static IEGraphicFileType	fileTypeForContents(const char * szBuf, UT_uint32 iNumbytes);
	
	static bool		enumerateDlgLabels(UT_uint32 ndx,
										   const char ** pszDesc,
										   const char ** pszSuffixList,
										   IEGraphicFileType * ft);
	static UT_uint32	getImporterCount(void);

	static UT_Error		constructImporter(const char * szFilename,
										  IEGraphicFileType ft,
										  IE_ImpGraphic **ppieg);

	virtual ~IE_ImpGraphic() {}

	//  Note subclassers:  ownership of pBB is passes here, so
	//  free pBB if you don't need it.
    	virtual UT_Error	importGraphic(UT_ByteBuf* pBB, 
					      FG_Graphic ** ppfg) = 0;

 	virtual UT_Error	importGraphic(const char * szFilename,
					      FG_Graphic ** ppfg);

	virtual UT_Error	convertGraphic(UT_ByteBuf* pBB,
					       UT_ByteBuf** ppBB) = 0;

private:
};

#endif /* IE_IMP_H */
