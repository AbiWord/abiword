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
									  IEFileType out_ieft)
		: IE_MailMerge::IE_MailMerge_Listener (), m_doc (pDoc),
		  m_szFile(szOut), m_count(0), m_ieft(out_ieft)
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

			if (UT_OK == m_doc->saveAs (out_file.utf8_str(), m_ieft))
				return true;
			return false;
		}
	
private:
	PD_Document *m_doc;
	UT_UTF8String m_szFile;
	UT_uint32 m_count;
	IEFileType m_ieft;
};

class Print_MailMerge_Listener : public IE_MailMerge::IE_MailMerge_Listener
{
public:

	explicit Print_MailMerge_Listener (PD_Document * pd,
									   GR_GraphicsFactory & factory,
									   const UT_UTF8String & szFile)
		: IE_MailMerge::IE_MailMerge_Listener (), m_doc (pd),
		  m_szFile(szFile), m_factory(factory)
		{
		}

	virtual ~Print_MailMerge_Listener ()
		{
		}
		
	virtual PD_Document* getMergeDocument () const
		{
			return m_doc;
		}
	
	virtual bool fireUpdate () 
		{
			GR_Graphics *pGraphics = m_factory.getGraphics();

			FL_DocLayout *pDocLayout = new FL_DocLayout(m_doc,pGraphics);
			FV_View printView(XAP_App::getApp(),0,pDocLayout);
			pDocLayout->setView (&printView);
			pDocLayout->fillLayouts();
			pDocLayout->formatAll();
			
#ifdef XP_UNIX_TARGET_GTK
			PS_Graphics *psGr = static_cast<PS_Graphics*>(pGraphics);
			psGr->setColorSpace(GR_Graphics::GR_COLORSPACE_COLOR);
			psGr->setPageSize(printView.getPageSize().getPredefinedName());
#endif
			
			s_actuallyPrint (m_doc, pGraphics, 
							 &printView, m_szFile.utf8_str(), 
							 1, true, 
							 pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
							 1, pDocLayout->countPages());
			
			DELETEP(pDocLayout);
			
			// sure, we'll process more data if it exists
			return true;
		}
	
private:
	PD_Document *m_doc;
	UT_UTF8String m_szFile;

	GR_GraphicsFactory & m_factory;
};

static void handleMerge(const char * szMailMergeFile,
						IE_MailMerge::IE_MailMerge_Listener & listener){
	IE_MailMerge * pie = NULL;
	UT_Error errorCode = IE_MailMerge::constructMerger(szMailMergeFile, IEMT_Unknown, &pie);
	if (!errorCode)
	{
		pie->setListener (&listener);
		pie->mergeFile (szMailMergeFile);
		DELETEP(pie);
	}
}

/////////////////////////////////////////////////////////////////

void AP_Convert::convertTo(const char * szSourceFilename,
						   IEFileType sourceFormat,
						   const char * szTargetFilename,
						   IEFileType targetFormat)
{
	UT_Error error = UT_OK;

	PD_Document * pNewDoc = new PD_Document(XAP_App::getApp());
	UT_return_if_fail(pNewDoc);

	error = pNewDoc->readFromFile(szSourceFilename, sourceFormat);

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
		
		return;
	}

	if (m_mergeSource.size()) {
		IE_MailMerge::IE_MailMerge_Listener * listener = new Save_MailMerge_Listener (pNewDoc, szTargetFilename, targetFormat);
		handleMerge (m_mergeSource.utf8_str(), *listener);
		DELETEP(listener);
	} else {
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
			break;
		}
	}

	UNREFP(pNewDoc);
}

void AP_Convert::convertTo(const char * szFilename, const char * szTargetSuffixOrFilename)
{
  UT_return_if_fail(szTargetSuffixOrFilename);
  UT_return_if_fail(strlen(szTargetSuffixOrFilename)>0);

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

  convertTo(szFilename, IEFT_Unknown, file.c_str(), ieft);
}

void AP_Convert::setVerbose(int level)
{
	if ((level >= 0) && (level <= 2))
		m_iVerbose = level;
}

void AP_Convert::convertToPNG ( const char * szSourceFileName )
{
	// can't allocate src statically and then DELETEP.
	// note that src goes into dest (shouldn't that be documented?)
	// for consistency, we allocate UT_ByteBuf explicitly.
	UT_ByteBuf *src = new UT_ByteBuf();
	UT_ByteBuf *dest = NULL ;

	if (szSourceFileName && src->insertFromFile (0, szSourceFileName))
    {
		IE_ImpGraphic * pGraphic = NULL;

		if (UT_OK == IE_ImpGraphic::constructImporter (src,
													   IEGFT_Unknown,
													   &pGraphic))
		{
			if (UT_OK == pGraphic->convertGraphic (src, &dest))
			{
				// generate new filename with .png extension
				char * fileDup = UT_strdup (szSourceFileName);
				char * tmp = strrchr(fileDup, '.');
				if (tmp != NULL)
					*tmp = '\0';

				UT_String szDestFileName (fileDup);
				szDestFileName += ".png";

				FREEP(fileDup);

				if ( dest->writeToFile ( szDestFileName.c_str() ) )
				{
					// success
					DELETEP(dest);
					DELETEP(pGraphic);
					return;
				}
			}
		}

		DELETEP (pGraphic);
    }

	// failure
	DELETEP (dest);

	printf ("Conversion to PNG failed\n");
}

void AP_Convert::print(const char * szFile, GR_GraphicsFactory & pFactory)
{
	// get the current document
	PD_Document *pDoc = new PD_Document(XAP_App::getApp());
	pDoc->readFromFile(szFile, IEFT_Unknown);

	if (m_mergeSource.size()){
		IE_MailMerge::IE_MailMerge_Listener * listener = new Print_MailMerge_Listener(pDoc, pFactory, szFile);
		
		handleMerge (m_mergeSource.utf8_str(), *listener);
		DELETEP(listener);
	} else {
							 
		GR_Graphics *pGraphics = pFactory.getGraphics ();
		
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
		
		s_actuallyPrint (pDoc, pGraphics, 
						 &printView, szFile, 
						 1, true, 
						 pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
						 1, pDocLayout->countPages());
		
		DELETEP(pDocLayout);
		DELETEP(pGraphics);
	}

	UNREFP(pDoc);
}
