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


#ifndef IE_IMP_GRAPHICASDOCUMENT_H
#define IE_IMP_GRAPHICASDOCUMENT_H

#include <stdio.h>
#include "ie_imp.h"
#include "ie_impGraphic.h"
class PD_Document;

// This creates a new empty document and inserts the specified image

class IE_Imp_GraphicAsDocument : public IE_Imp
{
public:
	IE_Imp_GraphicAsDocument(PD_Document * pDocument);
	~IE_Imp_GraphicAsDocument();

	virtual UT_Error	importFile(const char * szFilename);
   	void		pasteFromBuffer(PD_DocumentRange * pDocRange,
					unsigned char * pData, UT_uint32 lenData) {}
	void 		setGraphicImporter(IE_ImpGraphic* importer) 
	     { DELETEP(m_pGraphicImporter); m_pGraphicImporter = importer; }

protected:

	IE_ImpGraphic * m_pGraphicImporter;
};

#endif /* IE_IMP_GRAPHICASDOCUMENT_H */
