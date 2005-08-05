/* AbiWord
 * Copyright (C) 1998-2005 AbiSource, Inc.
 * Copyright (C) 2005 Dom Lachowicz <cinamod@hotmail.com>
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

#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_exp_PDF.h"
#include "pd_Document.h"
#include "xap_UnixGnomePrintGraphics.h"
#include "fv_View.h"
#include "ap_EditMethods.h"

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT IE_Exp_PDF : public IE_Exp
{
public:
  typedef enum
    {
      BACKEND_PS,
      BACKEND_PDF
    } Format;

  IE_Exp_PDF::Format mFormat;

  IE_Exp_PDF(PD_Document * pDocument, IE_Exp_PDF::Format format)
    : IE_Exp(pDocument), mFormat(format)
  {
  }

  virtual ~IE_Exp_PDF()
  {

  }

  virtual UT_Error _writeDocument(void)
  {
    UT_Error exit_status = UT_ERROR;

    GnomePrintJob *job = NULL;
    GnomePrintConfig *config = NULL;

    job = gnome_print_job_new (NULL);
    if(!job)
      goto exit_writeDocument;

    config = gnome_print_job_get_config (job);
    if(!config)
      goto exit_writeDocument;

    if(mFormat == BACKEND_PS) {
      if (!gnome_print_config_set (config, (const guchar *)"Printer", (const guchar *)"GENERIC")) {
	goto exit_writeDocument;
      }
    }

    if(mFormat == BACKEND_PDF) {
      if (!gnome_print_config_set (config, (const guchar *)"Printer", (const guchar *)"PDF"))
	goto exit_writeDocument;
    }
    
    if (gnome_print_job_print_to_file (job, getFileName()) != GNOME_PRINT_OK)
      goto exit_writeDocument;

    XAP_UnixGnomePrintGraphics * print_graphics = new XAP_UnixGnomePrintGraphics(job);

    // create a new layout and view object for the doc
    FL_DocLayout *pDocLayout = new FL_DocLayout(getDoc(), print_graphics);
    FV_View * printView = new FV_View(XAP_App::getApp(),0,pDocLayout);
    printView->getLayout()->fillLayouts();
    printView->getLayout()->formatAll();
    printView->getLayout()->recalculateTOCFields();
        
    s_actuallyPrint (getDoc(), print_graphics,
		     printView, getFileName(), 
		     1, true, 
		     pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
		     pDocLayout->countPages(), 1);
    
    DELETEP(pDocLayout);
    DELETEP(printView);

    /* free()'d for us by GnomePrint */
    config = NULL;
    job = NULL;
    exit_status = UT_OK;

  exit_writeDocument:
    if(config)
      g_object_unref (G_OBJECT (config));
    if(job)
      g_object_unref (G_OBJECT (job));

    return exit_status;
  }
};

/*****************************************************************/
/*****************************************************************/

IE_Exp_PS_Sniffer::IE_Exp_PS_Sniffer()
: IE_ExpSniffer("application/postscript", false)
{
}

IE_Exp_PS_Sniffer::~IE_Exp_PS_Sniffer ()
{
}
  
UT_Confidence_t IE_Exp_PS_Sniffer::supportsMIME (const char * szMIME)
{
  if(!UT_stricmp(szMIME, "application/postscript"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}
  
bool IE_Exp_PS_Sniffer::recognizeSuffix (const char * szSuffix)
{
  return !UT_stricmp(szSuffix,".ps");
}

bool IE_Exp_PS_Sniffer::getDlgLabels (const char ** szDesc,
				      const char ** szSuffixList,
				      IEFileType * ft)
{
  *szDesc = "Postscript (.ps)";
  *szSuffixList = "*.ps";
  *ft = getFileType();
  return true;
}

UT_Error IE_Exp_PS_Sniffer::constructExporter (PD_Document * pDocument,
					       IE_Exp ** ppie)
{
  *ppie = new IE_Exp_PDF(pDocument, IE_Exp_PDF::BACKEND_PS);
  return UT_OK;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_PDF_Sniffer::IE_Exp_PDF_Sniffer()
: IE_ExpSniffer("application/pdf", false)
{
}

IE_Exp_PDF_Sniffer::~IE_Exp_PDF_Sniffer ()
{
}
  
UT_Confidence_t IE_Exp_PDF_Sniffer::supportsMIME (const char * szMIME)
{
  if(!UT_stricmp(szMIME, "application/pdf"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}
  
bool IE_Exp_PDF_Sniffer::recognizeSuffix (const char * szSuffix)
{
  return !UT_stricmp(szSuffix,".pdf");
}

bool IE_Exp_PDF_Sniffer::getDlgLabels (const char ** szDesc,
				       const char ** szSuffixList,
				      IEFileType * ft)
{
  *szDesc = "PDF (.pdf)";
  *szSuffixList = "*.pdf";
  *ft = getFileType();
  return true;
}

UT_Error IE_Exp_PDF_Sniffer::constructExporter (PD_Document * pDocument,
					       IE_Exp ** ppie)
{
  *ppie = new IE_Exp_PDF(pDocument, IE_Exp_PDF::BACKEND_PDF);
  return UT_OK;
}

/*****************************************************************/
/*****************************************************************/
