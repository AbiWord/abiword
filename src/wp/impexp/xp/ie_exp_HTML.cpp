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

#include "ut_debugmsg.h"
#include "ut_assert.h"
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
#include "pl_Listener.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

#include "fd_Field.h"

#include "fl_AutoNum.h"

#include "ie_impexp_HTML.h"
#include "ie_exp_HTML.h"

#ifdef HTML_TABLES_SUPPORTED
#include "ie_Table.h"
#endif

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML_Sniffer::IE_Exp_HTML_Sniffer ()
#ifdef HTML_NAMED_CONSTRUCTORS
	: IE_ExpSniffer(IE_IMPEXPNAME_XHTML)
#endif
{
	// 
}

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

#ifdef HTML_ENABLE_HTML4

// HTML 4

IE_Exp_HTML4_Sniffer::IE_Exp_HTML4_Sniffer ()
#ifdef HTML_NAMED_CONSTRUCTORS
	: IE_ExpSniffer(IE_IMPEXPNAME_HTML)
#endif
{
	// 
}

bool IE_Exp_HTML4_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!(UT_stricmp (szSuffix, ".html")) || !(UT_stricmp (szSuffix, ".htm")));
}

UT_Error IE_Exp_HTML4_Sniffer::constructExporter (PD_Document * pDocument,
												  IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
	if (p) p->set_HTML4 ();
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

#endif /* HTML_ENABLE_HTML4 */

#ifdef HTML_ENABLE_PHTML

// XHTML w/ PHP instructions for AbiWord Web Docs

IE_Exp_PHTML_Sniffer::IE_Exp_PHTML_Sniffer ()
#ifdef HTML_NAMED_CONSTRUCTORS
	: IE_ExpSniffer(IE_IMPEXPNAME_PHTML)
#endif
{
	// 
}

bool IE_Exp_PHTML_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!(UT_stricmp (szSuffix, ".phtml")));
}

UT_Error IE_Exp_PHTML_Sniffer::constructExporter (PD_Document * pDocument,
												  IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
	if (p) p->set_PHTML ();
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_PHTML_Sniffer::getDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "AbiWord Web Document (.phtml)";
	*pszSuffixList = "*.phtml";
	*ft = getFileType();
	return true;
}

#endif /* HTML_ENABLE_PHTML */

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML::IE_Exp_HTML (PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_exp_opt.bIs4         = false;
	m_exp_opt.bIsAbiWebDoc = false;

	m_error = UT_OK;
}

IE_Exp_HTML::~IE_Exp_HTML ()
{
	// 
}

/*****************************************************************/
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
	space in the process.
 */
static char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str)
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
	s_HTML_Listener (PD_Document * pDocument, IE_Exp_HTML * pie, bool bClipBoard,
					 const IE_Exp_HTML_Options & exp_opt);

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
	void	_outputStyles (const PP_AttrProp * pAP);
	void	_openSection (PT_AttrPropIndex api);
	void	_closeSection (void);

	/* these next two use m_utf8_0
	 */
	void	_recordCSStyle (const char * ClassName);
	void	_appendInheritanceLine (const char * ClassName, UT_UTF8String & utf8str,
									bool record_css = false);

	void	_openTag (PT_AttrPropIndex api, PL_StruxDocHandle sdh);
	void	_closeTag (void);
	void	_closeSpan (void);
	void	_openSpan (PT_AttrPropIndex api);

#ifdef HTML_TABLES_SUPPORTED
	void	_openTable (PT_AttrPropIndex api);
	void	_closeTable ();
	void	_openCell (PT_AttrPropIndex api);
	void	_closeCell ();
#endif

	void	_outputData (const UT_UCSChar * p, UT_uint32 length);
	bool	_inherits (const char * style, const char * from);
	void	_storeStyles (void);
	
	void	_writeImage (const UT_ByteBuf * pByteBuf,
						 const UT_String & imagedir, const UT_String & filename);
	void	_handleImage (PT_AttrPropIndex api);
	void	_handleField (const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api);
	void	_handleHyperlink (PT_AttrPropIndex api);
	void	_handleBookmark (PT_AttrPropIndex api);

#ifdef HTML_META_SUPPORTED
	void    _handleMetaTag (const char * key, UT_UTF8String & value);
	void    _handleMeta ();
#endif

	PD_Document *		m_pDocument;
	IE_Exp_HTML *		m_pie;
	bool				m_bClipBoard;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInTList;
	bool				m_bInTListItem;
	bool				m_bInSpan;
	bool				m_bNextIsSpace;
	bool				m_bWroteText;
	bool				m_bFirstWrite;

#ifdef HTML_TABLES_SUPPORTED
	ie_Table		m_TableHelper;
#endif

	// Need to look up proper type, and place to stick #defines...
  
	UT_uint16		m_iBlockType;	// BT_*
	UT_uint16		m_iListDepth;	// 0 corresponds to not in a list
	UT_Stack		m_utsListType;
	UT_uint16		m_iImgCnt;
	UT_Wctomb		m_wmctomb;

	enum WhiteSpace
	{
		ws_None = 0,
		ws_Pre  = 1,
		ws_Post = 2,
		ws_Both = 3
	};

	/* low-level; these may use m_utf8_0 but not m_utf8_1
	 */
	void			tagNewIndent (UT_uint32 extra = 0);
	void			tagOpenClose (const UT_UTF8String & content, bool suppress,
								  WhiteSpace ws = ws_Both);
	void			tagOpen  (UT_uint32 tagID, const UT_UTF8String & content,
							  WhiteSpace ws = ws_Both);
	void			tagClose (UT_uint32 tagID, const UT_UTF8String & content,
							  WhiteSpace ws = ws_Both);
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
	void			textTrusted (const UT_UTF8String & text);
	void			textUntrusted (const char * text);

	/* for emulation of lists using tables:
	 */
	fl_AutoNum *	tlistLookup (UT_uint32 listid);
	void			tlistNumber (fl_AutoNum * list, PL_StruxDocHandle sdh);
	void			tlistPush ();
	void			tlistPushItem (const XML_Char * szListID,
								   const XML_Char * szMarginLeft, PL_StruxDocHandle sdh);
	void			tlistPop ();
	void			tlistPopItem ();

	UT_uint16		listDepth ();
	UT_uint16		listType ();
	void			listPush (UT_uint16 type, const char * ClassName);
	void			listPop ();
	void			listPopToDepth (UT_uint16 depth);

	const char *	bodyStyle (const char * key);
	const char *	bodyStyle (const char * key, const char * value);
	void			bodyStyleClear ();

	const char *	blockStyle (const char * key);
	const char *	blockStyle (const char * key, const char * value);
	void			blockStyleClear ();

	bool			compareStyle (const char * key, const char * value);

	/* temporary strings; use with extreme caution
	 */
	UT_UTF8String	m_utf8_0; // low-level
	UT_UTF8String	m_utf8_1; // intermediate

	UT_UTF8String	m_utf8_span; // span tag-string cache

	UT_Stack		m_tagStack;

	UT_uint32		m_styleIndent;

	UT_uint32		m_tlistIndent;
	UT_uint32		m_tlistListID;
	List_Type		m_tlistType;

	UT_StringPtrMap	m_BodyStyle;
	UT_StringPtrMap	m_BlockStyle;

	/* Export Options
	 */
	bool			m_bIs4;
	bool			m_bIsAbiWebDoc;
};

const char * s_HTML_Listener::bodyStyle (const char * key)
{
	if ( key == 0) return 0;
	if (*key == 0) return 0;

	const void * vptr = m_BodyStyle.pick (key);
	return reinterpret_cast<const char *>(vptr);
}

const char * s_HTML_Listener::bodyStyle (const char * key, const char * value)
{
	if ( key == 0) return 0;
	if (*key == 0) return 0;

	const void * vptr = m_BodyStyle.pick (key);
	if (vptr)
		{
			m_BodyStyle.remove (key, 0);
			free (const_cast<void *>(vptr));
		}
	if (value == 0) return 0;

	char * new_value = UT_strdup (value);
	if (new_value == 0) return 0; // ??

	if (!m_BodyStyle.insert (key, new_value))
		{
			free (new_value);
			new_value = 0;
		}
	return new_value;
}

void s_HTML_Listener::bodyStyleClear ()
{
	UT_HASH_PURGEDATA (char *, &m_BodyStyle,  free);
	m_BodyStyle.clear ();
}

const char * s_HTML_Listener::blockStyle (const char * key)
{
	if ( key == 0) return 0;
	if (*key == 0) return 0;

	const void * vptr = m_BlockStyle.pick (key);
	return reinterpret_cast<const char *>(vptr);
}

const char * s_HTML_Listener::blockStyle (const char * key, const char * value)
{
	if ( key == 0) return 0;
	if (*key == 0) return 0;

	const void * vptr = m_BlockStyle.pick (key);
	if (vptr)
		{
			m_BlockStyle.remove (key, 0);
			free (const_cast<void *>(vptr));
		}
	if (value == 0) return 0;

	char * new_value = UT_strdup (value);
	if (new_value == 0) return 0; // ??

	if (!m_BlockStyle.insert (key, new_value))
		{
			free (new_value);
			new_value = 0;
		}
	return new_value;
}

void s_HTML_Listener::blockStyleClear ()
{
	UT_HASH_PURGEDATA (char *, &m_BlockStyle, free);
	m_BlockStyle.clear ();
}

bool s_HTML_Listener::compareStyle (const char * key, const char * value)
{
	/* require both key & value to be non-empty strings
	 */
	if (( key == 0) || ( value == 0)) return false;
	if ((*key == 0) || (*value == 0)) return false;

	bool match = true;

	const char * css_value = blockStyle (key);
	if (css_value == 0)
		{
			css_value = bodyStyle (key);
			if (css_value == 0)
				{
					match = false;
				}
		}
	if (match) match = (UT_strcmp (css_value, value) == 0);

	return match;
}

void s_HTML_Listener::tagNewIndent (UT_uint32 extra)
{
	m_utf8_0 = "";

	UT_uint32 depth = m_tagStack.getDepth () + extra;

	UT_uint32 i;  // MSVC DOES NOT SUPPORT CURRENT for SCOPING RULES!!!
	for (i = 0; i < (depth >> 3); i++) m_utf8_0 += "\t";
	for (i = 0; i < (depth &  7); i++) m_utf8_0 += " ";
}

/* NOTE: We terminate each line with a \r\n sequence to make IE think
 *       that our XHTML is really HTML. This is a stupid IE bug.
 *       Sorry.
 */

void s_HTML_Listener::tagOpenClose (const UT_UTF8String & content, bool suppress,
									WhiteSpace ws)
{
	if (ws & ws_Pre)
		tagNewIndent ();
	else
		m_utf8_0 = "";

	m_utf8_0 += "<";
	m_utf8_0 += content;
	if (suppress)
		m_utf8_0 += ">";
	else
		m_utf8_0 += " />";

	if (ws & ws_Post) m_utf8_0 += "\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_HTML_Listener::tagOpen (UT_uint32 tagID, const UT_UTF8String & content,
							   WhiteSpace ws)
{
	if (ws & ws_Pre)
		tagNewIndent ();
	else
		m_utf8_0 = "";

	m_utf8_0 += "<";
	m_utf8_0 += content;
	m_utf8_0 += ">";

	if (ws & ws_Post) m_utf8_0 += "\r\n";

	m_pie->write (m_utf8_0.utf8_str ());

	void * vptr = reinterpret_cast<void *>(tagID);
	m_tagStack.push (vptr);
}

void s_HTML_Listener::tagClose (UT_uint32 tagID, const UT_UTF8String & content,
								WhiteSpace ws)
{
	tagClose (tagID);

	if (ws & ws_Pre)
		tagNewIndent ();
	else
		m_utf8_0 = "";

	m_utf8_0 += "</";
	m_utf8_0 += content;
	m_utf8_0 += ">";

	if (ws & ws_Post) m_utf8_0 += "\r\n";

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

void s_HTML_Listener::textTrusted (const UT_UTF8String & text)
{
	if (text.byteLength ())
	{
		m_bWroteText = true;
		m_pie->write (text.utf8_str ());
	}
}

void s_HTML_Listener::textUntrusted (const char * text)
{
	if ( text == 0) return;
	if (*text == 0) return;

	m_utf8_0 = "";

	char buf[2];
	buf[1] = 0;

	const char * ptr = text;
	while (*ptr)
		{
			if ((*ptr & 0x7f) == *ptr) // ASCII
				{
					switch (*ptr)
						{
						case '<':
							m_utf8_0 += "&lt;";
							break;
						case '>':
							m_utf8_0 += "&gt;";
							break;
						case '&':
							m_utf8_0 += "&amp;";
							break;
						default:
							buf[0] = *ptr;
							m_utf8_0 += buf;
							break;
						}
				}
			/* TODO: translate non-ASCII characters
			 */
			ptr++;
		}
	if (m_utf8_0.byteLength ()) m_pie->write (m_utf8_0.utf8_str ());
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
	if (!m_bIs4)
		{
			if (!m_bIsAbiWebDoc) // PHP doesn't like <?xml ?> at the start, for some reason
				{                // ?? belongs to a different option... [TODO]
					m_utf8_1 = "version=\"1.0\"";
					tagPI ("xml", m_utf8_1);
				}
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
	if (!m_bIs4)
		{
			m_utf8_1 += " xmlns=\"http://www.w3.org/1999/xhtml\"";
#ifndef HTML_NO_AWML
			m_utf8_1 += " xmlns:awml=\"http://www.abisource.com/awml.dtd\"";
#endif
		}
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

#ifdef HTML_META_SUPPORTED
	UT_UTF8String titleProp;

	if (m_pDocument->getMetaDataProp (PD_META_KEY_TITLE, titleProp) && titleProp.size ())
		textTrusted (titleProp.escapeXML ());
	else
		textUntrusted (m_pie->getFileName ());
#else
	textUntrusted (m_pie->getFileName ());
#endif

	tagClose (TT_TITLE, m_utf8_1);

#ifdef HTML_META_SUPPORTED
	/* write out our metadata properties
	 */
	_handleMeta ();
#endif

	if (!m_bIsAbiWebDoc) // belongs to a different option... [TODO]
		{
			const PP_AttrProp * pAP = 0;
			bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

			if (bHaveProp) _outputStyles (pAP);
		}

	if (m_bIsAbiWebDoc)
		{
			m_utf8_1 = "\r\n  include($DOCUMENT_ROOT.'/x-header.php');\r\n ?>\r\n";
			tagPI ("php", m_utf8_1);
		}

	/* end <head> section of HTML document
	 */
	m_utf8_1 = "head";
	tagClose (TT_HEAD, m_utf8_1);

	/* start <body> section of HTML document
	 */
	m_utf8_1 = "body";
	tagOpen (TT_BODY, m_utf8_1);

	if (m_bIsAbiWebDoc)
		{
			m_utf8_1 = "\r\n  include($DOCUMENT_ROOT.'/x-page-begin.php');\r\n ?>\r\n";
			tagPI ("php", m_utf8_1);
		}

	m_bFirstWrite = false;
}

void s_HTML_Listener::_outputEnd ()
{
	if (m_bIsAbiWebDoc)
		{
			m_utf8_1 = "\r\n  include($DOCUMENT_ROOT.'/x-page-end.php');\r\n ?>\r\n";
			tagPI ("php", m_utf8_1);
		}

	/* end <body> section of HTML document
	 */
	m_utf8_1 = "body";
	tagClose (TT_BODY, m_utf8_1);

	/* end <head> section of HTML document
	 */
	m_utf8_1 = "html";
	tagClose (TT_HTML, m_utf8_1);
}

void s_HTML_Listener::_outputStyles (const PP_AttrProp * pAP)
{
	/* some cascading style rules
	 */
	const XML_Char * szName  = 0;
	const XML_Char * szValue = 0;

	m_utf8_1 = "style type=\"text/css\"";
	tagOpen (TT_STYLE, m_utf8_1);
	tagCommentOpen ();

	/* global page styles refer to the <body> tag
	 */
	PD_Style * pStyle = 0;
	m_pDocument->getStyle ("Normal", &pStyle);

	if (pAP && pStyle)
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

					/* keep a record of CSS body style
					 */
					bodyStyle ((const char *) szName, m_utf8_1.utf8_str ());

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

#ifdef HTML_TABLES_SUPPORTED
			m_utf8_1 = "table";
			styleOpen (m_utf8_1);

			m_utf8_1  = "100%";
			styleNameValue ("width", m_utf8_1);

			styleClose (); // end of: table { }

			m_utf8_1 = "td";
			styleOpen (m_utf8_1);

			m_utf8_1 = "left";
			styleNameValue ("text-align", m_utf8_1);

			m_utf8_1 = "top";
			styleNameValue ("vertical-align", m_utf8_1);

			styleClose (); // end of td
#endif /* HTML_TABLES_SUPPORTED */
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
					s_removeWhiteSpace ((const char *) szStyleName, m_utf8_0); // careful!!
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
}

void s_HTML_Listener::_openSection (PT_AttrPropIndex api)
{
	if (m_bFirstWrite) _outputBegin (api);

	if (m_bInSection) _closeSection ();

	m_utf8_1 = "div";
	tagOpen (TT_DIV, m_utf8_1);

	m_bInSection = true;
}

void s_HTML_Listener::_closeSection (void)
{
	if (m_bInSection && (tagTop () == TT_DIV))
		{
			m_utf8_1 = "div";
			tagClose (TT_DIV, m_utf8_1);
		}
	m_bInSection = false;
}

/*!	This function returns true if the name of the PD_Style which style is based
	on, without whitespace, is the same as `from`, and otherwise returns false.
 */
bool s_HTML_Listener::_inherits (const char * style, const char * from)
{
	if ((style == 0) || (from == 0)) return false;

	bool bret = false;

	PD_Style * pStyle = 0;
	
	if (m_pDocument->getStyle (style, &pStyle))
		if (pStyle)
			{
				PD_Style * pBasedOn = pStyle->getBasedOn ();
				if (pBasedOn)
					{
						/* The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME
						 * attribute within the style
						 */
						const XML_Char * szName = 0;
						pBasedOn->getAttribute (PT_NAME_ATTRIBUTE_NAME, szName);

						if (szName)
							{
								/* careful!!
								 */
								s_removeWhiteSpace ((const char *) szName, m_utf8_0);

								if (m_utf8_0.utf8_str ())
									bret = (UT_strcmp (from, m_utf8_0.utf8_str ()) == 0);
							}
					}
			}
	return bret;
}

void s_HTML_Listener::_recordCSStyle (const char * ClassName)
{
	if ( ClassName == 0) return;
	if (*ClassName == 0) return;

	PD_Style * pStyle = 0;

	m_pDocument->getStyle (ClassName, &pStyle);
	if (pStyle == 0) return;

	const XML_Char * szName  = 0;
	const XML_Char * szValue = 0;

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
							m_utf8_0 = (const char *) szValue;
						}
					else
						{
							m_utf8_0  = "'";
							m_utf8_0 += (const char *) szValue;
							m_utf8_0 += "'";
						}
				}
			else if (UT_strcmp (szName, "color") == 0)
				{
					if (IS_TRANSPARENT_COLOR (szValue)) continue;

					m_utf8_0  = "#";
					m_utf8_0 += (const char *) szValue;
				}
			else m_utf8_0 = (const char *) szValue;

			/* keep a record of CSS body style
			 */
			blockStyle ((const char *) szName, m_utf8_0.utf8_str ());
		}
}

void s_HTML_Listener::_appendInheritanceLine (const char * ClassName, UT_UTF8String & utf8str,
											  bool record_css)
{
	PD_Style * pStyle = 0;

	if (m_pDocument->getStyle (ClassName, &pStyle))
		if (pStyle)
			{
				PD_Style * pBasedOn = pStyle->getBasedOn ();
				if (pBasedOn)
					{
						/* The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME
						 * attribute within the style
						 */
						const XML_Char * szName = 0;
						pBasedOn->getAttribute (PT_NAME_ATTRIBUTE_NAME, szName);

						if (szName)
							{
#ifdef HTML_MULTIPLE_CLASS_INHERITANCE
								_appendInheritanceLine ((const char *) szName, utf8str, record_css);
								utf8str += " ";
#else
								/* need to recurse for style recording, but discard extra classes
								 */
								_appendInheritanceLine ((const char *) szName, m_utf8_0, record_css);
#endif
							}
					}
			}
	if (record_css) _recordCSStyle (ClassName);

	s_removeWhiteSpace (ClassName, m_utf8_0); // careful!!
	utf8str += m_utf8_0;
}

fl_AutoNum * s_HTML_Listener::tlistLookup (UT_uint32 listid)
{
	fl_AutoNum * pMatch = 0;
	fl_AutoNum * pAutoNum = 0;

	for (UT_uint32 k = 0; (m_pDocument->enumLists (k, &pAutoNum)); k++)
		if (pAutoNum)
			if (pAutoNum->getID () == listid)
				{
					pMatch = pAutoNum;
					break;
				}
	return pMatch;
}

void s_HTML_Listener::tlistNumber (fl_AutoNum * list, PL_StruxDocHandle sdh)
{
	fl_AutoNum * parent = list->getParent ();
	if (parent)
		{
			tlistNumber (parent, list->getParentItem ());
			textUntrusted (".");
		}
	UT_uint32 value = list->getValue (sdh);
	switch (m_tlistType)
		{
		default:
		case NUMBERED_LIST:
			{
				char buf[16];
				sprintf (buf, "%lu", (unsigned long) value);
				textUntrusted (buf);
			}
			break;
		case LOWERCASE_LIST:
			textUntrusted (fl_AutoNum::dec2ascii (value - 1, 97));
			break;
		case UPPERCASE_LIST:
			textUntrusted (fl_AutoNum::dec2ascii (value - 1, 65));
			break;
		case LOWERROMAN_LIST:
			textUntrusted (fl_AutoNum::dec2roman (value, true));
			break;
		case UPPERROMAN_LIST:
			textUntrusted (fl_AutoNum::dec2roman (value, false));
			break;
		}
}

void s_HTML_Listener::tlistPush ()
{
	if (m_bInTList) tlistPop ();

	/* list item, of any depth, is represented as a table with one
	 * row and two columns:
	 */
	m_utf8_1 = "table cols=\"2\" width=\"100%\"";
	tagOpen (TT_TABLE, m_utf8_1);

	if (IS_BULLETED_LIST_TYPE (m_tlistType))
		{
			m_utf8_1 = "tr";
			tagOpen (TT_TR, m_utf8_1);

			unsigned long indent = (m_tlistIndent > 18) ? (m_tlistIndent - 9) : 9;

			char buf[16];
			sprintf (buf, "%lu", indent);

			m_utf8_1  = "td width=\"";
			m_utf8_1 += buf;
			m_utf8_1 += "\"";
			tagOpen (TT_TD, m_utf8_1, ws_Pre);

			m_utf8_1 = "&nbsp;";
			textTrusted (m_utf8_1);

			m_utf8_1  = "td";
			tagClose (TT_TD, m_utf8_1, ws_Post);
			tagOpen (TT_TD, m_utf8_1);

			m_utf8_1 = "ul style=\"list-style-type: ";
			switch (m_tlistType)
				{
				case BULLETED_LIST:
				case TRIANGLE_LIST:
				case IMPLIES_LIST:
				case HAND_LIST:
					m_utf8_1 += "disc";
					break;

				case DASHED_LIST:
				case DIAMOND_LIST:
				case TICK_LIST:
				case HEART_LIST:
					m_utf8_1 += "circle";
					break;

				case SQUARE_LIST:
				case STAR_LIST:
				case BOX_LIST:
				default:
					m_utf8_1 += "square";
					break;
				}
			m_utf8_1 += "\"";
			tagOpen (TT_UL, m_utf8_1);
		}
	m_bInTList = true;
}

void s_HTML_Listener::tlistPushItem (const XML_Char * szListID,
									 const XML_Char * szMarginLeft, PL_StruxDocHandle sdh)
{
	if (m_bInTListItem) tlistPopItem ();

	if (szListID == 0) return;

	unsigned long listid;
	if (sscanf (szListID, "%lu", &listid) != 1) return;

	fl_AutoNum * list = tlistLookup (listid);
	if (list == 0)
		{
			UT_DEBUGMSG(("list lookup failed!\n"));
			return;
		}

	unsigned long indent = 18;
	if (szMarginLeft)
		{
			double dpts = UT_convertToPoints (szMarginLeft);
			if (dpts > 18) indent = static_cast<unsigned long>(dpts);
		}

	if (m_bInTList)
		if ((indent != (unsigned long) m_tlistIndent) ||
			(listid != (unsigned long) m_tlistListID))
			{
				/* different list or a change in indentation...
				 * need to start a new table
				 */
				tlistPop ();
			}
	if (!m_bInTList)
		{
			m_tlistListID = (UT_uint32) listid;
			m_tlistIndent = (UT_uint32) indent;

			m_tlistType = list->getType ();

			tlistPush ();
		}

	m_bInTListItem = true;

	if (IS_BULLETED_LIST_TYPE (m_tlistType))
		{
			/* by this point we're sitting in an unordered list;
			 * new list item will be created on return...
			 */
			return;
		}

	m_utf8_1 = "tr";
	tagOpen (TT_TR, m_utf8_1);

	char buf[16];
	sprintf (buf, "%lu", indent);

	m_utf8_1  = "td width=\"";
	m_utf8_1 += buf;
	m_utf8_1 += "\" style=\"text-align: right\" align=\"right\" valign=\"top\"";

	/* the left being the number/bullet point...
	 * (generate text/tags/etc. to represent the number/bullet point)
	 */
	tagOpen (TT_TD, m_utf8_1, ws_Pre);

	tlistNumber (list, sdh);

	m_utf8_1 = "td";
	tagClose (TT_TD, m_utf8_1, ws_Post);

	/* and the right column being the list item text/tags/etc.
	 * new table column will be created on return...
	 */
}

void s_HTML_Listener::tlistPop ()
{
	if (!m_bInTList) return;

	if (m_bInTListItem) tlistPopItem ();

	if (IS_BULLETED_LIST_TYPE (m_tlistType))
		{
			if (tagTop () == TT_UL)
				{
					m_utf8_1 = "ul";
					tagClose (TT_UL, m_utf8_1);
				}
			if (tagTop () == TT_TD)
				{
					m_utf8_1 = "td";
					tagClose (TT_TD, m_utf8_1);
				}
			if (tagTop () == TT_TR)
				{
					m_utf8_1 = "tr";
					tagClose (TT_TR, m_utf8_1);
				}
		}
	if (tagTop () == TT_TABLE)
		{
			m_utf8_1 = "table";
			tagClose (TT_TABLE, m_utf8_1);
		}
	m_bInTList = false;
}

void s_HTML_Listener::tlistPopItem ()
{
	if (!m_bInTListItem) return;

	if (IS_BULLETED_LIST_TYPE (m_tlistType))
		{
			if (tagTop () == TT_LI)
				{
					m_utf8_1 = "li";
					tagClose (TT_LI, m_utf8_1, ws_Post);
				}
		}
	else
		{
			if (tagTop () == TT_TD)
				{
					m_utf8_1 = "td";
					tagClose (TT_TD, m_utf8_1, ws_Post);
				}
			if (tagTop () == TT_TR)
				{
					m_utf8_1 = "tr";
					tagClose (TT_TR, m_utf8_1);
				}
		}
	m_bInTListItem = false;
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
	if (tagTop () == TT_LI)
		{
			m_utf8_1 = "li";
			tagClose (TT_LI, m_utf8_1, ws_Post);
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
	_appendInheritanceLine (ClassName, m_utf8_1, true);
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
			tagClose (TT_LI, m_utf8_1, ws_Post);
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

void s_HTML_Listener::_openTag (PT_AttrPropIndex api, PL_StruxDocHandle sdh)
{
	if (m_bFirstWrite) _outputBegin (api);

	if (!m_bInSection) return;

	blockStyleClear ();

	if (m_bInBlock)
		{
			if (tagTop () == TT_A)
				{
					m_utf8_1 = "a";
					tagClose (TT_A, m_utf8_1, ws_None);
				}
			if (tagTop () != TT_LI) _closeTag ();
		}
	m_bWroteText = false;

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	
	if (!bHaveProp || (pAP == 0)) // <p> with no style attribute, and no properties either
		{
			listPopToDepth (0);

			m_utf8_1 = "p";
			tagOpen (TT_P, m_utf8_1, ws_Pre);

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
#ifndef HTML_PSEUDO_LISTS
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
					tagClose (TT_LI, m_utf8_1, ws_Post);
				}

			tagID = TT_LI;
			tagPending = true;

			m_utf8_1 = "li";
#else
			const XML_Char * szP_MarginLeft = 0;
			pAP->getProperty ("margin-left", szP_MarginLeft);

			tlistPushItem (szListID, szP_MarginLeft, sdh);

			if (IS_BULLETED_LIST_TYPE (m_tlistType))
				{
					tagID = TT_LI;
					m_utf8_1 = "li";
				}
			else
				{
					tagID = TT_TD;
					m_utf8_1 = "td";
				}
			tagPending = true;
#endif
		}
	else if (have_style)
		{
			if (m_bInTList) tlistPop ();
			listPopToDepth (0);

			bool bAddInheritance = false;
#ifndef HTML_NO_AWML
			bool bAddAWMLStyle = m_bIs4 ? false : true;
#else
			bool bAddAWMLStyle = false;
#endif

			if ((UT_strcmp ((const char *) szValue, "Heading 1") == 0) ||
				(UT_strcmp ((const char *) szValue, "Numbered Heading 1") == 0))
				{
					m_iBlockType = BT_HEADING1;
					tagID = TT_H1;
					tagPending = true;
					m_utf8_1 = "h1";
					if (UT_strcmp ((const char *) szValue, "Heading 1") == 0) bAddAWMLStyle = false;
				}
			else if ((UT_strcmp ((const char *) szValue, "Heading 2") == 0) ||
					 (UT_strcmp ((const char *) szValue, "Numbered Heading 2") == 0))
				{
					m_iBlockType = BT_HEADING2;
					tagID = TT_H2;
					tagPending = true;
					m_utf8_1 = "h2";
					if (UT_strcmp ((const char *) szValue, "Heading 2") == 0) bAddAWMLStyle = false;
				}
			else if ((UT_strcmp ((const char *) szValue, "Heading 3") == 0) ||
					 (UT_strcmp ((const char *) szValue, "Numbered Heading 3") == 0))
				{
					m_iBlockType = BT_HEADING3;
					tagID = TT_H3;
					tagPending = true;
					m_utf8_1 = "h3";
					if (UT_strcmp ((const char *) szValue, "Heading 3") == 0) bAddAWMLStyle = false;
				}
			else if (UT_strcmp ((const char *) szValue, "Block Text") == 0)
				{
					m_iBlockType = BT_BLOCKTEXT;
					tagID = TT_BLOCKQUOTE;
					tagPending = true;
					m_utf8_1 = "blockquote";
					bAddAWMLStyle = false;
				}
			else if (UT_strcmp ((const char *) szValue, "Plain Text") == 0)
				{
					m_iBlockType = BT_PLAINTEXT;
					tagID = TT_PRE;
					tagPending = true;
					m_utf8_1 = "pre";
					bAddAWMLStyle = false;
				}
			else if (UT_strcmp ((const char *) szValue, "Normal") == 0)
				{
					m_iBlockType = BT_NORMAL;
					tagID = TT_P;
					tagPending = true;
					m_utf8_1 = "p";
					bAddAWMLStyle = false;
				}
			else if (_inherits ((const char *) szValue, "Heading1"))
				{
					m_iBlockType = BT_HEADING1;
					tagID = TT_H1;
					tagPending = true;
					m_utf8_1 = "h1";
					bAddInheritance = true;
				}
			else if (_inherits ((const char *) szValue, "Heading2"))
				{
					m_iBlockType = BT_HEADING2;
					tagID = TT_H2;
					tagPending = true;
					m_utf8_1 = "h2";
					bAddInheritance = true;
				}
			else if (_inherits ((const char *) szValue, "Heading3"))
				{
					m_iBlockType = BT_HEADING3;
					tagID = TT_H3;
					tagPending = true;
					m_utf8_1 = "h3";
					bAddInheritance = true;
				}
			else if (_inherits ((const char *) szValue, "BlockText"))
				{
					m_iBlockType = BT_BLOCKTEXT;
					tagID = TT_BLOCKQUOTE;
					tagPending = true;
					m_utf8_1 = "blockquote";
					bAddInheritance = true;
				}
			else if (_inherits ((const char *) szValue, "PlainText"))
				{
					m_iBlockType = BT_PLAINTEXT;
					tagID = TT_PRE;
					tagPending = true;
					m_utf8_1 = "pre";
					bAddInheritance = true;
				}
			else if (_inherits ((const char *) szValue, "Normal"))
				{
					m_iBlockType = BT_NORMAL;
					tagID = TT_P;
					tagPending = true;
					m_utf8_1 = "p";
					bAddInheritance = true;
				}
			else
				{
					m_iBlockType = BT_NORMAL;
					tagID = TT_P;
					tagPending = true;
					m_utf8_1 = "p";
				}

			if (bAddInheritance)
				{
					m_utf8_1 += " class=\"";
					_appendInheritanceLine ((const char*) szValue, m_utf8_1, true);
					m_utf8_1 += "\"";
				}
			if (bAddAWMLStyle)
				{
					m_utf8_1 += " awml:style=\"";
					m_utf8_1 += szValue;
					m_utf8_1 += "\"";
				}
		}	
	else // not a list, no style
		{
			if (m_bInTList) tlistPop ();
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
	if ((tagID != TT_LI) && (tagID != TT_TD))
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

			bool first = true;

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

	tagOpen (tagID, m_utf8_1, ws_Pre);

	m_bInBlock = true;
}

void s_HTML_Listener::_closeTag (void)
{
	if (!m_bInBlock) return;

	if (m_bInSpan) _closeSpan ();
	
	if (tagTop () == TT_A)
		{
			m_utf8_1 = "a";
			tagClose (TT_A, m_utf8_1, ws_None);
		}
	if (m_iBlockType == BT_NORMAL)
		{
#ifdef HTML_EMPTY_PARA_LF
			if (!m_bWroteText) // TODO: is this really ideal?
				{
					m_utf8_1 = "br";
					tagOpenClose (m_utf8_1, m_bIs4, ws_None);
				}
#endif /* HTML_EMPTY_PARA_LF */
			if (tagTop () == TT_P)
				{
					m_utf8_1 = "p";
					tagClose (TT_P, m_utf8_1, ws_Post);
				}
		}
	else if (m_iBlockType == BT_HEADING1) 
		{
			if (tagTop () == TT_H1)
				{
					m_utf8_1 = "h1";
					tagClose (TT_H1, m_utf8_1, ws_Post);
				}
		}
	else if (m_iBlockType == BT_HEADING2)
		{
			if (tagTop () == TT_H2)
				{
					m_utf8_1 = "h2";
					tagClose (TT_H2, m_utf8_1, ws_Post);
				}
		}
	else if (m_iBlockType == BT_HEADING3)
		{
			if (tagTop () == TT_H3)
				{
					m_utf8_1 = "h3";
					tagClose (TT_H3, m_utf8_1, ws_Post);
				}
		}
	else if (m_iBlockType == BT_BLOCKTEXT)
		{
			if (tagTop () == TT_BLOCKQUOTE)
				{
					m_utf8_1 = "blockquote";
					tagClose (TT_BLOCKQUOTE, m_utf8_1, ws_Post);
				}
		}
	else if (m_iBlockType == BT_PLAINTEXT)
		{
			if (tagTop () == TT_PRE)
				{
					m_utf8_1 = "pre";
					tagClose (TT_PRE, m_utf8_1, ws_Post);
				}
		}
	else if (m_bInTListItem)
		{
			/* we're in a table-list item
			 */
			tlistPopItem ();
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
					tagClose (TT_P, m_utf8_1, ws_Post);
				}
		}
	m_bInBlock = false;
}

void s_HTML_Listener::_openSpan (PT_AttrPropIndex api)
{
	if (m_bFirstWrite) _outputBegin (api);

	if (!m_bInBlock) return;

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);
	
	if (!bHaveProp || (pAP == 0))
		{
			if (m_bInSpan) _closeSpan ();
			return;
		}

	const XML_Char * szP_FontWeight = 0;
	const XML_Char * szP_FontStyle = 0;
	const XML_Char * szP_FontSize = 0;
	const XML_Char * szP_FontFamily = 0;
	const XML_Char * szP_TextDecoration = 0;
	const XML_Char * szP_TextPosition = 0;
	const XML_Char * szP_Color = 0;
	const XML_Char * szP_BgColor = 0;

	pAP->getProperty ("font-weight",     szP_FontWeight);
	pAP->getProperty ("font-style",      szP_FontStyle);
	pAP->getProperty ("font-size",       szP_FontSize);
	pAP->getProperty ("font-family",     szP_FontFamily);
	pAP->getProperty ("text-decoration", szP_TextDecoration);
	pAP->getProperty ("text-position",   szP_TextPosition);
	pAP->getProperty ("color",           szP_Color);
	pAP->getProperty ("bgcolor",         szP_BgColor);

	bool first = true;

	m_utf8_1 = "span style=\"";

	/* TODO: this bold/italic check needs re-thought
	 */
	if (szP_FontWeight)
		if (UT_strcmp (szP_FontWeight, "bold") == 0)
			if (!compareStyle ("font-weight", "bold"))
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "font-weight: bold";
					first = false;
				}
	if (szP_FontStyle)
		if (UT_strcmp (szP_FontStyle, "italic") == 0)
			if (!compareStyle ("font-style", "italic"))
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "font-style: italic";
					first = false;
				}

	if (szP_FontSize)
		{
			char * old_locale = setlocale (LC_NUMERIC, "C");
			char buf[16];
			sprintf (buf, "%g", UT_convertToPoints (szP_FontSize));
			setlocale (LC_NUMERIC, old_locale);

			m_utf8_0  = buf;
			m_utf8_0 += "pt";

			if (!compareStyle ("font-size", m_utf8_0.utf8_str ()))
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "font-size: ";
					m_utf8_1 += m_utf8_0;
					first = false;
				}
		}
	if (szP_FontFamily)
		{
			if ((UT_strcmp (szP_FontFamily, "serif")      == 0) ||
				(UT_strcmp (szP_FontFamily, "sans-serif") == 0) ||
				(UT_strcmp (szP_FontFamily, "cursive")    == 0) ||
				(UT_strcmp (szP_FontFamily, "fantasy")    == 0) ||
				(UT_strcmp (szP_FontFamily, "monospace")  == 0))
				{
					m_utf8_0  = (const char *) szP_FontFamily;
				}
			else
				{
					m_utf8_0  = "'";
					m_utf8_0 += (const char *) szP_FontFamily;
					m_utf8_0 += "'";
				}
			if (!compareStyle ("font-family", m_utf8_0.utf8_str ()))
				{
					if (!first) m_utf8_1 += "; ";
					m_utf8_1 += "font-family: ";
					m_utf8_1 += m_utf8_0;
					first = false;
				}
		}
	if (szP_TextDecoration)
		{
			bool bUnderline   = (strstr (szP_TextDecoration, "underline")    != NULL);
			bool bLineThrough = (strstr (szP_TextDecoration, "line-through") != NULL);
			bool bOverline    = (strstr (szP_TextDecoration, "overline")     != NULL);

			if (bUnderline || bLineThrough || bOverline)
				{
					m_utf8_0 = "";
					if (bUnderline) m_utf8_0 += "underline";
					if (bLineThrough)
						{
							if (bUnderline) m_utf8_0 += ", ";
							m_utf8_0 += "line-through";
						}
					if (bOverline)
						{
							if (bUnderline || bLineThrough) m_utf8_0 += ", ";
							m_utf8_0 += "overline";
						}
					if (!compareStyle ("text-decoration", m_utf8_0.utf8_str ()))
						{
							if (!first) m_utf8_1 += "; ";
							m_utf8_1 += "text-decoration: ";
							m_utf8_1 += m_utf8_0;
							first = false;
						}
				}
		}
	if (szP_TextPosition)
		{
			if (UT_strcmp (szP_TextPosition, "superscript") == 0)
				{
					if (!compareStyle ("vertical-align", "superscript"))
						{
							if (!first) m_utf8_1 += "; ";
							m_utf8_1 += "vertical-align: superscript";
							first = false;
						}
				}
			else if (UT_strcmp (szP_TextPosition, "subscript") == 0)
				{
					if (!compareStyle ("vertical-align", "subscript"))
						{
							if (!first) m_utf8_1 += "; ";
							m_utf8_1 += "vertical-align: subscript";
							first = false;
						}
				}
		}
	if (szP_Color)
		if (!IS_TRANSPARENT_COLOR (szP_Color))
			{
				m_utf8_0  = "#";
				m_utf8_0 += szP_Color;

				if (!compareStyle ("color", m_utf8_0.utf8_str ()))
					{
						if (!first) m_utf8_1 += "; ";
						m_utf8_1 += "color: ";
						m_utf8_1 += m_utf8_0;
						first = false;
					}
			}
	if (szP_BgColor)
		if (!IS_TRANSPARENT_COLOR (szP_BgColor))
			{
				m_utf8_0  = "#";
				m_utf8_0 += szP_BgColor;

				if (!compareStyle ("background", m_utf8_0.utf8_str ()))
					{
						if (!first) m_utf8_1 += "; ";
						m_utf8_1 += "background: ";
						m_utf8_1 += m_utf8_0;
						first = false;
					}
			}

	bool bInSpan = false;

	if (first)
		{
			/* no style elements specified
			 */
			m_utf8_1 = "span";
		}
	else
		{
			m_utf8_1 += "\"";
			bInSpan = true;
		}

#ifdef BIDI_ENABLED
	/* if the dir-override is set, or dir is 'rtl' or 'ltr', we will output
	 * the dir property; however, this property cannot be within a style 
	 * sheet, so anything that needs to be added to this code and belongs 
	 * withing a style property must be above us; further it should be noted 
	 * that there is a good chance that the html browser will not handle it 
	 * correctly. For instance IE will take dir=rtl as an indication that 
	 * the span should have rtl placement on a line, but it will ignore this 
	 * value when printing the actual span.
	 */
	const XML_Char * szP_DirOverride = 0;

	pAP->getProperty ("dir-override", szP_DirOverride);

	if (szP_DirOverride)
		if ((*szP_DirOverride == 'l') || (*szP_DirOverride == 'r'))
			{
				m_utf8_1 += " dir=\"";
				m_utf8_1 += szP_DirOverride;
				m_utf8_1 += "\"";
				bInSpan = true;
			}
#endif

	if (bInSpan)
		{
			if (m_bInSpan)
				{
					if (m_utf8_span == m_utf8_1) return; // this span same as last...
					m_utf8_span = m_utf8_1;
					_closeSpan ();
				}
			else m_utf8_span = m_utf8_1;

			tagOpen (TT_SPAN, m_utf8_span, ws_None);
			m_bInSpan = true;
		}
	else if (m_bInSpan) _closeSpan ();
}

void s_HTML_Listener::_closeSpan ()
{
	if (tagTop () == TT_A)
		{
			m_utf8_1 = "a";
			tagClose (TT_A, m_utf8_1, ws_None);
		}
	if (tagTop () == TT_SPAN)
		{
			m_utf8_1 = "span";
			tagClose (TT_SPAN, m_utf8_1, ws_None);
		}
	m_bInSpan = false;
}

#ifdef HTML_TABLES_SUPPORTED

void s_HTML_Listener::_openTable(PT_AttrPropIndex api)
{
	if (m_bFirstWrite)
	{
		_outputBegin(api);
	}

	if (!m_bInSection)
	{
		return;
	}

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if (bHaveProp && pAP)
	{

	  UT_sint32 cellPadding = 1;
	  UT_sint32 border = 1;

	  const char * prop = NULL ;

	  prop = m_TableHelper.getTableProp("table-line-thickness");
	  if(prop)
	    {
	      border = atoi(prop);
	    }

#if 0
	  pSectionAP->getProperty("table-col-spacing", (const XML_Char *&)pszTableColSpacing);
	  pSectionAP->getProperty("table-row-spacing", (const XML_Char *&)pszTableRowSpacing);
	  pSectionAP->getProperty("cell-margin-left", (const XML_Char *&)pszLeftOffset);
	  pSectionAP->getProperty("cell-margin-top", (const XML_Char *&)pszTopOffset);
	  pSectionAP->getProperty("cell-margin-right", (const XML_Char *&)pszRightOffset);
	  pSectionAP->getProperty("cell-margin-bottom", (const XML_Char *&)pszBottomOffset);
#endif

	  UT_sint32 nCols = m_TableHelper.getNumCols();
	  
	  UT_String tableSpec = UT_String_sprintf("<table cellpadding=\"%d\" border=\"%d\" rules=\"all\">\r\n",
						  cellPadding, border);
	  m_pie->write(tableSpec.c_str(), tableSpec.size());

	  char * old_locale = setlocale(LC_NUMERIC, "C");
	  UT_String colSpec = UT_String_sprintf("<colgroup width=\"%f%%\" span=\"%d\" />\r\n", 100./nCols, nCols);
	  m_pie->write(colSpec.c_str(), colSpec.size());
	  setlocale(LC_NUMERIC, old_locale);

	  m_pie->write("<tbody>\r\n");
	}
}

void s_HTML_Listener::_closeTable()
{
  m_pie->write("</tbody>\r\n</table>\r\n");
}

void s_HTML_Listener::_openCell(PT_AttrPropIndex api)
{
	if (m_bFirstWrite)
	{
		_outputBegin(api);
	}

	if (!m_bInSection)
	{
		return;
	}
	if(m_TableHelper.getNestDepth() < 1)
	{
		_openTable(api);
	}
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if (bHaveProp && pAP)
	{
	  UT_sint32 rowspan = 1, colspan = 1;

	  rowspan = m_TableHelper.getBot() - m_TableHelper.getTop();
	  colspan = m_TableHelper.getRight() - m_TableHelper.getLeft();

	  if (m_TableHelper.getLeft() == 0)
	    {
	      // beginning of a new row
	      m_pie->write("<tr>\r\n");
	    }

	  UT_String td ("<td");

	  const char* pszBgColor = m_TableHelper.getCellProp("bgcolor");
	  if(pszBgColor && pszBgColor[0])
	    {
	      UT_String bgcolor (UT_String_sprintf(" bgcolor=\"#%s\"", pszBgColor));
	      td += bgcolor;
	    }
	  
	  if (rowspan > 1)
	    td += UT_String_sprintf(" rowspan=\"%d\"", rowspan);

	  if (colspan > 1)
	    td += UT_String_sprintf(" colspan=\"%d\"", colspan);

	  td += ">\r\n";

	  m_pie->write(td.c_str());	  
	}
}

void s_HTML_Listener::_closeCell()
{
  if(m_TableHelper.getNestDepth() <1)
  {
	  return;
  }
  m_pie->write("</td>\r\n");
  if ( m_TableHelper.getNumCols () == m_TableHelper.getRight () )
    {
      // logical end of a row
      m_pie->write("</tr>\r\n");
    } 
}

#endif /* HTML_TABLES_SUPPORTED */

void s_HTML_Listener::_outputData (const UT_UCSChar * data, UT_uint32 length)
{
	if (!m_bInBlock) return;

	m_utf8_1 = "";

	bool prev_space = false;
	const UT_UCSChar * ucs_ptr = data;
	for (UT_uint32 i = 0; i < length; i++)
		{
			bool space = false;

			switch (*ucs_ptr)
				{
				case UCS_FF: // page break, convert to line break
				case UCS_LF:
					/* LF -- representing a Forced-Line-Break
					 */
					if (m_utf8_1.byteLength ()) textTrusted (m_utf8_1);
					m_utf8_1 = "br";
					tagOpenClose (m_utf8_1, m_bIs4, ws_None);
					m_utf8_1 = "";
					break;

				case UCS_LQUOTE:
				case UCS_RQUOTE:
					/* Smart quotes get translated back into normal quotes
					 */
					m_utf8_1 += "'";
					break;

				case UCS_LDBLQUOTE:
					m_utf8_1 += "&ldquo;";
					break;

				case UCS_RDBLQUOTE:
					m_utf8_1 += "&rdquo;";
					break;

				case UCS_EN_DASH: // TODO: isn't there a better way?
				case UCS_EM_DASH:
					m_utf8_1 += "-";
					break;

				default:
					if ((*ucs_ptr & 0x007f) == *ucs_ptr) // ASCII
						{
							char c = static_cast<char>(*ucs_ptr & 0x007f);

							if (isspace ((int) ((unsigned char) c)))
								{
									if (prev_space)
										m_utf8_1 += "&nbsp;";
									else
#ifdef HTML_UCS4
										m_utf8_1.appendUCS4 (ucs_ptr, 1);
#else
										m_utf8_1.append (ucs_ptr, 1);
#endif
									space = true;
								}
							else switch (c)
								{
								case '<':
									m_utf8_1 += "&lt;";
									break;
								case '>':
									m_utf8_1 += "&gt;";
									break;
								case '&':
									m_utf8_1 += "&amp;";
									break;
								default:
#ifdef HTML_UCS4
									m_utf8_1.appendUCS4 (ucs_ptr, 1);
#else
									m_utf8_1.append (ucs_ptr, 1);
#endif
									break;
								}
						}
#ifdef HTML_UCS4
					else m_utf8_1.appendUCS4 (ucs_ptr, 1); // !ASCII, just append... ??
#else
					else m_utf8_1.append (ucs_ptr, 1); // !ASCII, just append... ??
#endif
					break;
				}
			prev_space = space;
			ucs_ptr++;
		}
	if (m_utf8_1.byteLength ()) textTrusted (m_utf8_1);
}

s_HTML_Listener::s_HTML_Listener (PD_Document * pDocument, IE_Exp_HTML * pie, bool bClipBoard,
								  const IE_Exp_HTML_Options & exp_opt) :
	m_pDocument (pDocument),
	m_pie(pie),
	m_bClipBoard(bClipBoard),
	m_bInSection(false),
	m_bInBlock(false),
	m_bInTList(false),
	m_bInTListItem(false),
	m_bInSpan(false),
	m_bNextIsSpace(false),
	m_bWroteText(false),
	m_bFirstWrite(true),
#ifdef HTML_TABLES_SUPPORTED
	m_TableHelper(pDocument),
#endif /* HTML_TABLES_SUPPORTED */
	m_iBlockType(0),
	m_iListDepth(0),
	m_iImgCnt(0),
	m_styleIndent(0),
	m_tlistIndent(0),
	m_tlistListID(0)
{
	m_bIs4         = exp_opt.bIs4;
	m_bIsAbiWebDoc = exp_opt.bIsAbiWebDoc;
}

s_HTML_Listener::~s_HTML_Listener()
{
	_closeTag ();

	listPopToDepth (0);

	_closeSection ();

	_outputEnd ();

	bodyStyleClear ();
	blockStyleClear ();
}

/* dataid   is the raw string with the data ID
 * imagedir is the name of the directory in which we'll write the image
 * filename is the name of the file to which we'll write the image
 * url      is the URL which we'll use
 */
void s_HTML_Listener::_writeImage (const UT_ByteBuf * pByteBuf,
								   const UT_String & imagedir,
								   const UT_String & filename)
{
	/* hmm, bit lazy this - attempt to create directory whether or not
	 * it exists already... if it does, well hey. if this fails to
	 * create a directory then fopen() will fail as well, so no biggie
	 */
	m_pDocument->getApp()->makeDirectory (imagedir.c_str (), 0750);

	UT_String path(imagedir);
	path += "/";
	path += filename;

	FILE * out = fopen (path.c_str (), "wb+");
	if (out)
		{
			fwrite (pByteBuf->getPointer (0), sizeof (UT_Byte), pByteBuf->getLength (), out);
			fclose (out);
		}
}

/* TODO: is there a better way to do this?
 */
static UT_UTF8String s_string_to_url (UT_String & str)
{
	UT_UTF8String url;

	static const char hex[16] = {
		'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
	};
	char buf[4];
	buf[0] = '%';
	buf[3] = 0;

	const char * ptr = str.c_str ();
	while (*ptr)
		{
			bool isValidPunctuation = false;
			switch (*ptr)
				{
				case '-': // TODO: any others?
				case '_':
				case '.':
					isValidPunctuation = true;
					break;
				default:
					break;
				}
			unsigned char u = (unsigned char) *ptr;
			if (!isalnum ((int) u) && !isValidPunctuation)
				{
					buf[1] = hex[(u >> 4) & 0x0f];
					buf[2] = hex[ u       & 0x0f];
					url += buf;
				}
			else
				{
					buf[2] = (char) *ptr;
					url += (buf + 2);
				}
			ptr++;
		}
	return url;
}

void s_HTML_Listener::_handleImage (PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	if (!bHaveProp || (pAP == 0)) return;

	const XML_Char * szDataID = 0;
	pAP->getAttribute ("dataid", szDataID);

	if (szDataID == 0) return;

 	const char * szName = 0;
	const char * szMimeType = 0;

	const UT_ByteBuf * pByteBuf = 0;

	UT_uint32 k = 0;
	while (m_pDocument->enumDataItems (k, 0, &szName, &pByteBuf, (void**) &szMimeType))
		{
			k++;
			if (szName == 0) continue;
			if (UT_strcmp (szDataID, szName) == 0) break;

			szName = 0;
			szMimeType = 0;
			pByteBuf = 0;
		}
	if ((pByteBuf == 0) || (szMimeType == 0)) return; // ??

	if (UT_strcmp (szMimeType, "image/png") != 0)
		{
			UT_DEBUGMSG(("Object not of MIME type image/png - ignoring...\n"));
			return;
		}

	const char * dataid = UT_basename ((const char *) szDataID);

	const char * suffix = dataid + strlen (dataid);
	const char * suffid = suffix;
	const char * ptr = 0;

	/* Question: What does the DataID look like for images pasted
	 *           from the clipboard?
	 */
	ptr = suffix;
	while (ptr > dataid)
		if (*--ptr == '_')
			{
				suffix = ptr;
				suffid = suffix;
				break;
			}
	ptr = suffix;
	while (ptr > dataid)
		if (*--ptr == '.')
			{
				suffix = ptr;
				break;
			}
	if (dataid == suffix) return;

	/* hmm; who knows what locale the system uses
	 */
	UT_String imagebasedir = UT_basename (m_pie->getFileName ());
	imagebasedir += "_data";
	UT_String imagedir = m_pie->getFileName ();
	imagedir += "_data";

	UT_String filename(dataid,suffix-dataid);
	filename += suffid;
	filename += ".png";

	UT_UTF8String url;

	url += s_string_to_url (imagebasedir);
	url += "/";
	url += s_string_to_url (filename);

	/* szDataID is the raw string with the data ID
	 * imagedir is the name of the directory in which we'll write the image
	 * filename is the name of the file to which we'll write the image
	 * url      is the URL which we'll use
	 */
	_writeImage (pByteBuf, imagedir, filename);

	m_utf8_1 = "img";

	m_utf8_1 += " src=\"";
	m_utf8_1 += url;
	m_utf8_1 += "\"";

	const XML_Char * szWidth  = 0;
	const XML_Char * szHeight = 0;

	pAP->getProperty ("width",  szWidth);
	pAP->getProperty ("height", szHeight);

	char buf[16];

	if (szWidth)
		{
			sprintf (buf, "%d", (int) UT_convertToDimension (szWidth, DIM_PX));
			m_utf8_1 += " width=\"";
			m_utf8_1 += buf;
			m_utf8_1 += "\"";
		}
	if(szHeight)
		{
			sprintf (buf, "%d", (int) UT_convertToDimension (szHeight, DIM_PX));
			m_utf8_1 += " height=\"";
			m_utf8_1 += buf;
			m_utf8_1 += "\"";
		}

	tagOpenClose (m_utf8_1, m_bIs4, ws_None);
}

void s_HTML_Listener::_handleField (const PX_ChangeRecord_Object * pcro,
									PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	if (!bHaveProp || (pAP == 0)) return;

	const XML_Char * szType = 0;
	pAP->getAttribute ("type", szType);

	if (szType == 0) return;

	fd_Field * field = pcro->getField ();

	if (UT_strcmp (szType, "list_label") != 0)
		{
			m_utf8_1 = "span";

			m_utf8_1 += " class=\"ABI_FIELD_";
			m_utf8_1 += szType;
			m_utf8_1 += "\"";

			tagOpen (TT_SPAN, m_utf8_1, ws_None);

			textUntrusted (field->getValue ());

			m_utf8_1 = "span";
			tagClose (TT_SPAN, m_utf8_1, ws_None);
		}
}

void s_HTML_Listener::_handleHyperlink (PT_AttrPropIndex api)
{
	m_utf8_1 = "a";

	if (tagTop () == TT_A)
		{
			tagClose (TT_A, m_utf8_1, ws_None);
		}

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);

	if (!bHaveProp || (pAP == 0)) return;

	const XML_Char * szHRef = 0;
	pAP->getAttribute ("xlink:href", szHRef);

	if (szHRef) // trust this to be a valid URL??
		{
			m_utf8_1 += " href=\"";
			m_utf8_1 += szHRef;
			m_utf8_1 += "\"";

			tagOpen (TT_A, m_utf8_1, ws_None);
		}
}

void s_HTML_Listener::_handleBookmark (PT_AttrPropIndex api)
{
	m_utf8_1 = "a";

	if (tagTop () == TT_A)
		{
			tagClose (TT_A, m_utf8_1, ws_None);
		}

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);

	if (!bHaveProp || (pAP == 0)) return;

	const XML_Char * szType = 0;
	pAP->getAttribute ("type", szType);

	if (szType == 0) return; // ??

	if (UT_XML_stricmp (szType, "start") == 0)
		{
			const XML_Char * szName = 0;
			pAP->getAttribute ("name", szName);

			if (szName)
				{
					m_utf8_1 += " name=\"";
					m_utf8_1 += szName;
					m_utf8_1 += "\"";

					if (!m_bIs4)
						{
							m_utf8_1 += " id=\"";
							m_utf8_1 += szName;
							m_utf8_1 += "\"";
						}
					tagOpen (TT_A, m_utf8_1, ws_None);
				}
		}
}

#ifdef HTML_META_SUPPORTED

void s_HTML_Listener::_handleMetaTag (const char * key, UT_UTF8String & value)
{
	m_utf8_1  = "meta name=\"";
	m_utf8_1 += key;
	m_utf8_1 += "\" content=\"";
	m_utf8_1 += value.escapeXML ();
	m_utf8_1 += "\"";

	tagOpenClose (m_utf8_1, m_bIs4);
}

void s_HTML_Listener::_handleMeta ()
{
	UT_UTF8String metaProp;
	
	if (m_pDocument->getMetaDataProp (PD_META_KEY_TITLE,    metaProp) && metaProp.size ())
	    _handleMetaTag ("Title",    metaProp);

	if (m_pDocument->getMetaDataProp (PD_META_KEY_CREATOR,  metaProp) && metaProp.size ())
		_handleMetaTag ("Author",   metaProp);
	
	if (m_pDocument->getMetaDataProp (PD_META_KEY_KEYWORDS, metaProp) && metaProp.size ())
	    _handleMetaTag ("Keywords", metaProp);
	
	if (m_pDocument->getMetaDataProp (PD_META_KEY_SUBJECT,  metaProp) && metaProp.size ())
		_handleMetaTag ("Subject",  metaProp);

#if 0
	// now generically dump all of our data to meta stuff
	const void * val = NULL ;
	for ( val = cursor.first(); cursor.is_valid(); val = cursor.next () )
	    {
			if ( val )
				{
					UT_String *stringval = (UT_String*) val;
					if(stringval->size () > 0)
						{
							_handleMetaTag(cursor.key().c_str(), stringval->c_str()));
						}
				}
		}
#endif

}

#endif /* HTML_META_SUPPORTED */

bool s_HTML_Listener::populate (PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * pcr)
{
	if (m_bFirstWrite && m_bClipBoard)
		{
			_openSection (0);
			_openTag (0, 0);
		}

	switch (pcr->getType ())
		{
		case PX_ChangeRecord::PXT_InsertSpan:
			{
				const PX_ChangeRecord_Span * pcrs = 0;
				pcrs = static_cast<const PX_ChangeRecord_Span *>(pcr);

				PT_AttrPropIndex api = pcr->getIndexAP ();

				_openSpan (api);

				PT_BufIndex bi = pcrs->getBufIndex ();
				_outputData (m_pDocument->getPointer (bi), pcrs->getLength ());

				// don't _closeSpan (); - leave open in case of identical sequences

				return true;
			}

		case PX_ChangeRecord::PXT_InsertObject:
			{
				if (m_bInSpan) _closeSpan ();

				m_bWroteText = true;

				const PX_ChangeRecord_Object * pcro = 0;
				pcro = static_cast<const PX_ChangeRecord_Object *>(pcr);

				PT_AttrPropIndex api = pcr->getIndexAP ();

				switch (pcro->getObjectType ())
					{
					case PTO_Image:
						if (m_bClipBoard) break; // TODO: data-URL??
						_handleImage (api);
						return true;

					case PTO_Field:
						_handleField (pcro, api);
						return true;

					case PTO_Hyperlink:
						_handleHyperlink (api);
						return true;

					case PTO_Bookmark:
						_handleBookmark (api);
						return true;

					default:
						UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled object type!\n"));
						return false;
					}
			}

		case PX_ChangeRecord::PXT_InsertFmtMark:
			return true;
		
		default:
			UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled record type!\n"));
			return false;
		}
}

bool s_HTML_Listener::populateStrux (PL_StruxDocHandle sdh,
									 const PX_ChangeRecord * pcr,
									 PL_StruxFmtHandle * psfh)
{
	if (m_bFirstWrite && m_bClipBoard)
		{
			_openSection (0);
			_openTag (0, 0);
		}

	UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux);

	*psfh = 0; // we don't need it.

	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);

	PT_AttrPropIndex api = pcr->getIndexAP ();

	switch (pcrx->getStruxType ())
		{
		case PTX_SectionEndnote:
		case PTX_SectionHdrFtr:
		case PTX_Section:
			{
				if (m_bInBlock) _closeTag (); // possible problem with lists??
				_openSection (api);
				return true;
			}

		case PTX_Block:
			{
				_openTag (api, sdh);
				return true;
			}

#ifdef HTML_TABLES_SUPPORTED
	case PTX_SectionTable:
	  {
	    m_TableHelper.OpenTable(sdh,pcr->getIndexAP()) ;
	    _closeSpan();
	    _closeTag();
	    _openTable(pcr->getIndexAP());
	    return true;
	  }

	case PTX_SectionCell:
	  {
		  if(m_TableHelper.getNestDepth() <1)
		  {
			  m_TableHelper.OpenTable(sdh,pcr->getIndexAP()) ;
			  _closeSpan();
			  _closeTag();
			  _openTable(pcr->getIndexAP());
		  }
		  m_TableHelper.OpenCell(pcr->getIndexAP()) ;
		  _closeSpan();
		  _closeTag();
		  _openCell(pcr->getIndexAP());
		  return true;
	  }

	case PTX_EndTable:
	  {
	    _closeTag();
	    _closeTable();
	    m_TableHelper.CloseTable();
	    return true;
	  }

	case PTX_EndCell:
	  {
	    _closeTag();
	    _closeCell();
		if(m_TableHelper.getNestDepth() <1)
		{
			return true;
		}

	    m_TableHelper.CloseCell();
	    return true;
	  }
#endif /* HTML_TABLES_SUPPORTED */

#if 0
	case PTX_EndFrame:
	case PTX_EndMarginnote:
	case PTX_EndFootnote:
	case PTX_SectionFrame:
	case PTX_SectionMarginnote:
	case PTX_SectionFootnote:
	case PTX_EndEndnote:
#endif
		default:
			UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled strux type!\n"));
			return false;
		}
}

/*****************************************************************/
/*****************************************************************/

bool s_HTML_Listener::change (PL_StruxFmtHandle /*sfh*/,
							  const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_HTML_Listener::insertStrux (PL_StruxFmtHandle /*sfh*/,
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

bool s_HTML_Listener::signal (UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_HTML::_writeDocument ()
{
#ifdef HTML_DIALOG_OPTIONS
	// if (!AP_Dialog_HtmlOptions::popup (m_exp_opt)) return UT_OK; // ??
	/* where is the place to pop-up a dialog? what happens if the user cancels? */
#endif

	bool bClipBoard = (getDocRange () != NULL);

	s_HTML_Listener * pListener = new s_HTML_Listener(getDoc(),this,bClipBoard,m_exp_opt);
	if (pListener == 0) return UT_IE_NOMEMORY;

	PL_Listener * pL = static_cast<PL_Listener *>(pListener);

	bool okay = true;
	if (bClipBoard)
		{
			okay = getDoc()->tellListenerSubset (pL, getDocRange ());
		}
	else okay = getDoc()->tellListener (pL);

	DELETEP(pListener);
	
	if ((m_error == UT_OK) && (okay == true)) return UT_OK;
	return UT_IE_COULDNOTWRITE;
}
