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
#include "gr_UnixPangoGraphics.h"
#include "fv_View.h"
#include "ap_EditMethods.h"
#include "xap_App.h"
#include "ut_misc.h"

#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-input-stdio.h>
#include <glib/gstdio.h>

#include <unistd.h>
#include <set>

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

    GR_UnixPangoPrintGraphics *print_graphics = NULL;
    FL_DocLayout *pDocLayout = NULL;
    FV_View *printView = NULL;
    char *filename = NULL;
    int fd;

    std::set<UT_uint32> pages;
    const std::string & pages_prop = getProperty ("pages");

    double mrgnTop, mrgnBottom, mrgnLeft, mrgnRight, width, height;
    bool portrait;
    bool pdfWorkAround = (mFormat == BACKEND_PDF ? true : false);

    mrgnTop = getDoc()->m_docPageSize.MarginTop(DIM_MM);
    mrgnBottom = getDoc()->m_docPageSize.MarginBottom(DIM_MM);
    mrgnLeft = getDoc()->m_docPageSize.MarginLeft(DIM_MM);
    mrgnRight = getDoc()->m_docPageSize.MarginRight(DIM_MM);
    width = getDoc()->m_docPageSize.Width (DIM_MM);
    height = getDoc()->m_docPageSize.Height (DIM_MM);
    portrait = getDoc()->m_docPageSize.isPortrait();
  
    GnomePrintJob * job = gnome_print_job_new (NULL);

    // this has to happen before s_setup_config, because it will overwrite the page size and margins and etc...
    GnomePrintConfig *config = gnome_print_job_get_config (job);
    if(!config)
      goto exit_writeDocument;

    if(mFormat == BACKEND_PS) {
      if (!gnome_print_config_set (config, (const guchar *)"Printer", (const guchar *)"GENERIC"))
	goto exit_writeDocument;
    } else if(mFormat == BACKEND_PDF) {
      if (!gnome_print_config_set (config, (const guchar *)"Printer", (const guchar *)"PDF"))
	goto exit_writeDocument;
    }

    if (pdfWorkAround && !portrait)
      {
	double tmp = width;
	width = height;
	height = tmp;
      }

    GR_UnixPangoPrintGraphics::s_setup_config (config, mrgnTop, mrgnBottom, mrgnLeft, mrgnRight,
					       width, height, 1, portrait);    

    // acts kinda like tempnam()
    fd = g_file_open_tmp(NULL, &filename, NULL);
    if(!filename || fd == -1) { // shouldn't ever fail, but be pedantic
      UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
      goto exit_writeDocument;
    }
    
    close(fd);
    if (gnome_print_job_print_to_file (job, filename) != GNOME_PRINT_OK)
      goto exit_writeDocument;
    
    print_graphics = new GR_UnixPangoPrintGraphics(job);

    if (pdfWorkAround && !portrait)
      {
	print_graphics->setPdfWorkaround();
      }
    
    // create a new layout and view object for the doc
    pDocLayout = new FL_DocLayout(getDoc(), print_graphics);
    printView = new FV_View(XAP_App::getApp(),0,pDocLayout);
    printView->getLayout()->fillLayouts();
    printView->getLayout()->formatAll();
    printView->getLayout()->recalculateTOCFields();

    if (!pages_prop.empty())
      {
	char **page_descriptions = g_strsplit(pages_prop.c_str(), ",", -1);
	
	int i = 0;
	while (page_descriptions[i] != NULL)
	  {
	    char *description = page_descriptions[i];
	    i++;
	    
	    int start_page, end_page;
	    
	    if (2 == sscanf(description, "%d-%d", &start_page, &end_page))
	      {
	      }
	    else if (1 == sscanf(description, "%d", &start_page))
	      {
		end_page = start_page;
	      }
	    else
	      {
		// invalid page specification
		continue;
	      }
	    
	    for (int pageno = start_page; pageno <= end_page; pageno++)
	      {
		if ((pageno > 0) && (pageno <= (int)pDocLayout->countPages()))
		  pages.insert(pageno);
	      }
	  }
	
	g_strfreev(page_descriptions);
      }
    
    if (pages.empty())
      {
	for (UT_uint32 i = 1; i <= pDocLayout->countPages(); i++)
	  {
	    pages.insert(i);
	  }
      }    

    s_actuallyPrint (getDoc(), print_graphics,
		     printView, getFileName(), 
		     1, true, 
		     pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
		     pages);
    
    // copy filename back into getFp()
    if(_copyFile(filename))
      exit_status = UT_OK;
    
    /* g_free()'d for us by GnomePrint */
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
  if(!g_ascii_strcasecmp(szMIME, "application/postscript"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}
  
bool IE_Exp_PS_Sniffer::recognizeSuffix (const char * szSuffix)
{
  return !g_ascii_strcasecmp(szSuffix,".ps");
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
  if(!g_ascii_strcasecmp(szMIME, "application/pdf"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}
  
bool IE_Exp_PDF_Sniffer::recognizeSuffix (const char * szSuffix)
{
  return !g_ascii_strcasecmp(szSuffix,".pdf");
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
