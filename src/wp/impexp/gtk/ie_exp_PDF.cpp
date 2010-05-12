/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 1998-2005 AbiSource, Inc.
 * Copyright (C) 2005 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2008 Robert Staudinger
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
#include "gr_CairoPrintGraphics.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "ap_EditMethods.h"
#include "xap_App.h"
#include "ut_misc.h"

#include <cairo-pdf.h>
#include <cairo-ps.h>

#if 0
#include <cairo-svg.h>
#endif

#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-input-stdio.h>
#include <glib/gstdio.h>

#include <unistd.h>
#include <set>

/*****************************************************************/
/*****************************************************************/

static cairo_status_t
ie_exp_cairo_write_func (void *closure, const unsigned char *data, unsigned int length)
{
	if (!gsf_output_write((GsfOutput*)closure, length, data))
		return CAIRO_STATUS_WRITE_ERROR;
    return CAIRO_STATUS_SUCCESS;
}

class ABI_EXPORT IE_Exp_Cairo : public IE_Exp
{
public:
  typedef enum
    {
      BACKEND_PS,
      BACKEND_PDF,
	  BACKEND_SVG
    } Format;
  
  IE_Exp_Cairo::Format mFormat;
  
  IE_Exp_Cairo(PD_Document * pDocument, IE_Exp_Cairo::Format format)
    : IE_Exp(pDocument), mFormat(format)
  {
  }
  
  virtual ~IE_Exp_Cairo()
  {
  }

  virtual UT_Error _writeDocument(void)
  {
    cairo_t *cr = NULL;
    cairo_surface_t *surface = NULL;
    GR_CairoPrintGraphics *print_graphics = NULL;
    FL_DocLayout *pDocLayout = NULL;
    FV_View *printView = NULL;

    std::set<UT_sint32> pages;
    const std::string & pages_prop = getProperty ("pages");

    double width, height;
    bool portrait;

    width = getDoc()->m_docPageSize.Width (DIM_IN);
    height = getDoc()->m_docPageSize.Height (DIM_IN);
    portrait = getDoc()->m_docPageSize.isPortrait();

	// Cairo expects the width/height of the surface in points, with 1 point == 1/72 inch). For details, see
	// http://cairographics.org/manual/cairo-pdf-surface.html#cairo-pdf-surface-create-for-stream
	// Fixes bug 11343 - export to pdf uses wrong document size
	UT_uint32 dpi = 72;
	if (BACKEND_PDF == mFormat)
		surface = cairo_pdf_surface_create_for_stream(ie_exp_cairo_write_func, getFp(), width * dpi, height * dpi);
	else if (BACKEND_PS == mFormat)
		surface = cairo_ps_surface_create_for_stream(ie_exp_cairo_write_func, getFp(), width * dpi, height * dpi);
	else if (BACKEND_SVG == mFormat)
		{
			// surface = cairo_svg_surface_create_for_stream(ie_exp_cairo_write_func, getFp(), width * dpi, height * dpi);
			return UT_ERROR;
		}
	else
		{
			return UT_ERROR;
		}

	cr = cairo_create(surface);
	cairo_surface_destroy(surface), surface = NULL;

	print_graphics = new GR_CairoPrintGraphics(cr, dpi);
    pDocLayout = new FL_DocLayout(getDoc(), print_graphics);
    printView = new FV_View(XAP_App::getApp(), 0, pDocLayout);
    printView->getLayout()->fillLayouts();
    printView->getLayout()->formatAll();
    printView->getLayout()->recalculateTOCFields();

	// TODO lifecycle of "surface" and "cr"?

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
		  for (UT_sint32 i = 1; i <= pDocLayout->countPages(); i++)
			  {
				  pages.insert(i);
			  }
      }    
	
    s_actuallyPrint (getDoc(), print_graphics,
					 printView, getFileName(), 
					 1, true, 
					 pDocLayout->getWidth(), pDocLayout->getHeight() / pDocLayout->countPages(), 
					 pages);
	
    DELETEP(print_graphics);

    DELETEP(pDocLayout);
    DELETEP(printView);
    DELETEP(print_graphics);
    return UT_OK;
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
  *ppie = new IE_Exp_Cairo(pDocument, IE_Exp_Cairo::BACKEND_PS);
  return UT_OK;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_SVG_Sniffer::IE_Exp_SVG_Sniffer()
: IE_ExpSniffer("image/svg+xml", false)
{
}

IE_Exp_SVG_Sniffer::~IE_Exp_SVG_Sniffer ()
{
}
  
UT_Confidence_t IE_Exp_SVG_Sniffer::supportsMIME (const char * szMIME)
{
  if(!g_ascii_strcasecmp(szMIME, "image/svg+xml") ||
	 !g_ascii_strcasecmp(szMIME, "image/svg") ||
	 !g_ascii_strcasecmp(szMIME, "image/svg-xml") ||
	 !g_ascii_strcasecmp(szMIME, "image/vnd.adobe.svg+xml") ||
	 !g_ascii_strcasecmp(szMIME, "text/xml-svg"))
    return UT_CONFIDENCE_PERFECT;
  return UT_CONFIDENCE_ZILCH;
}
  
bool IE_Exp_SVG_Sniffer::recognizeSuffix (const char * szSuffix)
{
  return !g_ascii_strcasecmp(szSuffix,".svg");
}

bool IE_Exp_SVG_Sniffer::getDlgLabels (const char ** szDesc,
				      const char ** szSuffixList,
				      IEFileType * ft)
{
  *szDesc = "Scalable Vector Graphics (.svg)";
  *szSuffixList = "*.svg";
  *ft = getFileType();
  return true;
}

UT_Error IE_Exp_SVG_Sniffer::constructExporter (PD_Document * pDocument,
					       IE_Exp ** ppie)
{
  *ppie = new IE_Exp_Cairo(pDocument, IE_Exp_Cairo::BACKEND_SVG);
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
  *ppie = new IE_Exp_Cairo(pDocument, IE_Exp_Cairo::BACKEND_PDF);
  return UT_OK;
}

/*****************************************************************/
/*****************************************************************/
