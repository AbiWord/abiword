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
#include "gr_UnixPangoGraphics.h"
#include "fv_View.h"
#include "ap_EditMethods.h"

#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-input-stdio.h>
#include <glib/gstdio.h>

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
    FL_DocLayout *pDocLayout = NULL;
    FV_View * printView = NULL;
    GR_Graphics * print_graphics = NULL;
    job = gnome_print_job_new (NULL);
    bool bRes;
    XAP_UnixGnomePrintGraphics * gnome_print_graphics;
    GR_GraphicsFactory * pGF;
    UT_uint32 iDefaultPrintClass;
    char *filename = NULL;
    
    if(!job)
      goto exit_writeDocument;
    
    config = gnome_print_job_get_config (job);
    if(!config)
      goto exit_writeDocument;
    
    if(mFormat == BACKEND_PS) {
      if (!gnome_print_config_set (config, (const guchar *)"Printer", (const guchar *)"GENERIC"))
	goto exit_writeDocument;
    }
    
    if(mFormat == BACKEND_PDF) {
      if (!gnome_print_config_set (config, (const guchar *)"Printer", (const guchar *)"PDF"))
	goto exit_writeDocument;
    }
    
    // acts kinda like tempnam()
    int fd = g_file_open_tmp(NULL, &filename, NULL);
    if(!filename || fd == -1) { // shouldn't ever fail, but be pedantic
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      goto exit_writeDocument;
    }
    
    close(fd);
    bRes = (gnome_print_job_print_to_file (job, filename) == GNOME_PRINT_OK);
    if (!bRes)
      goto exit_writeDocument;
    
    pGF = XAP_App::getApp()->getGraphicsFactory();
    if (!pGF)
      goto exit_writeDocument;
    
    gnome_print_graphics = new XAP_UnixGnomePrintGraphics(job);
    iDefaultPrintClass = pGF->getDefaultClass(false);
    
    if(iDefaultPrintClass == GRID_UNIX_PANGO_PRINT || iDefaultPrintClass == GRID_UNIX_PANGO)
      {
	print_graphics = new GR_UnixPangoPrintGraphics(gnome_print_graphics);
      }
    else
      {
	print_graphics = gnome_print_graphics;
      }

    // create a new layout and view object for the doc
    pDocLayout = new FL_DocLayout(getDoc(), print_graphics);
    printView = new FV_View(XAP_App::getApp(),0,pDocLayout);
    printView->getLayout()->fillLayouts();
    printView->getLayout()->formatAll();
    printView->getLayout()->recalculateTOCFields();
        
    s_actuallyPrint (getDoc(), print_graphics,
		     printView, getFileName(), 
		     1, true, 
		     pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
		     pDocLayout->countPages(), 1);
    
    // copy filename back into getFp()
    if(_copyFile(filename))
      exit_status = UT_OK;
    
    /* free()'d for us by GnomePrint */
    config = NULL;
    job = NULL;
    
  exit_writeDocument:
    if(filename)
      {
	// clean up temporary file
	g_remove(filename);
	g_free (filename);
      }
    
    if(config)
      g_object_unref (G_OBJECT (config));
    if(job)
      g_object_unref (G_OBJECT (job));
    
    DELETEP(pDocLayout);
    DELETEP(printView);
    DELETEP(print_graphics);
    return exit_status;
  }
  
private:
  bool _copyFile(const char * filename)
  {
    GsfInput * printed_file = gsf_input_stdio_new(filename, NULL);
    if(printed_file)
      {
	size_t remaining = gsf_input_size(printed_file);
	guint8 buf[1024];
	
	while(remaining > 0)
	  {
	    size_t nread = MIN(remaining, sizeof(buf));
	    gsf_output_write(getFp(), nread, gsf_input_read(printed_file, nread, buf));
	    remaining -= nread;
	  }
	
	g_object_unref(G_OBJECT(printed_file));

	return true;
      }

    return false;
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
  *szDesc = "Portable Document Format (.pdf)";
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
