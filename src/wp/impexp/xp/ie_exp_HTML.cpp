/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 2007, 2009 Hubert Figuiere
 * Copyright (C) 2003-2005 Mark Gilbert <mg_abimail@yahoo.com>
 * Copyright (C) 2002, 2004 Francis James Franklin <fjf@alinameridon.com>
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
#include <time.h>

#include "ut_locale.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "ut_wctomb.h"
#include "ut_path.h"
#include "ut_math.h"
#include "ut_misc.h"
#include "ut_string_class.h"
#include "ut_png.h"

#include "xap_App.h"
#include "xap_EncodingManager.h"

#include "pt_Types.h"
#include "pl_Listener.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "pp_PropertyMap.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "ut_mbtowc.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "gr_Graphics.h"

#include "fd_Field.h"

#include "fl_AutoNum.h"

#include "ie_types.h"
#include "ie_TOC.h"
#include "ie_impexp_HTML.h"
#include "ie_exp_HTML.h"
#include "ap_Strings.h"

#ifdef HTML_DIALOG_OPTIONS
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#endif

#ifdef HTML_TABLES_SUPPORTED
#include "ie_Table.h"
#endif

#define MYEOL "\n"
#define MAX_LINE_LEN 200

// This defines sMathSVGScript which when embedded in html4 documents
// displays MathML and SVG

#include "MathSVGScript.h"

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML_Sniffer::IE_Exp_HTML_Sniffer ()
#ifdef HTML_NAMED_CONSTRUCTORS
	: IE_ExpSniffer(IE_IMPEXPNAME_XHTML, true)
#endif
{
	// 
}

bool IE_Exp_HTML_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!g_ascii_strcasecmp (szSuffix, ".xhtml") || 
			!(g_ascii_strcasecmp (szSuffix, ".html")) || 
			!(g_ascii_strcasecmp (szSuffix, ".htm")));
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
	*pszDesc = "HTML/XHTML (.html)";
	*pszSuffixList = "*.html";
	*ft = getFileType ();
	return true;
}

UT_Confidence_t IE_Exp_HTML_Sniffer::supportsMIME (const char * szMimeType)
{
	if(!strcmp(szMimeType, IE_MIMETYPE_XHTML) ||
	   !strcmp(szMimeType, "application/xhtml") ||
	   !strcmp(szMimeType, "text/html"))
		return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

#ifdef HTML_ENABLE_HTML4

// HTML 4

IE_Exp_HTML4_Sniffer::IE_Exp_HTML4_Sniffer ()
#ifdef HTML_NAMED_CONSTRUCTORS
	: IE_ExpSniffer(IE_IMPEXPNAME_HTML, true)
#endif
{
	// 
}

bool IE_Exp_HTML4_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!(g_ascii_strcasecmp (szSuffix, ".html")) || !(g_ascii_strcasecmp (szSuffix, ".htm")));
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

UT_Confidence_t IE_Exp_HTML4_Sniffer::supportsMIME (const char * szMimeType)
{
	if(!strcmp(szMimeType, "text/html"))
		return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

#endif /* HTML_ENABLE_HTML4 */

#ifdef HTML_ENABLE_PHTML

// XHTML w/ PHP instructions for AbiWord Web Docs

IE_Exp_PHTML_Sniffer::IE_Exp_PHTML_Sniffer ()
#ifdef HTML_NAMED_CONSTRUCTORS
	: IE_ExpSniffer(IE_IMPEXPNAME_PHTML, false)
#endif
{
	// 
}

bool IE_Exp_PHTML_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!(g_ascii_strcasecmp (szSuffix, ".phtml")));
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
	*pszDesc = "XHTML+PHP (.phtml)";
	*pszSuffixList = "*.phtml";
	*ft = getFileType();
	return true;
}

#endif /* HTML_ENABLE_PHTML */

#ifdef HTML_ENABLE_MHTML

// Multipart HTML: http://www.rfc-editor.org/rfc/rfc2557.txt

IE_Exp_MHTML_Sniffer::IE_Exp_MHTML_Sniffer ()
#ifdef HTML_NAMED_CONSTRUCTORS
	: IE_ExpSniffer(IE_IMPEXPNAME_MHTML, true)
#endif
{
	// 
}

bool IE_Exp_MHTML_Sniffer::recognizeSuffix (const char * szSuffix)
{
	return (!(g_ascii_strcasecmp (szSuffix, ".mht")));
}

UT_Error IE_Exp_MHTML_Sniffer::constructExporter (PD_Document * pDocument,
												  IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
	if (p) p->set_MHTML ();
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_MHTML_Sniffer::getDlgLabels(const char ** pszDesc,
										const char ** pszSuffixList,
										IEFileType * ft)
{
	*pszDesc = "Multipart HTML (.mht)";
	*pszSuffixList = "*.mht";
	*ft = getFileType();
	return true;
}

#endif /* HTML_ENABLE_MHTML */

/*****************************************************************/
/*****************************************************************/

/* TODO: is there a better way to do this?
 */
static UT_UTF8String s_string_to_url (const UT_String & str)
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
		if (!isalnum (static_cast<int>(u)) && !isValidPunctuation)
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

static UT_UTF8String s_string_to_url (const UT_UTF8String & str)
{
	UT_String s(str.utf8_str());
	return s_string_to_url(s);
}

static const char * s_prop_list[] = {
	"background-color",	"transparent",
	"color",			"",
	"font-family",		"",
	"font-size",		"medium",
	"font-style",		"normal",
	"font-variant",		"normal",
	"font-weight",		"normal",
	"height",			"auto",
	"margin-bottom",	"0pt",
	"margin-left",		"0pt",
	"margin-right",		"0pt",
	"margin-top",		"0pt",
	"orphans",			"2",
	"text-align",		"",
	"text-decoration",	"none",
	"text-transform",	"none",
	"text-indent",		"0in",
	"vertical-align",	"baseline",
	"widows",			"2",
	"width",			"auto",
	0, 0
};
static const UT_uint32 s_PropListLen = G_N_ELEMENTS(s_prop_list) - 2; /* don't include the zeros */

/*!	This function returns true if the given property is a valid CSS
  property.  It is based on the list in pp_Property.cpp, and, as such,
  is quite brittle.

  prop_default may be zero on return, indicating that the default is not fixed
*/
static bool is_CSS (const char * prop_name, const char ** prop_default = 0)
{
	if (prop_name == 0)
		return false;
	if (*prop_name == 0)
		return false;

	bool bCSS = false;

	for (UT_uint32 i = 0; i < s_PropListLen; i += 2)
	{
		if (!strcmp (prop_name, s_prop_list[i]))
		{
			if (prop_default) *prop_default = s_prop_list[i+1];
			bCSS = true;
			break;
		}
	}
	return bCSS;
}

/*!	This function copies a string to a new string, removing all the white
  space in the process.
*/
static char * s_removeWhiteSpace (const char * text, UT_UTF8String & utf8str,
								  bool bLowerCase = true)
{
	utf8str = "";

	if (text)
	{
		char buf[2]; // ick! [TODO ??]
		buf[1] = 0;
		const char * ptr = text;
		while (*ptr)
		{
			if (isspace ((int) ((unsigned char) *ptr)))
			{
				buf[0] = '_';
			}
			else
			{
				buf[0] = *ptr;
			}
			utf8str += buf;
			ptr++;
		}

		if(bLowerCase)
			utf8str.lowerCase();
	}
	return 0;
}

/*****************************************************************/
/*****************************************************************/

#define IS_TRANSPARENT_COLOR(c) (!strcmp(c, "transparent"))

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	5
#define BT_PLAINTEXT	6
#define BT_NUMBEREDLIST	7
#define BT_BULLETLIST	8

class s_HTML_Listener;
class s_HTML_HdrFtr_Listener;

bool m_bSecondPass = false;
bool m_bInAFENote = false;
bool m_bInAnnotation = false;

class ABI_EXPORT s_StyleTree : public PL_Listener
{
private:
	PD_Document *	m_pDocument; // root element of tree only (atm, anyway)

	s_StyleTree *	m_parent;
	s_StyleTree **	m_list;

	UT_uint32		m_count;
	UT_uint32		m_max;

	bool			m_bInUse;

	UT_UTF8String	m_style_name;
	UT_UTF8String	m_class_name;
	UT_UTF8String	m_class_list;

	PD_Style *		m_style;

	typedef std::map<std::string, std::string> map_type;
	map_type m_map;

	s_StyleTree (s_StyleTree * parent, const char * name, PD_Style * style);
public:
	s_StyleTree (PD_Document * pDocument);
	~s_StyleTree ();

private:
	bool add (const char * style_name, PD_Style * style);
public:
	bool add (const char * style_name, PD_Document * pDoc);

private:
	void inUse ();
public:
	const s_StyleTree * findAndUse (const char * style_name);

	const s_StyleTree * find (const char * style_name) const;
	const s_StyleTree * find (PD_Style * style) const;

	bool descends (const char * style_name) const;

	template<typename StyleListener>
	void print (StyleListener * listener) const;

	const s_StyleTree * operator[] (UT_uint32 i) const
	{
		return (i < m_count) ? m_list[i] : 0;
	}
	UT_uint32 count () const { return m_count; }

	const UT_UTF8String & style_name () const { return m_style_name; }
	const UT_UTF8String & class_name () const { return m_class_name; }
	const UT_UTF8String & class_list () const { return m_class_list; }

	const std::string & lookup (const std::string & prop_name) const;

private:
	void	styleCheck (PT_AttrPropIndex api);
public:
	/* implementation of PL_Listener
	 */
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
};

class ABI_EXPORT s_HTML_Listener : public PL_Listener
{
	friend class s_HTML_HdrFtr_Listener;
public:
	s_HTML_Listener (PD_Document * pDocument, IE_Exp_HTML * pie, bool bClipBoard,
					 bool bTemplateBody, const XAP_Exp_HTMLOptions * exp_opt,
					 s_StyleTree * style_tree,
					 UT_UTF8String & linkCSS,
					 UT_UTF8String & title);

	~s_HTML_Listener ();

	bool	populate (PL_StruxFmtHandle sfh,
					  const PX_ChangeRecord * pcr);

	bool	populateStrux (PL_StruxDocHandle sdh,
						   const PX_ChangeRecord * pcr,
						   PL_StruxFmtHandle * psfh);

	//bool 	startOfDocument ();  // Unused
	bool 	endOfDocument ();

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
	void    startEmbeddedStrux(void);

private:
	void 	_outputBegin (PT_AttrPropIndex api);
	void 	_outputEnd ();
	bool 	_openStyleSheet (UT_UTF8String & css_path);
	void 	_closeStyleSheet ();
	void	_outputStyles (const PP_AttrProp * pAP);
	void	_openSection (PT_AttrPropIndex api, UT_uint16 iSectionSpecialType);
	void	_closeSection (void);

	void	_openTag (PT_AttrPropIndex api, PL_StruxDocHandle sdh);
	void	_closeTag (void);
	void	_closeSpan (void);
	void	_openSpan (PT_AttrPropIndex api);

	void _popUnendedStructures(void);

#ifdef HTML_TABLES_SUPPORTED
	void	_openTable (PT_AttrPropIndex api);
	void	_closeTable ();
	void	_openRow (PT_AttrPropIndex api);
	void	_openCell (PT_AttrPropIndex api);
	void	_closeCell ();
#endif

	void	_openTextBox (PT_AttrPropIndex api);
	void	_openPosImage (PT_AttrPropIndex api);
	void	_closeTextBox ();
	
	void	_outputData (const UT_UCSChar * p, UT_uint32 length);
	bool	_inherits (const char * style, const char * from);
	void	_storeStyles (void);
 	void	_populateHeaderStyle ();
 	void	_populateFooterStyle ();

	void	_writeImageBase64 (const UT_ByteBuf * pByteBuf);
	void	_handleImage (PT_AttrPropIndex api);
	void	_handleImage (const PP_AttrProp * pAP, const char * szDataID, bool isPositioned);
	void	_handlePendingImages ();
	void	_handleField (const PX_ChangeRecord_Object * pcro, PT_AttrPropIndex api);
	void	_handleHyperlink (PT_AttrPropIndex api);
	void	_handleAnnotationMark (PT_AttrPropIndex api);
	void	_handleBookmark (PT_AttrPropIndex api);
	void	_handleMath (PT_AttrPropIndex api);
	void    _handleEmbedded (PT_AttrPropIndex api);
	void	_handleEmbedded (const PP_AttrProp * pAP, const gchar * szDataID, const UT_ByteBuf* pByteBuf, const std::string mimeType);

	bool	_getPropertySize(const PP_AttrProp * pAP, const gchar* szWidthProp, const gchar* szHeightProp, 
                             const gchar** szWidth, double& widthPercentage, const gchar** szHeight);
	UT_UTF8String _getStyleSizeString(const gchar * szWidth, double widthPercentage, UT_Dimension widthDim, 
                                      const gchar * szHeight, UT_Dimension heightDim);
	
#ifdef HTML_META_SUPPORTED
	void    _handleMetaTag (const char * key, UT_UTF8String & value);
	void    _handleMeta ();
#endif

	void	_doEndnotes ();
	void	_doFootnotes ();
	void	_doAnnotations ();
	void    _emitTOC (PT_AttrPropIndex api);

	PD_Document *				m_pDocument;
	PT_AttrPropIndex			m_apiLastSpan;
	IE_Exp_HTML *				m_pie;
	bool						m_bClipBoard;
	bool						m_bTemplateBody;
	const XAP_Exp_HTMLOptions *	m_exp_opt;
	s_StyleTree *				m_style_tree;

	inline bool		get_HTML4 ()        const { return m_exp_opt->bIs4; }
	inline bool		get_PHTML ()        const { return m_exp_opt->bIsAbiWebDoc; }
	inline bool		get_Declare_XML ()  const { return m_exp_opt->bDeclareXML && !m_exp_opt->bIs4; }
	inline bool		get_Allow_AWML ()   const { return m_exp_opt->bAllowAWML  && !m_exp_opt->bIs4; }
	inline bool		get_Embed_CSS ()    const { return m_exp_opt->bEmbedCSS; }
	inline bool		get_Link_CSS ()     const { return m_exp_opt->bLinkCSS; }
	inline bool		get_Abs_Units ()    const { return m_exp_opt->bAbsUnits; }
	inline bool		get_Scale_Units ()  const { return m_exp_opt->bScaleUnits; }
	inline UT_uint32 get_Compact ()     const { return m_exp_opt->iCompact; }
	inline bool		get_Embed_Images () const { return m_exp_opt->bEmbedImages; }
	inline bool		get_Multipart ()    const { return m_exp_opt->bMultipart; }
	inline bool     get_Class_Only()    const { return m_exp_opt->bClassOnly; }

	bool			m_bInSection;
	bool			m_bInFrame;
	bool			m_bInTextBox; // Necessary?  Possibly not.  Convenient and safe?  Yes.
	bool			m_bInTOC;
	bool			m_bInBlock;
	bool			m_bInSpan;
	bool			m_bNextIsSpace;
	bool			m_bWroteText;
	bool			m_bFirstWrite;
	bool			m_bQuotedPrintable;
	bool			m_bHaveHeader;
	bool			m_bHaveFooter;

#ifdef HTML_TABLES_SUPPORTED
	ie_Table		m_TableHelper;
#endif

	// Need to look up proper type, and place to stick #defines...
  
	UT_uint32		m_iBlockType;	// BT_*
	UT_uint32		m_iListDepth;	// 0 corresponds to not in a list
	UT_NumberStack	m_utsListType;
	UT_uint32		m_iImgCnt;
	UT_Wctomb		m_wmctomb;

	enum WhiteSpace
		{
			ws_None = 0,
			ws_Pre  = 1,
			ws_Post = 2,
			ws_Both = 3
		};

	/* low-level; these may use m_utf8_0 but not m_utf8_1
	 * WARNING: Use of purely tag-operative methods (tagOpen,tagClose,etc..) may circumvent
	 * important checks and preparations done in strux-operative methods (_open*,_close*,etc...)
	 * and thus these should only be used by likewise low-level code.
	 */
	void			tagRaw (UT_UTF8String & content);
	UT_uint32		tagIndent ();
	void			tagNewIndent (UT_UTF8String & utf8, UT_uint32 depth);
	void			tagNewIndent (UT_uint32 extra = 0);
	void			tagOpenClose (const UT_UTF8String & content, bool suppress,
								  WhiteSpace ws = ws_Both);
	void			tagOpen  (UT_uint32 tagID, const UT_UTF8String & content,
							  WhiteSpace ws = ws_Both);
	void			tagClose (UT_uint32 tagID, const UT_UTF8String & content,
							  WhiteSpace ws = ws_Both);
	void			tagClose (UT_uint32 tagID);
	void			tagOpenBroken  (const UT_UTF8String & content,
									WhiteSpace ws = ws_Pre);
	void			tagCloseBroken (const UT_UTF8String & content, bool suppress,
									WhiteSpace ws = ws_Post);
	UT_uint32		tagTop ();
	void			tagPop ();
	void			tagPI (const char * target, const UT_UTF8String & content);
	void			tagComment (const UT_UTF8String & content);
	void			tagCommentOpen ();
	void			tagCommentClose ();
	void			styleIndent ();
	
public:
	void			styleOpen (const UT_UTF8String & rule);
	void			styleClose ();
	void			styleNameValue (const char * name, const UT_UTF8String & value);
	void			setHaveHeader();
	void			setHaveFooter();
private:
	void			styleText (const UT_UTF8String & content);
	void			textTrusted (const UT_UTF8String & text);
	void			textUntrusted (const char * text);

	void			multiHeader (const UT_UTF8String & title);
	void			multiBoundary (bool end = false);
	void			multiField (const char * name, const UT_UTF8String & value);
	void			multiBreak ();

	UT_uint32		listDepth ();
	UT_uint32		listType ();
	void			listPush (UT_uint32 type, const char * ClassName);
	void			listPop ();
	void			listPopToDepth (UT_uint32 depth);

	bool			compareStyle (const char * key, const char * value);
	void            _fillColWidthsVector();
	void            _setCellWidthInches(void);
	
	void                addFootnote(PD_DocumentRange * pDocRange);         
	void                addEndnote(PD_DocumentRange * pDocRange);
	void                addAnnotation(PD_DocumentRange * pDocRange);
	UT_uint32           getNumFootnotes(void);
	UT_uint32           getNumEndnotes(void);
	UT_uint32           getNumAnnotations(void);
	
	/* temporary strings; use with extreme caution
	 */
	UT_UTF8String	m_utf8_0; // low-level
	UT_UTF8String	m_utf8_1; // intermediate

	UT_UTF8String	m_utf8_span;     // span tag-string cache
	UT_UTF8String	m_utf8_style;    // current block style

	const s_StyleTree *		m_StyleTreeInline; // current  inline  style tree, if any
	const s_StyleTree *		m_StyleTreeBlock;  // current  block   style tree, if any
	const s_StyleTree *		m_StyleTreeBody;   // document default style tree, if any

	const PP_AttrProp * m_pAPStyles;

	UT_UTF8String	m_utf8_css_path; // Multipart HTML: cache for content location

	UT_NumberStack	m_tagStack;

	UT_uint32		m_styleIndent;

	GsfOutput *		m_fdCSS;

	UT_GenericStringMap<UT_UTF8String*>	m_SavedURLs;

	bool            m_bIgnoreTillEnd;
	bool			m_bIgnoreTillNextSection;
	PT_DocPosition  m_iEmbedStartPos;

	double          m_dPageWidthInches;
	double          m_dSecLeftMarginInches;
	double          m_dSecRightMarginInches;
	double          m_dSecTopMarginInches;
	double          m_dSecBottomMarginInches;
	double          m_dCellWidthInches;
	UT_GenericVector<double*> m_vecDWidths;
	UT_UTF8String & m_sLinkCSS;
	UT_UTF8String & m_sTitle;

	UT_uint32       m_iOutputLen;
	bool            m_bCellHasData;
	UT_GenericVector<PD_DocumentRange *> m_vecFootnotes;
	UT_GenericVector<PD_DocumentRange *> m_vecEndnotes;
	UT_GenericVector<PD_DocumentRange *> m_vecAnnotations;

	IE_TOCHelper *  m_toc;
	int m_heading_count;
};

class ABI_EXPORT s_HTML_HdrFtr_Listener : public PL_Listener
{
	friend class s_HTML_Listener;
public:
	s_HTML_HdrFtr_Listener (PD_Document * pDocument, IE_Exp_HTML * pie, PL_Listener * pHTML_Listener);

	~s_HTML_HdrFtr_Listener ();

	bool	populate (PL_StruxFmtHandle sfh,
					  const PX_ChangeRecord * pcr);

	bool	populateStrux (PL_StruxDocHandle sdh,
						   const PX_ChangeRecord * pcr,
						   PL_StruxFmtHandle * psfh);

	//See note in _writeDocument
	//bool 	startOfDocument ();
	bool 	endOfDocument ();

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
	void	doHdrFtr(bool bHeader);
private:
			PD_DocumentRange * 	m_pHdrDocRange;
			PD_DocumentRange * 	m_pFtrDocRange;
			PD_Document 	 *	m_pDocument;
			PL_Listener 	 *	m_pHTML_Listener;
};

/*****************************************************************/
/*****************************************************************/

bool s_HTML_Listener::compareStyle (const char * key, const char * value)
{
	/* require both key & value to be non-empty strings
	 */
	if (( key == 0) || ( value == 0)) return false;
	if ((*key == 0) || (*value == 0)) return false;

	std::string css_name(key);

	std::string css_value;

	if (m_StyleTreeInline)
		css_value = m_StyleTreeInline->lookup (css_name);
	if (m_StyleTreeBlock && css_value.empty())
		css_value = m_StyleTreeBlock->lookup (css_name);
	if (m_StyleTreeBody  && css_value.empty())
		css_value = m_StyleTreeBody->lookup (css_name);

	return (css_value == value);
}

void s_HTML_Listener::tagRaw (UT_UTF8String & content)
{
#ifdef HTML_ENABLE_MHTML
	if (m_bQuotedPrintable) content.escapeMIME ();
#endif
	// fputs (content.utf8_str (), stdout);
	m_pie->write (content.utf8_str (), content.byteLength ());
	m_iOutputLen += content.byteLength();
}

UT_uint32 s_HTML_Listener::tagIndent ()
{
	return m_tagStack.getDepth ();
}

void s_HTML_Listener::tagNewIndent (UT_UTF8String & utf8, UT_uint32 depth)
{
	UT_uint32 i;

	for (i = 0; i < (depth >> 3); i++) utf8 += "\t";
	for (i = 0; i < (depth &  7); i++) utf8 += " ";
}

void s_HTML_Listener::tagNewIndent (UT_uint32 extra)
{
	m_utf8_0 = "";

	if(get_Compact())
		return;

	tagNewIndent (m_utf8_0, m_tagStack.getDepth () + extra);
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

	if (ws & ws_Post && !get_Compact()) m_utf8_0 += MYEOL;

	if(get_Compact())
	{
		if(m_iOutputLen + m_utf8_0.byteLength() > get_Compact())
		{
			m_pie->write(MYEOL, strlen(MYEOL));
			m_iOutputLen = 0;
		}
	}
	
	tagRaw (m_utf8_0);
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

	if (ws & ws_Post && !get_Compact()) m_utf8_0 += MYEOL;

	tagRaw (m_utf8_0);

	m_tagStack.push (tagID);
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

	if (ws & ws_Post && !get_Compact()) m_utf8_0 += MYEOL;

	if(get_Compact())
	{
		if(m_iOutputLen + m_utf8_0.byteLength() > get_Compact())
		{
			m_pie->write(MYEOL, strlen(MYEOL));
			m_iOutputLen = 0;
		}
	}
	
	tagRaw (m_utf8_0);
}

void s_HTML_Listener::tagClose (UT_uint32 tagID)
{
	UT_uint32 i = 0;
	m_tagStack.pop ((UT_sint32*)&i);

	if (i == tagID) return;

	UT_DEBUGMSG(("WARNING: possible tag mis-match in XHTML output!\n"));
	UT_DEBUGMSG(("WARNING:     Tag requested %i, tag found %i\n", tagID, i));
}

/* use with *extreme* caution! (this is used by images with data-URLs)
 */
void s_HTML_Listener::tagOpenBroken (const UT_UTF8String & content,
									 WhiteSpace ws)
{
	if (ws & ws_Pre)
	{
		tagNewIndent ();
		m_utf8_0 += "<";
	}
	else
		m_utf8_0 = "<";

	m_utf8_0 += content;

	tagRaw (m_utf8_0);
}

/* use with *extreme* caution! (this is used by images with data-URLs)
 */
void s_HTML_Listener::tagCloseBroken (const UT_UTF8String & content, bool suppress,
									  WhiteSpace ws)
{
	m_utf8_0 = content;

	if (suppress)
		m_utf8_0 += " >";
	else
		m_utf8_0 += " />";

	if (ws & ws_Post && !get_Compact())
		m_utf8_0 += MYEOL;

	if(get_Compact())
	{
		if(m_iOutputLen + m_utf8_0.byteLength() > get_Compact())
		{
			m_pie->write(MYEOL, strlen(MYEOL));
			m_iOutputLen = 0;
		}
	}
	
	tagRaw (m_utf8_0);
}

UT_uint32 s_HTML_Listener::tagTop ()
{
	UT_sint32 i = 0;
	if (m_tagStack.viewTop (i)) return (UT_uint32)i;
	return 0;
}

void s_HTML_Listener::tagPop ()
{
	// Don't forget to not put tags without explicit closure in here.
	switch (tagTop ())
	{
		case TT_TD:
			{
				tagClose (TT_TD, "td");
			}
			break;
		case TT_TR:
			{
				tagClose (TT_TR, "tr");
			}
			break;
		case TT_TBODY:
			{
				tagClose (TT_TBODY, "tbody");
			}
			break;
		case TT_TABLE:
			{
				tagClose (TT_TABLE, "table");
			}
			break;
		case TT_DIV:
			{
				tagClose (TT_DIV, "div");
			}
			break;
		case TT_P:
			{
				tagClose(TT_P, "p");
			}
			break;
		case TT_H1:
			{
				tagClose(TT_H1, "h1");
			}
			break;
		case TT_H2:
			{
				tagClose(TT_H2, "h2");
			}
			break;
		case TT_H3:
			{
				tagClose(TT_H3, "h3");
			}
			break;
		case TT_H4:
			{
				tagClose(TT_H4, "h4");
			}
			break;
		case TT_SPAN:
			{
				tagClose (TT_SPAN, "span");
			}
			break;
		case TT_BDO:
			{
				tagClose (TT_BDO, "bdo");
			}
			break;
		case TT_LI:
			{
				tagClose (TT_LI, "li");
			}
			break;
		case TT_UL:
			{
				tagClose (TT_UL, "ul");
			}
			break;
		case TT_OL:
		       {
				tagClose (TT_OL, "ol");
			}
			break;
		default:
			{
				UT_DEBUGMSG(("tagPop: unhandled tag closure! %d\n",tagTop()));
				m_utf8_1 ="error - not handled";
				/* There has got to be a better way than this.
						I think putting up the usual "Write error -
						file a bug and attach this" message is preferable
						to spitting out nonsense html. -MG */
				tagClose (tagTop(), m_utf8_1); // prevents hangs see 7524
			}
			break;
	}
}

void s_HTML_Listener::tagPI (const char * target, const UT_UTF8String & content)
{
	tagNewIndent ();

	m_utf8_0 += "<?";
	m_utf8_0 += target;
	m_utf8_0 += " ";
	m_utf8_0 += content;
	m_utf8_0 += "?>";
	if(!get_Compact())
		m_utf8_0 += MYEOL;

	tagRaw (m_utf8_0);
}

void s_HTML_Listener::tagComment (const UT_UTF8String & content)
{
	tagNewIndent ();

	m_utf8_0 += "<!-- ";
	m_utf8_0 += content;
	m_utf8_0 += " -->";
	if(!get_Compact())
		m_utf8_0 += MYEOL;

	tagRaw (m_utf8_0);
}

void s_HTML_Listener::tagCommentOpen ()
{
	tagNewIndent ();

	m_utf8_0 += "<!--";
	if(!get_Compact())
		m_utf8_0 += MYEOL;

	tagRaw (m_utf8_0);
}

void s_HTML_Listener::tagCommentClose ()
{
	tagNewIndent (2);

	m_utf8_0 += "-->";
	if(!get_Compact())
		m_utf8_0 += MYEOL;

	tagRaw (m_utf8_0);
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
	m_utf8_0 += " {";
	if(!get_Compact())
		m_utf8_0 += MYEOL;

	if (m_fdCSS)
		gsf_output_write (m_fdCSS, m_utf8_0.byteLength (), (const guint8*)m_utf8_0.utf8_str ());
	else
		tagRaw (m_utf8_0);

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

	m_utf8_0 += "}";
	if(!get_Compact())
		m_utf8_0 += MYEOL;

	if (m_fdCSS)
		gsf_output_write (m_fdCSS, m_utf8_0.byteLength (), (const guint8*)m_utf8_0.utf8_str ());
	else
		tagRaw (m_utf8_0);
}

void s_HTML_Listener::styleNameValue (const char * name, const UT_UTF8String & value)
{
	styleIndent ();

	m_utf8_0 += name;
	m_utf8_0 += ":";
	m_utf8_0 += value;
	m_utf8_0 += ";";
	if(!get_Compact())
		m_utf8_0 += MYEOL;

	if (m_fdCSS)
		gsf_output_write (m_fdCSS, m_utf8_0.byteLength (), (const guint8*)m_utf8_0.utf8_str ());
	else
		tagRaw (m_utf8_0);
}

void s_HTML_Listener::styleText (const UT_UTF8String & content)
{
	if (m_fdCSS)
		gsf_output_write (m_fdCSS, content.byteLength (), (const guint8*)content.utf8_str ());
	else
	{
		m_utf8_0 = content;
		tagRaw (m_utf8_0);
	}
}

void s_HTML_Listener::textTrusted (const UT_UTF8String & text)
{
	if (text.byteLength ())
	{
		m_utf8_0 = text;
		tagRaw (m_utf8_0);

		m_bWroteText = true;
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
	if (m_utf8_0.byteLength ()) tagRaw (m_utf8_0);
}

static const char * s_boundary = "AbiWord_multipart_boundary____________";

void s_HTML_Listener::multiHeader (const UT_UTF8String & title)
{
	m_utf8_1 = "<Saved by AbiWord>";
	multiField ("From", m_utf8_1);

	multiField ("Subject", title);

	time_t tim = time (NULL);
	struct tm * pTime = localtime (&tim);
	char timestr[64];
	strftime (timestr, 63, "%a, %d %b %Y %H:%M:%S +0100", pTime); // hmm, hard-code time zone
	timestr[63] = 0;

	m_utf8_1 = timestr;
	multiField ("Date", m_utf8_1);

	m_utf8_1 = "1.0";
	multiField ("MIME-Version", m_utf8_1);

	m_utf8_1  = "multipart/related;" MYEOL "\tboundary=\"";
	m_utf8_1 += s_boundary;
	m_utf8_1 += "\";" MYEOL "\ttype=\"";

	if (get_HTML4 ())
		m_utf8_1 += IE_MIMETYPE_HTML;
	else
		m_utf8_1 += IE_MIMETYPE_XHTML;

	m_utf8_1 += "\"";

	multiField ("Content-Type", m_utf8_1);
	multiBoundary ();

	if (get_HTML4 ())
		m_utf8_1 = IE_MIMETYPE_HTML;
	else
		m_utf8_1 = IE_MIMETYPE_XHTML;

	m_utf8_1 += ";charset=\"UTF-8\"";

	multiField ("Content-Type", m_utf8_1);

	m_utf8_1  = "quoted-printable";
	multiField ("Content-Transfer-Encoding", m_utf8_1);
	multiBreak ();

	m_bQuotedPrintable = true;
}

void s_HTML_Listener::multiBoundary (bool end)
{
	m_utf8_0  = MYEOL "--";
	m_utf8_0 += s_boundary;

	if (end)
		m_utf8_0 += "--" MYEOL;
	else
		m_utf8_0 += MYEOL;
	// fputs (m_utf8_0.utf8_str (), stdout);
	m_pie->write (m_utf8_0.utf8_str (), m_utf8_0.byteLength ());
	m_iOutputLen += m_utf8_0.byteLength();
}

void s_HTML_Listener::multiField (const char * name, const UT_UTF8String & value)
{
	m_utf8_0  = name;
	m_utf8_0 += ":";
	m_utf8_0 += value;
	if(!get_Compact())
		m_utf8_0 += MYEOL;
	// fputs (m_utf8_0.utf8_str (), stdout);
	m_pie->write (m_utf8_0.utf8_str (), m_utf8_0.byteLength ());
	m_iOutputLen += m_utf8_0.byteLength();
	
}

void s_HTML_Listener::multiBreak ()
{
	m_utf8_0 = MYEOL;
	// fputs (m_utf8_0.utf8_str (), stdout);
	m_pie->write (m_utf8_0.utf8_str (), m_utf8_0.byteLength ());
	m_iOutputLen += m_utf8_0.byteLength();
}

/* intermediate methods
 */

static const char * s_DTD_XHTML_AWML = "!DOCTYPE html PUBLIC \"-//ABISOURCE//DTD XHTML plus AWML 2.2//EN\" \"http://www.abisource.com/2004/xhtml-awml/xhtml-awml.mod\"";

static const char * s_DTD_XHTML = "!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\"";

static const char * s_DTD_HTML4 = "!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\"";

static const char * s_Delimiter = 
"=======================================================";

static const char * s_Header[2] = {
   "Created by AbiWord, a free, Open Source wordprocessor. ",
   "For more information visit http://www.abisource.com.   "
};

static const char * s_HeaderCompact = "Created by Abiword, www.abisource.com";

void s_HTML_Listener::_outputBegin (PT_AttrPropIndex api)
{
	if (m_bTemplateBody)
	{
		m_bFirstWrite = false;
		return;
	}

	if(m_sTitle.byteLength() == 0)
	{
#ifdef HTML_META_SUPPORTED
		m_pDocument->getMetaDataProp (PD_META_KEY_TITLE, m_sTitle);

		if (m_sTitle.byteLength () == 0 && m_pie->getFileName () != NULL) 
			m_sTitle = UT_basename(m_pie->getFileName ());
#else
		m_sTitle = UT_basename(m_pie->getFileName ());
#endif
	}
	
	if (get_Multipart ()) multiHeader (m_sTitle);

	/* print XML header
	 */
	if (!get_HTML4 ())
	{
		if (get_Declare_XML ())
		{
			m_utf8_1 = "version=\"1.0\" encoding=\"UTF-8\"";
			tagPI ("xml", m_utf8_1);
		}
		if (get_Allow_AWML ())
			m_utf8_1 = s_DTD_XHTML_AWML;
		else
			m_utf8_1 = s_DTD_XHTML;
		tagOpenClose (m_utf8_1, true);
	}
	else
	{
		m_utf8_1 = s_DTD_HTML4;
		tagOpenClose (m_utf8_1, true);
	}
	
	/* open root element, i.e. <html>; namespace it if XHTML
	 */
	m_utf8_1 = "html";
	if (!get_HTML4 ())
	{
		m_utf8_1 += " xmlns=\"http://www.w3.org/1999/xhtml\"";
		if (get_Allow_AWML ()) m_utf8_1 += " xmlns:awml=\"http://www.abisource.com/2004/xhtml-awml/\"";
	}
	tagOpen (TT_HTML, m_utf8_1);
	
	/* start <head> section of HTML document
	 */
	m_utf8_1 = "head";
	tagOpen (TT_HEAD, m_utf8_1);

	/* print header comment
	 * we insert them that let, because IE6 expect to find <HTML> root within
	 * 6 lines.
	 */
	if(get_Compact())
	{
		m_utf8_1 = s_HeaderCompact;
		tagComment (m_utf8_1);
	}
	else
	{
		const UT_UTF8String delimiter(s_Delimiter);
		tagComment (delimiter);
		for (UT_uint32 hdri = 0; hdri < G_N_ELEMENTS(s_Header); hdri++)
		{
			m_utf8_1 = s_Header[hdri];
			tagComment (m_utf8_1);
		}
		tagComment (delimiter);
	}
	//
	// Export the script to enable mathml and SVG in HTML4
	//
	if(m_pDocument->hasMath())
	{
		m_pie->write(sMathSVGScript.utf8_str(),sMathSVGScript.size());
	}
	
	/* we add a meta tag describing the document's charset as UTF-8
	 * even with XHTML because Safari and Camino fail to recognize
	 * charset. This still validate W3C.
	 */

	m_utf8_1 = "meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-8\"";
	tagOpenClose (m_utf8_1, get_HTML4 ());

	/* set page's title in browser
	 */
	m_utf8_1 = "title";
	tagOpen (TT_TITLE, m_utf8_1, ws_Pre);

#ifdef HTML_META_SUPPORTED
	textTrusted (m_sTitle.escapeXML ()); // TODO: back-port this method?
#else
	textUntrusted (m_sTitle.utf8_str ());
#endif

	tagClose (TT_TITLE, m_utf8_1, ws_Post);

#ifdef HTML_META_SUPPORTED
	/* write out our metadata properties
	 */
	_handleMeta ();
#endif

	if (!get_PHTML ())
	{
		const PP_AttrProp * pAP = 0;
		bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

		if (bHaveProp && pAP)
		{
			_outputStyles (pAP);
			if (!get_Embed_CSS ()) m_pAPStyles = pAP;
		}
	}

	if (get_PHTML ())
	{
		m_utf8_1 = MYEOL "  include($_SERVER['DOCUMENT_ROOT'].'/x-header.php');" MYEOL " ";
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

	if (get_PHTML ())
	{
		m_utf8_1 = MYEOL "  include($_SERVER['DOCUMENT_ROOT'].'/x-page-begin.php');" MYEOL " ";
		tagPI ("php", m_utf8_1);
	}

	m_bFirstWrite = false;
}

void s_HTML_Listener::_outputEnd ()
{
	if (m_bInBlock) _closeTag ();

	while (true)
	{
		UT_uint32 top = tagTop ();
		if ((top == TT_BODY) || !top)
			break;
		tagPop ();
	}
	if (m_bTemplateBody) return;

	if (get_PHTML ())
	{
		m_utf8_1 = MYEOL "  include($_SERVER['DOCUMENT_ROOT'].'/x-page-end.php');" MYEOL " ";
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

	if (get_Multipart ())
	{
		m_bQuotedPrintable = false;

		if (m_pAPStyles)
		{
			_outputStyles (m_pAPStyles);
			m_bQuotedPrintable = false;
		}
		_handlePendingImages ();

		multiBoundary (true);
	}
}

bool s_HTML_Listener::_openStyleSheet (UT_UTF8String & css_relative_path)
{
	UT_UTF8String cssdir(m_pie->getFileName ());
	cssdir += "_files";

	UT_go_directory_create (cssdir.utf8_str(), 0750, NULL);

	UT_UTF8String css_path = cssdir;
	css_path += "/style.css";

	if (m_utf8_css_path.byteLength ()) // Multipart HTML: style-sheet segment
	{
		multiBoundary ();

		m_utf8_1  = IE_MIMETYPE_CSS;
		m_utf8_1 += ";charset=\"UTF-8\"";

		multiField ("Content-Type",     m_utf8_1);
		multiField ("Content-Location", m_utf8_css_path);

		m_utf8_1  = "quoted-printable";
		multiField ("Content-Transfer-Encoding", m_utf8_1);
		multiBreak ();

		m_bQuotedPrintable = true;
	}
	else if (!get_Multipart ())
	{
		m_fdCSS = UT_go_file_create (css_path.utf8_str (), NULL);
		if (m_fdCSS == NULL) return false;
	}

	char * base_name = UT_go_basename_from_uri (m_pie->getFileName ());
	if (base_name)
		css_relative_path = base_name;
	css_relative_path += "/styles.css";
	g_free(base_name);

	return true;
}

void s_HTML_Listener::_closeStyleSheet ()
{
	if (m_fdCSS) { 
		gsf_output_close (m_fdCSS); 
		g_object_unref (G_OBJECT (m_fdCSS));
		m_fdCSS = 0;
	}
}

// TODO: Use the styleIndent code to clean up this output
void s_HTML_Listener::_populateHeaderStyle() {
	const gchar * staticCSSHeaderProps [9] = {"position: relative;","width: 100%;","height: auto;",
		"top: 0;","bottom: auto;","right: 0;","left: 0;","}",NULL}; // Static properties for headers
	m_utf8_1 = "#header {"; // Reinitialize the variable, now to deal with the header-identified div
	m_utf8_1 += MYEOL;	
	for(unsigned short int propIdx = 0; propIdx < 8; propIdx += 1)
	{
		m_utf8_1 += staticCSSHeaderProps[propIdx];
		m_utf8_1 += MYEOL;
	}
	styleText(m_utf8_1);
}
// TODO: Use the styleIndent code to clean up this output
void s_HTML_Listener::_populateFooterStyle() {
	const gchar * staticCSSFooterProps [9] = {"position: relative;","width: 100%;","height: auto;",
		"top: auto;","bottom: 0;","right: 0;","left: 0;","}",NULL}; // Static properties for footers
	m_utf8_1 = "#footer {"; // Reinitialize the variable, now to deal with the footer-identified div
	m_utf8_1 += MYEOL;
	for(unsigned short int propIdx = 0; propIdx < 8; propIdx += 1)
	{
		m_utf8_1 += staticCSSFooterProps[propIdx];
		m_utf8_1 += MYEOL;
	}
	styleText(m_utf8_1);
}

void s_HTML_Listener::_outputStyles (const PP_AttrProp * pAP)
{
	// make sure any unit conversions use correct locale
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	/* some cascading style rules
	 */
	const gchar * szName  = 0;
	const gchar * szValue = 0;

	if (get_Embed_CSS ())
	{
		m_utf8_1 = "style type=\"text/css\"";
		tagOpen (TT_STYLE, m_utf8_1);
		tagCommentOpen ();
	}
	else if(get_Link_CSS())
	{
		m_utf8_1  = "link href=\"";
		m_utf8_1 += m_sLinkCSS;
		m_utf8_1 += "\" rel=\"stylesheet\" type=\"text/css\"";

		tagOpenClose (m_utf8_1, get_HTML4 ());

		// do not export style definitions ...
		return;
	}
	else
	{
		UT_UTF8String css_path;

		if (!_openStyleSheet (css_path)) return;

		if (!get_Multipart () || (m_utf8_css_path.byteLength () == 0))
		{
			m_utf8_1  = "link href=\"";
			m_utf8_1 += css_path;
			m_utf8_1 += "\" rel=\"stylesheet\" type=\"text/css\"";

			tagOpenClose (m_utf8_1, get_HTML4 ());

			if (get_Multipart ())
			{
				m_utf8_css_path = css_path;
				return;
			}
		}

		/* first line of style sheet is an encoding declaration
		 */
		m_utf8_1 = "@charset \"UTF-8\";";
		if(!get_Compact())
			m_utf8_0 += MYEOL MYEOL;
		
		styleText (m_utf8_1);
	}

	// stylesheet stolen from wikipedia
	styleText("#toc,\n"
".toc,\n"
".mw-warning {\n"
"	border: 1px solid #aaa;\n"
"	background-color: #f9f9f9;\n"
"	padding: 5px;\n"
"	font-size: 95%;\n"
"}\n"
"#toc h2,\n"
".toc h2 {\n"
"	display: inline;\n"
"	border: none;\n"
"	padding: 0;\n"
"	font-size: 100%;\n"
"	font-weight: bold;\n"
"}\n"
"#toc #toctitle,\n"
".toc #toctitle,\n"
"#toc .toctitle,\n"
".toc .toctitle {\n"
"	text-align: center;\n"
"}\n"
"#toc ul,\n"
".toc ul {\n"
"	list-style-type: none;\n"
"	list-style-image: none;\n"
"	margin-left: 0;\n"
"	padding-left: 0;\n"
"	text-align: left;\n"
"}\n"
"#toc ul ul,\n"
".toc ul ul {\n"
"	margin: 0 0 0 2em;\n"
"}\n"
"#toc .toctoggle,\n"
".toc .toctoggle {\n"
"	font-size: 94%;\n"
"}");

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
		m_utf8_1 = "@media print, projection, embossed";
		styleOpen (m_utf8_1);

		m_utf8_1 = "body";
		styleOpen (m_utf8_1);
		
		// Set margins for paged media to match those set in AbiWord
		// TODO: consolidate all places of awml-css21 matching into one UT/PP function
		const gchar * marginProps [10] = {"page-margin-top","padding-top",
			"page-margin-bottom","padding-bottom",
			"page-margin-left","padding-left",
			"page-margin-right","padding-right",
			NULL, NULL};
		for (unsigned short int propIdx = 0; propIdx < 8; propIdx += 2)
		{
			szValue = PP_evalProperty (marginProps[propIdx], 0, 0, pAP, m_pDocument, true);
			m_utf8_1 = static_cast<const char *>(szValue);
			styleNameValue (marginProps[propIdx + 1], m_utf8_1);
		}

		styleClose (); // end of: body { }
		styleClose (); // end of: @media print { }
		
		if(m_bHaveHeader) _populateHeaderStyle();
		if(m_bHaveFooter) _populateFooterStyle();
		
		m_utf8_1 = "body";
		styleOpen (m_utf8_1);

		for (UT_uint32 i = 0; i < pStyle->getPropertyCount (); i++)
		{
			pStyle->getNthProperty (i, szName, szValue);

			if (( szName == 0) || ( szValue == 0)) continue; // paranoid? moi?
			if ((*szName == 0) || (*szValue == 0)) continue;

			if (strstr (szName, "margin")) continue;
			if (!is_CSS (reinterpret_cast<const char *>(szName))) continue;

			if (strcmp (szName, "font-family") == 0)
			{
				if ((strcmp (szValue, "serif")      == 0) ||
					(strcmp (szValue, "sans-serif") == 0) ||
					(strcmp (szValue, "cursive")    == 0) ||
					(strcmp (szValue, "fantasy")    == 0) ||
					(strcmp (szValue, "monospace")  == 0))
				{
					m_utf8_1 = static_cast<const char *>(szValue);
				}
				else
				{
					m_utf8_1  = "'";
					m_utf8_1 += static_cast<const char *>(szValue);
					m_utf8_1 += "'";
				}
			}
			else if (strcmp (szName, "color") == 0)
			{
				if (IS_TRANSPARENT_COLOR (szValue)) continue;

				m_utf8_1 = UT_colorToHex(szValue, true);
			}
			else m_utf8_1 = static_cast<const char *>(szValue);

			styleNameValue (szName, m_utf8_1);
		}
		szValue = PP_evalProperty ("background-color", 0, 0, pAP, m_pDocument, true);
		if(szValue && *szValue && !IS_TRANSPARENT_COLOR (szValue))
		{
			m_utf8_1 = UT_colorToHex(szValue, true);

			styleNameValue ("background-color", m_utf8_1);
		}
		styleClose (); // end of: body { }

#ifdef HTML_TABLES_SUPPORTED
		szValue = PP_evalProperty ("width", 0, 0, pAP, m_pDocument, true);
		
		m_utf8_1 = "table";
		styleOpen (m_utf8_1);

		if(get_Abs_Units() && szValue && *szValue)
		{
			double dMM = UT_convertToDimension(szValue, DIM_MM);
			UT_UTF8String_sprintf(m_utf8_1, "%.1fmm", dMM);
			styleNameValue ("width", m_utf8_1);
		}
		else if(get_Scale_Units() && szValue && *szValue)
		{
			m_utf8_1  = "100%";
			styleNameValue ("width", m_utf8_1);
		}
		// else do nothing, because in flow-based document width is left to box model
		
		styleClose (); // end of: table { }

		m_utf8_1 = "td";
		styleOpen (m_utf8_1);

		m_utf8_1 = "collapse";
		styleNameValue ("border-collapse", m_utf8_1);

		m_utf8_1 = "left";
		styleNameValue ("text-align", m_utf8_1);

		m_utf8_1 = "top";
		styleNameValue ("vertical-align", m_utf8_1);

		styleClose (); // end of: td { }
#endif /* HTML_TABLES_SUPPORTED */
	}

	m_style_tree->print (this);

	if (get_Embed_CSS ())
	{
		tagCommentClose ();
		m_utf8_1 = "style";
		tagClose (TT_STYLE, m_utf8_1);
	}
	else _closeStyleSheet ();
}

/*!
 * This closes open section tags and starts new one for embedded struxes
 */
void s_HTML_Listener::startEmbeddedStrux(void)
{
	if (m_bInSection) _closeSection ();

	m_utf8_1 = "div";
	tagOpen (TT_DIV, m_utf8_1);
	m_bInSection = true;
}

void s_HTML_Listener::_openSection (PT_AttrPropIndex api, UT_uint16 iSectionSpecialType)
{
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	
	if (m_bFirstWrite) _outputBegin (api);

	if (m_bInSection) _closeSection ();

	const PP_AttrProp* pSectionAP = NULL;
	m_pDocument->getAttrProp(api, &pSectionAP);
	
	m_utf8_1 = "div";
	
	switch (iSectionSpecialType)
	{
		case 1:
		{
			m_utf8_1 += " id=\"header\"";
			m_bInSection = true;
			break;
		}
		case 2:
		{
			m_utf8_1 += " id=\"footer\"";
			m_bInSection = true;
			break;
		}
		case 3:
		{
			m_utf8_1 += " id=\"main\"";
			break;
		}
		default:
			m_bInSection = true;
			break;
	}

	tagOpen (TT_DIV, m_utf8_1);
	m_dPageWidthInches = m_pDocument->m_docPageSize.Width(DIM_IN);

	const char* pszLeftMargin = NULL;
	const char* pszRightMargin = NULL;
	const char* pszTopMargin = NULL;
	const char* pszBottomMargin = NULL;
	pSectionAP->getProperty("page-margin-left", (const gchar *&)pszLeftMargin);
	pSectionAP->getProperty("page-margin-right", (const gchar *&)pszRightMargin);
	pSectionAP->getProperty("page-margin-top", (const gchar *&)pszTopMargin);
	pSectionAP->getProperty("page-margin-bottom", (const gchar *&)pszBottomMargin);
	
	if(pszLeftMargin && pszLeftMargin[0])
	{
		m_dSecLeftMarginInches = UT_convertToInches(pszLeftMargin);
	}
	else
	{
		m_dSecLeftMarginInches = 1.0;
	}

	if(pszRightMargin && pszRightMargin[0])
	{
		m_dSecRightMarginInches = UT_convertToInches(pszRightMargin);
	}
	else
	{
		m_dSecRightMarginInches = 1.0;
	}
	
	if(pszTopMargin && pszTopMargin[0])
	{
		m_dSecTopMarginInches = UT_convertToInches(pszTopMargin);
	}
	else
	{
		m_dSecTopMarginInches = 1.0;
	}

	if(pszBottomMargin && pszBottomMargin[0])
	{
		m_dSecBottomMarginInches = UT_convertToInches(pszBottomMargin);
	}
	else
	{
		m_dSecBottomMarginInches = 1.0;
	}
	
}

void s_HTML_Listener::_closeSection (void)
{
	// When we start tracking list ideas and doing store-first-write-later on them,
	// and then start supporting unified discontinuous lists,
	// complications with questionable worthwhileness,
	// we will no longer have to pop out for every section break even when there are identical listIds spanning multiple sections.
	// Until then, this is necessary.
	listPopToDepth(0);
	
	if (tagTop() == TT_SPAN) {
		UT_DEBUGMSG(("_closeSection closing span\n"));
		tagClose(TT_SPAN, "span");
	}
	
	if (m_bInBlock && (tagTop() == TT_P)) { // If only the first is true, we have a first-order tag mismatch.  The alternative with not testing the latter is a second-order tag mismatch.
		UT_DEBUGMSG(("_closeSection closing block\n"));
	//		_closeTag (); // We need to investigate the tag stack usage of this, and whether or not we really would rather specify the tag in all cases.
		tagClose(TT_P, "p");
	}
	// Need to investigate whether we can safely uncomment this without undoing heading work, or any other kind using unended structures like lists.
	// _popUnendedStructures(); // Close lists, and possibly other stuff.  Even if it theoretically can span sections, we run a high risk of corrupting the document.

	if (m_bInSection && (tagTop () == TT_DIV))
	{
		m_utf8_1 = "div";
		UT_DEBUGMSG(("_closeSection closing div\n"));
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
				const gchar * szName = 0;
				pBasedOn->getAttribute (PT_NAME_ATTRIBUTE_NAME, szName);

				if (szName)
				{
					/* careful!!
					 */
					s_removeWhiteSpace (static_cast<const char *>(szName), m_utf8_0, true);

					if (m_utf8_0.utf8_str ())
						bret = (strcmp (from, m_utf8_0.utf8_str ()) == 0);
				}
			}
		}
	return bret;
}

UT_uint32 s_HTML_Listener::listDepth ()
{
	return static_cast<UT_uint32>(m_utsListType.getDepth ());
}

UT_uint32 s_HTML_Listener::listType ()
{
	UT_sint32 i = 0;
	m_utsListType.viewTop (i);
	return (UT_uint32)i;
}

void s_HTML_Listener::listPush (UT_uint32 type, const char * /*ClassName*/)
{
	if (tagTop () == TT_LI)
	{
		m_utf8_1 = MYEOL;
		tagRaw (m_utf8_1);
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
	tagOpen (tagID, m_utf8_1);

	m_utsListType.push (static_cast<UT_sint32>(type));
}

void s_HTML_Listener::listPop ()
{
	if (tagTop () == TT_SPAN)
	{
		// We don't just tagPop() for the sake of prettiness, apparently.
		m_utf8_1 = "span";
		tagClose (TT_SPAN, m_utf8_1, ws_Post);
	}
#if 0
   // This code may come in handy if AbiWord ever supported listed frames, which is conceivable
	if (m_bInFrame && tagTop () == TT_DIV) // Frame embedded in list, hopefully.  I _really_ hope we don't have a first order section in a list.
	{
	     if(m_bInTextBox)
	        _closeTextBox();
	     else
	     {
	        UT_DEBUGMSG(("WARNING: Popping a frame which is not a textbox within a list item, heaven help us \n"));
		 
	       m_utf8_1 = "div";
		tagClose (TT_DIV, m_utf8_1);
	     }
	}
#endif
	if (tagTop () == TT_LI)
	{
		m_utf8_1 = "li";
		tagClose (TT_LI, m_utf8_1);
	}

	UT_sint32 type = 0;
	m_utsListType.pop (&type);

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

	if (tagTop () == TT_LI)
	{
		m_utf8_1 = "";
		tagNewIndent (m_utf8_1, tagIndent () - 1);
		tagRaw (m_utf8_1);
	}
}

void s_HTML_Listener::listPopToDepth (UT_uint32 depth)
{
	if (listDepth () <= depth) return;

	UT_uint32 count = listDepth () - depth;
	for (UT_uint32 i = 0; i < count; i++) listPop ();
}

void s_HTML_Listener::_openTag (PT_AttrPropIndex api, PL_StruxDocHandle /*sdh*/)
{
	if (m_bFirstWrite) _openSection (api, 0);

	if (!m_bInSection) return;

	m_StyleTreeInline = 0;
	m_StyleTreeBlock  = 0;

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

	const gchar * szDefault = "Normal"; // TODO: should be/is a #define somewhere?

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	
	if ((!bHaveProp || (pAP == 0)) && !m_bInFrame) // [Appears to be and we're assuming it's a] <p> with no style attribute, and no properties either, and not a frame embedded in the list
	{
		listPopToDepth (0);

		m_utf8_1 = "p";
		tagOpen (TT_P, m_utf8_1, ws_Pre);

		m_utf8_style = szDefault;

		m_iBlockType = BT_NORMAL;
		m_bInBlock = true;
		return;
	}

	UT_uint32 tagID = TT_OTHER;

	bool tagPending = false;

	const gchar * szValue = 0;
	const gchar * szLevel = 0;
	const gchar * szListID = 0;
	const gchar * szStyleType = 0;

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
	bool bClassAsTag = false;

	if (have_listID) zero_listID = (strcmp (szListID, "0") == 0);

	/* A nonzero "listid" attribute value indicates that we
	 * are in a list item, so we need to process it, HTML-style.
	 */

	/* Specify a default style name for this list item if it
	 * doesn't already have one.
	 */
	if (!have_style) szValue = szDefault;

	m_utf8_style = szValue;

	m_StyleTreeBlock = m_style_tree->find (szValue);

	if (!zero_listID)
	{
		/* Desired list type (numbered / bullet)
		 */
		if (!pAP->getProperty ("list-style", szStyleType)) szStyleType = szValue;

		if (strcmp (static_cast<const char *>(szStyleType), "Bullet List") == 0)
		{
			m_iBlockType = BT_BULLETLIST;
		}
		else 
		{
			m_iBlockType = BT_NUMBEREDLIST;
		}

		/* Find out how deeply nested this list item is.
		 */
		pAP->getAttribute ("level", szLevel);
		if(szLevel)
		{
			m_iListDepth = atoi (static_cast<const char *>(szLevel));
		}
		else
		{
			m_iListDepth = 0;
		}
		/* TODO: why can m_iListDepth be zero sometimes ?? (numbered headings?)
		 * UPDATE: I don't know, but it probably shouldn't be handled this way because the definition of 0
		 *	    from earlier in the code is that it means we are not in a list of any sort.
		 *	    For numbered headings and the like, if that's the handling of it, we may actually end up
		 *	    with more than one list after having 'risen to desired list depth'. -MG
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

		if (m_StyleTreeBlock)
			if (m_StyleTreeBlock->class_list().byteLength ())
			{
				UT_UTF8String escape;
				m_utf8_1 += " class=\"";
				if(get_Class_Only())
				{
					escape = m_StyleTreeBlock->class_name ();
					m_utf8_1 += escape.escapeXML();
				}
				else
				{
					escape = m_StyleTreeBlock->class_list ();
					m_utf8_1 += escape.escapeXML();
				}
				
				m_utf8_1 += "\"";
			}
	}
	else if (have_style)
	{
		listPopToDepth (0);

		const s_StyleTree * tree = m_StyleTreeBlock;

		bool bAddAWMLStyle = false;
		if (get_Allow_AWML () && !get_HTML4 ()) bAddAWMLStyle = true;

		if ((g_ascii_strcasecmp (static_cast<const char *>(szValue), "Heading 1") == 0) ||
			(g_ascii_strcasecmp (static_cast<const char *>(szValue), "Numbered Heading 1") == 0))
		{
			m_iBlockType = BT_HEADING1;
			tagID = TT_H1;
			tagPending = true;
			bClassAsTag = true;

			if (m_toc->docHasTOC()) {
				m_utf8_1 = UT_UTF8String_sprintf("h1 id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			}
			else
				m_utf8_1 = "h1";

			if (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Heading 1") == 0)
				bAddAWMLStyle = false;
		}
		else if ((g_ascii_strcasecmp (static_cast<const char *>(szValue), "Heading 2") == 0) ||
				 (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Numbered Heading 2") == 0))
		{
			m_iBlockType = BT_HEADING2;
			tagID = TT_H2;
			tagPending = true;
			bClassAsTag = true;

			if (m_toc->docHasTOC()) {
				m_utf8_1 = UT_UTF8String_sprintf("h2 id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			}
			else
				m_utf8_1 = "h2";

			if (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Heading 2") == 0)
				bAddAWMLStyle = false;
		}
		else if ((g_ascii_strcasecmp (static_cast<const char *>(szValue), "Heading 3") == 0) ||
				 (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Numbered Heading 3") == 0))
		{
			m_iBlockType = BT_HEADING3;
			tagID = TT_H3;
			tagPending = true;
			bClassAsTag = true;

			if (m_toc->docHasTOC()) {
				m_utf8_1 = UT_UTF8String_sprintf("h3 id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			}
			else
				m_utf8_1 = "h3";

			if (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Heading 3") == 0)
				bAddAWMLStyle = false;
		}
		else if (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Block Text") == 0)
		{
			m_iBlockType = BT_BLOCKTEXT;
			tagID = TT_BLOCKQUOTE;
			tagPending = true;
			bClassAsTag = true;
			m_utf8_1 = "blockquote";
			bAddAWMLStyle = false;
		}
		else if (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Plain Text") == 0)
		{
			m_iBlockType = BT_NORMAL;
			tagID = TT_P;
			tagPending = true;
			bClassAsTag = true;
			m_utf8_1 = "p class=\"plain_text\"";
			bAddAWMLStyle = false;
		}
		else if (g_ascii_strcasecmp (static_cast<const char *>(szValue), "Normal") == 0)
		{
			m_iBlockType = BT_NORMAL;
			tagID = TT_P;
			tagPending = true;
			bClassAsTag = true;

			// if class-only is specified, we want
			// class=Normal because changing definition of
			// unqualified <p> in a stylesheet tends to be
			// rather messy; it is much easier to define
			// P.Normal
			if(get_Class_Only())
				m_utf8_1 = "p class=\"Normal\"";
			else
				m_utf8_1 = "p";
					
			bAddAWMLStyle = false;
		}
		else if (tree == 0) // hmm...
		{
			m_iBlockType = BT_NORMAL;
			tagID = TT_P;
			tagPending = true;
			m_utf8_1 = "p";
		}
		else if (tree->descends ("Heading 1"))
		{
			m_iBlockType = BT_HEADING1;
			tagID = TT_H1;
			tagPending = true;
			bClassAsTag = true;

			if (m_toc->docHasTOC()) {
				m_utf8_1 = UT_UTF8String_sprintf("h1 id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			} else {
				m_utf8_1 = "h1";
			}
		}
		else if (tree->descends ("Heading 2"))
		{
			m_iBlockType = BT_HEADING2;
			tagID = TT_H2;
			tagPending = true;
			bClassAsTag = true;

			if (m_toc->docHasTOC()) {
				m_utf8_1 = UT_UTF8String_sprintf("h2 id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			} else {
				m_utf8_1 = "h2";
			}
		}
		else if (tree->descends ("Heading 3"))
		{
			m_iBlockType = BT_HEADING3;
			tagID = TT_H3;
			tagPending = true;
			bClassAsTag = true;

			if (m_toc->docHasTOC()) {
				m_utf8_1 = UT_UTF8String_sprintf("h3 id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			} else {
				m_utf8_1 = "h3";
			}
		}
		else if (tree->descends ("Block Text"))
		{
			m_iBlockType = BT_BLOCKTEXT;
			tagID = TT_BLOCKQUOTE;
			tagPending = true;
			bClassAsTag = true;
			m_utf8_1 = "blockquote";
		}
		else if (tree->descends ("Plain Text"))
		{
			m_iBlockType = BT_NORMAL;
			tagID = TT_P;
			tagPending = true;
			bClassAsTag = true;
			m_utf8_1 = "p class=\"plain_text\"";
		}
		else if (tree->descends ("Normal"))
		{
			m_iBlockType = BT_NORMAL;
			tagID = TT_P;
			tagPending = true;

			if (m_toc->docHasTOC() && m_toc->isTOCStyle(szValue)) {
				m_utf8_1 = UT_UTF8String_sprintf("p id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			} else {
				m_utf8_1 = "p";
			}
		}
		else
		{
			m_iBlockType = BT_NORMAL;
			tagID = TT_P;
			tagPending = true;

			if (m_toc->docHasTOC() && m_toc->isTOCStyle(szValue)) {
				m_utf8_1 = UT_UTF8String_sprintf("p id=\"AbiTOC%d__\"", m_heading_count);
				m_heading_count++;
			} else {
				m_utf8_1 = "p";
			}
		}

		if (tree && !bClassAsTag)
			if (tree->class_list().byteLength ())
			{
				UT_UTF8String escape;
				m_utf8_1 += " class=\"";
				if(get_Class_Only())
				{
					escape = tree->class_name ();
					m_utf8_1 += escape.escapeXML();
				}
				else
				{
					escape = tree->class_list ();
					m_utf8_1 += escape.escapeXML();
				}
				m_utf8_1 += "\"";
			}
		if (bAddAWMLStyle)
		{
			UT_UTF8String escape = szValue;
			m_utf8_1 += " awml:style=\"";
			m_utf8_1 += escape.escapeXML();
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

	const gchar * szP_DomDir = 0;
	pAP->getProperty ("dom-dir", szP_DomDir);

	if (szP_DomDir) // any reason why this can't be used with
					// <blockquote> or <pre> ?? no
	{
		m_utf8_1 += " dir=\"";
		m_utf8_1 += szP_DomDir;
		m_utf8_1 += "\"";
	}
	
	if(get_Class_Only())
		goto class_only;
	
	{
		const gchar * szP_TextAlign = 0;
		const gchar * szP_MarginBottom = 0;
		const gchar * szP_MarginTop = 0;
		const gchar * szP_MarginLeft = 0;
		const gchar * szP_MarginRight = 0;
		const gchar * szP_TextIndent = 0;

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
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "text-align:";
				m_utf8_1 += szP_TextAlign;
				first = false;
			}
			if (szP_MarginBottom)
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "margin-bottom:";
				m_utf8_1 += szP_MarginBottom;
				first = false;
			}
			if (szP_MarginTop)
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "margin-top:";
				m_utf8_1 += szP_MarginTop;
				first = false;
			}
			if (szP_MarginRight)
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "margin-right:";
				m_utf8_1 += szP_MarginRight;
				first = false;
			}
			if (szP_MarginLeft)
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "margin-left:";
				m_utf8_1 += szP_MarginLeft;
				first = false;
			}
			if (szP_TextIndent)
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "text-indent:";
				m_utf8_1 += szP_TextIndent;
				first = false;
			}

			m_utf8_1 += "\"";
		}
	}
	
 class_only:	
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
			tagOpenClose (m_utf8_1, get_HTML4 (), ws_None);
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
	if (m_bFirstWrite) _openTag (api, 0);

	if (!m_bInBlock) return;

	m_StyleTreeInline = 0;

	// make sure any unit conversions are correct
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);
	
	if (m_bInSpan && m_apiLastSpan == api)
		return;
	
	if (!bHaveProp || (pAP == 0))
	{
		if (m_bInSpan) _closeSpan ();
			return;
	}

	const gchar * szA_Style = 0;

	bool have_style = pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, szA_Style);
	if (have_style)
		if (m_utf8_style == szA_Style) // inline style is block style; ignore
			have_style = false;

	const s_StyleTree * tree = 0;
	if (have_style)
		tree = m_style_tree->find (szA_Style);

	m_StyleTreeInline = tree;

	bool first = true;
	bool bInSpan = false;
	
	if (tree)
		if (tree->class_list().byteLength ())
		{
			UT_UTF8String escape;
			m_utf8_1 = "span class=\"";
			if(get_Class_Only())
			{
				escape = tree->class_name ();
				m_utf8_1 += escape.escapeXML();
			}
			else
			{
				escape = tree->class_list ();
				m_utf8_1 += escape.escapeXML();
			}
			
			m_utf8_1 += "\"";
			bInSpan = true;
			first = false;
			if(get_Class_Only())
				goto class_only;
		}
	
	{
		const gchar * szP_FontWeight = 0;
		const gchar * szP_FontStyle = 0;
		const gchar * szP_FontSize = 0;
		const gchar * szP_FontFamily = 0;
		const gchar * szP_TextDecoration = 0;
		const gchar * szP_TextPosition = 0;
		const gchar * szP_TextTransform = 0;
		const gchar * szP_Color = 0;
		const gchar * szP_BgColor = 0;
		const gchar * szP_Display = 0;

		pAP->getProperty ("font-weight",     szP_FontWeight);
		pAP->getProperty ("font-style",      szP_FontStyle);
		pAP->getProperty ("font-size",       szP_FontSize);
		pAP->getProperty ("font-family",     szP_FontFamily);
		pAP->getProperty ("text-decoration", szP_TextDecoration);
		pAP->getProperty ("text-position",   szP_TextPosition);
		pAP->getProperty ("text-transform",  szP_TextTransform);
		pAP->getProperty ("color",           szP_Color);
		pAP->getProperty ("bgcolor",         szP_BgColor);
		pAP->getProperty ("display",         szP_Display);

		if(first)
			m_utf8_1 = "span style=\"";
		else
			m_utf8_1 += " style=\"";	

		/* TODO: this bold/italic check needs re-thought
		 */
		if (szP_FontWeight)
			if (strcmp (szP_FontWeight, "bold") == 0)
				if (!compareStyle ("font-weight", "bold"))
				{
					if (!first) m_utf8_1 += ";";
					m_utf8_1 += "font-weight:bold";
					first = false;
				}
		if (szP_FontStyle)
			if (strcmp (szP_FontStyle, "italic") == 0)
				if (!compareStyle ("font-style", "italic"))
				{
					if (!first) m_utf8_1 += ";";
					m_utf8_1 += "font-style:italic";
					first = false;
				}

		if (szP_FontSize)
		{
			char buf[16];

			{
				sprintf (buf, "%g", UT_convertToPoints (szP_FontSize));
			}

			m_utf8_0  = buf;
			m_utf8_0 += "pt";

			if (!compareStyle ("font-size", m_utf8_0.utf8_str ()))
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "font-size:";
				m_utf8_1 += m_utf8_0;
				first = false;
			}
		}
		if (szP_FontFamily)
		{
			if ((strcmp (szP_FontFamily, "serif")      == 0) ||
				(strcmp (szP_FontFamily, "sans-serif") == 0) ||
				(strcmp (szP_FontFamily, "cursive")    == 0) ||
				(strcmp (szP_FontFamily, "fantasy")    == 0) ||
				(strcmp (szP_FontFamily, "monospace")  == 0))
			{
				m_utf8_0  = static_cast<const char *>(szP_FontFamily);
			}
			else
			{
				m_utf8_0  = "'";
				m_utf8_0 += static_cast<const char *>(szP_FontFamily);
				m_utf8_0 += "'";
			}
			if (!compareStyle ("font-family", m_utf8_0.utf8_str ()))
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "font-family:";
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
					if (!first) m_utf8_1 += ";";
					m_utf8_1 += "text-decoration:";
					m_utf8_1 += m_utf8_0;
					first = false;
				}
			}
		}
		if (szP_TextTransform)
			{
				if (!compareStyle ("text-transform", szP_TextTransform))
					{
						if (!first) m_utf8_1 += ";";
						m_utf8_1 += "text-transform:";
						m_utf8_1 += szP_TextTransform;
						first = false;
					}
			}

		if (szP_TextPosition)
		{
			if (strcmp (szP_TextPosition, "superscript") == 0)
			{
				if (!compareStyle ("vertical-align", "super"))
				{
					if (!first) m_utf8_1 += ";";
					m_utf8_1 += "vertical-align:super";
					first = false;
				}
			}
			else if (strcmp (szP_TextPosition, "subscript") == 0)
			{
				if (!compareStyle ("vertical-align", "sub"))
				{
					if (!first) m_utf8_1 += ";";
					m_utf8_1 += "vertical-align:sub";
					first = false;
				}
			}
		}
		if (szP_Color && *szP_Color)
			if (!IS_TRANSPARENT_COLOR (szP_Color))
			{
				m_utf8_0 = UT_colorToHex(szP_Color, true);

				if (!compareStyle ("color", m_utf8_0.utf8_str ()))
				{
					if (!first) m_utf8_1 += ";";
					m_utf8_1 += "color:";
					m_utf8_1 += m_utf8_0;
					first = false;
				}
			}
		if (szP_BgColor && *szP_BgColor)
			if (!IS_TRANSPARENT_COLOR (szP_BgColor))
			{
				m_utf8_0 = UT_colorToHex(szP_BgColor, true);

				if (!compareStyle ("background", m_utf8_0.utf8_str ()))
				{
					if (!first) m_utf8_1 += ";";
					m_utf8_1 += "background:";
					m_utf8_1 += m_utf8_0;
					first = false;
				}
			}

		if (szP_Display)
		{
			if (strcmp (szP_Display, "none") == 0)
			{
				if (!first) m_utf8_1 += ";";
				m_utf8_1 += "display:none";
				first = false;
			}
		}
	}
	
 class_only:
	
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

	const gchar * szP_Lang = 0;
	pAP->getProperty ("lang",         szP_Lang);
	
	if (szP_Lang)
		{
			if (!get_HTML4 ()) {
				// we want to emit xml:lang in addition to lang
				m_utf8_1 += " xml:lang=\"";
				m_utf8_1 += szP_Lang;
				m_utf8_1 += "\"";
			}

			m_utf8_1 += " lang=\"";
			m_utf8_1 += szP_Lang;
			m_utf8_1 += "\"";
			bInSpan = true;
		}
	
	if (bInSpan)
	{
		if (m_bInSpan)
		{
			_closeSpan ();
		}
		
		m_utf8_span = m_utf8_1;

		tagOpen (TT_SPAN, m_utf8_span, ws_None);

		/* if the dir-override is set, or dir is 'rtl' or 'ltr', we will output
		 * the dir property; however, this property cannot be within a style 
		 * sheet, so anything that needs to be added to this code and belongs 
		 * within a style property must be above us; further it should be noted 
		 * that there is a good chance that the html browser will not handle it 
		 * correctly. For instance IE will take dir=rtl as an indication that 
		 * the span should have rtl placement on a line, but it will ignore this 
		 * value when printing the actual span.
		 */
		const gchar * szP_DirOverride = 0;
		
		pAP->getProperty ("dir-override", szP_DirOverride);
		
		if (szP_DirOverride)
			if (/* (*szP_DirOverride == 'l') || */(*szP_DirOverride == 'r'))
				{
					m_utf8_1 = "bdo dir=\"";
					m_utf8_1 += szP_DirOverride;
					m_utf8_1 += "\"";

					tagOpen (TT_BDO, m_utf8_1, ws_None);
				}
		m_apiLastSpan = api;
		m_bInSpan = true;
	}
	else if (m_bInSpan) _closeSpan ();
}

void s_HTML_Listener::_closeSpan ()
{
	if (tagTop () == TT_A)
	{
		tagClose (TT_A, "a", ws_None);
	}
	if (tagTop () == TT_BDO)
	{
		tagClose (TT_BDO, "bdo", ws_None);
	}
	if (tagTop () == TT_SPAN)
	{
		tagClose (TT_SPAN, "span", ws_None);
	}
	m_bInSpan = false;
}

/*! 	Close up all HTML-structures for which we haven't a definitive end in the piecetable,
 *   such as lists.
 *		\todo Somebody needs to check for others aside from lists, in order to preempt ugly bugs.
 */
void s_HTML_Listener::_popUnendedStructures (void)
{
	if(m_iListDepth)
	  listPopToDepth(0);
}

#ifdef HTML_TABLES_SUPPORTED

void s_HTML_Listener::_fillColWidthsVector(void)
{
	// make sure any unit conversions are correct
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	//
	// Positioned columns controls
	//
	const char * pszColumnProps = m_TableHelper.getTableProp("table-column-props");
	UT_DEBUGMSG(("Number columns in table %d \n",m_TableHelper.getNumCols ()));
	if(m_vecDWidths.getItemCount() > 0)
	{
		UT_VECTOR_PURGEALL(double *,m_vecDWidths);
		m_vecDWidths.clear();
	}
	if(pszColumnProps && *pszColumnProps)
	{
		/*
		  These will be properties applied to all columns. To start with, just the 
		  widths of each column are specifed. These are translated to layout units.
 
		  The format of the string of properties is:

		  table-column-props:1.2in/3.0in/1.3in/;

		  So we read back in pszColumnProps
		  1.2in/3.0in/1.3in/

		  The "/" characters will be used to delineate different column entries.
		  As new properties for each column are defined these will be delineated with "_"
		  characters. But we'll cross that bridge later.
		*/
		UT_DEBUGMSG(("table-column-props:%s \n",pszColumnProps));
		UT_String sProps = pszColumnProps;
		UT_sint32 sizes = sProps.size();
		UT_sint32 i =0;
		UT_sint32 j =0;
		while(i < sizes)
		{
			for (j=i; (j<sizes) && (sProps[j] != '/') ; j++) {}
			if(sProps[j] == 0)
			{
				// reached the end of the props string without finding
				// any further sizes
				break;
			}
			
			if((j+1)>i && sProps[j] == '/')
			{
				UT_String sSub = sProps.substr(i,(j-i));
				i = j + 1;
				double * pDWidth = new double;
				*pDWidth = UT_convertToInches(sSub.c_str());
				m_vecDWidths.addItem(pDWidth);
			}
		}
	}
	//
	// automatic column widths set to total width divided by nCols
	//
	else
	{
		// double total = m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;
		UT_sint32 nCols = m_TableHelper.getNumCols ();
		double totWidth = m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;
		double colWidth = totWidth/nCols;
		UT_sint32 i = 0;
		for(i =0; i< nCols; i++)
		{
			double * pDWidth = new double;
			*pDWidth = colWidth;
			m_vecDWidths.addItem(pDWidth);
		}
	}
}


void s_HTML_Listener::_openTable (PT_AttrPropIndex api)
{
	// make sure any unit conversions are correct
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if (m_bFirstWrite) _openSection (api, 0);

	if (!m_bInSection) return;

	if(m_iListDepth)
	     listPopToDepth(0); // AbiWord does not support tables in LIs, neither do we.  For AbiWord, an LI is a special <p>.  See next line.

	if (m_bInBlock) _closeTag (); // HTML does not make it any more desirable to embed a table in a <p> than AbiWord.

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api,&pAP);

	if (!bHaveProp || (pAP == 0)) return;

	//UT_sint32 cellPadding = 0;
	UT_UTF8String styles;

	const char * prop = m_TableHelper.getTableProp ("table-line-thickness");

	UT_sint32 border = 0;

	if(prop && atof(prop) != 0.0)
		border = 1;

	UT_UTF8String border_default = "1pt";
	if (prop)
	{
		double dPT = UT_convertToDimension(prop, DIM_PT);
		border_default = UT_UTF8String_sprintf("%.2fpt", dPT);
	}

#if 0
	const gchar * pszLeftOffset = 0;
	const gchar * pszTopOffset = 0;
	const gchar * pszRightOffset = 0;
	const gchar * pszBottomOffset = 0;

	pSectionAP->getProperty ("cell-margin-left",   pszLeftOffset);
	pSectionAP->getProperty ("cell-margin-top",    pszTopOffset);
	pSectionAP->getProperty ("cell-margin-right",  pszRightOffset);
	pSectionAP->getProperty ("cell-margin-bottom", pszBottomOffset);
#endif
	const char * pszWidth = m_TableHelper.getTableProp ("width");
	if (get_Abs_Units()) {
		if (pszWidth) {
			if (styles.byteLength ()) styles += ";";
			styles += "width:";
			// use mm (inches are too big, since we want to use an int).
			double dMM = UT_convertToDimension(pszWidth, DIM_MM);
			UT_UTF8String t2;
			UT_UTF8String_sprintf(t2, "%.1fmm", dMM);
			styles += t2;
		}
	} else if (get_Scale_Units()) {
		// TEST ME!
		if (pszWidth) {
			if (styles.byteLength ()) styles += ";";
			styles += "width:";
			double tMM = UT_convertToDimension(pszWidth, DIM_MM);
			double totWidth = m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;
			UT_UTF8String tws = UT_UTF8String_sprintf("%d", totWidth);
			double pMM = UT_convertToDimension(tws.utf8_str(), DIM_MM);
			double dPCT = tMM / pMM;
			UT_UTF8String t2;
			UT_UTF8String_sprintf(t2, "%d%%", dPCT);
			styles += t2;
		}
	} 
	else {
		// this should match abi because tables always cover width by default
		if (styles.byteLength ()) styles += ";";
		styles += "width:100%";
	}

	const char * pszBgColor = m_TableHelper.getTableProp ("bgcolor");
	if (pszBgColor == NULL)
		pszBgColor = m_TableHelper.getTableProp ("background-color");
	if (pszBgColor)
	{
		if (styles.byteLength ()) styles += ";";
		styles += "background-color:";

		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBgColor);
		if (hash)
			styles += hash;
		else
			styles += pszBgColor;
	}

	const char * pszBorderColor = NULL;

	pszBorderColor = m_TableHelper.getTableProp ("color");
	if (pszBorderColor)
	{
		if (styles.byteLength ()) styles += ";";
		styles += "color:";

		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			styles += hash;
		else
			styles += pszBorderColor;
	}

	// more often than not border attributes are same all around, so
	// we want to use the border shortcut
	// 0-L, 1-R, 2-T, 3-B
	double dB[4] = {0.0,0.0,0.0,0.0};
	UT_UTF8String sB[4];
	UT_UTF8String sC[4];
	UT_UTF8String sS[4];
	
	pszBorderColor = m_TableHelper.getTableProp ("bot-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[3]= hash;
		else
			sC[3]= pszBorderColor;
	}
	pszBorderColor = m_TableHelper.getTableProp ("left-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[0] = hash;
		else
			sC[0] = pszBorderColor;
	}
	pszBorderColor = m_TableHelper.getTableProp ("right-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[1] = hash;
		else
			sC[1] = pszBorderColor;
	}
	pszBorderColor = m_TableHelper.getTableProp ("top-color");
	if (pszBorderColor)
	{
		UT_HashColor color;
		const char * hash = color.setHashIfValid (pszBorderColor);
		if (hash)
			sC[2] = hash;
		else
			sC[2] = pszBorderColor;
	}

	const char * pszBorderStyle = NULL;

	pszBorderStyle = m_TableHelper.getTableProp ("bot-style");
	if (pszBorderStyle)
	{
		sS[3]= PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}
	pszBorderStyle = m_TableHelper.getTableProp ("left-style");
	if (pszBorderStyle)
	{
		sS[0] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}
	pszBorderStyle = m_TableHelper.getTableProp ("right-style");
	if (pszBorderStyle)
	{
		sS[1] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}
	pszBorderStyle = m_TableHelper.getTableProp ("top-style");
	if (pszBorderStyle)
	{
		sS[2] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
	}

	const char * pszBorderWidth = NULL;

	pszBorderWidth = m_TableHelper.getTableProp ("bot-thickness");
	if (pszBorderWidth)
	{
		dB[3] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[3] = UT_UTF8String_sprintf("%.2fpt", dB[3]);
	}
	else
		sB[3] += border_default;
	pszBorderWidth = m_TableHelper.getTableProp ("left-thickness");
	if (pszBorderWidth)
	{
		dB[0] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[0] = UT_UTF8String_sprintf("%.2fpt", dB[0]);
	}
	else
		sB[0] = border_default;
	pszBorderWidth = m_TableHelper.getTableProp ("right-thickness");
	if (pszBorderWidth)
	{
		dB[1] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[1] = UT_UTF8String_sprintf("%.2fpt", dB[1]);
	}
	else
		sB[1] = border_default;
	pszBorderWidth = m_TableHelper.getTableProp ("top-thickness");
	if (pszBorderWidth)
	{
		dB[2] = UT_convertToDimension(pszBorderWidth, DIM_PT);
		sB[2] = UT_UTF8String_sprintf("%.2fpt", dB[2]);
	}
	else
		sB[2] += border_default;

	// now we need to decide which attributes are to be used in the
	// shortcut
	UT_uint32 iBCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iCCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iSCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
	UT_uint32 iBMaxIndx = 0, iCMaxIndx = 0, iSMaxIndx = 0;
	UT_uint32 i = 0;
	
	for(i = 0; i < 4; ++i)
	{
		for(UT_sint32 j = i+1; j < 4; j++)
		{
			if(dB[i] == dB[j])
			{
				iBCount[i]++;
				iBCount[j]++;
			}
		}
	}

	for(i = 1; i < 4; i++)
	{
		if(iBMaxIndx < iBCount[i])
			iBMaxIndx = i;
	}

	for(i = 0; i < 4; ++i)
	{
		for(UT_sint32 j = i+1; j < 4; j++)
		{
			if(sC[i] == sC[j])
			{
				iCCount[i]++;
				iCCount[j]++;
			}
		}
	}

	for(i = 1; i < 4; i++)
	{
		if(iCMaxIndx < iCCount[i])
			iCMaxIndx = i;
	}

	for(i = 0; i < 4; ++i)
	{
		for(UT_sint32 j = i+1; j < 4; j++)
		{
			if(sS[i] == sS[j])
			{
				iSCount[i]++;
				iSCount[j]++;
			}
		}
	}

	for(i = 1; i < 4; i++)
	{
		if(iSMaxIndx < iSCount[i])
			iSMaxIndx = i;
	}
	
	if(styles.size() != 0) styles += ";";
	
	styles += "border:";
	styles += sB[iBMaxIndx];

	if(sS[iSMaxIndx].size())
	{
		styles += " ";
		styles += sS[iSMaxIndx];
	}
	

	if(sC[iCMaxIndx].size())
	{
		styles += " ";
		styles += sC[iCMaxIndx];
	}

	if(styles.size() != 0) styles += ";";
	styles += "border-collapse:collapse;empty-cells:show;table-layout:fixed";
	// only add the border style if we didn't already add it in the "border shortcut"
	if (!sS[iSMaxIndx].size()) styles += ";border-style:solid";
	
	if(iBCount[iBMaxIndx] != 3)
	{
		for(i = 0; i < 4; ++i)
		{
			if(i == iBMaxIndx || dB[i] == dB[iBMaxIndx] || sB[i].size() == 0)
				continue;

			switch(i)
			{
				case 0: styles += "border-left-width:"; break;
				case 1: styles += "border-right-width:";  break;
				case 2: styles += "border-top-width:";  break;
				case 3: styles += "border-bottom-width:";  break;
			}

			styles += sB[i];
			styles += ";";
		}
	}
	
	if(iSCount[iSMaxIndx] != 3)
	{
		for(i = 0; i < 4; ++i)
		{
			if(i == iSMaxIndx || sS[i] == sS[iSMaxIndx] || sS[i].size() == 0)
				continue;

			switch(i)
			{
				case 0: styles += "border-left-style:"; break;
				case 1: styles += "border-right-style:"; break;
				case 2: styles += "border-top-style:"; break;
				case 3: styles += "border-bottom-style:"; break;
			}

			styles += sS[i];
			styles += ";";
		}
	}

	if(iCCount[iCMaxIndx] != 3)
	{
		for(i = 0; i < 4; ++i)
		{
			if(i == iCMaxIndx  || sC[i] == sC[iCMaxIndx] || sC[i].size() == 0)
				continue;

			switch(i)
			{
				case 0: styles += "border-left-color:"; break;
				case 1: styles += "border-right-color:"; break;
				case 2: styles += "border-top-color:"; break;
				case 3: styles += "border-bottom-color:"; break;
			}

			styles += sC[i];
			styles += ";";
		}
	}

	const char * p = styles.utf8_str();
	UT_UTF8String s;
	if(p[styles.byteLength()-1] == ';')
	{
		s.append(p, styles.byteLength()-1);
	}
	else
	{
		s = p;
	}
	
	//m_utf8_1  = "table cellpadding=\"";
	//m_utf8_1 += UT_UTF8String_sprintf ("%d\" border=\"%d", cellPadding, border);
	m_utf8_1 = UT_UTF8String_sprintf ("table cellpadding=\"0\" border=\"%d\" style=\"", border);
	m_utf8_1 += s;
	m_utf8_1 += "\"";


	UT_sint32 nCols = m_TableHelper.getNumCols ();
	double totWidth = m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;

	double colWidth = 100.0 / static_cast<double>(nCols);
	tagOpen (TT_TABLE, m_utf8_1);
	_fillColWidthsVector();
	i = 0;
	if(m_vecDWidths.getItemCount() > 0)
	{
		m_utf8_1 = "colgroup";
		tagOpen(TT_COLGROUP, m_utf8_1);

		for(UT_sint32 j = 0; (j< nCols) && (j<m_vecDWidths.getItemCount());j++)
		{
			double * pDWidth = m_vecDWidths.getNthItem(j);
			double percent = 100.0*(*pDWidth/totWidth);

			{
				/*m_utf8_1  = "colgroup";   // methinks zat colgaroup ist incoddect hier, this can be deleted when well tested below

				if(get_Abs_Units())
				{
					// colgroup width only allows pixels or relative
					// widths; we need to use style for absolute units
					double dMM = UT_convertInchesToDimension(*pDWidth, DIM_MM);
					m_utf8_1 += UT_UTF8String_sprintf (" span=\"%d\" style=\"width:%.1fmm\"", 1, dMM);
				}
				else if(get_Scale_Units())
				{
					UT_sint32 iPercent = (UT_sint32)(percent + 0.5);
					m_utf8_1 += UT_UTF8String_sprintf (" width=\"%d%%\" span=\"%d\"", iPercent,1);
				} // Else do nothing, viva la box model!
				*/
				m_utf8_1  = "col";

				if(get_Abs_Units())
				{
					// colgroup width only allows pixels or relative
					// widths; we need to use style for absolute units
					double dMM = UT_convertInchesToDimension(*pDWidth, DIM_MM);
					m_utf8_1 += UT_UTF8String_sprintf (" style=\"width:%.1fmm\"", 1, dMM);
				}
				else if(get_Scale_Units())
				{
					UT_sint32 iPercent = (UT_sint32)(percent + 0.5);
					m_utf8_1 += UT_UTF8String_sprintf (" width=\"%d%%\"", iPercent,1);
				} // Else do nothing, viva la box model!
								
				UT_DEBUGMSG(("Output width def %s \n",m_utf8_1.utf8_str()));
			}

			tagOpenClose (m_utf8_1, false);
			m_utf8_1.clear();		
		}

		m_utf8_1 = "colgroup";
		tagClose(TT_COLGROUP, m_utf8_1);
 	}
	else
	{
		tagOpen (TT_TABLE, m_utf8_1);

		{
			// colgroup correct here in a sense
			// TODO: distinction might be made for AbsUnits and sans width for default
			m_utf8_1  = "colgroup width=\"";
			UT_sint32 iPercent = (UT_sint32)(colWidth + 0.5);			
			m_utf8_1 += UT_UTF8String_sprintf ("%d%%\" span=\"%d", iPercent, nCols);
			m_utf8_1 += "\"";
		}

		tagOpenClose (m_utf8_1, false);
	}

	m_utf8_1 = "tbody style=\"border:inherit\"";
	tagOpen (TT_TBODY, m_utf8_1);
}

void s_HTML_Listener::_closeTable ()
{
	m_utf8_1 = "tbody";
	tagClose (TT_TBODY, m_utf8_1);

	m_utf8_1 = "table";
	tagClose (TT_TABLE, m_utf8_1);
	UT_VECTOR_PURGEALL(double *,m_vecDWidths);
	m_vecDWidths.clear();
	if(m_TableHelper.getNestDepth() > 0)
	{
		_fillColWidthsVector();
		_setCellWidthInches();
	}
}

void s_HTML_Listener::_setCellWidthInches(void)
{
	UT_sint32 left = m_TableHelper.getLeft ();
	UT_sint32 right = m_TableHelper.getRight ();
	double tot = 0;
	UT_sint32 i =0;

	UT_ASSERT_HARMLESS((UT_sint32)m_vecDWidths.size() >= (right-1));

	for(i=left; i<right; i++)
	{
		// probably covering up some sort of issue
		// but we assert above, so we'll notice it again
		if (i < (UT_sint32)m_vecDWidths.size ())
			tot += *(m_vecDWidths.getNthItem(i));
	}
	m_dCellWidthInches = tot;
}

void s_HTML_Listener::_openRow (PT_AttrPropIndex api)
{
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	
	if (tagTop () == TT_TR)
	{
		m_utf8_1 = "tr";
		tagClose (TT_TR, m_utf8_1);
	}
	if (tagTop () != TT_TBODY)
	{
		_openTable (api);
	}

	m_utf8_1 = "tr style=\"border:inherit";
	// Possible TODO: No relative because no height of table, right?
	// Can height of table be calculated?
	if(get_Abs_Units()) 
	{
		const PP_AttrProp * pAP = NULL;
		bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
		
		if (bHaveProp && pAP)
		{
			const char * pszValue;
			if(pAP->getProperty("height", pszValue))
			{
				double dMM = UT_convertToDimension(pszValue, DIM_MM);
				m_utf8_1 += UT_UTF8String_sprintf(";height:%.1fmm", dMM);
			}
			else
			{
				// we have a problem; need to set it to something,
				// otherwise empty rows disappear from view
				// ideally, we would want it set to the font size, but
				// I do not think we can ascertain it at this stage.
				m_utf8_1 += ";height:5mm";
			}
			
		}
	}

	m_utf8_1 += "\"";
	tagOpen (TT_TR, m_utf8_1);
}

void s_HTML_Listener::_openCell (PT_AttrPropIndex api)
{
	UT_LocaleTransactor t(LC_NUMERIC, "C");
	
	m_bCellHasData = false;
	
	if (m_bFirstWrite) _openSection (api, 0);

	if (!m_bInSection) return;

	if (m_TableHelper.getNestDepth () < 1) _openTable(api);

	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
 	_setCellWidthInches();
	if (bHaveProp && pAP)
	{
		double dColSpacePT = 0;
		double dRowSpacePT = 0;
		const gchar * pszTableColSpacing = m_TableHelper.getTableProp ("table-col-spacing");
		const gchar * pszTableRowSpacing = m_TableHelper.getTableProp ("table-row-spacing");

		if(pszTableColSpacing)
			dColSpacePT = UT_convertToDimension(pszTableColSpacing, DIM_PT);

		if(pszTableRowSpacing)
			dRowSpacePT = UT_convertToDimension(pszTableRowSpacing, DIM_PT);

		UT_UTF8String styles;

		if(dColSpacePT == dRowSpacePT)
		{
			styles += UT_UTF8String_sprintf("padding: %.2fpt", dColSpacePT);
		}
		else
		{
			styles += UT_UTF8String_sprintf("padding: %.2fpt %.2fpt", dRowSpacePT, dColSpacePT);
		}
		
		UT_sint32 rowspan = m_TableHelper.getBot ()   - m_TableHelper.getTop ();
		UT_sint32 colspan = m_TableHelper.getRight () - m_TableHelper.getLeft ();

		if (m_TableHelper.isNewRow()) // beginning of a new row
			_openRow (api);

		const char * pszBgColor = m_TableHelper.getCellProp ("bgcolor");
		if (pszBgColor == NULL)
			pszBgColor = m_TableHelper.getCellProp ("background-color");
		if (pszBgColor)
		{
			if (styles.byteLength ()) styles += ";";
			styles += "background-color:";

			UT_HashColor color;
			const char * hash = color.setHashIfValid (pszBgColor);
			if (hash)
				styles += hash;
			else
				styles += pszBgColor;
		}

		const char * pszBorderColor = NULL;

		pszBorderColor = m_TableHelper.getCellProp ("color");
		if (pszBorderColor)
		{
			if (styles.byteLength ()) styles += ";";
			styles += "color:";

			UT_HashColor color;
			const char * hash = color.setHashIfValid (pszBorderColor);
			if (hash)
				styles += hash;
			else
				styles += pszBorderColor;
		}

		// more often than not border attributes are same all around, so
		// we want to use the border shortcut
		// 0-L, 1-R, 2-T, 3-B
		double dB[4] = {0.0,0.0,0.0,0.0};
		UT_UTF8String sB[4];
		UT_UTF8String sC[4];
		UT_UTF8String sS[4];
		
		pszBorderColor = m_TableHelper.getCellProp ("bot-color");
		if (pszBorderColor)
		{
			UT_HashColor color;
			const char * hash = color.setHashIfValid (pszBorderColor);
			if (hash)
				sC[3] = hash;
			else
				sC[3] = pszBorderColor;
		}
		pszBorderColor = m_TableHelper.getCellProp ("left-color");
		if (pszBorderColor)
		{
			UT_HashColor color;
			const char * hash = color.setHashIfValid (pszBorderColor);
			if (hash)
				sC[0]= hash;
			else
				sC[0]= pszBorderColor;
		}
		pszBorderColor = m_TableHelper.getCellProp ("right-color");
		if (pszBorderColor)
		{
			UT_HashColor color;
			const char * hash = color.setHashIfValid (pszBorderColor);
			if (hash)
				sC[1] = hash;
			else
				sC[1] = pszBorderColor;
		}
		pszBorderColor = m_TableHelper.getCellProp ("top-color");
		if (pszBorderColor)
		{
			UT_HashColor color;
			const char * hash = color.setHashIfValid (pszBorderColor);
			if (hash)
				sC[2] = hash;
			else
				sC[2] = pszBorderColor;
		}

		const char * pszBorderStyle = NULL;

		pszBorderStyle = m_TableHelper.getCellProp ("bot-style");
		if (pszBorderStyle)
		{
			sS[3] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
		}
		pszBorderStyle = m_TableHelper.getCellProp ("left-style");
		if (pszBorderStyle)
		{
			sS[0]= PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
		}
		pszBorderStyle = m_TableHelper.getCellProp ("right-style");
		if (pszBorderStyle)
		{
			sS[1] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
		}
		pszBorderStyle = m_TableHelper.getCellProp ("top-style");
		if (pszBorderStyle)
		{
			sS[2] = PP_PropertyMap::linestyle_for_CSS (pszBorderStyle);
		}

		const char * pszBorderWidth = NULL;

		pszBorderWidth = m_TableHelper.getCellProp ("bot-thickness");
		if (pszBorderWidth)
		{
			dB[3] = UT_convertToDimension(pszBorderWidth, DIM_PT);
			sB[3] = UT_UTF8String_sprintf("%.2fpt", dB[3]);
		}
		pszBorderWidth = m_TableHelper.getCellProp ("left-thickness");
		if (pszBorderWidth)
		{
			dB[0] = UT_convertToDimension(pszBorderWidth, DIM_PT);
			sB[0] = UT_UTF8String_sprintf("%.2fpt", dB[0]);
		}
		pszBorderWidth = m_TableHelper.getCellProp ("right-thickness");
		if (pszBorderWidth)
		{
			dB[1] = UT_convertToDimension(pszBorderWidth, DIM_PT);
			sB[1] = UT_UTF8String_sprintf("%.2fpt", dB[1]);
		}
		pszBorderWidth = m_TableHelper.getCellProp ("top-thickness");
		if (pszBorderWidth)
		{
			dB[2] = UT_convertToDimension(pszBorderWidth, DIM_PT);
			sB[2] = UT_UTF8String_sprintf("%.2fpt", dB[2]);
		}

		// now we need to decide which attributes are to be used in the
		// shortcut
		UT_uint32 iBCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
		UT_uint32 iCCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
		UT_uint32 iSCount[4] = {0,0,0,0}; // 0 - L, 1 - R, 2 - T, 3 - B
		UT_uint32 iBMaxIndx = 0, iCMaxIndx = 0, iSMaxIndx = 0;
		UT_sint32 i = 0;
	
		for(i = 0; i < 4; ++i)
		{
			for(UT_sint32 j = i+1; j < 4; j++)
			{
				if(dB[i] == dB[j])
				{
					iBCount[i]++;
					iBCount[j]++;
				}
			}
		}

		for(i = 1; i < 4; i++)
		{
			if(iBMaxIndx < iBCount[i])
				iBMaxIndx = i;
		}

		for(i = 0; i < 4; ++i)
		{
			for(UT_sint32 j = i+1; j < 4; j++)
			{
				if(sC[i] == sC[j])
				{
					iCCount[i]++;
					iCCount[j]++;
				}
			}
		}

		for(i = 1; i < 4; i++)
		{
			if(iCMaxIndx < iCCount[i])
				iCMaxIndx = i;
		}

		for(i = 0; i < 4; ++i)
		{
			for(UT_sint32 j = i+1; j < 4; j++)
			{
				if(sS[i] == sS[j])
				{
					iSCount[i]++;
					iSCount[j]++;
				}
			}
		}

		for(i = 1; i < 4; i++)
		{
			if(iSMaxIndx < iSCount[i])
				iSMaxIndx = i;
		}
	
		if(styles.size() != 0) styles += ";";
	
		styles += "border:";

		if(sB[iBMaxIndx].size())
		{
			styles += sB[iBMaxIndx];
		}
		else
		{
			styles += "inherit";
		}
		
		styles += " ";

		if(sS[iSMaxIndx].size())
		{
			styles += sS[iSMaxIndx];
		}
		else
		{
			styles += "inherit";
		}
		
		styles += " ";

		if(sC[iCMaxIndx].size())
		{
			styles += sC[iCMaxIndx];
		}
		else
		{
			styles += "inherit";
		}
		
		if(styles.size() != 0) styles += ";";
		if(iBCount[iBMaxIndx] != 3)
		{
			for(i = 0; i < 4; ++i)
			{
				if((UT_uint32)i == iBMaxIndx || dB[i] == dB[iBMaxIndx])
					continue;

				switch(i)
				{
					case 0: styles += "border-left-width:"; break;
					case 1: styles += "border-right-width:"; break;
					case 2: styles += "border-top-width:"; break;
					case 3: styles += "border-bottom-width:"; break;
				}

				if(sB[i].size())
					styles += sB[i];
				else
					styles += "inherit";
				
				styles += ";";
			}
		}
	
		if(iSCount[iSMaxIndx] != 3)
		{
			for(i = 0; i < 4; ++i)
			{
				if((UT_uint32)i == iSMaxIndx || sS[i] == sS[iSMaxIndx])
					continue;

				switch(i)
				{
					case 0: styles += "border-left-style:"; break;
					case 1: styles += "border-right-style:"; break;
					case 2: styles += "border-top-style:"; break;
					case 3: styles += "border-bottom-style:"; break;
				}

				if(sS[i].size())
					styles += sS[i];
				else
					styles += "inherit";
				
				styles += ";";
			}
		}

		if(iCCount[iCMaxIndx] != 3)
		{
			for(i = 0; i < 4; ++i)
			{
				if((UT_uint32)i == iCMaxIndx  || sC[i] == sC[iCMaxIndx])
					continue;

				switch(i)
				{
					case 0: styles += "border-left-color:"; break;
					case 1: styles += "border-right-color:"; break;
					case 2: styles += "border-top-color:"; break;
					case 3: styles += "border-bottom-color:"; break;
				}

				if(sC[i].size())
					styles += sC[i];
				else
					styles += "inherit";
				
				styles += ";";
			}
		}

		const char * p = styles.utf8_str();
		UT_UTF8String s;
		if(p[styles.byteLength()-1] == ';')
		{
			s.append(p, styles.byteLength()-1);
		}
		else
		{
			s = p;
		}
		
		
		m_utf8_1 = "td";

		if (styles.byteLength ())
		{
			m_utf8_1 += " style=\"";
			m_utf8_1 += s;
			m_utf8_1 += "\"";
		}

		if (rowspan > 1)
		{
			m_utf8_1 += " rowspan=\"";
			m_utf8_1 += UT_UTF8String_sprintf ("%d", rowspan);
			m_utf8_1 += "\"";
		}
		if (colspan > 1)
		{
			m_utf8_1 += " colspan=\"";
			m_utf8_1 += UT_UTF8String_sprintf ("%d", colspan);
			m_utf8_1 += "\"";
		}
		tagOpen (TT_TD, m_utf8_1);
	}
}

void s_HTML_Listener::_closeCell ()
{
	if (m_TableHelper.getNestDepth () < 1) return;

	if(!m_bCellHasData)
	{
		// we need to insert a &nbsp; to make sure that the cell will
		// have its borders
		// this is not necessary; the same effect can be achieved by
		// setting "border-collapse:collapse;empty-cells:show"
   		UT_UTF8String s = " "; // This enables the table to be reimported in abi
		tagRaw(s);
	}
	
	/* The number of times _popUnendedStructures has to be used may be indicative of serious fundamental flows in the
	   current tag stack(s) implementation, an implementation which therefore is in dire need of an expert audit.
	   On the other hand, it may also be the necessary result of the already known shortcomings of our piecetable with
	   regard to the permission of unstruxed structures and unilateral struxes.  -MG */
	_popUnendedStructures();
	
	m_utf8_1 = "td";
	tagClose (TT_TD, m_utf8_1);
}

#endif /* HTML_TABLES_SUPPORTED */


void s_HTML_Listener::_openPosImage (PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	if (!bHaveProp || (pAP == 0)) return;

	const gchar * pszDataID = NULL;
	if(pAP->getAttribute(PT_STRUX_IMAGE_DATAID, (const gchar *&)pszDataID) && pszDataID)
		_handleImage(pAP,pszDataID,true);

}

void s_HTML_Listener::_openTextBox (PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
	if (!bHaveProp || (pAP == 0)) return;
	const gchar * tempProp = 0;
	
	if(m_bInTextBox)
	  _closeTextBox(); // Fortunately for the html exporter, abi does not permit nested frames.
	
	if(m_iListDepth)
	  listPopToDepth(0); // AbiWord does not support textboxes in LIs, neither do we.
	
	m_bInFrame = true;
	m_bInTextBox = true; // See comment by declaration
	/* --- Copied from closeSection --- */
	// TODO: Extract me into closePseudoSection.
	if (m_bInBlock)
		_closeTag (); // We need to investigate the tag stack usage of this, and whether or not we really would rather specify the tag in all cases.

	// Need to investigate whether we can safely uncomment this without undoing heading work, or any other kind using unended structures like lists.
	// _popUnendedStructures(); // Close lists, and possibly other stuff.  Even if it theoretically can span sections, we run a high risk of corrupting the document.

	if (m_bInSection && (tagTop () == TT_DIV))
	{
		m_utf8_1 = "div";
		tagClose (TT_DIV, m_utf8_1);
	}
	/* --- */
	m_utf8_1 = "div style=\""; // We represent the box with a div (block)
	
	// TODO: Enum frame properties (and in any case where props equal their css counterparts) separately
	// TODO: so here (and places like here) you can just iterate through it getting the prop and setting it.
	// TODO: Actually, you wouldn't have to limit it to where the props were identical, just have
	// TODO: { abiprop, cssprop }.  It would still require that the units used for both specs be compatible.
	//
	//	TODO: Take care of padding as well.
	const gchar * propNames[20] = {"bot-thickness","border-bottom-width",
									"top-thickness","border-top-width",
									"right-thickness","border-right-width",
									"left-thickness","border-left-width",
									"bot-color","border-bottom-color",
									"top-color","border-top-color",
									"right-color","border-right-color",
									"left-color","border-left-color",
									"background-color","background-color",
									NULL,NULL}; // [AbiWord property name, CSS21 property name]
	for(unsigned short int propIdx = 0; propIdx < 18; propIdx += 2)
	{
		if(pAP->getProperty(propNames[propIdx], tempProp))			// If we successfully retrieve a value (IOW, it's defined)
		{
			m_utf8_1 += propNames[propIdx + 1]; // Add the property name of the CSS equivalent
			m_utf8_1 += ": "; // Don't ask (:
			if(strstr(propNames[propIdx + 1], "color")) m_utf8_1 += "#"; // AbiWord tends to store colors as hex, which must be prefixed by # in CSS
			m_utf8_1 += tempProp; // Add the value
			m_utf8_1 += "; "; // Terminate the property
		}
	}
	
	//pAP->getProperty("bot-style", tempProp); // Get the bottom border style
	//<...>
	// We don't do this right now because we don't support multiple styles right now.
	// See bug 7935.  Until we support multiple styles, it is sufficient to set all solid.
	m_utf8_1 += " border: solid;";
	
	// This might need to be updated for textbox (and wrapped-image?) changes that
	// occured in 2.3. 

	// Get the wrap mode
	if(!pAP->getProperty("wrap-mode", tempProp) || !tempProp || !*tempProp)
		tempProp = "wrapped-both"; // this seems like a sane default

	if(!strcmp(tempProp, "wrapped-both"))
		m_utf8_1 += " clear: none;";
	else if(!strcmp(tempProp, "wrapped-left"))
		m_utf8_1 += " clear: right;";
	else if(!strcmp(tempProp, "wrapped-right"))
		m_utf8_1 += " clear: left;";
	else if(!strcmp(tempProp, "above-text"))
		m_utf8_1 += " clear: none; z-index: 999;";
	
	m_utf8_1 += "\"";
	
	tagOpen(TT_DIV, m_utf8_1);
	
	return;
}

void s_HTML_Listener::_closeTextBox ()
{
	// We don't need to close the block ourselves because _closeSection does it for us.

	// TODO: Extract me into closePseudoSection
	// We cannot use _closeSection(), we're not actually in a section.
	if (m_bInBlock)
		_closeTag (); // We need to investigate the tag stack usage of this, and whether or not we really would rather specify the tag in all cases.

	// Need to investigate whether we can safely uncomment this without undoing heading work, or any other kind using unended structures like lists.
	// _popUnendedStructures(); // Close lists, and possibly other stuff.  Even if it theoretically can span sections, we run a high risk of corrupting the document.

	if ((tagTop () == TT_DIV))
	{
		m_utf8_1 = "div";
		tagClose (TT_DIV, m_utf8_1);
	}
	else
	{
		UT_DEBUGMSG(("WARNING: Something gone awry with this textbox \n"));
	}

	// Fortunately for us, abi does not permit nested frames yet.
	m_bInFrame = false;
	m_bInTextBox = false;
}

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
				tagOpenClose (m_utf8_1, get_HTML4 (), ws_None);
				m_utf8_1 = "";
				break;

			case UCS_LDBLQUOTE:
				m_utf8_1 += "&ldquo;";
				m_bCellHasData = true;
				break;

			case UCS_RDBLQUOTE:
				m_utf8_1 += "&rdquo;";
				m_bCellHasData = true;
				break;

			case UCS_LQUOTE:
				m_utf8_1 += "&#145;";
				m_bCellHasData = true;
				break;

			case UCS_RQUOTE:
				m_utf8_1 += "&#146;";
				m_bCellHasData = true;
				break;

			case UCS_EN_DASH:
				m_utf8_1 += "&#150;";
				m_bCellHasData = true;
				break;

			case UCS_EM_DASH:
				m_utf8_1 += "&#151;";
				m_bCellHasData = true;
				break;

			default:
				if ((*ucs_ptr & 0x007f) == *ucs_ptr) // ASCII
				{
					m_bCellHasData = true;
					char c = static_cast<char>(*ucs_ptr & 0x007f);

					if (isspace (static_cast<int>(static_cast<unsigned char>(c))))
					{
						if (prev_space || (length == 1))
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
								  bool bTemplateBody, const XAP_Exp_HTMLOptions * exp_opt,
								  s_StyleTree * style_tree,
								  UT_UTF8String & linkCSS,
								  UT_UTF8String & title) :
	m_pDocument (pDocument),
		m_apiLastSpan(0),
		m_pie(pie),
		m_bClipBoard(bClipBoard),
		m_bTemplateBody(bTemplateBody),
		m_exp_opt(exp_opt),
		m_style_tree(style_tree),
		m_bInSection(false),
		m_bInFrame(false),
		m_bInTextBox(false),
		m_bInTOC(false),
		m_bInBlock(false),
		m_bInSpan(false),
		m_bNextIsSpace(false),
		m_bWroteText(false),
		m_bFirstWrite(true),
		m_bQuotedPrintable(false),
		m_bHaveHeader(false),
		m_bHaveFooter(false),
#ifdef HTML_TABLES_SUPPORTED
		m_TableHelper(pDocument),
#endif /* HTML_TABLES_SUPPORTED */
		m_iBlockType(0),
		m_iListDepth(0),
		m_iImgCnt(0),
		m_StyleTreeInline(0),
		m_StyleTreeBlock(0),
		m_StyleTreeBody(0),
		m_pAPStyles(0),
		m_styleIndent(0),
		m_fdCSS(0),
		m_bIgnoreTillEnd(false),
	    m_bIgnoreTillNextSection(false),
		m_iEmbedStartPos(0),
		m_dPageWidthInches(0.0),
		m_dSecLeftMarginInches(0.0),
		m_dSecRightMarginInches(0.0),
		m_dCellWidthInches(0.0),
		m_sLinkCSS(linkCSS),
		m_sTitle(title),
		m_iOutputLen(0),
		m_bCellHasData(true),  // we are not in cell to start with, set to true
		m_toc(0),
		m_heading_count(0)
{
	m_toc = new IE_TOCHelper (m_pDocument);
	m_StyleTreeBody = m_style_tree->find ("Normal");
}

s_HTML_Listener::~s_HTML_Listener()
{
	UT_DEBUGMSG(("deleteing lisnter %p \n",this));
	_closeTag ();

	listPopToDepth (0);

	_closeSection ();

	_outputEnd ();
	
	UT_VECTOR_PURGEALL(double *,m_vecDWidths);
	DELETEP(m_toc);
}


void s_HTML_Listener::_writeImageBase64 (const UT_ByteBuf * pByteBuf)
{
	char buffer[75];
	char * bufptr = 0;
	size_t buflen;
	size_t imglen = pByteBuf->getLength ();
	const char * imgptr = reinterpret_cast<const char *>(pByteBuf->getPointer (0));

	buffer[0] = '\r';
	buffer[1] = '\n';

	while (imglen)
	{
		buflen = 72;
		bufptr = buffer + 2;

		UT_UTF8_Base64Encode (bufptr, buflen, imgptr, imglen);

		*bufptr = 0;

		m_utf8_1 = buffer;
		textTrusted (m_utf8_1);
	}
}

bool s_HTML_Listener::_getPropertySize(const PP_AttrProp * pAP, const gchar* szWidthProp, const gchar* szHeightProp, 
                                       const gchar** szWidth, double& widthPercentage, const gchar** szHeight)
{
	UT_return_val_if_fail(pAP, false);
	UT_return_val_if_fail(szWidth, false)
	UT_return_val_if_fail(szHeight, false)

	// get the object width as displayed in AbiWord
	*szWidth = NULL;
	pAP->getProperty (szWidthProp, *szWidth);
	
	// get the object height as displayed in AbiWord
	*szHeight = NULL;
	pAP->getProperty (szHeightProp, *szHeight);
	
	// determine the total width of this object, so we can calculate the object's
	// width as a percentage of that
	widthPercentage = 100;
	if (*szWidth)
	{
		double total = 0;
		if(m_TableHelper.getNestDepth() > 0)
		{
			total = m_dCellWidthInches;
		}
		else
		{
			total =  m_dPageWidthInches - m_dSecLeftMarginInches - m_dSecRightMarginInches;
		}

		double dWidth = UT_convertToInches(*szWidth);
		widthPercentage = 100.0 * dWidth / total;
		if (widthPercentage > 100.)
			widthPercentage = 100.0;
	}
		
	return true;
}

UT_UTF8String s_HTML_Listener::_getStyleSizeString(const gchar * szWidth, double widthPercentage, UT_Dimension widthDim, 
                                             const gchar * szHeight, UT_Dimension heightDim)
{
	UT_UTF8String props;
	
	if (szWidth)
	{
		props += "width:";
		if (get_Scale_Units())
		{
			UT_sint32 iPercent = (UT_sint32)(widthPercentage + 0.5);
			props += UT_UTF8String_sprintf("%d%%", iPercent);
		}
		else
		{
			double d = UT_convertToDimension(szWidth, widthDim);
			props += UT_formatDimensionString(widthDim, d);
		}
	}

	if (szHeight)
	{
		if (props.size() > 0)
			props += "; ";
		props += "height:";
		double d = UT_convertToDimension(szHeight, heightDim);
		props += UT_formatDimensionString(heightDim , d);
	}	

	if (props.size() > 0)
		return "style=\"" + props + "\"";

	return "";
}

void s_HTML_Listener::_handleEmbedded (PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	if (!bHaveProp || (pAP == 0))
		return;

	const gchar * szDataID = 0;
	pAP->getAttribute ("dataid", szDataID);

	if (szDataID == 0)
		return;

    std::string mimeType;
	const UT_ByteBuf * pByteBuf = 0;
	if (!m_pDocument->getDataItemDataByName(szDataID, &pByteBuf, &mimeType, NULL))
	{
		return;
    }
	if ((pByteBuf == 0) || mimeType.empty())
	{
        return; // ??
    }

	_handleEmbedded (pAP, szDataID, pByteBuf, mimeType);
}

void s_HTML_Listener::_handleEmbedded (const PP_AttrProp * pAP, const gchar * szDataID, const UT_ByteBuf* pByteBuf, const std::string mimeType)
{
	// Code to export the embedded object as an <object>
	// with an <img> inside it, as a rendering fallback
	// http://www.w3.org/TR/1999/REC-html401-19991224/struct/objects.html#h-13.3

	UT_LocaleTransactor t(LC_NUMERIC, "C");
	
	const char * dataid = UT_basename (static_cast<const char *>(szDataID));
		
	const char * suffix = dataid + strlen (dataid);
	const char * suffid = suffix;
	const char * ptr = 0;
		
	/* Question: What does the DataID look like for objects pasted
	 *           from the clipboard?
	 */
	ptr = suffix;
	while (ptr > dataid)
	{
		if (*--ptr == '_')
			{
				suffix = ptr;
				suffid = suffix;
				break;
			}
	}
	ptr = suffix;
	while (ptr > dataid)
	{
		if (*--ptr == '.')
		{
			suffix = ptr;
			// break;
		}
	}
	if (dataid == suffix)
		return;

	char * base_name = NULL;
	if (m_pie->getFileName ())
		base_name = UT_go_basename_from_uri (m_pie->getFileName ());
		
	/* hmm; who knows what locale the system uses
	 */
	UT_UTF8String objectbasedir = "clipboard";
	if (base_name)
		objectbasedir = base_name;
	objectbasedir += "_files";
	std::string objectdir = m_pie->getFileName () ? m_pie->getFileName () : "";
	objectdir += "_files";
		
	UT_UTF8String filename(dataid,suffix-dataid);
	filename += suffid;
	filename += (mimeType == "image/svg+xml" ? ".svg" : ".obj");

	FREEP(base_name);
		
	UT_UTF8String url;
		
	url += s_string_to_url (objectbasedir);
	url += "/";
	url += s_string_to_url (filename);
		
	if (get_Multipart ())
	{
		UT_UTF8String * save_url = new UT_UTF8String(url);
		if (save_url == 0)
			return;
			
		if (!m_SavedURLs.insert (szDataID, save_url)) // arg. failed. skip object
		{
			DELETEP(save_url);
			return;
		}
	}

	/* szDataID is the raw string with the data ID
	 * objectdir is the name of the directory in which we'll write the object
	 * filename is the name of the file to which we'll write the object
	 * url      is the URL which we'll use
	 */
	if (!get_Embed_Images () && !get_Multipart ())
	{
		IE_Exp::writeBufferToFile(pByteBuf, objectdir, filename.utf8_str());
	}
		
	m_utf8_1 = "object";

	// determine the width and height of the object
	// NOTE: properly scaling SVG objects is not possible at the moment. See
	// http://bugzilla.abisource.com/show_bug.cgi?id=12152#c4 on why this is.
	const gchar * szWidth = 0;
	const gchar * szHeight = 0;
	double widthPercentage;
	if (!_getPropertySize(pAP, "width", "height", &szWidth, widthPercentage, &szHeight))
		return;
	UT_DEBUGMSG(("Size of object: %sx%s\n", szWidth ? szWidth : "(null)", szHeight ? szHeight : "(null)"));
	m_utf8_1 += " " + _getStyleSizeString(szWidth, widthPercentage, DIM_MM, szHeight, DIM_MM);

	// objects have a mimetype describing what their content is supposed to be
	m_utf8_1 += UT_UTF8String_sprintf(" type=\"%s\"", mimeType.c_str());

	m_tagStack.push (TT_OBJECT);
	if (!get_Embed_Images () || get_Multipart ())
	{
		m_utf8_1 += " data=\"";
		m_utf8_1 += url;
		m_utf8_1 += "\"";

		tagOpenBroken (m_utf8_1, ws_None);

		m_utf8_1 = "";
		tagCloseBroken (m_utf8_1, true, ws_None);
	}
	else
	{
		m_utf8_1 += UT_UTF8String_sprintf(" data=\"data:%s;base64,", mimeType.c_str());
		tagOpenBroken (m_utf8_1, ws_None);

		_writeImageBase64 (pByteBuf);
		
		m_utf8_1 = "\"";
		tagCloseBroken (m_utf8_1, true, ws_None);
	}

	// Embed an image fallback for this object, if the browser can't handle
	// the object.
	if (mimeType == "image/svg+xml")
	{	
		// Puting SVG data in an <object> tag with an embedded <embed> tag
		// seems to be the most portable way to place an SVG in an (X)HTML document,
		// at least according to http://wiki.svg.org/SVG_and_HTML.
		
		// However, the deprecated <embed> would be only be useful for support
		// ancient browsers like NS4, and it wouldn't be valid XHTML either.
		// Therefor, we'll just leave the <embed> tag out.

		// It would however be neat if we would rasterize the SVG and put an <img> 
		// inside this <object> tag as a fallback for browsers lacking SVG support. 
		// We do this for other embedded objects (see the "else" block below), 
		// but AbiWord does not currently have a PNG snapshot available for SVGs.
		// Since it is currently non-trivial to make one in a crossplatform manner,
		// we'll just leave it for now.
	}
	else
	{
		// embed an <img> version of the object, as a rendering fallback
		UT_UTF8String imgDataID("snapshot-png-");
		imgDataID += szDataID;
		_handleImage (pAP, imgDataID.utf8_str(), false);
	}

	m_utf8_1 = "object";
	tagClose(TT_OBJECT, m_utf8_1);
}

void s_HTML_Listener::_handleImage (PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	if (!bHaveProp || (pAP == 0)) return;
	
	const gchar * szDataID = 0;
	pAP->getAttribute ("dataid", szDataID);

	if (szDataID == 0)
		return;

	_handleImage (pAP, szDataID,false);
}

void s_HTML_Listener::_handleImage (const PP_AttrProp * pAP, const char * szDataID, bool bIsPositioned)
{
	UT_LocaleTransactor t(LC_NUMERIC, "C");

    std::string mimeType;

	const UT_ByteBuf * pByteBuf = 0;

	if (!m_pDocument->getDataItemDataByName(szDataID, &pByteBuf, 
                                            &mimeType, NULL))
		return;

	if ((pByteBuf == 0) || mimeType.empty()) 
        return; // ??

	if (mimeType == "image/svg+xml")
	{
		// We export SVGs as embedded objects. See _handleEmbedded() for details on why this is.
		_handleEmbedded (pAP, szDataID, pByteBuf, mimeType);
		return;
	}
	
	if ((mimeType != "image/png") && (mimeType != "image/jpeg"))
	{
		UT_DEBUGMSG(("Object not of a suppored MIME type - ignoring...\n"));
		return;
	}

	const char * dataid = UT_basename (static_cast<const char *>(szDataID));

	const char * suffix = dataid + strlen (dataid);
	const char * suffid = suffix;
	const char * ptr = 0;

	/* Question: What does the DataID look like for images pasted
	 *           from the clipboard?
	 */
	ptr = suffix;
	while (ptr > dataid)
	{
		if (*--ptr == '_')
		{
			suffix = ptr;
			suffid = suffix;
			break;
		}
	}
	ptr = suffix;
	while (ptr > dataid)
	{
		if (*--ptr == '.')
		{
			suffix = ptr;
			// break;
		}
	}
	if (dataid == suffix)
		return;

	char * base_name = NULL;
	if (m_pie->getFileName ())
		base_name = UT_go_basename_from_uri (m_pie->getFileName ());

	/* hmm; who knows what locale the system uses
	 */
	UT_UTF8String imagebasedir = "clipboard";
	if (base_name)
		imagebasedir = base_name;
	imagebasedir += "_files";
	std::string imagedir = m_pie->getFileName () ? m_pie->getFileName () : "";
	imagedir += "_files";

	UT_UTF8String filename(dataid,suffix-dataid);
	filename += suffid;

	std::string ext;
    if(m_pDocument->getDataItemFileExtension(dataid, ext, true))
	{
        filename += ext;
    }
    else
	{
        filename += ".png";
    }
        
	FREEP(base_name);

	UT_UTF8String url;

	url += s_string_to_url (imagebasedir);
	url += "/";
	url += s_string_to_url (filename);

	if (get_Multipart ())
	{
		UT_UTF8String * save_url = new UT_UTF8String(url);
		if (save_url == 0) return;

		if (!m_SavedURLs.insert (szDataID, save_url)) // arg. failed. skip image
		{
			DELETEP(save_url);
			return;
		}
	}

	/* szDataID is the raw string with the data ID
	 * imagedir is the name of the directory in which we'll write the image
	 * filename is the name of the file to which we'll write the image
	 * url      is the URL which we'll use
	 */
	if (!get_Embed_Images () && !get_Multipart ())
	{
		IE_Exp::writeBufferToFile(pByteBuf, imagedir, filename.utf8_str());
	}
	m_utf8_1 = "img";
	if(bIsPositioned)
	{
		const gchar * szXPos = NULL;
		UT_sint32 ixPos = 0;
		if(pAP->getProperty("xpos",szXPos))
		{
			    ixPos= UT_convertToLogicalUnits(szXPos);
		}
		else if(pAP->getProperty("frame-col-xpos",szXPos))
		{
			    ixPos= UT_convertToLogicalUnits(szXPos);
		}
		else if(pAP->getProperty("frame-page-xpos",szXPos))
		{
			    ixPos= UT_convertToLogicalUnits(szXPos);
		}
		if(ixPos > UT_convertToLogicalUnits("1.0in"))
		{
				m_utf8_1 += " align=\"right\" ";
		}
		else
		{
				m_utf8_1 += " align=\"left\" ";
		}
	}

	const gchar * szWidth = 0;
	const gchar * szHeight = 0;
	double widthPercentage = 0;
	if (!_getPropertySize(pAP, !bIsPositioned ? "width" : "frame-width", "height", &szWidth, widthPercentage, &szHeight))
		return;
	UT_DEBUGMSG(("Size of Image: %sx%s\n", szWidth ? szWidth : "(null)", szHeight ? szHeight : "(null)"));
	m_utf8_1 += " " + _getStyleSizeString(szWidth, widthPercentage, DIM_MM, szHeight, DIM_MM);	

	const gchar * szTitle  = 0;
	UT_UTF8String escape;
	pAP->getAttribute ("title",  szTitle);
	if (szTitle) {
		escape = szTitle;
		m_utf8_1 += " title=\"";
		m_utf8_1 += escape.escapeXML();
		m_utf8_1 += "\"";
		escape.clear();
	}

	const gchar * szAlt  = 0;
	pAP->getAttribute ("alt",  szAlt);
	m_utf8_1 += " alt=\"";
	if (szAlt) {
		escape = szAlt;
		m_utf8_1 += escape.escapeXML();
	}
	m_utf8_1 += "\"";

	const gchar * szLang  = 0;
	pAP->getProperty ("lang",  szLang);
	if (szLang) {
		if (!get_HTML4 ()) {
			// we want to emit xml:lang in addition to lang
			m_utf8_1 += " xml:lang=\"";
			m_utf8_1 += szLang;
			m_utf8_1 += "\"";
		}

		m_utf8_1 += " lang=\"";
		m_utf8_1 += szLang;
		m_utf8_1 += "\"";
	}

	if (!get_Embed_Images () || get_Multipart ())
	{
		m_utf8_1 += " src=\"";
		m_utf8_1 += url;
		m_utf8_1 += "\"";

		tagOpenClose (m_utf8_1, get_HTML4 (), ws_None);

		return;
	}

	m_utf8_1 += " src=\"data:";
	m_utf8_1 += mimeType + ";base64,";
	tagOpenBroken (m_utf8_1, ws_None);

	_writeImageBase64 (pByteBuf);

	m_utf8_1 = "\"";
	tagCloseBroken (m_utf8_1, get_HTML4 (), ws_None);
}

void s_HTML_Listener::_handlePendingImages ()
{
	UT_GenericStringMap<UT_UTF8String*>::UT_Cursor cursor (&m_SavedURLs);

	const UT_UTF8String * val = 0;
	for (val = cursor.first (); cursor.is_valid (); val = cursor.next ())
	{
		const char * dataid = cursor.key().c_str ();

		const UT_UTF8String * saved_url = val;
		UT_UTF8String * url = const_cast<UT_UTF8String *>(saved_url);

        std::string mimeType;

		const UT_ByteBuf * pByteBuf = 0;

		if (!m_pDocument->getDataItemDataByName(dataid, &pByteBuf, &mimeType, NULL))
			return;

		if (pByteBuf) // this should always be found, but just in case...
		{
			multiBoundary ();

			m_utf8_1 = mimeType;
			multiField ("Content-Type", m_utf8_1);

			m_utf8_1 = "base64";
			multiField ("Content-Transfer-Encoding", m_utf8_1);

			multiField ("Content-Location", *url);

			_writeImageBase64 (pByteBuf);

			multiBreak ();
		}
		DELETEP(url);
	}
	m_SavedURLs.clear ();
}

void s_HTML_Listener::_handleField (const PX_ChangeRecord_Object * pcro,
									PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	if (!bHaveProp || (pAP == 0))
		return;

	const gchar * szType = 0;
	pAP->getAttribute ("type", szType);

	if (szType == 0) return;

	fd_Field * field = pcro->getField ();

	m_pie->populateFields ();

	if (strcmp (szType, "list_label") != 0)
	{
		// TODO: Text styles?  (maybe not)
		// TODO: Line before footnotes?  Or use the table as in embedded target idea?
			// See also: abi bug 7612
		// TODO: Optional:
		// TODO: Embedded target for footnote linking (frame adjusts on the footnotes when ref clicked).
		
		m_utf8_1 = "span";
		
		// TODO: branch out to get the format of the refs/anchors correct.
		if ((strcmp (szType, "footnote_anchor") == 0) ||
			(strcmp (szType, "endnote_anchor") == 0) ||
			(strcmp (szType, "footnote_ref") == 0) ||
			(strcmp (szType, "endnote_ref") == 0))
		{
			const gchar * szA_Style = 0;
			bool have_style = pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, szA_Style);
			if (have_style)
			{
				const s_StyleTree * tree = 0;
				tree = m_style_tree->find (szA_Style);
				if (tree)
				{
					if (tree->class_list().byteLength ())
					{
						UT_UTF8String escape = tree->class_name ();
						m_utf8_1 += " class=\"";
						m_utf8_1 += escape.escapeXML();
						m_utf8_1 += "\"";
					}
				}
			}
			else
			{
				m_utf8_1 += " class=\"ABI_FIELD_";
				m_utf8_1 += szType;
				m_utf8_1 += "\"";
			}
			const gchar * szA_Props = 0;
			bool have_props = pAP->getAttribute (PT_PROPS_ATTRIBUTE_NAME, szA_Props);
			if(have_props)
			{
				m_utf8_1 += " style=\"";
				m_utf8_1 += szA_Props;
				m_utf8_1 += "\"";
			}
			
			gchar * szTypeCpy = new gchar[strlen(szType) + 2];
			strncpy(szTypeCpy, szType, strlen(szType)+1);
			const gchar * noteToken = (gchar *)strtok((char *)szTypeCpy, "_");
			gchar * idAttr = new gchar[strlen(noteToken) + 4];
			strncpy(idAttr, noteToken, strlen(noteToken)+1);
			const gchar * partToken = (gchar *)strtok(NULL, "_");
			const gchar * szID = 0;
			const gchar * szNoteNumInit = 0;
			UT_uint32 noteNumInit = 1;

			UT_UTF8String notePNString;
			UT_UTF8String notePLString;
			UT_UTF8String notePIDString;
			
			// Take into account document-*note-initial
				// This block preps for getting a document-level property.
				PT_AttrPropIndex docApi = m_pDocument->getAttrPropIndex();
				const PP_AttrProp * pDAP = NULL;
				m_pDocument->getAttrProp (docApi, &pDAP);
				if(!strcmp(noteToken, "footnote") && pDAP->getProperty("document-footnote-initial", szNoteNumInit))
					noteNumInit = atoi(szNoteNumInit);
				else if(!strcmp(noteToken, "endnote") && pDAP->getProperty("document-endnote-initial", szNoteNumInit))
					noteNumInit = atoi(szNoteNumInit);

			UT_uint32 ID = 0;
			if(pAP->getAttribute (strcat(idAttr, "-id"), szID) && szID)
				ID = atoi(szID);
			
			UT_UTF8String_sprintf(notePIDString, " id=\"%s_%s-%d\"", noteToken, partToken, (ID + noteNumInit));
			m_utf8_1 += notePIDString;
			tagOpen (TT_SPAN, m_utf8_1, ws_None);
			m_utf8_1 = "a";
			
			const char *hrefPartAtom = (strcmp(partToken, "anchor") ? "anchor" : "ref");
			
			UT_UTF8String_sprintf(notePLString, " href=\"#%s_%s-%d\"", noteToken, hrefPartAtom, (ID + noteNumInit));
			m_utf8_1 += notePLString;
			tagOpen (TT_A, m_utf8_1, ws_None);
			UT_UTF8String_sprintf(notePNString, "%d", (ID + noteNumInit));
			m_pie->write (notePNString.utf8_str (), notePNString.byteLength ());
			textUntrusted (field->getValue ());
			m_utf8_1 = "a";
			tagClose (TT_A, m_utf8_1, ws_None);
			DELETEPV(idAttr);
			DELETEPV(szTypeCpy);
		}
		else
		{
			m_utf8_1 = "span";
			// TODO: Find out if we can change this back to just getting the style from the document,
			//       which is what is done with notes.
			m_utf8_1 += " class=\"ABI_FIELD_";
			m_utf8_1 += szType;
			m_utf8_1 += "\"";
			tagOpen (TT_SPAN, m_utf8_1, ws_None);
			textUntrusted (field->getValue ());
		}
		
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

	if (!bHaveProp || (pAP == 0))
		return;

	const gchar * szHRef = 0;
	pAP->getAttribute ("xlink:href", szHRef);

	if (szHRef) // trust this to be a valid URL??
	{
		UT_UTF8String url = szHRef;
		url.escapeURL();
		
		m_utf8_1 += " href=\"";
		m_utf8_1 += url;
		m_utf8_1 += "\"";

		tagOpen (TT_A, m_utf8_1, ws_None);
	}
}

void s_HTML_Listener::_handleAnnotationMark (PT_AttrPropIndex api)
{
	m_utf8_1 = "a";

	if (tagTop () == TT_A)
	{
		tagClose (TT_A, m_utf8_1, ws_None);
	}

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);

	if (!bHaveProp || (pAP == 0)) return;

	// Set a forward link to the annotation to be added at the end of the 
	// document.
		
	m_utf8_1 += " href=\"#annotation-";
	UT_UTF8String num;
	UT_UTF8String_sprintf(num,"%d",m_vecAnnotations.getItemCount());
	m_utf8_1 += num;
	m_utf8_1 += "\"";
	tagOpen (TT_A, m_utf8_1, ws_None);
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

	if (!bHaveProp || (pAP == 0))
		return;

	const gchar * szType = 0;
	pAP->getAttribute ("type", szType);

	if (szType == 0)
		return; // ??

	if (g_ascii_strcasecmp (szType, "start") == 0)
	{
		const gchar * szName = 0;
		pAP->getAttribute ("name", szName);

		if (szName)
		{
			UT_UTF8String escape = szName;
			escape.escapeXML();

			m_utf8_1 += " name=\"";
			m_utf8_1 += escape;
			m_utf8_1 += "\"";

			if (!get_HTML4 ())
			{
				m_utf8_1 += " id=\"";
				m_utf8_1 += escape;
				m_utf8_1 += "\"";
			}
			tagOpen (TT_A, m_utf8_1, ws_None);
		}
	}
}


void s_HTML_Listener::_handleMath (PT_AttrPropIndex api)
{

	m_utf8_1 = "a";
	if (tagTop () == TT_A)
	{
		tagClose (TT_A, m_utf8_1, ws_None);
	}
	m_utf8_1 = "";
	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);

	if (!bHaveProp || (pAP == 0)) return;

	const gchar * szDataID = 0;
	bool bFound = pAP->getAttribute ("dataid", szDataID);

	if (szDataID == 0) return; // ??
	UT_UTF8String sMathML;
	//
	// OK shovel the mathml into the HTML stream
	//
	if (bFound && szDataID)
	{
		const UT_ByteBuf * pByteBuf = NULL;
		bFound = m_pDocument->getDataItemDataByName(szDataID, 
													 &pByteBuf,
													 NULL, NULL);
		if(bFound)
		{
			UT_UCS4_mbtowc myWC;
			sMathML.appendBuf( *pByteBuf, myWC);
			tagRaw(sMathML);
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

	tagOpenClose (m_utf8_1, get_HTML4 ());
}

void s_HTML_Listener::_handleMeta ()
{
	if (!m_pie->isCopying ())
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
						UT_String *stringval = static_cast<UT_String*>(val);
						if(stringval->size () > 0)
							{
								_handleMetaTag(cursor.key().c_str(), stringval->c_str()));
					}
			}
#endif

	}
}

#endif /* HTML_META_SUPPORTED */

bool s_HTML_Listener::populate (PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * pcr)
{
	if (!m_bSecondPass || (m_bSecondPass && m_bInAFENote)) {
		if (m_bFirstWrite && m_bClipBoard)
		{
			_openSection (0, 0);
			_openTag (0, 0);
		}
		if(m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
		{
			return true;
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
							_handleImage (api);
							return true;
	
						case PTO_Field:
							_handleField (pcro, api);
							return true;
	
						case PTO_Hyperlink:
							_handleHyperlink (api);
							return true;

						case PTO_Annotation:
							_handleAnnotationMark (api);
							return true;
	
						case PTO_Bookmark:
							_handleBookmark (api);
							return true;						

						case PTO_Math:
							_handleMath (api);
							return true;

						case PTO_Embed:
							_handleEmbedded (api);
							return true;
	
						default:
							UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled object type: %d!\n", pcro->getObjectType ()));
							UT_ASSERT_HARMLESS(UT_TODO);
							return true;
					}
				}
	
			case PX_ChangeRecord::PXT_InsertFmtMark:
				return true;
			
			default:
				UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled record type!\n"));
				return true;
		}
	}
	else return true;
}

bool s_HTML_Listener::populateStrux (PL_StruxDocHandle sdh,
					const PX_ChangeRecord * pcr,
					PL_StruxFmtHandle * psfh)
{
	UT_return_val_if_fail (pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);

	*psfh = 0; // we don't need it.

	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);

	PT_AttrPropIndex api = pcr->getIndexAP ();

	switch (pcrx->getStruxType ())
	{
		case PTX_Section:
		{
			m_bIgnoreTillNextSection = false;
			
			// TODO: It may be wise to look into the necessity of an _popUnendedStructures here.  However,
			// that may also not play nice with structures, if any, which span sections.  Unended structures
			// can theoretically do so, such as lists that attach to a extrastructural listID.  However, we
			// also (as of this writing) do not support incontiguous lists.  Again, there's an ambiguity
			// regarding how this should be handled because the behaviour of the piecetable is incompletely
			// defined.
			// UPDATE: We're going to put one in _closeSection for safety's sake.  If it makes some output
			// 	    less pretty, please do file bugs and we can fix that on a case by case basis, but
			// 	    that's preferable to spitting out corrupt html.
			
			if(m_bIgnoreTillEnd)
			{
				return true;  // Nested sections could be the sign of a severe problem, even if caused by import
			}
			
			// This block prepares us for getting document-level properties (namely, the endnote-place-endsection one stored in doEndnotes)
			PT_AttrPropIndex docApi = m_pDocument->getAttrPropIndex();
			const gchar * doEndnotes = NULL;
			const PP_AttrProp * pDAP = NULL;
			m_pDocument->getAttrProp (docApi, &pDAP);
			
			// If the d-e-p-e.s. prop is defined	(getProp call succeeds and returns TRUE), and it is 1 (TRUE), we're supposed to spit out the endnotes every section.
			if(pDAP->getProperty("document-endnote-place-endsection", doEndnotes) && atoi(doEndnotes))
			{
				_doEndnotes(); // Spit out the endnotes that have accumulated for this past section.
			}
			
			if (m_bInBlock) _closeTag (); // possible problem with lists??
			_openSection (api, 0); // Actually start the next section, which is why we're here.
			return true;
		}

		case PTX_Block:
		{
			if(m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
			{
				return true;
			}
			if (m_bFirstWrite && m_bClipBoard) _openSection (0, 0);
			_openTag (api, sdh);
			return true;
		}

#ifdef HTML_TABLES_SUPPORTED
		case PTX_SectionTable:
		{
			if(m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
			{
				return true;
			}
			if (m_bFirstWrite && m_bClipBoard) _openSection (0, 0);

			m_TableHelper.OpenTable(sdh,pcr->getIndexAP()) ;
			_closeSpan();
			_closeTag();
			_openTable(pcr->getIndexAP());
			return true;
		}

		case PTX_SectionCell:
		{
			if(m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
			{
				return true;
			}
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
			if(m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
			{
				return true;
			}
			_closeTag();
			m_utf8_1 = "tr";
			tagClose (TT_TR, m_utf8_1);
			m_TableHelper.CloseTable();
			_closeTable();
			return true;
		}

		case PTX_EndCell:
		{
			if(m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
			{
				return true;
			}
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
		
		case PTX_SectionFootnote:
		case PTX_SectionEndnote:
		case PTX_SectionAnnotation:
		{
			// We should use strux-specific position markers, as this sets a precarious
			// precedent for nested struxes.
			m_iEmbedStartPos = pcrx->getPosition() + 1;
			m_bIgnoreTillEnd = true;
			return true;
		}
		case PTX_EndFootnote:
		case PTX_EndEndnote:
		case PTX_EndAnnotation:
		{
			PD_DocumentRange * pDocRange = new PD_DocumentRange(m_pDocument, m_iEmbedStartPos, pcrx->getPosition());
			if(pcrx->getStruxType () == PTX_EndFootnote)
			{
				addFootnote(pDocRange);
			}
			else if (pcrx->getStruxType () == PTX_EndEndnote)
			{
				addEndnote(pDocRange);
			}
			else
			{
				addAnnotation(pDocRange);
			}
			m_bIgnoreTillEnd = false;
			return true;
		}
		case PTX_SectionFrame:
		{
			// We do this individually for explicitly handled types of frame, because we don't know the consequences
			// of doing it generally.
			// m_bInFrame = true; // Fortunately for the html exporter, abi does not permit nested frames.
			
			if(m_iListDepth)
              	  listPopToDepth(0); // AbiWord does not support frames in LIs, neither do we.

			if(m_bIgnoreTillEnd || m_bIgnoreTillNextSection)
			{
				return true;
			}
			// Set up to get and get the type of frame (a property thereof)
			const PP_AttrProp * pAP = 0;
			bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
			if (!bHaveProp || (pAP == 0)) return true;
			const gchar * szType = 0;
			if((pAP->getProperty ("frame-type", szType)) && szType)
			{
			     if (!strcmp(szType, "textbox"))
			     {
					 _openTextBox(pcr->getIndexAP()); // Open a new text box
					 return true;
			     }
				 if(!strcmp(szType, "image"))
				 { 
					 _openPosImage(pcr->getIndexAP()); // Output positioned image
				 }
			}
			return true;
		}

		case PTX_EndFrame:
		{
			_closeTextBox();
			return true;
		}
#if 0
		case PTX_EndMarginnote:
		case PTX_SectionMarginnote:
#endif
			// Ignore HdrFtr for now
		case PTX_SectionHdrFtr:
		{
			/* We need to close unended structures (like lists, which are known only as paragraphs with listIDs)
			   because the HdrFtr comes after all such things except for those which are contained within it. -MG */
			// This call may be unnecessary. -MG
			_popUnendedStructures();
			m_bIgnoreTillNextSection = true;
			return true;
		}
		case PTX_SectionTOC: 
		{
			_emitTOC (pcr->getIndexAP());
			return true;
		}
		case PTX_EndTOC:
		{
			return true;
		}
		default:
			UT_DEBUGMSG(("WARNING: ie_exp_HTML.cpp: unhandled strux type: %d!\n", pcrx->getStruxType ()));
			UT_ASSERT_HARMLESS(UT_TODO);
			return true;
	}


}

bool s_HTML_Listener::endOfDocument () {
	m_bIgnoreTillNextSection = false;
	_popUnendedStructures(); // We need to clean up after ourselves lest we fsck up the footer and anything else that comes after this.
	/* Remaining endnotes, whether from the last of multiple sections or from all sections */
	_doEndnotes();
	
	_doFootnotes();

	_doAnnotations();
	
	return true;
}

void s_HTML_Listener::_emitTOC (PT_AttrPropIndex api) {
	if (m_toc)
	{

		const PP_AttrProp * pAP = 0;
		bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);
		const gchar * szValue = 0;
		UT_UTF8String tocHeadingUTF8;

		listPopToDepth(0);
		
		if (tagTop() == TT_SPAN) {
			UT_DEBUGMSG(("_closeSection closing span\n"));
			tagClose(TT_SPAN, "span");
		}
		
		if (m_bInBlock && (tagTop() == TT_P)) { // If only the first is true, we have a first-order tag mismatch.  The alternative with not testing the latter is a second-order tag mismatch.
			UT_DEBUGMSG(("_closeSection closing block\n"));
			//		_closeTag (); // We need to investigate the tag stack usage of this, and whether or not we really would rather specify the tag in all cases.
			tagClose(TT_P, "p");
		}
		
		if(bHaveProp && pAP && pAP->getProperty("toc-heading", szValue)) // user-defined TOC heading
		{
			tocHeadingUTF8 = szValue;
			//_outputdata() below makes escapeXML() redundant here
		}
		else
		{ 
			const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
			pSS->getValueUTF8(AP_STRING_ID_TOC_TocHeading, tocHeadingUTF8);
		}

		bool bEmitHeading = true;

		if(bHaveProp && pAP && pAP->getProperty("toc-has-heading", szValue)) //check to see if the TOC heading is hidden
		{
			if(atoi(szValue) == 0)
				bEmitHeading = false;
		}

		// We can't use escapeXML() on tocHeadingUTF8 here because it will
		// cause values to be doubly escaped, e.g. " will become &amp;quot;.
		// Instead, just use a new variable, tocSummary.

		UT_UTF8String tocSummary = tocHeadingUTF8;
		m_utf8_1 = UT_UTF8String_sprintf("table class=\"toc\" summary=\"%s\"", tocSummary.escapeXML().utf8_str());

		tagOpen (TT_TABLE, m_utf8_1);

		m_utf8_1 = "tr";
		tagOpen (TT_TR, m_utf8_1);
		
		m_utf8_1 = "td";
		tagOpen (TT_TD, m_utf8_1);

		m_utf8_1 = "div class=\"toctitle\"";
		tagOpen (TT_DIV, m_utf8_1);

		if(bEmitHeading)
		{
			UT_UCS4String tocHeading(tocHeadingUTF8.utf8_str());
			m_utf8_1 = "h2";
			tagOpen (TT_H2, m_utf8_1);
			m_bInBlock = true;
			_outputData (tocHeading.ucs4_str(), tocHeading.length());
			m_bInBlock = false;
			tagClose (TT_H2, "h2");
		}
		
		tagClose (TT_DIV, "div");

		int level1_depth = 0;
		int level2_depth = 0;
		int level3_depth = 0;
		int level4_depth = 0;

		m_bInTOC = true;
		for (int i = 0; i < m_toc->getNumTOCEntries(); i++) {
			int tocLevel = 0;		
			
			UT_UCS4String tocText(m_toc->getNthTOCEntry(i, &tocLevel).utf8_str());

			{
				UT_LocaleTransactor t(LC_NUMERIC, "C");
				m_utf8_1 = UT_UTF8String_sprintf("p style=\"text-indent:%gin\"", ((tocLevel-1) * .5));
			}
			
			UT_UCS4String tocLevelText;
			if(tocLevel == 1)
			{
				level1_depth++;
				level2_depth = level3_depth = level4_depth = 0;
				
				tocLevelText = UT_UTF8String_sprintf("[%d] ", level1_depth).ucs4_str();
			}
			else if(tocLevel == 2)
			{
				level2_depth++;
				level3_depth = level4_depth = 0;
				tocLevelText = UT_UTF8String_sprintf("[%d.%d] ", level1_depth, level2_depth).ucs4_str();
			}
			else if(tocLevel == 3)
			{
				level3_depth++;
				level4_depth = 0;
				tocLevelText = UT_UTF8String_sprintf("[%d.%d.%d] ", level1_depth, level2_depth, level3_depth).ucs4_str();
			}
			else if(tocLevel == 4)
			{
				level4_depth++;
				tocLevelText = UT_UTF8String_sprintf("[%d.%d.%d.%d] ", level1_depth, level2_depth, level3_depth, level4_depth).ucs4_str();
			}
			
			UT_UTF8String tocLink(UT_UTF8String_sprintf("<a href=\"#AbiTOC%d__\">", i));
			tagOpen (TT_P, m_utf8_1);
			m_bInBlock = true;
			m_pie->write(tocLink.utf8_str(), tocLink.byteLength());
			_outputData (tocLevelText.ucs4_str(), tocLevelText.length());
			_outputData (tocText.ucs4_str(), tocText.length());
			m_pie->write("</a>", 4);
			m_bInBlock = false;
			tagClose (TT_P, "p");
		}

		tagClose (TT_TD, "td");
		tagClose (TT_TT, "tr");
		tagClose (TT_TABLE, "table");

		m_bInTOC = false;
	}
}

void s_HTML_Listener::_doEndnotes () {
	//
	// Output Endnotes
	//
	UT_uint32 i = 0;
	for(i=0; i< getNumEndnotes(); i++)
	{
		PD_DocumentRange * pDocRange = m_vecEndnotes.getNthItem(i);
		m_bInAFENote = true;
		m_pDocument->tellListenerSubset(this,pDocRange);
		m_bInAFENote = false;
		// Some combined bug fixes make tagpops no longer necessary, afaict. -MG
	}
	UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecEndnotes);
}

void s_HTML_Listener::_doFootnotes () {	
	//
	// Output footnotes
	//
	UT_uint32 i = 0, nFootnotes = getNumFootnotes();
	if(nFootnotes > 0)
	{
		startEmbeddedStrux();
	}
	for(i = 0; i < nFootnotes; i = i + 1)
	{
		PD_DocumentRange * pDocRange = m_vecFootnotes.getNthItem(i);
		m_bInAFENote = true;
		m_pDocument->tellListenerSubset(this,pDocRange);
		m_bInAFENote = false;
		// Some combined bug fixes make tagpops no longer necessary, afaict.
	}
	UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecFootnotes);
}


void s_HTML_Listener::_doAnnotations () {	
	//
	// Output Annotations
	//
	UT_uint32 i = 0, nAnnotations = getNumAnnotations();
	if(nAnnotations > 0) {
		startEmbeddedStrux();
	}
	UT_UTF8String sAnnTag;
	for(i = 0; i < nAnnotations; i = i + 1)
	{
		PD_DocumentRange * pDocRange = m_vecAnnotations.getNthItem(i);
		m_bInAnnotation = true;
		m_bInAFENote = true;
		//
		// Write a blank paragraph just before Each annotation containing the
		// tag id of the annotation.
		//
	    sAnnTag = "<a name=\"annotation-";
		UT_UTF8String num;
		UT_UTF8String_sprintf(num,"%d",i);
		sAnnTag += num;
		sAnnTag += "\"";

		if(!get_HTML4())
			sAnnTag += "/>";
		else
			sAnnTag += "></a>";

		m_pie->write(sAnnTag.utf8_str(),sAnnTag.byteLength());
		m_pDocument->tellListenerSubset(this,pDocRange);
		m_bInAFENote = false;
		m_bInAnnotation = false;
		_closeTag(); // close the annotation <p>
	}
	UT_VECTOR_PURGEALL(PD_DocumentRange *,m_vecAnnotations);
}


/*****************************************************************/
/*****************************************************************/

bool s_HTML_Listener::change (PL_StruxFmtHandle /*sfh*/,
							  const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_HARMLESS(0);						// this function is not used.
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
	UT_ASSERT_HARMLESS(0);						// this function is not used.
	return false;
}

bool s_HTML_Listener::signal (UT_uint32 /* iSignal */)
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return false;
}

/*****************************************************************/
/*****************************************************************/

s_HTML_HdrFtr_Listener::s_HTML_HdrFtr_Listener (PD_Document * pDocument, IE_Exp_HTML * /*pie*/, PL_Listener * pHTML_Listener) :
	m_pHdrDocRange(NULL),
	m_pFtrDocRange(NULL),
	m_pDocument(pDocument),
	m_pHTML_Listener(pHTML_Listener)
{
}

s_HTML_HdrFtr_Listener::~s_HTML_HdrFtr_Listener ()
{
}

void s_HTML_HdrFtr_Listener::doHdrFtr (bool bHeader)
{
	s_HTML_Listener * pHL = (s_HTML_Listener *)m_pHTML_Listener;
	if(bHeader && pHL->m_bHaveHeader) {
	pHL->_openSection(0, 1);
	m_pDocument->tellListenerSubset(m_pHTML_Listener, m_pHdrDocRange);
	pHL->_closeSection(); }
	if(!bHeader && pHL->m_bHaveFooter) {
	pHL->_openSection(0, 2);
	m_pDocument->tellListenerSubset(m_pHTML_Listener, m_pFtrDocRange);
	pHL->_closeSection(); }
	if(bHeader && pHL->m_bHaveHeader) {
	pHL->_openSection(0, 3); }
	if(bHeader)
		DELETEP(m_pHdrDocRange);
	else
		DELETEP(m_pFtrDocRange);
}

bool s_HTML_HdrFtr_Listener::populateStrux (PL_StruxDocHandle sdh,
					const PX_ChangeRecord * pcr,
					PL_StruxFmtHandle * psfh)
{
	/* Housekeeping and prep */
	UT_return_val_if_fail (pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
	*psfh = 0; // we don't need it.
	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);
	PT_AttrPropIndex api = pcr->getIndexAP ();
	switch (pcrx->getStruxType ())
	{
		case PTX_SectionHdrFtr:
		{
			const PP_AttrProp * pAP = 0;
			bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);
		
			if (!bHaveProp || (pAP == 0)) return true;
		
			const gchar * szType = 0;
			pAP->getAttribute ("type", szType);
			/* // */
			
			PT_DocPosition m_iHdrFtrStartPos = m_pDocument->getStruxPosition(sdh) + 1;
			PT_DocPosition m_iHdrFtrStopPos = 0;
			PL_StruxDocHandle * nextSDH = NULL;
			bool bHaveNextSection = m_pDocument->getNextStruxOfType(sdh, PTX_Section, nextSDH);
			if (bHaveNextSection)
			{
				m_iHdrFtrStopPos  = m_pDocument->getStruxPosition(nextSDH);
			}
			else
			{
				m_pDocument->getBounds(true, m_iHdrFtrStopPos);
			}
			PD_DocumentRange * pDocRange = new PD_DocumentRange(m_pDocument, m_iHdrFtrStartPos, m_iHdrFtrStopPos);
			if(!strcmp(szType, "header"))
			{
				m_pHdrDocRange = pDocRange;
				s_HTML_Listener * pHL = (s_HTML_Listener *)m_pHTML_Listener;
				pHL->setHaveHeader();
			}
			else
			{
				m_pFtrDocRange = pDocRange;
				s_HTML_Listener * pHL = (s_HTML_Listener *)m_pHTML_Listener;
				pHL->setHaveFooter();
			}
			return true;
		}
		default:
			return true;
	}
}

bool s_HTML_HdrFtr_Listener::populate (PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * /*pcr*/)
{
	return true;
}

bool s_HTML_HdrFtr_Listener::change (PL_StruxFmtHandle /*sfh*/,
							  const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_HARMLESS(0);						// this function is not used.
	return false;
}

bool s_HTML_HdrFtr_Listener::insertStrux (PL_StruxFmtHandle /*sfh*/,
								   const PX_ChangeRecord * /*pcr*/,
								   PL_StruxDocHandle /*sdh*/,
								   PL_ListenerId /* lid */,
								   void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
															   PL_ListenerId /* lid */,
															   PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT_HARMLESS(0);						// this function is not used.
	return false;
}

bool s_HTML_HdrFtr_Listener::signal (UT_uint32 /* iSignal */)
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return false;
}

/*****************************************************************/
/*****************************************************************/

s_StyleTree::s_StyleTree (s_StyleTree * parent, const char * _style_name, PD_Style * style) :
	m_pDocument(0),
	m_parent(parent),
	m_list(0),
	m_count(0),
	m_max(0),
	m_bInUse(false),
	m_style_name(_style_name),
	m_class_name(_style_name),
	m_class_list(_style_name),
	m_style(style)
{
	UT_LocaleTransactor t(LC_NUMERIC, "C");

	if ((m_style_name == "Heading 1") ||
		(m_style_name == "Heading 2") ||
		(m_style_name == "Heading 3") ||
		(m_style_name == "Normal"))
	{
		m_class_name = "";
		m_class_list = "";
	}
	else
	{
		s_removeWhiteSpace (_style_name, m_class_name, true);
		m_class_list = m_class_name;
	}

	if (parent->class_list () != "")
	{
		m_class_list += " ";
		m_class_list += parent->class_list ();
	}

	UT_uint32 j = 0;

	const gchar * szName  = 0;
	const gchar * szValue = 0;

	UT_UTF8String name;
	UT_UTF8String value;

	while (style->getNthProperty (j++, szName, szValue))
	{
		name  = szName;
		value = szValue;

		/* map property names to CSS equivalents
		 */
		if (name == "text-position")
		{
			name = "vertical-align";
			if(value == "superscript")
				value = "super";
			else if(value == "subscript")
				value = "sub";
			
		}
		else if (name == "bgcolor")
		{
			name = "background-color";
		}
		else if (!is_CSS (szName)) continue;

		/* map property values to CSS equivalents
		 */
		if (name == "font-family")
		{
			if (!((value == "serif") || (value == "sans-serif") ||
				  (value == "cursive") || (value == "fantasy") || (value == "monospace")))
			{
				value  = "'";
				value += szValue;
				value += "'";
			}
		}
		else if ((name == "color") || (name == "background-color"))
		{
			if (!value.empty() && (value != "transparent"))
			{
				value = UT_colorToHex(szValue, true);
			}
		}
		else if (strstr(name.utf8_str(), "width"))
		{
			if(strstr(name.utf8_str(), "border"))
			{
				double dPT = UT_convertToDimension(value.utf8_str(), DIM_PT);
				value = UT_UTF8String_sprintf("%.2fpt", dPT);
			}
			else
			{
				double dMM = UT_convertToDimension(value.utf8_str(), DIM_MM);
				value = UT_UTF8String_sprintf("%.1fmm", dMM);
			}
		}
		
		const std::string & cascade_value = lookup (name.utf8_str());
		if (!cascade_value.empty())
			if (cascade_value == value)
				continue;

		m_map.insert(map_type::value_type(name.utf8_str(), 
										  value.utf8_str()));
	}
	if ((m_style_name == "Heading 1") ||
		(m_style_name == "Heading 2") ||
		(m_style_name == "Heading 3") ||
		(m_style_name == "Section Heading") ||
		(m_style_name == "Chapter Heading"))
	{
		m_map.insert(map_type::value_type("page-break-after", "avoid"));
	}
}

s_StyleTree::s_StyleTree (PD_Document * pDocument) :
	m_pDocument(pDocument),
		m_parent(0),
		m_list(0),
		m_count(0),
		m_max(0),
		m_bInUse(false),
		m_style_name("None"),
		m_class_name(""),
		m_class_list(""),
		m_style(0)
{
	const char ** ptr = s_prop_list;
	while(*ptr) 
	{
		m_map.insert(map_type::value_type(*ptr, *(ptr+1)));
		ptr += 2;
	}
}

s_StyleTree::~s_StyleTree ()
{
	for (UT_uint32 i = 0; i < m_count; i++)
	{
		DELETEP(m_list[i]);
	}
	FREEP (m_list);
}

bool s_StyleTree::add (const char * _style_name, PD_Style * style)
{
	if (m_list == 0)
	{
		m_list = reinterpret_cast<s_StyleTree **>(g_try_malloc (8 * sizeof (s_StyleTree *)));
		if (m_list == 0) return false;
		m_max = 8;
	}
	if (m_count == m_max)
	{
		s_StyleTree ** more = 0;
		more = reinterpret_cast<s_StyleTree **>(g_try_realloc (m_list, (m_max + 8) * sizeof (s_StyleTree *)));
		if (more == 0) return false;
		m_list = more;
		m_max += 8;
	}

	s_StyleTree * tree = 0;

    try
		{
			tree = new s_StyleTree(this,_style_name,style);
		}
	catch(...)
		{
			tree = 0;
		}

	if (tree == 0) return false;

	m_list[m_count++] = tree;

	return true;
}

bool s_StyleTree::add (const char * _style_name, PD_Document * pDoc)
{
	if ((pDoc == 0) || (_style_name == 0) || (*_style_name == 0)) 
		return false;

	if (m_parent) 
		return m_parent->add (_style_name, pDoc);

	if (find (_style_name)) 
		return true;

	PD_Style * style = 0;
	pDoc->getStyle (_style_name, &style);
	if (!style) 
		return false;

	s_StyleTree * parent = NULL;

	PD_Style * basis = style->getBasedOn ();

	const gchar * parent_name = NULL;
	if (basis && 
		basis->getAttribute (PT_NAME_ATTRIBUTE_NAME, parent_name) &&
		strcmp(_style_name, parent_name) != 0)
	{
		parent = const_cast<s_StyleTree*>(find (basis));
		if (parent == NULL)
		{
			const gchar * basis_name = 0;
			basis->getAttribute (PT_NAME_ATTRIBUTE_NAME, basis_name);
			if (!basis_name) 
				return false;

			if (basis->getBasedOn() && basis->getBasedOn()->getName() &&
				!strcmp(_style_name, basis->getBasedOn()->getName()))
			{
				parent = this;
			}
			else
			{
				if (!add (basis_name, pDoc)) 
					return false;

				parent = const_cast<s_StyleTree*>(find (basis));
			}
		}
	}
	else 
		parent = this;

	if (!parent) {
		UT_ASSERT_NOT_REACHED();
		return false;
	}
	return parent->add (_style_name, style);
}

void s_StyleTree::inUse ()
{
	m_bInUse = true;
	if (m_parent)
		m_parent->inUse ();
}

const s_StyleTree * s_StyleTree::findAndUse (const char * _style_name)
{
	const s_StyleTree * style_tree = find (_style_name);

	if (style_tree)
	{
		s_StyleTree * tree = const_cast<s_StyleTree *>(style_tree);
		tree->inUse ();
	}
	return style_tree;
}

const s_StyleTree * s_StyleTree::find (const char * _style_name) const
{
	if (m_style_name == _style_name) 
		return this;

	const s_StyleTree * tree = 0;

	for (UT_uint32 i = 0; i < m_count; i++)
	{
		tree = m_list[i]->find (_style_name);
		if (tree) 
			break;
	}
	return tree;
}

const s_StyleTree * s_StyleTree::find (PD_Style * style) const
{
	const gchar * _style_name = 0;
	style->getAttribute (PT_NAME_ATTRIBUTE_NAME, _style_name);
	if (!_style_name) 
		return NULL;

	return find (_style_name);
}

bool s_StyleTree::descends (const char * _style_name) const
{
	if (m_parent == 0) 
		return false;

	// the name comparison has to be be case insensitive
	if (!g_ascii_strcasecmp(m_style_name.utf8_str(), _style_name)) 
		return true;

	return m_parent->descends (_style_name);
}

template<typename StyleListener>
void s_StyleTree::print (StyleListener * listener) const
{
	if (!m_bInUse) return;

	if (strstr (m_style_name.utf8_str (), "List")) return;

	if (m_parent)
	{
		UT_UTF8String selector("*.");
		if (m_class_name.byteLength ())
		{
			UT_UTF8String tmp = m_class_name;
			tmp.escapeXML();
			selector += tmp.utf8_str();
		}
		else
		{
			if (m_style_name == "Normal")
				selector = "p, h1, h2, h3, li";
			else if (m_style_name == "Heading 1")
				selector = "h1";
			else if (m_style_name == "Heading 2")
				selector = "h2";
			else if (m_style_name == "Heading 3")
				selector = "h3";
		}
		listener->styleOpen (selector);

		for (map_type::const_iterator iter = m_map.begin();
			 iter != m_map.end(); iter++) {
			listener->styleNameValue((*iter).first.c_str(), 
									 (*iter).second.c_str());
		}
		listener->styleClose ();
	}
	for (UT_uint32 i = 0; i < m_count; i++)
	{
		m_list[i]->print (listener);
	}
}

const std::string & s_StyleTree::lookup (const std::string & prop_name) const
{
	static std::string empty;
	map_type::const_iterator prop_iter = m_map.find(prop_name);

	if (prop_iter == m_map.end()) {
		if(m_parent) {
			return m_parent->lookup (prop_name);
		}
		else {
			return empty;
		}
	}
	return (*prop_iter).second;
}

void s_StyleTree::styleCheck (PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);

	if (bHaveProp && pAP)
	{
		const gchar * szStyle = NULL;
		bool have_style  = pAP->getAttribute (PT_STYLE_ATTRIBUTE_NAME, szStyle);

		if (have_style && szStyle)
		{
			findAndUse (szStyle);
		}
	}
}

bool s_StyleTree::populate (PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * pcr)
{
	switch (pcr->getType ())
	{
		case PX_ChangeRecord::PXT_InsertSpan:
			styleCheck (pcr->getIndexAP ());
			break;
		case PX_ChangeRecord::PXT_InsertObject:
			styleCheck (pcr->getIndexAP ());
			break;
		default:
			break;
	}
	return true;
}

bool s_StyleTree::populateStrux (PL_StruxDocHandle /*sdh*/,
								 const PX_ChangeRecord * pcr,
								 PL_StruxFmtHandle * psfh)
{
	UT_return_val_if_fail (pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);

	*psfh = 0; // we don't need it.

	const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *>(pcr);

	switch (pcrx->getStruxType ())
	{
		case PTX_Block:
			styleCheck (pcr->getIndexAP ());
			break;
		case PTX_SectionFootnote:
			styleCheck (pcr->getIndexAP ());
			break;
		case PTX_SectionEndnote:
			styleCheck (pcr->getIndexAP ());
			break;
		default:
			break;
	}
	return true;
}

bool s_StyleTree::change (PL_StruxFmtHandle /*sfh*/,
						  const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_HARMLESS(0);						// this function is not used.
	return false;
}

bool s_StyleTree::insertStrux (PL_StruxFmtHandle /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/,
							   PL_StruxDocHandle /*sdh*/,
							   PL_ListenerId /* lid */,
							   void (* /*pfnBindHandles*/)(PL_StruxDocHandle /* sdhNew */,
														   PL_ListenerId /* lid */,
														   PL_StruxFmtHandle /* sfhNew */))
{
	UT_ASSERT_HARMLESS(0);						// this function is not used.
	return false;
}

bool s_StyleTree::signal (UT_uint32 /* iSignal */)
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return false;
}

/*****************************************************************/
/*****************************************************************/

#ifdef TH_SKIP_REST
#undef TH_SKIP_REST
#endif
#define TH_SKIP_REST  1

#ifdef TH_SKIP_THIS
#undef TH_SKIP_THIS
#endif
#define TH_SKIP_THIS  2

class ABI_EXPORT s_TemplateHandler : public UT_XML::ExpertListener
{
public:
	s_TemplateHandler (PD_Document * pDocument, IE_Exp_HTML * pie);

	~s_TemplateHandler ();

	/* Implementation of ExpertListener
	 */
	void	StartElement (const gchar * name, const gchar ** atts);
	void	EndElement (const gchar * name);
	void	CharData (const gchar * buffer, int length);
	void	ProcessingInstruction (const gchar * target, const gchar * data);
	void	Comment (const gchar * data);
	void	StartCdataSection ();
	void	EndCdataSection ();
	void	Default (const gchar * buffer, int length);

private:
	void	_handleMetaTag (const char * key, UT_UTF8String & value);
	void	_handleMeta ();

	bool	echo () const;
	bool	condition (const gchar * data) const;

	PD_Document *	m_pDocument;
	IE_Exp_HTML *	m_pie;

	bool			m_cdata;
	bool			m_empty;

	UT_UTF8String	m_utf8;
	UT_UTF8String	m_root;
	typedef std::map<std::string, std::string> hash_type;
	hash_type		m_hash;
	UT_NumberStack	m_mode;
};

s_TemplateHandler::s_TemplateHandler (PD_Document * pDocument, IE_Exp_HTML * pie) :
	m_pDocument(pDocument),
		m_pie(pie),
		m_cdata(false),
		m_empty(false)
{
	const std::string & prop = m_pie->getProperty ("href-prefix");
	if (!prop.empty())
		m_root = prop;
}

s_TemplateHandler::~s_TemplateHandler ()
{
	// 
}

void s_TemplateHandler::StartElement (const gchar * name, const gchar ** atts)
{
	if (!echo ()) return;

	if (m_empty)
	{
		m_pie->write (">", 1);
		m_empty = false;
	}

	m_utf8  = "<";
	m_utf8 += name;

	if (atts)
	{
		const gchar ** attr = atts;

		UT_UTF8String tmp;

		while (*attr)
		{
			bool href = ( (strcmp (*attr, "href") == 0) ||
						  ((strcmp (*attr, "src" ) == 0) && (strcmp (name, "img") == 0)) );

			m_utf8 += " ";
			m_utf8 += *attr++;
			m_utf8 += "=\"";

			if (href && (**attr == '$'))
			{
				tmp  = m_root;
				tmp += (*attr++ + 1);
			}
			else
			{
				tmp = *attr++;
			}
			tmp.escapeXML ();

			m_utf8 += tmp;
			m_utf8 += "\"";
		}
	}
	m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());

	m_empty = true;
}

void s_TemplateHandler::EndElement (const gchar * name)
{
	if (!echo ()) return;

	if (m_empty)
	{
		m_pie->write (" />", 3);
		m_empty = false;
	}
	else
	{
		m_utf8  = "</";
		m_utf8 += name;
		m_utf8 += ">";
		m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
	}
}

void s_TemplateHandler::CharData (const gchar * buffer, int length)
{
	if (!echo ()) return;

	if (m_empty)
	{
		m_pie->write (">", 1);
		m_empty = false;
	}
	if (m_cdata)
	{
		m_pie->write (buffer, length);
		return;
	}
	m_utf8 = buffer;
	m_utf8.escapeXML ();
	m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
}

void s_TemplateHandler::ProcessingInstruction (const gchar * target, const gchar * data)
{
	bool bAbiXHTML = (strncmp (target, "abi-xhtml-", 10) == 0);

	if (!bAbiXHTML && !echo ()) return;

	if (m_empty)
	{
		m_pie->write (">", 1);
		m_empty = false;
	}
	if (!bAbiXHTML)
	{
		/* processing instruction not relevant to this - could be PHP, etc.
		 */
		m_utf8  = "<?";
		m_utf8 += target;
		m_utf8  = " ";
		m_utf8 += data;
		m_utf8  = "?>";
		m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
		return;
	}
	m_utf8 = target + 10;

	if ((m_utf8 == "insert") && echo ())
	{
		m_utf8 = data;

		if (m_utf8 == "title")
		{
#ifdef HTML_META_SUPPORTED
			m_utf8 = "";

			m_pDocument->getMetaDataProp (PD_META_KEY_TITLE, m_utf8);

			if (m_utf8.byteLength () == 0)
				m_utf8 = m_pie->getFileName ();

			m_utf8.escapeXML ();
			m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
#endif /* HTML_META_SUPPORTED */
		}
		else if (m_utf8 == "creator")
		{
#ifdef HTML_META_SUPPORTED
			m_utf8 = "";

			m_pDocument->getMetaDataProp (PD_META_KEY_CREATOR, m_utf8);

			if (m_utf8.byteLength ())
			{
				m_utf8.escapeXML ();
				m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
			}
#endif /* HTML_META_SUPPORTED */
		}
		else if (m_utf8 == "meta")
		{
#ifdef HTML_META_SUPPORTED
			_handleMeta ();
#endif /* HTML_META_SUPPORTED */
		}
		else if (m_utf8 == "body")
		{
			m_pie->_writeDocument (false, true);
		}
	}
	else if ((m_utf8 == "comment-replace") && echo ())
	{
		m_hash.clear ();
		UT_parse_attributes(data, m_hash);

		const std::string & sz_property(m_hash["property"]);
		const std::string & sz_comment(m_hash["comment"]);

		if (sz_property.size() && sz_comment.size())
		{
#ifdef HTML_META_SUPPORTED
			UT_UTF8String creator = "";
#endif /* HTML_META_SUPPORTED */
			std::string prop;

			if (sz_property == "meta::creator")
			{
#ifdef HTML_META_SUPPORTED
				m_pDocument->getMetaDataProp (PD_META_KEY_CREATOR, creator);

				if (creator.byteLength ())
					prop = creator.utf8_str();
#endif /* HTML_META_SUPPORTED */
			}
			else
			{
				prop = m_pie->getProperty (sz_property.c_str());
			}
			if (!prop.empty())
			{
				const UT_UTF8String DD("$$");

				m_utf8 = sz_comment.c_str();
				m_utf8.escape (DD, prop.c_str());

				m_pie->write ("<!--", 4);
				m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
				m_pie->write ( "-->", 3);
			}
		}
	}
	else if ((m_utf8 == "menuitem") && echo ())
	{
		m_hash.clear();
		UT_parse_attributes(data, m_hash);

		const std::string sz_property = m_hash["property"];
		const std::string sz_class    = m_hash["class"];
		std::string sz_href     = m_hash["href"];
		const std::string sz_label    = m_hash["label"];

		if (sz_property.size() && sz_class.size() 
			&& sz_href.size() && sz_label.size())
		{
			const char * href = sz_href.c_str();

			if (*href == '$')
			{
				m_utf8  = m_root;
				m_utf8 += href + 1;

				m_hash["href"] =  m_utf8.utf8_str();

				sz_href = m_utf8.utf8_str();
			}

			const std::string & prop = m_pie->getProperty(sz_property.c_str());

			bool ne = (prop != sz_class);

			m_utf8  = "<td class=\"";
			m_utf8 += sz_class;
			if (ne)
			{
				m_utf8 += "\"><a href=\"";
				m_utf8 += sz_href;
			}
			m_utf8 += "\"><div>";
			m_utf8 += sz_label;
			m_utf8 += "</div>";
			if (ne)
			{
				m_utf8 += "</a>";
			}
			m_utf8 += "</td>";

			m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
		}
	}
	else if (m_utf8 == "if")
	{
		if (echo ())
		{
			if (condition (data))
				m_mode.push (0);
			else
				m_mode.push (TH_SKIP_THIS);
		}
		else
		{
			m_mode.push (TH_SKIP_REST);
		}
	}
	else if (m_mode.getDepth ())
	{
		UT_sint32 mode;
		m_mode.viewTop (mode);

		if (m_utf8 == "elif")
		{
			if (mode == TH_SKIP_THIS)
			{
				if (condition (data))
				{
					m_mode.pop ();
					m_mode.push (0);
				}
			}
			else if (mode != TH_SKIP_REST)
			{
				m_mode.pop ();
				m_mode.push (TH_SKIP_REST);
			}
		}
		else if (m_utf8 == "else")
		{
			if (mode == TH_SKIP_THIS)
			{
				if (condition (data))
				{
					m_mode.pop ();
					m_mode.push (0);
				}
			}
			else if (mode != TH_SKIP_REST)
			{
				m_mode.pop ();
				m_mode.push (TH_SKIP_REST);
			}
		}
		else if (m_utf8 == "fi")
		{
			m_mode.pop ();
		}
	}
}

#ifdef HTML_META_SUPPORTED

void s_TemplateHandler::_handleMetaTag (const char * key, UT_UTF8String & value)
{
	m_utf8  = "<meta name=\"";
	m_utf8 += key;
	m_utf8 += "\" content=\"";
	m_utf8 += value.escapeXML ();
	m_utf8 += "\" />";
	m_utf8 += MYEOL;
	m_pie->write (m_utf8.utf8_str (), m_utf8.byteLength ());
}

void s_TemplateHandler::_handleMeta ()
{
	UT_UTF8String metaProp = "<meta http-equiv=\"content-type\" content=\"text/html;charset=UTF-8\" />" MYEOL;

	m_pie->write (metaProp.utf8_str (), metaProp.byteLength ());

	if (!m_pie->isCopying ()) {
		
		if (m_pDocument->getMetaDataProp (PD_META_KEY_CREATOR,  metaProp) && metaProp.size ())
			_handleMetaTag ("Author",   metaProp);
		
		if (m_pDocument->getMetaDataProp (PD_META_KEY_KEYWORDS, metaProp) && metaProp.size ())
			_handleMetaTag ("Keywords", metaProp);
		
		if (m_pDocument->getMetaDataProp (PD_META_KEY_SUBJECT,  metaProp) && metaProp.size ())
			_handleMetaTag ("Subject",  metaProp);
	}
}

#endif /* HTML_META_SUPPORTED */

bool s_TemplateHandler::echo () const
{
	if (!m_mode.getDepth ())
		return true;

	UT_sint32 mode;
	m_mode.viewTop (mode);

	return (mode == 0);
}

bool s_TemplateHandler::condition (const gchar * data) const
{
	const char * eq = strstr (data, "==");
	const char * ne = strstr (data, "!=");

	if (!eq && !ne)
		return false;
	if (eq && ne)
	{
		if (eq < ne)
			ne = 0;
		else
			eq = 0;
	}

	UT_UTF8String var;
	const char * value = NULL;

	if (eq)
	{
		var.assign (data, eq - data);
		value = eq + 2;
	}
	else
	{
		var.assign (data, ne - data);
		value = ne + 2;
	}
	const std::string & prop = m_pie->getProperty (var.utf8_str ());

	bool match;

	match = (prop == value);

	return (eq ? match : !match);
}

void s_TemplateHandler::Comment (const gchar * data)
{
	if (!echo ()) return;

	if (m_empty)
	{
		m_pie->write (">", 1);
		m_empty = false;
	}
	m_pie->write ("<!--", 4);
	m_pie->write (data, strlen (data));
	m_pie->write ("-->", 3);
}

void s_TemplateHandler::StartCdataSection ()
{
	if (!echo ()) return;

	if (m_empty)
	{
		m_pie->write (">", 1);
		m_empty = false;
	}
	m_pie->write ("<![CDATA[", 9);
	m_cdata = true;
}

void s_TemplateHandler::EndCdataSection ()
{
	if (!echo ()) return;

	if (m_empty)
	{
		m_pie->write (">", 1);
		m_empty = false;
	}
	m_pie->write ("]]>", 3);
	m_cdata = false;
}

void s_TemplateHandler::Default (const gchar * /*buffer*/, int /*length*/)
{
	// do nothing
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML::IE_Exp_HTML (PD_Document * pDocument)
	: IE_Exp(pDocument),
	  m_style_tree(new s_StyleTree(pDocument)),
	  m_bSuppressDialog(false)
{
	m_exp_opt.bIs4         = false;
	m_exp_opt.bIsAbiWebDoc = false;
	m_exp_opt.bDeclareXML  = true;
	m_exp_opt.bAllowAWML   = true;
	m_exp_opt.bEmbedCSS    = true;
	m_exp_opt.bLinkCSS     = false;
	m_exp_opt.bEmbedImages = false;
	m_exp_opt.bMultipart   = false;
	m_exp_opt.bClassOnly   = false;
	m_exp_opt.bAbsUnits    = false;
	m_exp_opt.iCompact     = 0;

	m_error = UT_OK;

#ifdef HTML_DIALOG_OPTIONS
	XAP_Dialog_HTMLOptions::getHTMLDefaults (&m_exp_opt, XAP_App::getApp ());
#endif
}

IE_Exp_HTML::~IE_Exp_HTML ()
{
	DELETEP(m_style_tree);
}

void IE_Exp_HTML::_buildStyleTree ()
{
	const PD_Style * p_pds = 0;
	const gchar * szStyleName = 0;

	UT_GenericVector<PD_Style*> * pStyles = NULL;
	getDoc()->enumStyles(pStyles);
	UT_return_if_fail( pStyles );
	UT_uint32 iStyleCount = getDoc()->getStyleCount();

	for (size_t n = 0; n < iStyleCount; n++)
	{
		p_pds = pStyles->getNthItem(n);
		UT_continue_if_fail( p_pds );

		szStyleName = p_pds->getName();
		
		if (p_pds == 0) continue;

		PT_AttrPropIndex api = p_pds->getIndexAP ();

		const PP_AttrProp * pAP_style = 0;
		bool bHaveProp = getDoc()->getAttrProp (api, &pAP_style);

		if (bHaveProp && pAP_style /* && p_pds->isUsed () */) // can't trust ->isUsed() :-(
		{
			m_style_tree->add (szStyleName, getDoc ());
		}
	}

	delete pStyles;
	
	if (isCopying ()) // clipboard
		getDoc()->tellListenerSubset (m_style_tree, getDocRange ());
	else
		getDoc()->tellListener (m_style_tree);
}

UT_Error IE_Exp_HTML::_doOptions ()
{
#ifdef HTML_DIALOG_OPTIONS
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame ();

	if (m_bSuppressDialog || !pFrame || isCopying ()) return UT_OK;
	if(pFrame)
	{
			AV_View * pView = pFrame->getCurrentView();
			if(pView)
			{
				GR_Graphics * pG = pView->getGraphics();
				if(pG && pG->queryProperties(GR_Graphics::DGP_PAPER))
				{
					return UT_OK;
				}
			}
		}
	/* run the dialog
	 */

	XAP_Dialog_Id id = XAP_DIALOG_ID_HTMLOPTIONS;

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(XAP_App::getApp()->getDialogFactory ());

	XAP_Dialog_HTMLOptions * pDialog
		= static_cast<XAP_Dialog_HTMLOptions *>(pDialogFactory->requestDialog (id));

	UT_return_val_if_fail (pDialog, false);

	pDialog->setHTMLOptions (&m_exp_opt, XAP_App::getApp ());

	pDialog->runModal (pFrame);

	/* extract what they did
	 */
	bool bSave = pDialog->shouldSave ();

	pDialogFactory->releaseDialog (pDialog);

	if (!bSave)
	{
		return UT_SAVE_CANCELLED;
	}
#endif
	return UT_OK;
}

UT_Error IE_Exp_HTML::_writeDocument ()
{
	UT_Error errOptions = _doOptions();

	if (errOptions == UT_SAVE_CANCELLED) //see Bug 10840
	{
		return UT_SAVE_CANCELLED;
	}
	else if (errOptions != UT_OK)
	{
		return UT_ERROR;
	}

	_buildStyleTree ();

	if (isCopying ()) // ClipBoard
	{
		m_exp_opt.bEmbedImages = true;
		return _writeDocument (true, false);
	}

	/* Export options:

	html4			yes | no	whether to write HTML4 or XHTML
	php-includes	yes | no	whether to add <?php instructions (for Abi's website)
	declare-xml		yes | no	whether to declare as <?xml (sometimes problematic with <?php )
	use-awml		yes | no	whether to add extra attributes in AWML namespace
	embed-css		yes | no	whether to embed the stylesheet
	link-css        <file>      styles in external sheet, insert
	appropriate <link> statement, do not
	export any style definition in the document
	class-only      yes | no    if text is formated with style, export only
	style name (class="style name") ignoring
	any explicit fmt properties
	embed-images	yes | no	whether to embed images in URLs
	html-template	<file>		use <file> as template for output
	href-prefix		<path>		use <path> as prefix for template href attributes marked with initial '$'
	title           <utf8 string> can contain the following special tokens
	                              %n - file name without extension
	                              %f - file name with extension
	                              %F - file name including full path
	abs-units       yes | no    use absolute rather than relative units in tables, etc. (defaults to no units)
	scale-units     yes | no    use scale (relative) rather than absolute units in tables, etc. (defaults to no units)
	compact         yes | no | number -- if set we avoid ouputing unnecessary whitespace; numerical value
	                                     indicates max line length (default MAX_LINE_LEN)
	*/

	std::string prop;

	prop = getProperty ("html4");
	if (!prop.empty())
		m_exp_opt.bIs4 = UT_parseBool (prop.c_str (), m_exp_opt.bIs4);

	prop = getProperty ("php-includes");
	if (!prop.empty())
		m_exp_opt.bIsAbiWebDoc = UT_parseBool (prop.c_str (), m_exp_opt.bIsAbiWebDoc);

	prop = getProperty ("declare-xml");
	if (!prop.empty())
		m_exp_opt.bDeclareXML = UT_parseBool (prop.c_str (), m_exp_opt.bDeclareXML);

	prop = getProperty ("use-awml");
	if (!prop.empty())
		m_exp_opt.bAllowAWML = UT_parseBool (prop.c_str (), m_exp_opt.bAllowAWML);

	prop = getProperty ("embed-css");
	if (!prop.empty())
		m_exp_opt.bEmbedCSS = UT_parseBool (prop.c_str (), m_exp_opt.bEmbedCSS);

	prop = getProperty ("abs-units");
	if (!prop.empty())
		m_exp_opt.bAbsUnits = UT_parseBool (prop.c_str (), m_exp_opt.bAbsUnits);

	prop = getProperty ("compact");
	if (!prop.empty())
	{
		UT_sint32 iLen = atoi(prop.c_str());
		if(iLen != 0)
			m_exp_opt.iCompact = (UT_uint32)iLen;
		else
		{
			m_exp_opt.iCompact = (UT_uint32)UT_parseBool (prop.c_str (), (bool)m_exp_opt.iCompact);
			if(m_exp_opt.iCompact)
				m_exp_opt.iCompact = MAX_LINE_LEN;
		}
	}
	

	prop = getProperty ("link-css");
	if (!prop.empty())
	{
		m_exp_opt.bEmbedCSS = false;
		m_exp_opt.bLinkCSS = true;
		m_sLinkCSS = prop;
	}
	
	prop = getProperty ("class-only");
	if (!prop.empty() && !g_ascii_strcasecmp("yes",prop.c_str()))
	{
		m_exp_opt.bClassOnly = true;
	}

	prop = getProperty ("title");
	if(!prop.empty())
	{
		m_sTitle.clear();
		// FIXME: less optimal -- hub
		UT_UTF8String utf8prop(prop.c_str());
		
		UT_UTF8Stringbuf::UTF8Iterator propIt = utf8prop.getIterator();

		UT_UCS4Char c = UT_UTF8Stringbuf::charCode(propIt.current());
		bool bToken = false;
		
		while(c)
		{
			if(bToken)
			{
				const char * fname = getDoc()->getFilename();
				if(fname)
				{
					const char * base = UT_basename(fname);
					UT_uint32 iNameLen = strlen(base);

					const char * dot  = strrchr(base, '.');
					if(dot)
					{
						iNameLen = dot - base;
					}
					
					switch(c)
					{
						case 'n':
							m_sTitle.append(base, iNameLen);
							break;
						
						case 'f':
							m_sTitle += base;
							break;

						case 'F':
							m_sTitle += fname;
							break;

						default:
							m_sTitle.appendUCS4(&c,1);
					}
				}
				
				bToken = false;
			}
			else if(c == '%')
			{
				bToken = true;
				//m_sTitle.appendUCS4(&c,1);
			}
			else
			{
				m_sTitle.appendUCS4(&c,1);
			}
			
			c = UT_UTF8Stringbuf::charCode(propIt.advance());
		}
	}
	
		

	prop = getProperty ("embed-images");
	if (!prop.empty())
		m_exp_opt.bEmbedImages = UT_parseBool(prop.c_str(), m_exp_opt.bEmbedImages);

	prop = getProperty ("html-template");
	if (prop.empty())
		return _writeDocument (false, false);

	/* template mode...
	 */
	m_exp_opt.bIs4 = false;

	UT_UTF8String declaration;

	if (m_exp_opt.bDeclareXML)
		declaration += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" MYEOL;

	declaration += "<";
	declaration += s_DTD_XHTML;
	declaration += ">" MYEOL;

	write (declaration.utf8_str (), declaration.byteLength ());

	s_TemplateHandler TH(getDoc(),this);

	UT_XML parser;
	parser.setExpertListener (&TH);

	UT_Error err = parser.parse (prop.c_str ());

	return err;
}

struct StyleListener
{
	UT_ByteBuf& m_sink;
	UT_UTF8String m_utf8_0;
	UT_uint32 m_styleIndent;

	StyleListener(UT_ByteBuf& sink)
		: m_sink(sink), m_styleIndent(0)
	{
	}

	bool get_Compact() { return false; }

	void tagRaw (UT_UTF8String & content)
	{
		m_sink.append((const UT_Byte*)content.utf8_str (), content.byteLength ());
	}

	void styleIndent ()
	{
		m_utf8_0 = "";
		
		for (UT_uint32 i = 0; i < m_styleIndent; i++) m_utf8_0 += "\t";
	}

	void styleOpen (const UT_UTF8String & rule)
	{
		styleIndent ();
		
		m_utf8_0 += rule;
		m_utf8_0 += " {";
		if(!get_Compact())
			m_utf8_0 += MYEOL;
		
		tagRaw (m_utf8_0);
		
		m_styleIndent++;
	}
	
	void styleClose ()
	{
		if (m_styleIndent == 0)
		{
			UT_DEBUGMSG(("WARNING: CSS style group over-closing!\n"));
			return;
		}
		m_styleIndent--;
		
		styleIndent ();
		
		m_utf8_0 += "}";
		if(!get_Compact())
			m_utf8_0 += MYEOL;
		
		tagRaw (m_utf8_0);
	}

	void styleNameValue (const char * name, const UT_UTF8String & value)
	{
		styleIndent ();
		
		m_utf8_0 += name;
		m_utf8_0 += ":";
		m_utf8_0 += value;
		m_utf8_0 += ";";
		if(!get_Compact())
			m_utf8_0 += MYEOL;

		tagRaw (m_utf8_0);
	}

	void styleText (const UT_UTF8String & content)
	{
		m_utf8_0 = content;
		tagRaw (m_utf8_0);
	}
};

void IE_Exp_HTML::printStyleTree(PD_Document *pDocument, UT_ByteBuf& sink)
{
	IE_Exp_HTML html(pDocument);
	html._buildStyleTree ();

	StyleListener listener(sink);
	html.m_style_tree->print(&listener);
}

UT_Error IE_Exp_HTML::_writeDocument (bool bClipBoard, bool bTemplateBody)
{
	s_HTML_Listener * pListener = new s_HTML_Listener(getDoc(),this,bClipBoard,bTemplateBody,
													  &m_exp_opt,m_style_tree,
													  m_sLinkCSS, m_sTitle);
	if (pListener == 0) return UT_IE_NOMEMORY;

	PL_Listener * pL = static_cast<PL_Listener *>(pListener);

	bool okay = true;

 	s_HTML_HdrFtr_Listener * pHdrFtrListener = new s_HTML_HdrFtr_Listener(getDoc(),this, pL);
 	if (pHdrFtrListener == 0) return UT_IE_NOMEMORY;
 	PL_Listener * pHFL = static_cast<PL_Listener *>(pHdrFtrListener);

 	if(!bClipBoard)
 	{
 		okay = getDoc()->tellListener (pHFL);
 		pHdrFtrListener->doHdrFtr(1);
 	}
 	
	if (bClipBoard)
	{
		okay = getDoc()->tellListenerSubset (pL, getDocRange ());
	}
	else if (okay) {
		okay = getDoc()->tellListener (pL);
		if(okay) okay = pListener->endOfDocument();
	}
	
	if(!bClipBoard)
 		pHdrFtrListener->doHdrFtr(0); // Emit footer
 	
	DELETEP(pListener);
	DELETEP(pHdrFtrListener);
	
	if ((m_error == UT_OK) && (okay == true)) return UT_OK;
	return UT_IE_COULDNOTWRITE;
}

void s_HTML_Listener::addFootnote(PD_DocumentRange * pDocRange)
{
	m_vecFootnotes.addItem(pDocRange);
}

void s_HTML_Listener::addEndnote(PD_DocumentRange * pDocRange)
{
	m_vecEndnotes.addItem(pDocRange);
}


void s_HTML_Listener::addAnnotation(PD_DocumentRange * pDocRange)
{
	m_vecAnnotations.addItem(pDocRange);
}

UT_uint32 s_HTML_Listener::getNumAnnotations(void)
{
	return m_vecAnnotations.getItemCount();
}

UT_uint32 s_HTML_Listener::getNumFootnotes(void)
{
	return m_vecFootnotes.getItemCount();
}

UT_uint32 s_HTML_Listener::getNumEndnotes(void)
{
	return m_vecEndnotes.getItemCount();
}

void s_HTML_Listener::setHaveHeader()
{
	m_bHaveHeader = true;
}

void s_HTML_Listener::setHaveFooter()
{
	m_bHaveFooter = true;
}
