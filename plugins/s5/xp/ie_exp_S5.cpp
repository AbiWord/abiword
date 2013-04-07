/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2008 Dominic Lachowicz (domlachowicz@gmail.com)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

/*
 * http://meyerweb.com/eric/tools/s5/
 *
 * S5 is a slide show format based entirely on XHTML, CSS, and JavaScript. With one file, 
 * you can run a complete slide show and have a printer-friendly version as well. The markup 
 * used for the slides is very simple, highly semantic, and completely accessible. Anyone 
 * with even a smidgen of familiarity with HTML or XHTML can look at the markup and figure 
 * out how to adapt it to their particular needs. Anyone familiar with CSS can create their 
 * own slide show theme. It's totally simple, and it's totally standards-driven.
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ut_stack.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_units.h"
#include "ut_wctomb.h"
#include "pt_Types.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_EncodingManager.h"
#include "fd_Field.h"
#include "fl_DocLayout.h"
#include "ie_exp.h"
#include "gr_Graphics.h"
#include "ie_Table.h"
#include "ut_locale.h"
#include "fv_View.h"
#include "xap_App.h"
#include "ut_path.h"
#include "ie_exp_HTML.h"
#include "xap_Module.h"
#include "ut_string_class.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_s5_register
#define abi_plugin_unregister abipgn_s5_unregister
#define abi_plugin_supports_version abipgn_s5_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("S5")
#endif

/*****************************************************************/
/*****************************************************************/

class IE_Exp_S5 : public IE_Exp
{
public:
	IE_Exp_S5(PD_Document * pDocument);
	virtual ~IE_Exp_S5();
	
protected:
	virtual UT_Error	_writeDocument(void);
	void _writeHeader();
	void _writeFooter();
	void _writeSlide(FV_View* view, UT_uint32 pageno);

	void _write(const char *fmt, ...);
};

class IE_Exp_S5_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_S5_Sniffer ();
	virtual ~IE_Exp_S5_Sniffer () {}

	virtual bool recognizeSuffix (const char * szSuffix);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);
};

/*****************************************************************/
/*****************************************************************/

// completely generic code to allow this to be a plugin

// we use a reference-counted sniffer
static IE_Exp_S5_Sniffer * m_sniffer = 0;

ABI_BUILTIN_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_S5_Sniffer ();
	}

	mi->name = "S5 Slideshow Exporter";
	mi->desc = "Export S5 Slideshows";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_return_val_if_fail (m_sniffer, 0);

	IE_Exp::unregisterExporter (m_sniffer);
	delete m_sniffer;
	m_sniffer = 0;

	return 1;
}

ABI_BUILTIN_FAR_CALL
int abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, 
								 UT_uint32 /*release*/)
{
  return 1;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_S5_Sniffer::IE_Exp_S5_Sniffer () :
  IE_ExpSniffer("AbiS5::S5")
{
  // 
}

bool IE_Exp_S5_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!g_ascii_strcasecmp(szSuffix, ".s5.html"));
}

UT_Error IE_Exp_S5_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	*ppie = new IE_Exp_S5(pDocument);
	return UT_OK;
}

bool IE_Exp_S5_Sniffer::getDlgLabels(const char ** pszDesc,
									 const char ** pszSuffixList,
									 IEFileType * ft)
{
	*pszDesc = "S5 Slideshow (.s5.html)";
	*pszSuffixList = "*.s5.html";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_S5::IE_Exp_S5(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
}

IE_Exp_S5::~IE_Exp_S5()
{
}

UT_Error IE_Exp_S5::_writeDocument(void)
{
    GR_Graphics *layout_graphics = NULL;
    FL_DocLayout *pDocLayout = NULL;
    FV_View *layoutView = NULL;

	layout_graphics = GR_Graphics::newNullGraphics();
	if (!layout_graphics)
		return UT_ERROR;

    // break on pages
    pDocLayout = new FL_DocLayout(getDoc(), layout_graphics);
    layoutView = new FV_View(XAP_App::getApp(),0,pDocLayout);
    layoutView->getLayout()->fillLayouts();
    layoutView->getLayout()->formatAll();
    layoutView->getLayout()->recalculateTOCFields();

	_writeHeader();

	for (UT_uint32 i = 0, nPages = pDocLayout->countPages(); i < nPages; i++)
		{
			_writeSlide(layoutView, i + 1);
			layoutView->warpInsPtNextPrevPage(true);
		}

	_writeFooter();

    DELETEP(pDocLayout);
    DELETEP(layoutView);
    DELETEP(layout_graphics);

	return UT_OK;
}

// http://meyerweb.com/eric/tools/s5/structure-ref.html

void IE_Exp_S5::_writeHeader()
{
	std::string title, author;

	std::string prop;

	prop = getProperty("title");
	if (prop.empty())
		getDoc()->getMetaDataProp (PD_META_KEY_TITLE, title);
	else
		title = prop;

	if (title.size() == 0 && getFileName () != NULL) 
		title = UT_basename(getFileName ());

	prop = getProperty("author");
	if (prop.empty())
		getDoc()->getMetaDataProp (PD_META_KEY_CREATOR, author);
	else
		author = prop;

	if (author.size() == 0)
		author = "UNKNOWN";

	// escapes inline
	title = UT_escapeXML(title);
	author = UT_escapeXML(author);

	write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n");
	write("\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
	write("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	write("<head>\n");
	_write("<title>%s</title>\n", title.c_str());
	write("<!-- metadata -->\n");
	write("<meta name=\"generator\" content=\"AbiWord\" />\n");
	write("<meta name=\"version\" content=\"S5 1.1\" />\n");
	_write("<meta name=\"author\" content=\"%s\" />\n", author.c_str());
	write("<!-- configuration parameters -->\n");
	write("<meta name=\"defaultView\" content=\"slideshow\" />\n");
	write("<meta name=\"controlVis\" content=\"hidden\" />\n");
	write("<!-- style sheet links -->\n");
	write("<link rel=\"stylesheet\" href=\"ui/default/slides.css\" type=\"text/css\" "
		  "media=\"projection\" id=\"slideProj\" />\n");
	write("<link rel=\"stylesheet\" href=\"ui/default/outline.css\" type=\"text/css\" "
		  "media=\"screen\" id=\"outlineStyle\" />\n");
	write("<link rel=\"stylesheet\" href=\"ui/default/print.css\" type=\"text/css\" "
		  "media=\"print\" id=\"slidePrint\" />\n");
	write("<link rel=\"stylesheet\" href=\"ui/default/opera.css\" type=\"text/css\" "
		  "media=\"projection\" id=\"operaFix\" />\n");

	prop = getProperty("suppress-styles");

	bool suppress_styles = false;
	if (!prop.empty())
		suppress_styles = UT_parseBool (prop.c_str(), false);

	if (!suppress_styles)
		{
			write("<style type=\"text/css\" media=\"all\">\n");
			
			UT_ByteBuf styles;
			IE_Exp_HTML::printStyleTree(getDoc(), styles);
			write ((const char *)styles.getPointer(0), styles.getLength());
			write("</style>\n");
		}

	write("<script src=\"ui/default/slides.js\" type=\"text/javascript\"></script>\n");
	write("</head>\n");
	write("<body>\n");
	write("<div class=\"layout\">\n");
	write("<div id=\"controls\"><!-- DO NOT EDIT --></div>\n");
	write("<div id=\"currentSlide\"><!-- DO NOT EDIT --></div>\n");
	write("<div id=\"header\"></div>\n");
	write("<div id=\"footer\">\n");
	_write("<h1>%s</h1>\n", title.c_str());
	write("</div>\n");
	write("</div>\n");
	write("<div class=\"presentation\">\n");
}

void IE_Exp_S5::_writeFooter()
{
	write("</div>\n");
	write("</body>\n");
	write("</html>\n");
}

void IE_Exp_S5::_writeSlide(FV_View* view, UT_uint32 pageno)
{
	// select the page's contents
	view->extSelNextPrevPage(true);

	PT_DocPosition bop = view->getSelectionAnchor() - 1;
	PT_DocPosition eop = bop + view->getSelectionLength();

	// last page has an off-by-1 issue
	if (view->getCurrentPageNumber() == pageno)
		eop += 1;

	PD_DocumentRange range(getDoc(), bop, eop);

	write("<div class=\"slide\">\n");

	// export the XHTML between those 2 positions
	UT_ByteBuf bufXHTML;

	IE_Exp_HTML pExpHtml(getDoc());
	pExpHtml.set_HTML4 (false);
	pExpHtml.copyToBuffer (&range, &bufXHTML);

	// HACK to grab the body, so as to not need changes to the HTML exporter
	const char *body = strstr((const char *)bufXHTML.getPointer(0), "<body>");
	const char *end_body = strstr((const char *)bufXHTML.getPointer(0), "</body>");

	if (body && end_body)
		write(body + strlen("<body>"), end_body - (body + strlen("<body>")));

	write("</div>\n");
}

void IE_Exp_S5::_write(const char *fmt, ...)
{
	UT_String str;

	va_list args;
	va_start (args, fmt);
	UT_String_vprintf (str, fmt, args);
	va_end (args);

	write(str.c_str(), str.size());
}
