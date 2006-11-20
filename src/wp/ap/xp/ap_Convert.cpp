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
#include "xap_App.h"
#include "ie_exp.h"
#include "ie_imp.h"
#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_DrawArgs.h"
#include "gr_Graphics.h"
#include "fv_View.h"
#include "fl_BlockLayout.h"
#include "ap_EditMethods.h"

// needed for convertToPNG
#include "ie_impGraphic.h"
#include "ut_bytebuf.h"
#include "ie_mailmerge.h"

#ifdef XP_UNIX_TARGET_GTK
// needed for unix printing
#include "xap_UnixPSGraphics.h"
#endif

//////////////////////////////////////////////////////////////////

AP_Convert::AP_Convert(int inVerbose)
	: m_iVerbose(inVerbose)
{
}

AP_Convert::~AP_Convert(void)
{
}

void AP_Convert::setMergeSource (const char * source)
{
	m_mergeSource = source;
}

/////////////////////////////////////////////////////////////////

class Save_MailMerge_Listener : public IE_MailMerge::IE_MailMerge_Listener
{
public:
	
	explicit Save_MailMerge_Listener (PD_Document * pDoc,
									  const UT_UTF8String & szOut,
									  IEFileType out_ieft,
									  const UT_UTF8String & szExpProps)
		: IE_MailMerge::IE_MailMerge_Listener (), m_doc (pDoc),
		  m_szFile(szOut), m_count(0), m_ieft(out_ieft), m_expProps(szExpProps)
		{
		}

	virtual ~Save_MailMerge_Listener ()
		{
		}
		
	virtual PD_Document* getMergeDocument () const
		{
			return m_doc;
		}
	
	virtual bool fireUpdate () 
		{
			if (!m_doc)
				return false;

			UT_UTF8String out_file (UT_UTF8String_sprintf("%s-%d",
														  m_szFile.utf8_str(),
														  m_count++));

			if (UT_OK == m_doc->saveAs (out_file.utf8_str(), m_ieft, m_expProps.utf8_str()))
				return true;
			return false;
		}
	
private:
	PD_Document *m_doc;
	UT_UTF8String m_szFile;
	UT_uint32 m_count;
	IEFileType m_ieft;
	UT_UTF8String m_expProps;
};

class Print_MailMerge_Listener : public IE_MailMerge::IE_MailMerge_Listener
{
public:

	explicit Print_MailMerge_Listener (PD_Document * pd,
									   GR_Graphics * pGraphics,
									   const UT_UTF8String & szFile)
		: IE_MailMerge::IE_MailMerge_Listener (), m_doc (pd),
		  m_szFile(szFile), m_pGraphics(pGraphics), m_bPrintedFirstPage(false), m_iter(1)
		{
		}

	virtual ~Print_MailMerge_Listener ()
		{
			if (m_bPrintedFirstPage)
				m_pGraphics->endPrint();
		}
		
	virtual PD_Document* getMergeDocument () const
		{
			return m_doc;
		}
	
	virtual bool fireUpdate () 
		{
			FL_DocLayout *pDocLayout = new FL_DocLayout(m_doc,m_pGraphics);
			FV_View printView(XAP_App::getApp(),0,pDocLayout);
			//pDocLayout->setView (&printView);
			pDocLayout->fillLayouts();
			pDocLayout->formatAll();
			pDocLayout->recalculateTOCFields();			

#ifdef XP_UNIX_TARGET_GTK
			if (!m_bPrintedFirstPage) {
				PS_Graphics *psGr = static_cast<PS_Graphics*>(m_pGraphics);
				psGr->setColorSpace(GR_Graphics::GR_COLORSPACE_COLOR);
				psGr->setPageSize(printView.getPageSize().getPredefinedName());
			}
#endif
				
			if (!m_bPrintedFirstPage)
				if (m_pGraphics->startPrint())
					m_bPrintedFirstPage = true;
			

			if (m_bPrintedFirstPage) {

				dg_DrawArgs da;
				memset(&da, 0, sizeof(da));
				da.pG = m_pGraphics;
				
				for (UT_uint32 k = 1; (k <= pDocLayout->countPages()); k++)
				{
					UT_uint32 iHeight = pDocLayout->getHeight() / pDocLayout->countPages();
					m_pGraphics->m_iRasterPosition = (k-1)*iHeight;
					m_pGraphics->startPage(m_szFile.utf8_str(), m_iter++, printView.getPageSize().isPortrait(), pDocLayout->getWidth(), iHeight);
					printView.draw(k-1, &da);
				}
			}

			DELETEP(pDocLayout);
			
			// sure, we'll process more data if it exists
			return true;
		}
	
private:
	PD_Document *m_doc;
	UT_UTF8String m_szFile;

	GR_Graphics * m_pGraphics;

	bool m_bPrintedFirstPage;
	UT_uint32 m_iter;
};

static UT_Error handleMerge(const char * szMailMergeFile,
			    IE_MailMerge::IE_MailMerge_Listener & listener){
	IE_MailMerge * pie = NULL;
	UT_Error errorCode = IE_MailMerge::constructMerger(szMailMergeFile, IEMT_Unknown, &pie);
	if (!errorCode)
	{
		pie->setListener (&listener);
		errorCode = pie->mergeFile (szMailMergeFile);
		DELETEP(pie);
	}

	return errorCode;
}

/////////////////////////////////////////////////////////////////

bool AP_Convert::convertTo(const char * szSourceFilename,
			   IEFileType sourceFormat,
			   const char * szTargetFilename,
			   IEFileType targetFormat)
{
	UT_Error error = UT_OK;

	PD_Document * pNewDoc = new PD_Document(XAP_App::getApp());
	UT_return_val_if_fail(pNewDoc, false);

	char * uri = UT_go_shell_arg_to_uri (szSourceFilename);
	error = pNewDoc->readFromFile(uri, sourceFormat, m_impProps.utf8_str());
	g_free (uri);

	if (error != UT_OK) {
		switch (error) {
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
		return (error == UT_OK);
	}

	if (m_mergeSource.size()) {
		uri = UT_go_shell_arg_to_uri (szTargetFilename);
		IE_MailMerge::IE_MailMerge_Listener * listener = new Save_MailMerge_Listener (pNewDoc, uri, targetFormat, m_expProps);
		g_free(uri);

		uri = UT_go_shell_arg_to_uri (m_mergeSource.utf8_str());
		handleMerge (uri, *listener);
		g_free (uri);
		DELETEP(listener);
	} else {
		uri = UT_go_shell_arg_to_uri (szTargetFilename);
		error = pNewDoc->saveAs(uri, targetFormat, m_expProps.utf8_str());
		g_free(uri);

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
			break;
		}
	}

	UNREFP(pNewDoc);

	return (error == UT_OK);
}

bool AP_Convert::convertTo(const char * szFilename, const char * szTargetSuffixOrFilename)
{
  UT_return_val_if_fail(szTargetSuffixOrFilename, false);
  UT_return_val_if_fail(strlen(szTargetSuffixOrFilename)>0, false);

  UT_String file;
  IEFileType ieft = IEFT_Unknown;

  char *tmp = NULL;

  if(NULL != (tmp = strrchr(const_cast<char *>(szTargetSuffixOrFilename), '.')))
    {
      // found an extension. use that instead, else just use AbiWord native format
      if(strlen(tmp) > 1)
		  ieft = IE_Exp::fileTypeForSuffix(tmp);
      else
		  ieft = IE_Exp::fileTypeForSuffix(".abw");
      file = szTargetSuffixOrFilename;
    }
  else
    {
      char * fileDup = UT_strdup ( szFilename );
      
      UT_String ext(".");

      ext += szTargetSuffixOrFilename;
      ieft = IE_Exp::fileTypeForSuffix(ext.c_str());
      
      tmp = strrchr(fileDup, '.');
      if (tmp != NULL)
		  *tmp = '\0';

      file = fileDup;
      file += ext;
  
      FREEP(fileDup);
    }

  return convertTo(szFilename, IEFT_Unknown, file.c_str(), ieft);
}

void AP_Convert::setVerbose(int level)
{
	if ((level >= 0) && (level <= 2))
		m_iVerbose = level;
}

bool AP_Convert::print(const char * szFile, GR_Graphics * pGraphics, const char * szFileExtension)
{
	// get the current document
	PD_Document *pDoc = new PD_Document(XAP_App::getApp());
	UT_Error err;
	char * uri = UT_go_shell_arg_to_uri (szFile);

	if( !szFileExtension )
		err = pDoc->readFromFile(uri, IEFT_Unknown, m_impProps.utf8_str());
	else
		err = pDoc->readFromFile(uri, IE_Imp::fileTypeForSuffix(szFileExtension), m_impProps.utf8_str());
	g_free(uri);

	if( err != UT_OK)
	{
		fprintf(stderr, "AbiWord: Error importing file. [%s]  Could not print \n", szFile);
		UNREFP(pDoc);
		return false;
	}
	if (m_mergeSource.size()){
		IE_MailMerge::IE_MailMerge_Listener * listener = new Print_MailMerge_Listener(pDoc, pGraphics, szFile);

		uri = UT_go_shell_arg_to_uri (m_mergeSource.utf8_str());
		handleMerge (uri, *listener);
		g_free (uri);
		
		DELETEP(listener);
	} else {
		
		// create a new layout and view object for the doc
		FL_DocLayout *pDocLayout = new FL_DocLayout(pDoc,pGraphics);
		FV_View printView(XAP_App::getApp(),0,pDocLayout);
		pDocLayout->setView (&printView);
		pDocLayout->fillLayouts();
		pDocLayout->formatAll();
		
#ifdef XP_UNIX_TARGET_GTK
		PS_Graphics *psGr = static_cast<PS_Graphics*>(pGraphics);
		psGr->setColorSpace(GR_Graphics::GR_COLORSPACE_COLOR);
		psGr->setPageSize(printView.getPageSize().getPredefinedName());
#endif
		
		if(!s_actuallyPrint (pDoc, pGraphics, 
				     &printView, szFile, 
				     1, true, 
				     pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
				     pDocLayout->countPages(), 1))
		  err = UT_SAVE_WRITEERROR;
		
		DELETEP(pDocLayout);
	}

	UNREFP(pDoc);

	return (err == UT_OK);
}


bool AP_Convert::printFirstPage(GR_Graphics * pGraphics,PD_Document * pDoc)
{
		// create a new layout and view object for the doc

	FL_DocLayout *pDocLayout = new FL_DocLayout(pDoc,pGraphics);
	FV_View printView(XAP_App::getApp(),0,pDocLayout);
	pDocLayout->setView (&printView);
	pDocLayout->fillLayouts();
	pDocLayout->formatAll();
		
#ifdef XP_UNIX_TARGET_GTK
	PS_Graphics *psGr = static_cast<PS_Graphics*>(pGraphics);
	psGr->setColorSpace(GR_Graphics::GR_COLORSPACE_COLOR);
	psGr->setPageSize(printView.getPageSize().getPredefinedName());
#endif
		
	bool success = s_actuallyPrint (pDoc, pGraphics, 
					&printView, "pngThumb", 
					1, true, 
					pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
					1, 1);
		
	DELETEP(pDocLayout);

	return success;
}
