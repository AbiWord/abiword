/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
 * Copyright (C) 2001-2002 AbiSource, Inc.
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


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "ut_wctomb.h"
#include "ut_path.h"
#include "ut_string_class.h"

#include "xap_App.h"
#include "xap_EncodingManager.h"

#include "pt_Types.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

#include "fd_Field.h"

#include "ie_exp_HTML.h"

// We terminate each line with a \r\n sequence to make IE think that
// our XHTML is really HTML. This is a stupid IE bug. Sorry

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_HTML_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!UT_stricmp (szSuffix, ".xhtml"));
}

UT_Error IE_Exp_HTML_Sniffer::constructExporter (PD_Document * pDocument,
												 IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_HTML_Sniffer::getDlgLabels (const char ** pszDesc,
										const char ** pszSuffixList,
										IEFileType * ft)
{
	*pszDesc = "XHTML 1.0 (.xhtml)";
	*pszSuffixList = "*.xhtml";
	*ft = getFileType ();
	return true;
}

// HTML 4

bool IE_Exp_HTML4_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!(UT_stricmp (szSuffix, ".html")) || !(UT_stricmp (szSuffix, ".htm")));
}

UT_Error IE_Exp_HTML4_Sniffer::constructExporter (PD_Document * pDocument,
												  IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument, true);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_HTML4_Sniffer::getDlgLabels (const char ** pszDesc,
										 const char ** pszSuffixList,
										 IEFileType * ft)
{
	*pszDesc = "HTML 4.0 (.html, .htm)";
	*pszSuffixList = "*.html; *.htm";
	*ft = getFileType ();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML::IE_Exp_HTML (PD_Document * pDocument, bool is4)
	: IE_Exp(pDocument),
	  m_pListener(0),
	  m_bIs4(is4)
{
	m_error = UT_OK;
}

IE_Exp_HTML::~IE_Exp_HTML ()
{
	// 
}

/*****************************************************************/

/*!	This function returns true if the given property is a valid CSS
	property.  It is based on the list in pp_Property.cpp, and, as such,
	is quite brittle.
 */
static bool is_CSS (const char* property)
{
	static const char * prop_list [] = {"background-color", "color", 
										"font-family", "font-size", 
										"font-stretch", "font-style", 
										"font-variant", "font-weight",
										"height", "margin-bottom", 
										"margin-left", "margin-right",
										"margin-top",
										"orphans", "text-align", 
										"text-decoration", "text-indent",
										"widows", "width"};

	#define PropListLen sizeof(prop_list)/sizeof(prop_list[0])

	for (UT_uint32 i = 0; i < PropListLen; i++) 
	{
		if (!UT_strcmp (property, prop_list[i]))
			return true;
	}
	return false;

	#undef PropListLen
}

/*!	This function copies a string to a new string, removing all the white
	space in the process.  Note that this function allocates the new
	string (and so the caller must make sure to deallocate it).
 */
static char * removeWhiteSpace (const char * text)
{
	char * temp = (char *) UT_calloc (strlen (text) + 1, sizeof (char));
	char * ref = temp;

	const char * ptr = text; // only a pointer for quick traversal

	while (*ptr)
	{
		if (!isspace (*ptr))
		{
			*temp++ = *ptr;
		}
		ptr++;
	}
	return ref;
}

/*!	This function copies a string to a new string, removing all the white
	space in the process.
 */
static char * removeWhiteSpace (const char * text, UT_UTF8String & utf8str)
{
	utf8str = "";

	if (text)
		{
			char buf[2]; // ick! [TODO ??]
			buf[1] = 0;
			const char * ptr = text;
			while (*ptr)
				{
					if (!isspace ((int) ((unsigned char) *ptr)))
						{
							buf[0] = *ptr;
							utf8str += buf;
						}
					ptr++;
				}
		}
	return 0;
}

/*****************************************************************/

#define TT_OTHER	0	// ?		Tag not recognized (not an error, though)

#define TT_HTML		1	// <html>	Document main/first tag

#define TT_HEAD		2	// <head>	(child of <html>; 1 instance; precedes <body>)
#define TT_BODY		3	// <body>	(child of <html>; 1 instance; follows <head>)

#define TT_TITLE	4	// <title>	(child of <head>)
#define TT_STYLE	5	// <style>	(child of <head>)
#define TT_DIV		6	// <div>	[B] Used with style sheets to produce layers, boxes, etc.
				//		AbiWord uses these in its XHTML output to denote sections
				//		but this seems an uncommon use.
#define TT_SPAN		7	// <span>	Like <div>, this is used in conjuction with style sheets
#define TT_LAYER	8	// <layer>	Layer

#define TT_LINK		9	// <link>	(child of <head>; no children; no end tag)
#define TT_BASE		10	// <base>	(child of <head>; no children; no end tag)
#define TT_META		11	// <meta>	(child of <head>; no children; no end tag)

#define TT_A		21	// <a>		Anchor or Hyperlink (can't nest links)

#define TT_H1		22	// <h1>		Headings
#define TT_H2		23	// <h2>
#define TT_H3		24	// <h3>
#define TT_H4		25	// <h4>		Normal size but bold, others relative (usually) (?)
#define TT_H5		26	// <h5>
#define TT_H6		27	// <h6>

#define TT_ADDRESS	29	// <address>	Author's address { set italics }

/* Structured Text: Phrase elements
 */
#define TT_EM		40	// <em>		Emphasis { toggle italics }
#define TT_STRONG	41	// <strong>	Stronger emphasis { set bold }
#define TT_DFN		42	// <dfn>	Defining instance { ? }
#define TT_CODE		43	// <code>	Computer code { use fixed width font }
#define TT_SAMP		44	// <samp>	Sample output { use fixed width font }
#define TT_KBD		45	// <kbd>	Text to be entered by user { use fixed width font }
#define TT_VAR		46	// <var>	Variable/Argument { use fixed width font }
#define TT_CITE		47	// <cite>	Citation/Reference { no action }
#define TT_ABBR		48	// <abbr>	Abbreviation { no action }
#define TT_ACRONYM	49	// <acronym>	Acronym { no action }

/* Structured Text: Quotations
 */
#define TT_Q		50	// <q>		Quote { quotation mark e.g. `, ', ", <<, - a job for smart quotes?  }
#define TT_BLOCKQUOTE	51	// <blockquote>	[B] Block quote { separate paragraph; indented }

/* Structured Text: Subscripts and superscripts
 */
#define TT_SUB		52	// <sub>	Subscript
#define TT_SUP		53	// <sup>	Superscript

/* Lines And Paragraphs: Paragraphs
 */
#define TT_P		54	// <p>		[B] New paragraph
				//		Can't contain other block elements, inc. self
				//		(Ignore empty paragraphs.)

/* Lines And Paragraphs: Controlling line breaks
 */
#define TT_BR		55	// <br>		Forced line break

/* Lines And Paragraphs: Preformatted text
 */
#define TT_PRE		56	// <pre>	Preformatted text { use fixed width font; white space critical }
				//		Can't contain: <img>, <object>, <big>, <small>, <sub>, <sup>

/* Lines And Paragraphs: Marking document changes
 */
#define TT_INS		57	// <ins>	Insert { set color red }
#define TT_DEL		58	// <del>	Delete { set color red; set strike-through }

/* Lists: Unordered lists, ordered lists, and list items
 */
#define TT_OL		59	// <ol>		Ordered list (children must be <li>)
#define TT_UL		60	// <ul>		Unordered list (children must be <li>)
#define TT_LI		61	// <li>		List item (</li> optional)

/* Lists: Definition lists
 */
#define TT_DL		62	// <dl>		Definition list
#define TT_DT		63	// <dt>		Definition term { set bold }
#define TT_DD		64	// <dd>		Definition description { indent }

/* Lists: <dir> & <menu>
 */
#define TT_DIR		TT_UL	// <dir>	Directory list { cf. <ul> }
#define TT_MENU		TT_UL	// <menu>	Menu list { cf. <ul> }

/* Tables: 
 */
#define TT_TABLE	65	// <table>	Table
#define TT_CAPTION	66	// <caption>	Caption (child of <table>)
#define TT_THEAD	67	// <thead>	Table Head (child of <table> acting as new <table>)
#define TT_TFOOT	68	// <tfoot>	Table Foot (child of <table> acting as new <table>)
#define TT_TBODY	69	// <tbody>	Table Body (child of <table> acting as new <table>)
#define TT_COLGROUP	70	// <colgroup>	Un-partition existing columns into one
#define TT_COL		71	// <col>	Subdivide grouped column (child of <colgroup>)
#define TT_TR		72	// <tr>		Row (child of <table>)
#define TT_TH		73	// <th>		Header { set bold } (child of <tr>)
#define TT_TD		74	// <td>		Data (child of <tr>)

/* Objects, Images & Applets: Including an image
 */
#define TT_IMG		75	// <img>	Image tag (empty; no children; no end tag)

/* Objects, Images & Applets: Generic inclusion
 */
#define TT_OBJECT	76	// <object>	Object

/* Objects, Images & Applets: Object initialization
 */
#define TT_PARAM	77	// <param>	Option for object (empty; no children; no end tag)

/* Objects, Images & Applets: Including an applet
 */
#define TT_APPLET	78	// <applet>	Applet

/* Objects, Images & Applets: Client-side image maps
 */
#define TT_MAP		79	// <map>	Map
#define TT_AREA		80	// <area>	Area with map (child of <map>; empty; no children; no end tag)

/* Alignment, Font Styles etc.: Alignment
 */
#define TT_CENTER	81	// <center>	= <div align="center">

/* Alignment, Font Styles etc.: Font style elements
 */
#define TT_TT		82	// <tt>		Fixed width { set fixed width }
#define TT_I		83	// <i>		Italic { set italics }
#define TT_B		84	// <b>		Bold { set bold }
#define TT_BIG		85	// <big>	Large { inc. font size }
#define TT_SMALL	86	// <small>	Small { dec. font size }
#define TT_S		87	// <s>		Strike-through { set strike-through }
#define TT_STRIKE	TT_S	// <strike>	Strike-through { set strike-through }
#define TT_U		88	// <u>		Underline { set underline }

/* Alignment, Font Styles etc.: Font modifier elements
 */
#define TT_FONT		89	// <font>	Set new font (mother of all...)
#define TT_BASEFONT	90	// <basefont>	Default font (font sizes calculated relative to base-font)

/* Alignment, Font Styles etc.: Rules
 */
#define TT_HR		91	// <hr>		Horizontal rule (empty; no children; no end tag)

/* Frames:
 */
#define TT_FRAMESET	92	// <frameset>	Set of frames { no action }
#define TT_FRAME	93	// <frame>	A frame (child of frameset) { no action }
#define TT_NOFRAMES	94	// <noframes>	Frameless version { no action }
#define TT_IFRAME	95	// <iframe>	Inline frame { no action }

/* Forms:
 */
#define TT_FORM		96	// <form>	Form { no action }
#define TT_INPUT	97	// <input>	Input { no action }
#define TT_BUTTON	98	// <button>	Button { no action }
#define TT_SELECT	99	// <select>	Select { no action }
#define TT_OPTGROUP	100	// <optgroup>	Opt. Group { no action }
#define TT_OPTION	101	// <option>	Option { no action }
#define TT_TEXTAREA	102	// <textarea>	Text area { no action }
#define TT_ISINDEX	103	// <isindex>	Index { no action }
#define TT_LABEL	104	// <label>	Label { no action }
#define TT_FIELDSET	105	// <fieldset>	Field set { no action }
#define TT_LEGEND	106	// <legend>	Legend { no action }

/* Scripts:
 */
#define TT_SCRIPT	107	// <script>	Script { no action }
#define TT_NOSCRIPT	108	// <noscript>	Alt. to script { no action }

/* Misc:
 */
#define TT_BDO		109	// <bdo>	BiDi override

/* Ruby:
 */
#define TT_RUBY		110	// <ruby>	Ruby block
#define TT_RP		111	// <rp>		Ruby parenthesis
#define TT_RT		112	// <rt>		Ruby text

/*****************************************************************/

#define IS_TRANSPARENT_COLOR(c) (!UT_strcmp(c, "transparent"))

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	5
#define BT_PLAINTEXT	6
#define BT_NUMBEREDLIST	7
#define BT_BULLETLIST	8

class s_HTML_Listener : public PL_Listener
{
public:
	s_HTML_Listener (PD_Document * pDocument, IE_Exp_HTML * pie, bool is4);

	~s_HTML_Listener ();

	bool	populate (PL_StruxFmtHandle sfh,
					  const PX_ChangeRecord * pcr);

	bool	populateStrux (PL_StruxDocHandle sdh,
						   const PX_ChangeRecord * pcr,
						   PL_StruxFmtHandle * psfh);

	bool	change (PL_StruxFmtHandle sfh,
					const PX_ChangeRecord * pcr);

	bool	insertStrux (PL_StruxFmtHandle sfh,
						 const PX_ChangeRecord * pcr,
						 PL_StruxDocHandle sdh,
						 PL_ListenerId lid,
						 void (*pfnBindHandles) (PL_StruxDocHandle sdhNew,
												 PL_ListenerId lid,
												 PL_StruxFmtHandle sfhNew));

	bool	signal (UT_uint32 iSignal);

private:
	void 	_outputBegin (PT_AttrPropIndex api);
	void 	_outputEnd ();
	void	_openSection (PT_AttrPropIndex api);
	void	_closeSection (void);
	void	_outputInheritanceLine (const char * ClassName);
	void	_appendInheritanceLine (const char * ClassName, UT_UTF8String & utf8str);
	void	_openTag (PT_AttrPropIndex api);
	void	_closeTag (void);
	void	_closeSpan (void);
	void	_openSpan (PT_AttrPropIndex api);
	void	_outputData (const UT_UCSChar * p, UT_uint32 length);
	bool	_inherits (const char * style, const char * from);
	void	_handleDataItems (void);
	void	_convertFontSize (UT_String & szDest, const char * pszFontSize);
	void	_convertColor (UT_String & szDest, const char * pszColor);
	void	_storeStyles (void);
	char *	_stripSuffix (const char * from, char delimiter);
	
	PD_Document *		m_pDocument;
	IE_Exp_HTML *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bNextIsSpace;
	bool				m_bWroteText;
	bool				m_bFirstWrite;
	bool				m_bIs4;
	const PP_AttrProp *	m_pAP_Span;

	// Need to look up proper type, and place to stick #defines...
  
	UT_uint16		m_iBlockType;	// BT_*
	UT_uint16		m_iListDepth;	// 0 corresponds to not in a list
	UT_uint16		m_iPrevListDepth;
	UT_Stack		m_utsListType;
	UT_Vector		m_utvDataIDs;	// list of data ids for image enumeration
	UT_uint16		m_iImgCnt;
	UT_Wctomb		m_wmctomb;

	/* low-level; these may use m_utf8_0 but not m_utf8_1
	 */
	void			tagNewIndent (UT_uint32 extra = 0);
	void			tagOpenClose (const UT_UTF8String & content, bool suppress_close);
	void			tagOpen  (UT_uint32 tagID, const UT_UTF8String & content);
	void			tagClose (UT_uint32 tagID, const UT_UTF8String & content);
	void			tagClose (UT_uint32 tagID);
	UT_uint32		tagTop ();
	void			tagPI (const char * target, const UT_UTF8String & content);
	void			tagComment (const UT_UTF8String & content);
	void			tagCommentOpen ();
	void			tagCommentClose ();
	void			styleIndent ();
	void			styleOpen (const UT_UTF8String & rule);
	void			styleClose ();
	void			styleNameValue (const char * name, const UT_UTF8String & value);
	void			textUntrusted (const char * text);

	UT_uint16		listDepth ();
	UT_uint16		listType ();
	void			listPush (UT_uint16 type, const char * ClassName);
	void			listPop ();
	void			listPopToDepth (UT_uint16 depth);

	/* temporary strings; use with extreme caution
	 */
	UT_UTF8String	m_utf8_0; // low-level
	UT_UTF8String	m_utf8_1; // intermediate

	UT_Stack		m_tagStack;

	UT_uint32		m_styleIndent;
};

void s_HTML_Listener::tagNewIndent (UT_uint32 extra)
{
	m_utf8_0 = "";

	UT_uint32 depth = m_tagStack.getDepth () + extra;

	for (UT_uint32 i = 0; i < (depth >> 3); i++) m_utf8_0 += "\t";
	for (UT_uint32 i = 0; i < (depth &  7); i++) m_utf8_0 += " ";
}

void s_HTML_Listener::tagOpenClose (const UT_UTF8String & content, bool suppress_close)
{
	tagNewIndent ();

	m_utf8_0 += "<";
	m_utf8_0 += content;
	if (suppress_close)
		m_utf8_0 += ">\r\n";
	else
		m_utf8_0 += "/>\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::tagOpen (UT_uint32 tagID, const UT_UTF8String & content)
{
	tagNewIndent ();

	m_utf8_0 += "<";
	m_utf8_0 += content;
	m_utf8_0 += ">\r\n";

	m_pie->write (m_utf8_0.utf8_str ());

	void * vptr = reinterpret_cast<void *>(tagID);
	m_tagStack.push (vptr);
}

void s_HTML_Listener::tagClose (UT_uint32 tagID, const UT_UTF8String & content)
{
	tagClose (tagID);

	tagNewIndent ();

	m_utf8_0 += "</";
	m_utf8_0 += content;
	m_utf8_0 += ">\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::tagClose (UT_uint32 tagID)
{
	void * vptr = 0;
	m_tagStack.pop (&vptr);

	if (reinterpret_cast<UT_uint32>(vptr) == tagID) return;

	UT_DEBUGMSG(("WARNING: possible tag mis-match in XHTML output!\n"));
}

UT_uint32 s_HTML_Listener::tagTop ()
{
	void * vptr = 0;
	if (m_tagStack.viewTop (&vptr)) return reinterpret_cast<UT_uint32>(vptr);
	return 0;
}

void s_HTML_Listener::tagPI (const char * target, const UT_UTF8String & content)
{
	tagNewIndent ();

	m_utf8_0 += "<?";
	m_utf8_0 += target;
	m_utf8_0 += " ";
	m_utf8_0 += content;
	m_utf8_0 += "?>\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::tagComment (const UT_UTF8String & content)
{
	tagNewIndent ();

	m_utf8_0 += "<!-- ";
	m_utf8_0 += content;
	m_utf8_0 += " -->\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::tagCommentOpen ()
{
	tagNewIndent ();

	m_utf8_0 += "<!--\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::tagCommentClose ()
{
	tagNewIndent (2);

	m_utf8_0 += "-->\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::styleIndent ()
{
	m_utf8_0 = "";

	for (UT_uint32 i = 0; i < m_styleIndent; i++) m_utf8_0 += "\t";
}

void s_HTML_Listener::styleOpen (const UT_UTF8String & rule)
{
	styleIndent ();

	m_utf8_0 += rule;
	m_utf8_0 += " {\r\n";

	m_pie->write (m_utf8_0.utf8_str ());

	m_styleIndent++;
}

void s_HTML_Listener::styleClose ()
{
	if (m_styleIndent == 0)
		{
			UT_DEBUGMSG(("WARNING: CSS style group over-closing!\n"));
			return;
		}
	m_styleIndent--;

	styleIndent ();

	m_utf8_0 += "}\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::styleNameValue (const char * name, const UT_UTF8String & value)
{
	styleIndent ();

	m_utf8_0 += name;
	m_utf8_0 += ": ";
	m_utf8_0 += value;
	m_utf8_0 += ";\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::textUntrusted (const char * text)
{
	/* TODO: translate characters
	 */
	m_utf8_0 = text;

	m_utf8_0 += "\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

/* intermediate methods
 */

static const char * s_DTD_XHTML = "!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"";

static const char * s_DTD_HTML4 = "!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" \"http://www.w3.org/TR/REC-html40/loose.dtd\"";

static const char * s_Delimiter = 
"================================================================================ ";

static const char * s_Header[3] = {
	"This HTML file was created by AbiWord.                                           ",
	"AbiWord is a free, Open Source word processor.                                   ",
	"You may obtain more information about AbiWord at www.abisource.com               "
};

void s_HTML_Listener::_outputBegin (PT_AttrPropIndex api)
{
	/* print XML header
	 */
	if ( !m_bIs4 )
		{
			m_utf8_1 = "version=\"1.0\"";
			tagPI ("xml", m_utf8_1);

			m_utf8_1 = s_DTD_XHTML;
			tagOpenClose (m_utf8_1, true);
		}
	else
		{
			m_utf8_1 = s_DTD_HTML4;
			tagOpenClose (m_utf8_1, true);
		}
	
	/* print header comment
	 */
	const UT_UTF8String delimiter(s_Delimiter);
	tagComment (delimiter);
	for (UT_uint32 hdri = 0; hdri < 3; hdri++)
		{
			m_utf8_1 = s_Header[hdri];
			tagComment (m_utf8_1);
		}
	tagComment (delimiter);

	/* open root element, i.e. <html>; namespace it if XHTML
	 */
	m_utf8_1 = "html";
	if (m_bIs4) m_utf8_1 += " xmlns=\"http://www.w3.org/1999/xhtml\"";
	tagOpen (TT_HTML, m_utf8_1);
	
	/* start <head> section of HTML document
	 */
	m_utf8_1 = "head";
	tagOpen (TT_HEAD, m_utf8_1);

	/* in the case of HTML4, we add a meta tag describing the document's charset as UTF-8
	 */
	if (m_bIs4)
		{
			m_utf8_1 = "meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\"";
			tagOpenClose (m_utf8_1, m_bIs4);
		}
	
	/* set page's title in browser
	 */
	m_utf8_1 = "title";
	tagOpen (TT_TITLE, m_utf8_1);
	textUntrusted (m_pie->getFileName ()); // TODO: is there a better title to use?
	tagClose (TT_TITLE, m_utf8_1);

	/* some cascading style rules
	 */
	const XML_Char * szName  = 0;
	const XML_Char * szValue = 0;

	m_utf8_1 = "style type=\"text/css\"";
	tagOpen (TT_STYLE, m_utf8_1);
	tagCommentOpen ();

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	if (bHaveProp && pAP)
		{
			/* global page styles refer to the <body> tag
			 */
			PD_Style * pStyle = 0;
			m_pDocument->getStyle ("Normal", &pStyle);
			
			if(pStyle)
				{
					/* Add normal styles to any descendent of the body for global effect
					 * 
					 * (I think @ rules are supposed to precede non-@ rules)
					 */
					m_utf8_1 = "@media print";
					styleOpen (m_utf8_1);

					m_utf8_1 = "body";
					styleOpen (m_utf8_1);

					szValue = PP_evalProperty ("page-margin-top", 0, 0, pAP, m_pDocument, true);
					m_utf8_1 = (const char *) szValue;
					styleNameValue ("padding-top", m_utf8_1);

					szValue = PP_evalProperty ("page-margin-bottom", 0, 0, pAP, m_pDocument, true);
					m_utf8_1 = (const char *) szValue;
					styleNameValue ("padding-bottom", m_utf8_1);

					szValue = PP_evalProperty ("page-margin-left", 0, 0, pAP, m_pDocument, true);
					m_utf8_1 = (const char *) szValue;
					styleNameValue ("padding-left", m_utf8_1);

					szValue = PP_evalProperty ("page-margin-right", 0, 0, pAP, m_pDocument, true);
					m_utf8_1 = (const char *) szValue;
					styleNameValue ("padding-right", m_utf8_1);

					styleClose (); // end of: body { }
					styleClose (); // end of: @media print { }

					m_utf8_1 = "body";
					styleOpen (m_utf8_1);

					for (UT_uint16 i = 0; i < pStyle->getPropertyCount (); i++)
						{
							pStyle->getNthProperty (i, szName, szValue);

							if (( szName == 0) || ( szValue == 0)) continue; // paranoid? moi?
							if ((*szName == 0) || (*szValue == 0)) continue;

							if (strstr (szName, "margin")) continue;
							if (!is_CSS (reinterpret_cast<const char *>(szName))) continue;

							if (UT_strcmp (szName, "font-family") == 0)
								{
									if ((UT_strcmp (szValue, "serif")      == 0) ||
										(UT_strcmp (szValue, "sans-serif") == 0) ||
										(UT_strcmp (szValue, "cursive")    == 0) ||
										(UT_strcmp (szValue, "fantasy")    == 0) ||
										(UT_strcmp (szValue, "monospace")  == 0))
										{
											m_utf8_1 = (const char *) szValue;
										}
									else
										{
											m_utf8_1  = "'";
											m_utf8_1 += (const char *) szValue;
											m_utf8_1 += "'";
										}
								}
							else if (UT_strcmp (szName, "color") == 0)
								{
									if (IS_TRANSPARENT_COLOR (szValue)) continue;

									m_utf8_1  = "#";
									m_utf8_1 += (const char *) szValue;
								}
							else m_utf8_1 = (const char *) szValue;

							styleNameValue (szName, m_utf8_1);
						}
					szValue = PP_evalProperty ("background-color", 0, 0, pAP, m_pDocument, true);
					if(!IS_TRANSPARENT_COLOR (szValue))
						{
							m_utf8_1  = "#";
							m_utf8_1 += (const char *) szValue;

							styleNameValue ("background-color", m_utf8_1);
						}
					styleClose (); // end of: body { }
				}
		}

	const PD_Style * p_pds = 0;
	const XML_Char * szStyleName = 0;

	for (size_t n = 0; m_pDocument->enumStyles (n, &szStyleName, &p_pds); n++)
		{
			if (p_pds == 0) continue;

			PT_AttrPropIndex api = p_pds->getIndexAP ();

			const PP_AttrProp * pAP_style = 0;
			bool bHaveProp = m_pDocument->getAttrProp (api, &pAP_style);

			if (bHaveProp && pAP_style && p_pds->isUsed ())
				{
					removeWhiteSpace ((const char *) szStyleName, m_utf8_0); // careful!!
					const char * myStyleName = m_utf8_0.utf8_str ();

					if (UT_strcmp (myStyleName, "Heading1") == 0)
						{
							m_utf8_1 = "h1, .";
						}
					else if (UT_strcmp (myStyleName, "Heading2") == 0)
						{
							m_utf8_1 = "h2, .";
						}
					else if (UT_strcmp (myStyleName, "Heading3") == 0)
						{
							m_utf8_1 = "h3, .";
						}
					else if (UT_strcmp (myStyleName, "BlockText") == 0)
						{
							m_utf8_1 = "blockquote, .";
						}
					else if (UT_strcmp (myStyleName, "PlainText") == 0)
						{
							m_utf8_1 = "pre, .";
						}
					else if (UT_strcmp (myStyleName, "Normal") == 0)
						{
							m_utf8_1 = "p, .";
						}
					else m_utf8_1 = ".";

					m_utf8_1 += m_utf8_0; // i.e., += myStyleName

					styleOpen (m_utf8_1);

					// TODO

					UT_uint32 i = 0;
					UT_uint32 j = 0;

					while (pAP_style->getNthAttribute (i++, szName, szValue))
						{
							if (!is_CSS (reinterpret_cast<const char *>(szName))) continue;

							/* see line 770 of this file for reasoning behind skipping here:
							 * [winner of the Nov.'02 "Comment of the Month" Award]
							 */
							if (strstr (szName, "margin") || !UT_strcmp (szName, "text-indent"))
								if (strstr (myStyleName, "List"))
									continue;
				
							m_utf8_1 = (const char *) szValue;
							styleNameValue (szName, m_utf8_1);
						}
					while (pAP_style->getNthProperty (j++, szName, szValue))
						{
							if (!is_CSS (reinterpret_cast<const char *>(szName))) continue;

							/* see line 770 of this file for reasoning behind skipping here:
							 */
							if (strstr (szName, "margin") || !UT_strcmp (szName, "text-indent"))
								if (strstr (myStyleName, "List"))
									continue;
				
							if (UT_strcmp (szName, "font-family") == 0)
								{
									if ((UT_strcmp (szValue, "serif")      == 0) ||
										(UT_strcmp (szValue, "sans-serif") == 0) ||
										(UT_strcmp (szValue, "cursive")    == 0) ||
										(UT_strcmp (szValue, "fantasy")    == 0) ||
										(UT_strcmp (szValue, "monospace")  == 0))
										{
											m_utf8_1 = (const char *) szValue;
										}
									else
										{
											m_utf8_1  = "'";
											m_utf8_1 += (const char *) szValue;
											m_utf8_1 += "'";
										}
								}
							else if (UT_strcmp (szName, "color") == 0)
								{
									if (IS_TRANSPARENT_COLOR (szValue)) continue;

									m_utf8_1  = "#";
									m_utf8_1 += (const char *) szValue;
								}
							else m_utf8_1 = (const char *) szValue;

							styleNameValue (szName, m_utf8_1);
						}
					styleClose (); // end of: ?, .? { }
				}
		}

	tagCommentClose ();
	m_utf8_1 = "style";
	tagClose (TT_STYLE, m_utf8_1);

	/* end <head> section of HTML document
	 */
	m_utf8_1 = "head";
	tagClose (TT_HEAD, m_utf8_1);

	/* start <body> section of HTML document
	 */
	m_utf8_1 = "body";
	tagOpen (TT_BODY, m_utf8_1);

	m_bFirstWrite = false;
}

void s_HTML_Listener::_outputEnd ()
{
	/* end <body> section of HTML document
	 */
	m_utf8_1 = "body";
	tagClose (TT_BODY, m_utf8_1);

	/* end <head> section of HTML document
	 */
	m_utf8_1 = "html";
	tagClose (TT_HTML, m_utf8_1);
}

void s_HTML_Listener::_openSection (PT_AttrPropIndex api)
{
	if (m_bFirstWrite) _outputBegin (api);

	m_utf8_1 = "div";
	tagOpen (TT_DIV, m_utf8_1);
}

void s_HTML_Listener::_closeSection (void)
{
	if (m_bInSection)
		{
			m_utf8_1 = "div";
			tagClose (TT_DIV, m_utf8_1);
		}
	m_bInSection = false;
}

/* TODO: ditch this variant eventually ??
 */
void s_HTML_Listener::_outputInheritanceLine (const char * ClassName)
{
	PD_Style * pStyle = 0;

	if (m_pDocument->getStyle (ClassName, &pStyle))
		if (pStyle)
			{
				PD_Style * pBasedOn = 0;
				pBasedOn = pStyle->getBasedOn ();
				if (pBasedOn)
					{
						/* The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME
						 * attribute within the style
						 */
						const XML_Char * szName = 0;
						pBasedOn->getAttribute (PT_NAME_ATTRIBUTE_NAME, szName);
				
						if (szName)
							{
								char * pName = removeWhiteSpace ((const char *) szName);
								_outputInheritanceLine (pName);
								FREEP (pName);
								m_pie->write (" ");
							}
					}
			}

	ClassName = removeWhiteSpace (ClassName);
	m_pie->write (ClassName);
	FREEP (ClassName);
}

void s_HTML_Listener::_appendInheritanceLine (const char * ClassName, UT_UTF8String & utf8str)
{
	UT_UTF8String tmp; // this is a recursive function - can't use class tmps

	PD_Style * pStyle = 0;

	if (m_pDocument->getStyle (ClassName, &pStyle))
		if (pStyle)
			{
				PD_Style * pBasedOn = 0;
				pBasedOn = pStyle->getBasedOn ();
				if (pBasedOn)
					{
						/* The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME
						 * attribute within the style
						 */
						const XML_Char * szName = 0;
						pBasedOn->getAttribute (PT_NAME_ATTRIBUTE_NAME, szName);

						if (szName)
							{
								removeWhiteSpace ((const char *) szName, tmp);
								_appendInheritanceLine (tmp.utf8_str (), utf8str);

								utf8str += " "; // TODO: should this be ", " or ??
							}
					}
			}

	removeWhiteSpace (ClassName, tmp);
	utf8str += tmp;
}

UT_uint16 s_HTML_Listener::listDepth ()
{
	return static_cast<UT_uint16>(m_utsListType.getDepth ());
}

UT_uint16 s_HTML_Listener::listType ()
{
	void * vptr = 0;
	m_utsListType.viewTop (&vptr);
	return static_cast<UT_uint16>(reinterpret_cast<UT_uint32>(vptr));
}

void s_HTML_Listener::listPush (UT_uint16 type, const char * ClassName)
{
	m_utf8_1 = "li";
	if (tagTop () == TT_LI)
		{
			tagClose (TT_LI, m_utf8_1);
		}
	if ((tagTop () == TT_UL) || (tagTop () == TT_OL))
		{
			tagOpen (TT_LI, m_utf8_1);
		}

	UT_uint32 tagID;

	if (type == BT_BULLETLIST)
		{
			tagID = TT_UL;
			m_utf8_1 = "ul";
		}
	else 
		{
			tagID = TT_OL;
			m_utf8_1 = "ol";
		}
	m_utf8_1 += " class=\"";
	_appendInheritanceLine (ClassName, m_utf8_1);
	m_utf8_1 += "\"";
	tagOpen (tagID, m_utf8_1);

	void * vptr = reinterpret_cast<void *>(static_cast<UT_uint32>(type));
	m_utsListType.push (vptr);
}

void s_HTML_Listener::listPop ()
{
	if (tagTop () == TT_LI)
		{
			m_utf8_1 = "li";
			tagClose (TT_LI, m_utf8_1);
		}

	void * vptr = 0;
	m_utsListType.pop (&vptr);
	UT_uint16 type = static_cast<UT_uint16>(reinterpret_cast<UT_uint32>(vptr));

	UT_uint32 tagID;

	if (type == BT_BULLETLIST)
		{
			tagID = TT_UL;
			m_utf8_1 = "ul";
		}
	else 
		{
			tagID = TT_OL;
			m_utf8_1 = "ol";
		}
	tagClose (tagID, m_utf8_1);
}

void s_HTML_Listener::listPopToDepth (UT_uint16 depth)
{
	if (listDepth () <= depth) return;

	UT_uint16 count = listDepth () - depth;
	for (UT_uint16 i = 0; i < count; i++) listPop ();
}

void s_HTML_Listener::_openTag (PT_AttrPropIndex api)
{
	if (m_bFirstWrite) _outputBegin (api);

	if (!m_bInSection) return;
	
	m_bWroteText = false;

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	
	if (!bHaveProp || (pAP == 0)) // <p> with no style attribute, and no properties either
		{
			listPopToDepth (0);

			m_utf8_1 = "p";
			tagOpen (TT_P, m_utf8_1);

			m_iBlockType = BT_NORMAL;
			m_bInBlock = true;
			return;
		}

	UT_uint16 tagID = TT_OTHER;

	bool tagPending = false;

	const XML_Char * szDefault = "Normal"; // TODO: should be/is a #define somewhere?

	const XML_Char * szValue = 0;
	const XML_Char * szLevel = 0;
	const XML_Char * szListID = 0;
	const XML_Char * szStyleType = 0;

	/*	This is the point at which we differentiate between different
	 *	types of tags in HTML.  We do a sequence of checks on the "style"
	 *	and other useful attributes of the current block.  First we check
	 *	if we are in a block which has a named "style" or which contains
	 *	list information.
	 *
	 *	Weaknesses in this code include the mutability of our stored
	 *	document state.  I've had it happen where the representation of
	 *	lists in the abiword format changes, which tends to break this
	 *	code.
	 */

	bool have_style  = pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME,  szValue);
	bool have_listID = pAP->getAttribute (PT_LISTID_ATTRIBUTE_NAME, szListID);
	bool zero_listID = true;

	if (have_listID) zero_listID = (UT_strcmp (szListID, "0") == 0);

	/* A nonzero "listid" attribute value indicates that we
	 * are in a list item, so we need to process it, HTML-style.
	 */

	/* Specify a default style name for this list item if it
	 * doesn't already have one.
	 */
	if (!have_style) szValue = szDefault;

	if (!zero_listID)
		{
			/* Find out how deeply nested this list item is.
			 */
			pAP->getAttribute ("level", szLevel);
			m_iListDepth = atoi ((const char *) szLevel);

			/* TODO: why can m_iListDepth be zero sometimes ??
			 */
			if (m_iListDepth == 0) m_iListDepth = 1;

			/* Rise to desired list depth if currently too deep
			 */
			listPopToDepth (m_iListDepth);

			/* Desired list type (numbered / bullet)
			 */
			if (!pAP->getProperty ("list-style", szStyleType)) szStyleType = szValue;

			if (UT_strcmp ((const char *) szStyleType, "Bullet List") == 0)
				{
					m_iBlockType = BT_BULLETLIST;
				}
			else 
				{
					m_iBlockType = BT_NUMBEREDLIST;
				}

			/* current list & desired list have same depth but different types...
			 * pop one and add new below
			 */
			if ((m_iListDepth == listDepth ()) && (m_iBlockType != listType ()))
				{
					listPop ();
				}

			/* If our list is getting deeper, we need to start a nested list.
			 * Assumption: list can only increase in depth by 1.
			 */
			if (m_iListDepth > listDepth ())
				{
					listPush (m_iBlockType, szValue);
				}
			else if (tagTop () == TT_LI)
				{
					m_utf8_1 = "li";
					tagClose (TT_LI, m_utf8_1);
				}

			tagID = TT_LI;
			tagPending = true;

			m_utf8_1 = "li";
		}
	else if (have_style)
		{
			listPopToDepth (0);

			bool bInherits;
			bool bAddInheritance = false;

			/* <p style="Heading 1"> ...
			 */
			bInherits = _inherits ((const char *) szValue, "Heading1");
			if ((UT_strcmp ((const char *) szValue, "Heading 1") == 0) || bInherits)
				{
					m_iBlockType = BT_HEADING1;

					tagID = TT_H1;
					tagPending = true;

					m_utf8_1 = "h1";

					bAddInheritance = bInherits;
				}

			/* <p style="Heading 2"> ...
			 */
			bInherits = _inherits ((const char *) szValue, "Heading2");
			if ((UT_strcmp ((const char *) szValue, "Heading 2") == 0) || bInherits)
				{
					m_iBlockType = BT_HEADING2;

					tagID = TT_H2;
					tagPending = true;

					m_utf8_1 = "h2";

					bAddInheritance = bInherits;
				}

			/* <p style="Heading 3"> ...
			 */
			bInherits = _inherits ((const char *) szValue, "Heading3");
			if ((UT_strcmp ((const char *) szValue, "Heading 3") == 0) || bInherits)
				{
					m_iBlockType = BT_HEADING3;

					tagID = TT_H3;
					tagPending = true;

					m_utf8_1 = "h3";

					bAddInheritance = bInherits;
				}

			/* <p style="Block Text"> ...
			 */
			bInherits = _inherits ((const char *) szValue, "BlockText");
			if ((UT_strcmp ((const char *) szValue, "Block Text") == 0) || bInherits)
				{
					m_iBlockType = BT_BLOCKTEXT;

					tagID = TT_BLOCKQUOTE;
					tagPending = true;

					m_utf8_1 = "blockquote";

					bAddInheritance = bInherits;
				}

			/* <p style="Plain Text"> ...
			 */
			bInherits = _inherits ((const char *) szValue, "PlainText");
			if ((UT_strcmp ((const char *) szValue, "Plain Text") == 0) || bInherits)
				{
					m_iBlockType = BT_PLAINTEXT;

					tagID = TT_PRE;
					tagPending = true;

					m_utf8_1 = "pre";

					bAddInheritance = bInherits;
				}

			/* <p style="Normal"> ...
			 */
			bInherits = _inherits ((const char *) szValue, "Normal");
			if ((UT_strcmp ((const char *) szValue, "Normal") == 0) || bInherits)
				{
					m_iBlockType = BT_NORMAL;

					tagID = TT_P;
					tagPending = true;

					m_utf8_1 = "p";

					bAddInheritance = bInherits;
				}

			if (!tagPending) // <p style=other... >
				{
					m_iBlockType = BT_NORMAL;

					tagID = TT_P;
					tagPending = true;

					m_utf8_1 = "p";
				}

			if (bAddInheritance)
				{
					m_utf8_1 += " class=\"";
					_appendInheritanceLine ((const char*) szValue, m_utf8_1);
					m_utf8_1 += "\"";
				}
		}	
	else // not a list, no style
		{
			listPopToDepth (0);

			m_iBlockType = BT_NORMAL;

			tagID = TT_P;
			tagPending = true;

			m_utf8_1 = "p";
		}
	if (!tagPending)
		{
			UT_DEBUGMSG(("WARNING: unexpected!\n"));
			return;
		}

	const XML_Char * szP_TextAlign = 0;
	const XML_Char * szP_MarginBottom = 0;
	const XML_Char * szP_MarginTop = 0;
	const XML_Char * szP_MarginLeft = 0;
	const XML_Char * szP_MarginRight = 0;
	const XML_Char * szP_TextIndent = 0;
	const XML_Char * szP_DomDir = 0;

	pAP->getProperty ("text-align",    szP_TextAlign);
	pAP->getProperty ("margin-bottom", szP_MarginBottom);
	pAP->getProperty ("margin-top",    szP_MarginTop);
	pAP->getProperty ("margin-right",  szP_MarginRight);

	/* NOTE: For both "margin-left" and "text-indent" for lists,
	 * Abi's behaviour and HTML's do not match.
	 * Furthermore, it seems like all blocks have a "margin-left"
	 * and "text-indent", even if they are zero, which adds
	 * significant clutter.  These are all manually taken care of
	 * below.  I think that the computation of these attributes
	 * needs to be rethought. - John
	 */
	if (tagID != TT_LI)
		{
			if (pAP->getProperty ("margin-left", szP_MarginLeft))
				if (strstr (szP_MarginLeft, "0.0000"))
					szP_MarginLeft = 0;

			if (pAP->getProperty ("text-indent", szP_TextIndent))
				if (strstr (szP_TextIndent, "0.0000"))
					szP_TextIndent = 0;
		}

#ifdef BIDI_ENABLED
	pAP->getProperty ("dom-dir", szP_DomDir);
#endif
	if (szP_DomDir) // any reason why this can't be used with <blockquote> or <pre> ??
		{
			m_utf8_1 += " dir=\"";
			m_utf8_1 += szP_DomDir;
			m_utf8_1 += "\"";
		}

	bool validProp = (szP_TextAlign || szP_MarginBottom || szP_MarginTop ||
					  szP_MarginLeft || szP_MarginRight || szP_TextIndent);

	/* Assumption: never get property set with block text, plain text. Probably true...
	 */
	if ((m_iBlockType != BT_PLAINTEXT) &&
		(m_iBlockType != BT_BLOCKTEXT) && validProp)
		{
			m_utf8_1 += " style=\"";

			bool first = false;

			if (szP_TextAlign)
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "text-align: ";
					m_utf8_1 += szP_TextAlign;
					first = false;
				}
			if (szP_MarginBottom)
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "margin-bottom: ";
					m_utf8_1 += szP_MarginBottom;
					first = false;
				}
			if (szP_MarginTop)
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "margin-top: ";
					m_utf8_1 += szP_MarginTop;
					first = false;
				}
			if (szP_MarginRight)
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "margin-right: ";
					m_utf8_1 += szP_MarginRight;
					first = false;
				}
			if (szP_MarginLeft)
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "margin-left: ";
					m_utf8_1 += szP_MarginLeft;
					first = false;
				}
			if (szP_TextIndent)
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "text-indent: ";
					m_utf8_1 += szP_TextIndent;
					first = false;
				}

			m_utf8_1 += "\"";
		}

	tagOpen (tagID, m_utf8_1);

	m_bInBlock = true;
}

void s_HTML_Listener::_closeTag (void)
{
	if (!m_bInBlock) return;

	if (m_iBlockType == BT_NORMAL)
		{
			if (!m_bWroteText)
				{
					m_utf8_1 = "br";
					tagOpenClose (m_utf8_1, m_bIs4);
				}
			if (tagTop () == TT_P)
				{
					m_utf8_1 = "p";
					tagClose (TT_P, m_utf8_1);
				}
		}
	else if (m_iBlockType == BT_HEADING1) 
		{
			if (tagTop () == TT_H1)
				{
					m_utf8_1 = "h1";
					tagClose (TT_H1, m_utf8_1);
				}
		}
	else if (m_iBlockType == BT_HEADING2)
		{
			if (tagTop () == TT_H2)
				{
					m_utf8_1 = "h2";
					tagClose (TT_H2, m_utf8_1);
				}
		}
	else if (m_iBlockType == BT_HEADING3)
		{
			if (tagTop () == TT_H3)
				{
					m_utf8_1 = "h3";
					tagClose (TT_H3, m_utf8_1);
				}
		}
	else if (m_iBlockType == BT_BLOCKTEXT)
		{
			if (tagTop () == TT_BLOCKQUOTE)
				{
					m_utf8_1 = "blockquote";
					tagClose (TT_BLOCKQUOTE, m_utf8_1);
				}
		}
	else if (m_iBlockType == BT_PLAINTEXT)
		{
			if (tagTop () == TT_PRE)
				{
					m_utf8_1 = "pre";
					tagClose (TT_PRE, m_utf8_1);
				}
		}
	else if (m_iBlockType == BT_NUMBEREDLIST || m_iBlockType == BT_BULLETLIST)
		{	
			/* do nothing, lists are handled differently, as they have multiple tags */ 
		}
 	else
		{
			UT_DEBUGMSG(("(defaulting to </p>)\n"));

			if (tagTop () == TT_P)
				{
					m_utf8_1 = "p";
					tagClose (TT_P, m_utf8_1);
				}
		}
	m_bInBlock = false;
}




void s_HTML_Listener::_convertColor(UT_String& szDest, const char* pszColor)
{
	/*
	  TODO we might want to be a little more careful about this.
	  The proper HTML color is #rrggbb, which is basically the same
	  as what we use this.  HTML browsers are likely to be more
	  forgiving than we are, so this is probably not a big
	  problem.
	*/
	szDest = pszColor;
}

void s_HTML_Listener::_convertFontSize(UT_String& szDest, const char* pszFontSize)
{
	double fSizeInPoints = UT_convertToPoints(pszFontSize);

	/*
	  TODO we can probably come up with a mapping of font sizes that
	  is more accurate than the code below.  I just guessed.
	*/
	
	if (fSizeInPoints <= 7)
	{
		szDest = "1";
	}
	else if (fSizeInPoints <= 10)
	{
		szDest = "2";
	}
	else if (fSizeInPoints <= 12)
	{
		szDest = "3";
	}
	else if (fSizeInPoints <= 16)
	{
		szDest = "4";
	}
	else if (fSizeInPoints <= 24)
	{
		szDest = "5";
	}
	else if (fSizeInPoints <= 36)
	{
		szDest = "6";
	}
	else
	{
		szDest = "7";
	}
}

/*
  Note that I've gone to lots of trouble to make sure
  that the HTML formatting tags are properly nested.
  The properties/tags are checked in the exact opposite
  order in closeSpan as they are in openSpan.  I guess
  what I SHOULD have done is write them out haphazardly,
  since most web browsers have to be able to handle that
  kind of &^%#&^ anyway.  But if I did that, someone
  might get the impression that I'm still holding a grudge
  or something.  :-)	--EWS
*/

void s_HTML_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (m_bFirstWrite)
	{
		_outputBegin(api);
	}

	if (!m_bInBlock)
	{
		return;
	}
	
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	bool span = false;
	bool textD = false;
#ifdef BIDI_ENABLED
	bool bDir = false;
#endif
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;
		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
			if (!span)
			{
			    m_pie->write("<span style=\"font-weight: bold");	
			    span = true;
			}
			else
			{
			    m_pie->write("; font-weight: bold");
			}
		}
		
		if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
			if (!span)
			{
			    m_pie->write("<span style=\"font-style: italic");	
			    span = true;
			}
			else
			{
			    m_pie->write("; font-style: italic");
			}
		}
		
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;
			
			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");
			
			while (q)
			{
				if (0 == UT_strcmp(q, "underline"))
				{
					if (!span)
				    {
						m_pie->write("<span style=\"text-decoration: underline");	
						span = true;
						textD = true;
				    }
					else if (!textD)
				    {
				        m_pie->write("; text-decoration: underline");
						textD = true;
				    }
					else
				    {
						m_pie->write(" underline");
				    }
				}
				
				q = strtok(NULL, " ");
			}
			
			free(p);
		}
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;
			
			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");
			
			while (q)
			{
				if (0 == UT_strcmp(q, "line-through"))
				{
					if (!span)
				    {
						m_pie->write("<span style=\"text-decoration: line-through");	
						span = true;
						textD = true;
				    }
				  else if (!textD)
				    {
				        m_pie->write("; text-decoration: line-through");
						textD = true;
				    }
					else
				    {
				        m_pie->write(" line-through");
				    }
				}
				
				q = strtok(NULL, " ");
			}
			
			free(p);
		}
		
		if (
			(pAP->getProperty("text-decoration", szValue))
			)
		{
			const XML_Char* pszDecor = szValue;
			
			XML_Char* p;
			if (!UT_cloneString((char *&)p, pszDecor))
			{
				// TODO outofmem
			}
			
			UT_ASSERT(p || !pszDecor);
			XML_Char*	q = strtok(p, " ");
			
			while (q)
			{
				if (0 == UT_strcmp(q, "overline"))
				{
					if (!span)
				    {
						m_pie->write("<span style=\"text-decoration: overline");	
						span = true;
						textD = true;
				    }
					else if (!textD)
				    {
				        m_pie->write("; text-decoration: overline");
						textD = true;
				    }
					else
				    {
				        m_pie->write("; overline");
				    }
				}
				
				q = strtok(NULL, " ");
			}
			
			free(p);
		}
		
		if (pAP->getProperty("text-position", szValue))
		{
		  if (!span)
		    {
		      m_pie->write("<span style=\"vertical-align: ");	
		      span = true;
		    }
		  else 
		    {
		      m_pie->write("; vertical-align: ");
		    }

			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("super");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("sub");
			}
		}
		
		if (
			(pAP->getProperty("color", szValue))
		    || (pAP->getProperty("font-size", szValue))
		    || (pAP->getProperty("font-family", szValue))
			|| (pAP->getProperty("bgcolor", szValue))
			)
		{
			const XML_Char* pszColor = NULL;
			const XML_Char* pszBgColor = NULL;
			const XML_Char* pszFontSize = NULL;
			const XML_Char* pszFontFamily = NULL;

			pAP->getProperty("color", pszColor);
			pAP->getProperty("font-size", pszFontSize);
			pAP->getProperty("font-family", pszFontFamily);
			pAP->getProperty("bgcolor", pszBgColor);

			if (pszColor)
			{
				if (!span)
				  {
				    if(IS_TRANSPARENT_COLOR(pszColor))
				      m_pie->write("<span style=\"");
				    else
				      {
					m_pie->write("<span style=\"color:#");
					m_pie->write(pszColor);
				      }
				    span = true;
				}
				else 
				{
				  if(!IS_TRANSPARENT_COLOR(pszColor))
				    {
				      m_pie->write("; color:#");	
				      m_pie->write(pszColor);
				    }
				}
			}
			
			if (pszFontFamily)
			{
				if (!span)
				{
					m_pie->write("<span style=\"font-family: ");	
					span = true;
				}
				else 
				{
					m_pie->write("; font-family: ");	
				}
				
				if(UT_strcmp((char*)pszFontFamily, "serif") != 0 ||
				   UT_strcmp((char*)pszFontFamily, "sans-serif") != 0 ||
				   UT_strcmp((char*)pszFontFamily, "cursive") != 0 ||
				   UT_strcmp((char*)pszFontFamily, "fantasy") != 0 ||
				   UT_strcmp((char*)pszFontFamily, "monospace") != 0)
				{
					m_pie->write("\'");
					m_pie->write(pszFontFamily);
					m_pie->write("\'");
				}						// only quote non-keyword family names
				else 
				{
					m_pie->write(pszFontFamily);
				}
	
			}
			
			if (pszFontSize)
			{
				if (!span)
				{
					char * old_locale = setlocale (LC_NUMERIC, "C");
					m_pie->write(UT_String_sprintf("<span style=\"font-size: %fpt", UT_convertToPoints(pszFontSize)));
					setlocale (LC_NUMERIC, old_locale);
					span = true;
				}
				else 
				{
					char * old_locale = setlocale (LC_NUMERIC, "C");
					m_pie->write(UT_String_sprintf("; font-size: %fpt", UT_convertToPoints(pszFontSize)));
					setlocale (LC_NUMERIC, old_locale);
				}
			}
			
			if (pszBgColor && !IS_TRANSPARENT_COLOR(pszBgColor))
			{
				if (!span)
				{
					m_pie->write("<span style=\"background: #");	
					m_pie->write(pszBgColor);
					span = true;
				}
				else 
				{
					m_pie->write("; background: #");
					m_pie->write(pszBgColor);
				}
			}
			else if (pszBgColor && !span)
			  {
			    m_pie->write("<span style=\"");
			    span = true;
			  }
		}

		char* szStyle = NULL;
		const XML_Char * pStyle;
		PD_Style* s = NULL;
		bool fnd = pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pStyle);
		if(pStyle && fnd)
		{
			szStyle = removeWhiteSpace((const char *)pStyle);
			m_pDocument->getStyle((char*) pStyle, &s);
		}

		m_bInSpan = true;
#ifdef BIDI_ENABLED
/*
	if the dir-override is set, or dir is 'rtl' or 'ltr', we will output
	the dir property; however, this property cannot be within a style 
	sheet, so anything that needs to be added to this code and belongs 
	withing a style property must be above us; further it should be noted 
	that there is a good chance that the html browser will not handle it 
	correctly. For instance IE will take dir=rtl as an indication that 
	the span should have rtl placement on a line, but it will ignore this 
	value when printing the actual span.
*/
		if(!span && (pAP->getProperty("dir-override", szValue) /*|| pAP->getProperty("dir", szValue)###TF*/))
		{
			if(*szValue == 'r' || *szValue == 'l')
			{
				m_pie->write("<span dir=\"");
				m_pie->write(szValue);
				bDir = true;
				span = true;
			}
		}

#endif
		if (span)
		{
			m_pie->write("\"");
#ifdef BIDI_ENABLED
			if (!bDir && (pAP->getProperty("dir-override", szValue) /*|| pAP->getProperty("dir", szValue)###TF*/))
			{
				if(*szValue == 'r' || *szValue == 'l')
				{
					m_pie->write(" dir=\"");
					m_pie->write(szValue);
					bDir = true;
					m_pie->write("\"");
				}
			}
#endif

			if(szStyle)
			{
				m_pie->write(" class=\"");
				_outputInheritanceLine(szStyle);
				m_pie->write("\"");
			}
			m_pie->write(">");
		}
		else if(szStyle && s && s->isCharStyle())
		{
			m_pie->write("<span class=\"");
			_outputInheritanceLine(szStyle);
			m_pie->write("\">");
		}
		else
		{	/* a span was opened which didn't contain any information
			to be exported to html */
			m_bInSpan = false;
		}
		FREEP(szStyle);
		
		m_pAP_Span = pAP;
	}
}

void s_HTML_Listener::_closeSpan(void)
{
	if (!m_bInSpan)
		return;

	const PP_AttrProp * pAP = m_pAP_Span;
	
	if (pAP)
	{

		bool closeSpan = false;
		const XML_Char * szValue;
		
		if (
			(pAP->getProperty("color", szValue))
		    || (pAP->getProperty("font-size", szValue))
		    || (pAP->getProperty("font-family", szValue))
		    || (pAP->getProperty("bgcolor", szValue))
			)
		{
			closeSpan = true;
		}
		
		else if (pAP->getProperty("text-position", szValue))
		{
			closeSpan = true;
		}

		else if (
			(pAP->getProperty("text-decoration", szValue))
			&& UT_strcmp(szValue, "none")
			)
		{
			closeSpan = true;
		}

		else if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
			closeSpan = true;
		}
		
		else if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
			closeSpan = true;
		}

		else if(pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szValue))
		{
			closeSpan = true;
		}

		if (closeSpan)
		{
		    m_pie->write("</span>");
		}

		m_pAP_Span = NULL;
	}

	m_bInSpan = false;
	return;
}

void s_HTML_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	UT_String sBuf;
	const UT_UCSChar * pData;

	if (!m_bInBlock)
	{
		return;
	}

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
		if (*pData != ' ' && *pData != '\t')
			// There has been some non-whitespace data output
			m_bWroteText = true;
		
		// Check to see if the character which follows this one is whitespace 
		pData++;
		if (*pData == ' ' || *pData == '\t')
		{
			m_bNextIsSpace = true;
		}
		else
		{
			m_bNextIsSpace = false;
		}
		pData--;

		switch (*pData)
		{
		case '<':
			sBuf += "&lt;";
			pData++;
			break;
			
		case '>':
			sBuf += "&gt;";
			pData++;
			break;
			
		case '&':
			sBuf += "&amp;";
			pData++;
			break;

		case ' ':
		case '\t':
		  // try to honor multiple spaces

			if(m_bNextIsSpace)
			{
				sBuf += "&nbsp;";
				pData++;
			}
			else
			{
				sBuf += *pData;
				pData++;
			}
			break;

		case UCS_LF:			// LF -- representing a Forced-Line-Break
			if (!m_bIs4)
			{
				sBuf += "<br />";
			}
			else
			{
				sBuf += "<br>";
			}
			pData++;
			break;

		case UCS_RQUOTE:				// Smart quotes get translated
			sBuf += "\'";				// back into normal quotes
			pData++;
			break;						// TODO: This handles apostrophes
										// (smart single right quotes)
										// what about the other types?
		case UCS_LQUOTE:
			sBuf += "\'";
			pData++;
			break;

		case UCS_RDBLQUOTE:
			sBuf += "&rdquo;";
			pData++;
			break;

		case UCS_LDBLQUOTE:
			sBuf += "&ldquo;";
			pData++;
			break;

		case UCS_EN_DASH:
		case UCS_EM_DASH:
			sBuf += "-";
			pData++;
			break;

		case UCS_FF:					// page break, convert to line break
			if (!m_bIs4)
			{
				sBuf += "<br />";
			}
			else
			{
				sBuf += "<br>";
			}
			pData++;
			break;
		
		default:
			if (*pData > 0x007f)
			{
#ifdef IE_EXP_HTML_UTF8_OPTIONAL
				// "try_nativeToU(0xa1) == 0xa1" looks dodgy to me (fjf)
				if(XAP_EncodingManager::get_instance()->isUnicodeLocale() || 
				   (XAP_EncodingManager::get_instance()->try_nativeToU(0xa1) == 0xa1))
				{
#endif /* IE_EXP_HTML_UTF8_OPTIONAL */
					XML_Char * pszUTF8 = UT_encodeUTF8char(*pData++);
					while (*pszUTF8)
					{
						sBuf += (char)*pszUTF8;
						pszUTF8++;
					}
#ifdef IE_EXP_HTML_UTF8_OPTIONAL
				}
				else
				{
					/*
					  Try to convert to native encoding and if
					  character fits into byte, output raw byte. This 
					  is somewhat essential for single-byte non-latin
					  languages like russian or polish - since
					  tools like grep and sed can be used then for
					  these files without any problem.
					  Networks and mail transfers are 8bit clean
					  these days.  - VH
					*/
					UT_UCSChar c = XAP_EncodingManager::get_instance()->try_UToNative(*pData);
					if (c==0 || c>255)
					{
					  sBuf += UT_String_sprintf("&#x%x;",*pData++);
					}
					else
					{
						sBuf += (char)c;
						pData++;
					}
				}
#endif /* IE_EXP_HTML_UTF8_OPTIONAL */
			}
			else
			{
				sBuf += (char)*pData++;
			}
			break;
		}
	}

	m_pie->write(sBuf.c_str(),sBuf.size());
}

/*!	This function returns true if the name of the PD_Style which style is based
	on, without whitespace, is the same as `from`, and otherwise returns false.
 */

bool s_HTML_Listener::_inherits(const char* style, const char* from)
{
	bool bret = false;

	PD_Style* pStyle = NULL;
	char* szName = NULL;
	const XML_Char * pName = NULL;
	
	if (m_pDocument->getStyle (style, &pStyle)) {
		
		if(pStyle && pStyle->getBasedOn())
		{
			pStyle = pStyle->getBasedOn();
//
// The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME attribute within the
// style
//
			pStyle->getAttribute(PT_NAME_ATTRIBUTE_NAME, 
								 pName);
			szName = removeWhiteSpace(pName);
			
			if(UT_strcmp(from, szName) == 0)
				bret = true;
			
			FREEP(szName);
		}
	}

	return bret;
}

s_HTML_Listener::s_HTML_Listener (PD_Document * pDocument, IE_Exp_HTML * pie, bool is4) :
	m_pDocument (pDocument),
	m_pie(pie),
	m_bInSection(false),
	m_bInBlock(false),
	m_bInSpan(false),
	m_bNextIsSpace(false),
	m_bWroteText(false),
	m_bFirstWrite(true),
	m_bIs4(is4),
	m_pAP_Span(0),
	m_iBlockType(0),
	m_iListDepth(0),
	m_iPrevListDepth(0),
	m_iImgCnt(0),
	m_styleIndent(0)
{
	// 
}

s_HTML_Listener::~s_HTML_Listener()
{
	_closeSpan();
	_closeTag();

	listPopToDepth (0);

	_closeSection();
	_handleDataItems();

	_outputEnd ();

	UT_VECTOR_FREEALL(char*, m_utvDataIDs);
}

bool s_HTML_Listener::populate(PL_StruxFmtHandle /*sfh*/,
									  const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = 
				static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			if (api)
			{
				_openSpan(api);
			}
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			if (api)
				_closeSpan();
			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			m_bWroteText = true;
			const PX_ChangeRecord_Object * pcro = 
				static_cast<const PX_ChangeRecord_Object *> (pcr);
			const XML_Char* szValue;
			UT_String buf;

			fd_Field* field;
			PT_AttrPropIndex api = pcr->getIndexAP();
			const PP_AttrProp * pAP = NULL;
			bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

			switch (pcro->getObjectType())
			{
			case PTO_Image:
				// TODO: differentiate between SVG and PNG
				// TODO: we do this in the img saving code

				if(bHaveProp && pAP && pAP->getAttribute("dataid", szValue))
				{
					char* dataid = strdup((char*) szValue);

					m_utvDataIDs.push_back(dataid);

					char * temp = _stripSuffix(UT_basename(szValue), '_');
					char * fstripped = _stripSuffix(temp, '.');
					FREEP(temp);
					UT_String_sprintf(buf, "%s.png", fstripped);
					FREEP(fstripped);
					
					m_pie->write("<img alt=\"AbiWord Image ");
					m_pie->write(buf);
					m_pie->write("\" src=\"");
					m_pie->write(UT_basename(m_pie->getFileName()));
					m_pie->write("_data/");
					m_pie->write(buf);
					m_pie->write("\" ");
					
					const XML_Char * szWidth = 0;
					const XML_Char * szHeight = 0;

					if(pAP->getProperty("width", szWidth) &&
					   pAP->getProperty("height", szHeight))
					  {
					    if(szWidth)
					      {
						UT_String_sprintf(buf, "%d", (int) UT_convertToDimension(szWidth, DIM_PX));
						m_pie->write (" width=\"");
						m_pie->write (buf);
						m_pie->write ("\" ");
					      }
					    if(szHeight)
					      {
						UT_String_sprintf(buf, "%d", (int) UT_convertToDimension(szHeight, DIM_PX));
						m_pie->write (" height=\"");
						m_pie->write (buf);
						m_pie->write ("\" ");
					      }
					  }

					// close the img tag with "/" only in XHTML
					if (! m_bIs4) {
						m_pie->write(" />\r\n");
					}
					else {
						m_pie->write(" >\r\n");
					}
				}
				return true;

			case PTO_Field:
				if(bHaveProp && pAP && pAP->getAttribute("type", szValue))
				{
					field = pcro->getField();

					if(UT_strcmp(szValue, "list_label") != 0)
					{
						m_pie->write("<span class=\"ABI_FIELD_");
						m_pie->write(szValue);
						m_pie->write("\">");
						m_pie->write(field->getValue());
						m_pie->write("</span>");
					}
				}
				return true;

			case PTO_Hyperlink:
				if(bHaveProp && pAP && pAP->getAttribute("xlink:href", szValue))
				{
					m_pie->write("<a href=\"");
					m_pie->write(szValue);
					m_pie->write("\">");
				}
				else
				{
					m_pie->write("</a>");
				}
				return true;

			case PTO_Bookmark:
				if(bHaveProp && pAP && pAP->getAttribute("type", szValue))
				{
					if( UT_XML_stricmp(szValue, "start") == 0 )
					{
						pAP->getAttribute("name", szValue);
						m_pie->write("<a name=\"");
						m_pie->write(szValue);
						m_pie->write("\">");
					}
					else
					{
						m_pie->write("</a>");
					}
				}
				return true;

			default:
				UT_ASSERT(0);
				return false;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_HTML_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
										   const PX_ChangeRecord * pcr,
										   PL_StruxFmtHandle * psfh)
{
	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);
	*psfh = 0;							// we don't need it.

	switch (pcrx->getStruxType())
	{
	case PTX_SectionHdrFtr:
	case PTX_Section:
	{
		_closeSpan();
		_closeTag();
		_closeSection();

		PT_AttrPropIndex indexAP = pcr->getIndexAP();
		const PP_AttrProp* pAP = NULL;
		if (m_pDocument->getAttrProp(indexAP, &pAP) && pAP)
		{
			const XML_Char* pszSectionType = NULL;
			pAP->getAttribute("type", pszSectionType);
			if (
				!pszSectionType
				|| (0 == UT_strcmp(pszSectionType, "doc"))
				)
			{
				_openSection(pcr->getIndexAP());
				m_bInSection = true;
			}
			else
			{
#if 1
			  // export headers & footers
				m_bInSection = true;
				_openSection(pcr->getIndexAP());
#else
				// don't export headers and footers
				m_bInSection = false ;
#endif
			}
		}
		else
		{
			m_bInSection = false;
		}
		
		return true;
	}

	case PTX_Block:
	{
		_closeSpan();
		_closeTag();
		_openTag(pcr->getIndexAP());
		return true;
	}

	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_HTML_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_HTML_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
									 const PX_ChangeRecord * /*pcr*/,
									 PL_StruxDocHandle /*sdh*/,
									 PL_ListenerId /* lid */,
									 void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
																 PL_ListenerId /* lid */,
																 PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_HTML_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}


/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_HTML::_writeDocument(void)
{
  bool err = true;
	m_pListener = new s_HTML_Listener(getDoc(),this, m_bIs4);
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	if (getDocRange())
	  err = getDoc()->tellListenerSubset(static_cast<PL_Listener *>(m_pListener),getDocRange());
	else 
	  err = getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener)) ;
	DELETEP(m_pListener);
	
	if ( m_error == UT_OK && err == true )
	  return UT_OK;
	return UT_IE_COULDNOTWRITE;
}

/*****************************************************************/
/*****************************************************************/

/*!
   removes the suffix from a string by searching backwards for the specified 
   character delimiter. If the delimiter is not found, a copy of the original 
   string is returned
   
   eg. _stripSuffix("/home/user/file.png, '.') returns "/home/user/file" 
       _stripSuffix("/home/user/foo_bar, '_') returns /home/user/foo 
       _stripSuffix("/home/user/file.png, '_') returns /home/user/file.png"
*/
char *s_HTML_Listener::_stripSuffix(const char* from, char delimiter)
{
    char * fremove_s = (char *)malloc(strlen(from)+1);
    strcpy(fremove_s, from);   

    char * p = fremove_s + strlen(fremove_s);
    while ((p >= fremove_s) && (*p != delimiter))
        p--;
	
    if (p >= fremove_s)
	*p = '\0';
    
    return fremove_s;
}

void s_HTML_Listener::_handleDataItems(void)
{
 	const char * szName;
	const char * szMimeType;
	const UT_ByteBuf * pByteBuf;
	
	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,(void**)&szMimeType)); k++)
	{
		UT_sint32 loc = -1;
		for (UT_uint32 i = 0; i < m_utvDataIDs.getItemCount(); i++)
		{
			if(UT_strcmp((char*) m_utvDataIDs[i], szName) == 0)
			{
				loc = i;
				break;
			}
		}
		
		if(loc > -1)
		{
			FILE *fp;
			UT_String fname; // EVIL EVIL bad hardcoded buffer size
			
			UT_String_sprintf(fname, "%s_data", m_pie->getFileName());
			int result = m_pDocument->getApp()->makeDirectory(fname.c_str(), 0750);
			
			if (!UT_strcmp(szMimeType, "image/svg-xml"))
				UT_String_sprintf(fname, "%s/%s_%d.svg", fname.c_str(), szName, loc);
			if (!UT_strcmp(szMimeType, "text/mathml"))
				UT_String_sprintf(fname, "%s/%s_%d.mathml", fname.c_str(), szName, loc);
			else // PNG Image
			{  
				char * temp = _stripSuffix(UT_basename(szName), '_');
				char * fstripped = _stripSuffix(temp, '.');
				FREEP(temp);
				UT_String_sprintf(fname, "%s/%s.png", fname.c_str(), fstripped);
				FREEP(fstripped);
			}
			
			if (!UT_isRegularFile(fname.c_str()))
			{
			    fp = fopen (fname.c_str(), "wb+");
			
			    if(!fp)
				    continue;
			
			    int cnt = 0, len = pByteBuf->getLength();
			
			    while (cnt < len)
			    {
				    cnt += fwrite (pByteBuf->getPointer(cnt), 
							     sizeof(UT_Byte), len-cnt, fp);
			    }
			
			    fclose(fp);
			}
		}
	}
	
	return;
}
