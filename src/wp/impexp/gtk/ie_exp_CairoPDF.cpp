/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
#include <unistd.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ie_exp_CairoPDF.h"
#include "pd_Document.h"
#include "gr_UnixPangoGraphics.h"
#include "fv_View.h"
#include "ap_EditMethods.h"
#include "xap_App.h"
#include "ut_misc.h"

#include <cairo/cairo-pdf.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-input-stdio.h>
#include <glib/gstdio.h>

#include <set>

/*****************************************************************/
/*****************************************************************/

class ABI_EXPORT IE_Exp_CairoPDF : public IE_Exp
{
public:

  IE_Exp_CairoPDF(PD_Document * pDocument)
    : IE_Exp(pDocument)
  {
  }
  
  virtual ~IE_Exp_CairoPDF()
  {
  }

  virtual UT_Error _writeDocument(void)
  {
    UT_Error exit_status = UT_ERROR;

    GR_PrintGraphics			*pGC;
    FL_DocLayout 				*pDocLayout;
    FV_View 					*pPrintView;
    char						*filename;
	cairo_surface_t				*surface;
	cairo_t						*cr;
    int							 fd;

    double mrgnTop = getDoc()->m_docPageSize.MarginTop(DIM_PT);
    double mrgnBottom = getDoc()->m_docPageSize.MarginBottom(DIM_PT);
    double mrgnLeft = getDoc()->m_docPageSize.MarginLeft(DIM_PT);
    double mrgnRight = getDoc()->m_docPageSize.MarginRight(DIM_PT);
    double width = getDoc()->m_docPageSize.Width (DIM_PT);
    double height = getDoc()->m_docPageSize.Height (DIM_PT);
    double portrait = getDoc()->m_docPageSize.isPortrait();
    const std::string & pages_prop = getProperty ("pages");
    std::set<UT_uint32> pages;

    filename = NULL;
    fd = g_file_open_tmp(NULL, &filename, NULL);
    if(!filename || fd == -1) { // shouldn't ever fail, but be pedantic
      UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
      goto exit_writeDocument;
    }
    close(fd);

	surface = cairo_pdf_surface_create(filename, width, height);
	cr = cairo_create(surface);
	// takes cairo_t* ownership
    pGC = new GR_PrintGraphics(cr);

    // create a new layout and view object for the doc
    pDocLayout = new FL_DocLayout(getDoc(), pGC);
    pPrintView = new FV_View(XAP_App::getApp(), 0, pDocLayout);
    pPrintView->getLayout()->fillLayouts();
    pPrintView->getLayout()->formatAll();
    pPrintView->getLayout()->recalculateTOCFields();

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

    s_actuallyPrint (getDoc(), pGC,
		     pPrintView, getFileName(), 
		     1, true, 
		     pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
			     pages);

    DELETEP(pDocLayout);
    DELETEP(pPrintView);
    DELETEP(pGC);
    
    // copy filename back into getFp()
    if(_copyFile(filename))
      exit_status = UT_OK;
    
  exit_writeDocument:
    if(filename)
      {
	// clean up temporary file
	g_remove(filename);
	g_free (filename);
      }
    
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

IE_Exp_CairoPDF_Sniffer::IE_Exp_CairoPDF_Sniffer()
: IE_ExpSniffer("application/pdf", false)
{
}

IE_Exp_CairoPDF_Sniffer::~IE_Exp_CairoPDF_Sniffer ()
{
}
  
UT_Confidence_t IE_Exp_CairoPDF_Sniffer::supportsMIME (const char * szMIME)
{
  if(!g_ascii_strcasecmp(szMIME, "application/pdf"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}
  
bool IE_Exp_CairoPDF_Sniffer::recognizeSuffix (const char * szSuffix)
{
  return !g_ascii_strcasecmp(szSuffix,".pdf");
}

bool IE_Exp_CairoPDF_Sniffer::getDlgLabels (const char ** szDesc,
				       const char ** szSuffixList,
				      IEFileType * ft)
{
  *szDesc = "Portable Document Format [using cairo] (.pdf)";
  *szSuffixList = "*.pdf";
  *ft = getFileType();
  return true;
}

UT_Error IE_Exp_CairoPDF_Sniffer::constructExporter (PD_Document * pDocument,
					       IE_Exp ** ppie)
{
  *ppie = new IE_Exp_CairoPDF(pDocument);
  return UT_OK;
}

/*****************************************************************/
/*****************************************************************/
