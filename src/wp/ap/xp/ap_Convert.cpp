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

#include <stdio.h>
#include <string.h>

#include "ap_Convert.h"
#include "ie_exp.h"
#include "ie_imp.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Frame.h"
#include "gr_Graphics.h"
#include "fv_View.h"
#include "fl_BlockLayout.h"

class XAP_App;

//////////////////////////////////////////////////////////////////

AP_Convert::AP_Convert(XAP_App *pApp)
  : m_iVerbose(1), m_pApp(pApp)
{
}

AP_Convert::~AP_Convert(void)
{
}

/////////////////////////////////////////////////////////////////

void AP_Convert::convertTo(const char * szSourceFilename,
							IEFileType sourceFormat,
							const char * szTargetFilename,
							IEFileType targetFormat)
{
	PD_Document * pNewDoc = new PD_Document(getApp());
	UT_Error error;
	UT_ASSERT(pNewDoc);

	if (m_iVerbose > 1)
		printf("AbiWord: [%s] -> [%s]\tStarting conversion...\n", szSourceFilename, szTargetFilename);

	error = pNewDoc->readFromFile(szSourceFilename, sourceFormat);

	switch (error) {
	case UT_OK:
		error = pNewDoc->saveAs(szTargetFilename, targetFormat);
		
		switch (error) {
		case UT_OK:
			if (m_iVerbose > 1)
				printf("AbiWord: [%s] -> [%s]\tConversion ok!\n", szSourceFilename, szTargetFilename);
			break;
		case UT_SAVE_EXPORTERROR:
			if (m_iVerbose > 0)
				fprintf(stderr, "AbiWord: Uch! Are you sure that you've specified a valid exporter?\n");
			break;
		case UT_SAVE_WRITEERROR:
			if (m_iVerbose > 0)
				fprintf(stderr, "AbiWord: Uch! Could not write the file [%s]\n", szTargetFilename);
			break;
		default:
			if (m_iVerbose > 0)
				fprintf(stderr, "AbiWord: could not write the file [%s]\n", szTargetFilename);
		}

		break;
	case UT_INVALIDFILENAME:
		if (m_iVerbose > 0)
			fprintf(stderr, "AbiWord: [%s] is not a valid file name.\n", szSourceFilename);
		break;
	case UT_IE_NOMEMORY:
		if (m_iVerbose > 0)
			fprintf(stderr, "AbiWord: Arrrgh... I don't have enough memory!\n");
		break;
	case UT_NOPIECETABLE:
		// TODO
	default:
		if (m_iVerbose > 0)
			fprintf(stderr, "AbiWord: could not open the file [%s]\n", szSourceFilename);
	}


	UNREFP(pNewDoc);
}

void AP_Convert::convertTo(const char * szFilename, const char * szTargetSuffix)
{
  UT_String ext("."), file;
  IEFileType ieft = 0;
  char *tmp = NULL;
  char * fileDup = UT_strdup ( szFilename );

  ext += szTargetSuffix;
  ieft = IE_Exp::fileTypeForSuffix(ext.c_str());

  tmp = strrchr(fileDup, '.');
  if (tmp != NULL)
    *tmp = '\0';
  
  file = fileDup;
  file += ext;
  
  FREEP(fileDup);

  convertTo(szFilename, IEFT_Unknown, file.c_str(), ieft);
}

void AP_Convert::convertTo(const char * szFilename, const char * szSourceSuffix, const char * szTargetSuffix)
{
  UT_String ext("."), sourceExt("."), file;
  IEFileType ieft = 0;
  IEFileType sourceIeft = 0;
  char *tmp = NULL;
  char * fileDup = UT_strdup ( szFilename );

  ext += szTargetSuffix;
  sourceExt += szSourceSuffix;

  ieft = IE_Exp::fileTypeForSuffix(ext.c_str());
  sourceIeft = IE_Imp::fileTypeForSuffix(sourceExt.c_str());

  tmp = strrchr(fileDup, '.');
  if (tmp != NULL)
    *tmp = '\0';

  file = fileDup;
  file += ext;
  
  FREEP( fileDup );

  convertTo(szFilename, sourceIeft, file.c_str(), ieft);
}

void AP_Convert::setVerbose(int level)
{
	if ((level >= 0) && (level <= 2))
		m_iVerbose = level;
}

void AP_Convert::print(XAP_Frame * pFrame, GR_Graphics * pGraphics)
{
  UT_DEBUGMSG(("DOM: AP_Convert::print\n"));

  UT_ASSERT(pFrame);
  UT_ASSERT(pGraphics);

  // get the current document
  PD_Document * doc = static_cast<PD_Document*>(pFrame->getCurrentDoc ());

  // create a new layout and view object for the doc
  FL_DocLayout * pDocLayout = new FL_DocLayout(doc,pGraphics);
  pDocLayout->formatAll();
  FV_View * pPrintView = new FV_View(pFrame->getApp(),pFrame,pDocLayout);

  // get the width, height, orient
  UT_sint32 iWidth = pDocLayout->getWidth();
  UT_sint32 iHeight = pDocLayout->getHeight() / pDocLayout->countPages();

  bool orient = pPrintView->getPageSize().isPortrait();
  pGraphics->setPortrait (orient);  

  // setup the drawing args
  dg_DrawArgs da;
  memset(&da, 0, sizeof(da));
  da.pG = NULL;

  if(pGraphics->startPrint())
    {
      // iterate over the pages, printing each one
      for (UT_uint32 k = 1; (k <= pDocLayout->countPages()); k++)
	{
	  pGraphics->m_iRasterPosition = (k-1)*iHeight;
	  pGraphics->startPage(doc->getFileName(), k, orient, iWidth, iHeight);
	  pPrintView->draw(k-1, &da);
	}
      pGraphics->endPrint();
    }
}
