/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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
#include "ut_debugmsg.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "ut_wctomb.h"
#include "ut_path.h"
#include "ut_stack.h"
#include "pt_Types.h"
#include "fd_Field.h"
#include "ie_exp_HTML.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"
#include "xap_App.h"
#include "xap_EncodingManager.h"

#include "ut_string_class.h"

// We terminate each line with a \r\n sequence to make IE think that
// our XHTML is really HTML. This is a stupid IE bug. Sorry

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_HTML_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!(UT_stricmp(szSuffix,".html")) || !(UT_stricmp(szSuffix,".xhtml"))
			|| !(UT_stricmp(szSuffix,".htm")));
}

UT_Error IE_Exp_HTML_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_HTML_Sniffer::getDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "XHTML 1.0 (.xhtml)";
	*pszSuffixList = "*.xhtml";
	*ft = getFileType();
	return true;
}

// HTML 4

bool IE_Exp_HTML4_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!(UT_stricmp(szSuffix,".html"))
		|| !(UT_stricmp(szSuffix,".htm")));
}

UT_Error IE_Exp_HTML4_Sniffer::constructExporter(PD_Document * pDocument,
													 IE_Exp ** ppie)
{
	IE_Exp_HTML * p = new IE_Exp_HTML(pDocument, true);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_HTML4_Sniffer::getDlgLabels(const char ** pszDesc,
									   const char ** pszSuffixList,
									   IEFileType * ft)
{
	*pszDesc = "HTML 4.0 (.html, .htm)";
	*pszSuffixList = "*.html; *.htm";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML::IE_Exp_HTML(PD_Document * pDocument, bool is4)
  : IE_Exp(pDocument), m_pListener(0), m_bIs4(is4)
{
	m_error = UT_OK;
}

IE_Exp_HTML::~IE_Exp_HTML()
{
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
	s_HTML_Listener(PD_Document * pDocument,
			IE_Exp_HTML * pie, bool is4);
	virtual ~s_HTML_Listener();

	virtual bool		populate(PL_StruxFmtHandle sfh,
								 const PX_ChangeRecord * pcr);

	virtual bool		populateStrux(PL_StruxDocHandle sdh,
									  const PX_ChangeRecord * pcr,
									  PL_StruxFmtHandle * psfh);

	virtual bool		change(PL_StruxFmtHandle sfh,
							   const PX_ChangeRecord * pcr);

	virtual bool		insertStrux(PL_StruxFmtHandle sfh,
									const PX_ChangeRecord * pcr,
									PL_StruxDocHandle sdh,
									PL_ListenerId lid,
									void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
															PL_ListenerId lid,
															PL_StruxFmtHandle sfhNew));

	virtual bool		signal(UT_uint32 iSignal);

protected:
	void				_closeSection(void);
	void				_closeTag(void);
	void				_closeSpan(void);
	void				_openTag(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);
	void				_outputData(const UT_UCSChar * p, UT_uint32 length);
	bool				_inherits(const char* style, const char* from);
	void				_outputInheritanceLine(const char* ClassName);
	void 				_outputBegin(PT_AttrPropIndex api);
	void				_handleDataItems(void);
	void				_convertFontSize(UT_String& szDest, const char* pszFontSize);
	void				_convertColor(UT_String& szDest, const char* pszColor);
	void				_storeStyles(void);
	char *				_stripSuffix(const char* from, char delimiter);

	PD_Document *		m_pDocument;
	IE_Exp_HTML *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bNextIsSpace;
	bool				m_bWroteText;
	bool				m_bFirstWrite;
        bool                            m_bIs4;
	const PP_AttrProp*	m_pAP_Span;

	// Need to look up proper type, and place to stick #defines...

	UT_uint16		m_iBlockType;	// BT_*
	UT_uint16		m_iListDepth;	// 0 corresponds to not in a list
	UT_uint16		m_iPrevListDepth;
	UT_Stack		m_utsListType;
	UT_Vector		m_utvDataIDs;	// list of data ids for image enumeration
        UT_uint16               m_iImgCnt;
	UT_Wctomb		m_wmctomb;
};

/*!	This function copies a string to a new string, removing all the white
	space in the process.  Note that this function allocates the new
	string (and so the caller must make sure to deallocate it).
 */

static char* removeWhiteSpace(const char * text)
{
	char* temp = static_cast<char *>(UT_calloc(strlen(text)+1, sizeof(char)));
	char* ref = temp;
	char* ptr = (char *)text; // only a pointer for quick traversal

	while(*ptr)
	{
		if(!isspace(*ptr))
		{
			*temp++ = *ptr;
		}
		ptr++;
	}

	return ref;
}

/*!	This function returns true if the given property is a valid CSS
	property.  It is based on the list in pp_Property.cpp, and, as such,
	is quite brittle.
 */
static bool is_CSS(const char* property)
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


void s_HTML_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}

	m_pie->write("</div>\r\n");
	m_bInSection = false;
	return;
}

void s_HTML_Listener::_closeTag(void)
{
	if (!m_bInBlock)
	{
		return;
	}

	if(m_iBlockType == BT_NORMAL)
	{
		if(!m_bWroteText)
		{
			if (!m_bIs4)
		    {
				m_pie->write("<br />\r\n");
		    }
			else
		    {
				m_pie->write("<br>\r\n");
		    }
		}
		m_pie->write("</p>\r\n");
	}
	else if(m_iBlockType == BT_HEADING1)
	{
		m_pie->write("</h1>\r\n");
	}
	else if(m_iBlockType == BT_HEADING2)
	{
		m_pie->write("</h2>\r\n");
	}
	else if(m_iBlockType == BT_HEADING3)
	{
		m_pie->write("</h3>\r\n");
	}
	else if(m_iBlockType == BT_BLOCKTEXT)
	{
		m_pie->write("</blockquote>\r\n");
	}
	else if(m_iBlockType == BT_PLAINTEXT)
	{
		m_pie->write("</pre>\r\n");
	}
	else if(m_iBlockType == BT_NUMBEREDLIST || m_iBlockType == BT_BULLETLIST)
	{
/* do nothing, lists are handled differently, as they have multiple tags */
	}
	// Add "catchall" for now
 	else
	{
		m_pie->write("</p>\r\n");
	}

	m_bInBlock = false;
	return;
}

void s_HTML_Listener::_openTag(PT_AttrPropIndex api)
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
	bool wasWritten = false;
	m_bWroteText = false;

	if (bHaveProp && pAP)
	{
		const XML_Char * szValue;
		const XML_Char * szDefault = "Normal";
		/*	TODO: Perhaps the above should be/is a #define somewhere?	*/
		const XML_Char * szLevel;
		const XML_Char * szListID;
		const XML_Char * szStyleType;
		UT_uint16 *popped, *pushed;

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

		if (
		   (pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szValue) ||
		   	(pAP->getAttribute(PT_LISTID_ATTRIBUTE_NAME, szListID) &&
			 0 != UT_strcmp(szListID, "0")))
		   )
		{
			if(pAP->getAttribute(PT_LISTID_ATTRIBUTE_NAME, szListID)
				&& 0 != UT_strcmp(szListID, "0"))
			{
				/*	A nonzero "listid" attribute value indicates that we
				 *	are in a list item, so we need to process it, HTML-style.
				 */

				/*	Specify a default style name for this list item if it
				 *	doesn't already have one.	*/
				if(!pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szValue))
					szValue = szDefault;

				/*	Find out how deeply nested this list item is.	*/
				pAP->getAttribute("level", szLevel);
				m_iListDepth = atoi((const char*) szLevel);
				if(!pAP->getProperty("list-style", szStyleType))
				{
					szStyleType = szValue;
				}

				/*	If our list is getting deeper, we need to start a
				 *	nested list.	*/
				if(m_iListDepth > m_iPrevListDepth)
				{
					if(UT_strcmp((const char*) szStyleType, "Bullet List") != 0)
					{
						m_iBlockType = BT_NUMBEREDLIST;
						m_pie->write("\r\n<ol class=\"");
					}
					else
					{
						m_iBlockType = BT_BULLETLIST;
						m_pie->write("\r\n<ul class=\"");
					}
					_outputInheritanceLine((const char*) szValue);
					m_pie->write("\">\r\n");
					pushed = new UT_uint16(m_iBlockType);
					m_utsListType.push(pushed);
				}
				/*	If our list is not getting deeper, but the list type
				 *	switched from a numbered list to a bullet list, then we
				 *	pop list types off the stack until we are at the same
				 *	depth, closing out lists as necessary.  This may only be
				 *	once, if we are already at the same depth. */
				else if(m_iPrevListDepth > 0 && (
					(m_iBlockType == BT_BULLETLIST && (
					 UT_strcmp((const char*) szStyleType,
					 	"Numbered List") == 0 ||
					 _inherits((const char*) szStyleType, "NumberedList"))) ||
					(m_iBlockType == BT_NUMBEREDLIST && (
					 UT_strcmp((const char*) szStyleType,
					 	"Bullet List") == 0 ||
					 _inherits((const char*) szStyleType, "BulletList"))) ) )
				{
					m_pie->write("</li>\r\n");

					/*	Toggle the current block type	*/
					m_iBlockType = (m_iBlockType == BT_NUMBEREDLIST ?
						BT_BULLETLIST : BT_NUMBEREDLIST);

					while(m_utsListType.pop((void**) &popped))
					{				/* handle the switch, including depths */
						if(m_iPrevListDepth == m_iListDepth &&
							*popped == m_iBlockType)
						{
							m_pie->write("<li>\r\n");
							break;
						}
						else if(m_iPrevListDepth == m_iListDepth)
						{
							if(m_iBlockType == BT_BULLETLIST)
							{
								m_pie->write("</ol>\r\n<ul class=\"");
							}
							else
							{
								m_pie->write("</ul>\r\n<ol class=\"");
							}

							_outputInheritanceLine((const char*) szValue);
							m_pie->write("\">\r\n");
							pushed = new UT_uint16(m_iBlockType);
							m_utsListType.push(pushed);

							break;
						}
						else if(*popped == BT_NUMBEREDLIST)
						{
							m_pie->write("</ol>\r\n");
						}
						else if(*popped == BT_BULLETLIST)
						{
							m_pie->write("</ul>\r\n");
						}

						if(--m_iPrevListDepth > 0) m_pie->write("</li>\r\n");
					}

				}
				else
				{
					m_pie->write("</li>\r\n");
					for(UT_uint16 i = 0; i < m_iPrevListDepth - m_iListDepth;
						i++)
					{
						if(m_utsListType.pop((void**) &popped) &&
							*popped == BT_NUMBEREDLIST)
						{
							m_pie->write("</ol>\r\n");
						}
						else
						{
							m_pie->write("</ul>\r\n");
						}
						DELETEP(popped);
						m_pie->write("</li>\r\n");
					}
				}
				m_pie->write("<li");
				m_iPrevListDepth = m_iListDepth;
				wasWritten = true;
			}
			else
			{
				for(UT_uint16 i = 0; i < m_iPrevListDepth; i++)
				{	// we're no longer in a list, close it
					if(m_utsListType.pop((void**) &popped) &&
						*popped == BT_NUMBEREDLIST)
					{
						m_pie->write("</li>\r\n</ol>\r\n");
					}
					else
					{
						m_pie->write("</li>\r\n</ul>\r\n");
					}
					DELETEP(popped);
				}
				m_iPrevListDepth = m_iListDepth = 0;

				if(UT_strcmp((const char*) szValue, "Heading 1") == 0 ||
					_inherits((const char*) szValue, "Heading1"))
				{
					// <p style="Heading 1"> ...

					m_iBlockType = BT_HEADING1;
					m_pie->write("\r\n<h1");
					if(_inherits((const char*) szValue, "Heading1"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(UT_strcmp((const char*) szValue, "Heading 2") == 0 ||
					_inherits((const char*) szValue, "Heading2"))
				{
					// <p style="Heading 2"> ...

					m_iBlockType = BT_HEADING2;
					m_pie->write("\r\n<h2");
					if(_inherits((const char*) szValue, "Heading2"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(UT_strcmp((const char*) szValue, "Heading 3") == 0 ||
					_inherits((const char*) szValue, "Heading3"))
				{
					// <p style="Heading 3"> ...

					m_iBlockType = BT_HEADING3;
					m_pie->write("\r\n<h3");
					if(_inherits((const char*) szValue, "Heading3"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(UT_strcmp((const char*) szValue, "Block Text") == 0 ||
					_inherits((const char*) szValue, "BlockText"))
				{
					// <p style="Block Text"> ...

					m_iBlockType = BT_BLOCKTEXT;
					m_pie->write("<blockquote");
					if(_inherits((const char*) szValue, "BlockText"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(UT_strcmp((const char*) szValue, "Plain Text") == 0 ||
					_inherits((const char*) szValue, "PlainText"))
				{
					// <p style="Plain Text"> ...

					m_iBlockType = BT_PLAINTEXT;
					m_pie->write("<pre");
					if(_inherits((const char*) szValue, "PlainText"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(UT_strcmp((const char*) szValue, "Normal") == 0 ||
					_inherits((const char*) szValue, "Normal"))
				{
					// <p style="Normal"> ...

					m_iBlockType = BT_NORMAL;
					m_pie->write("<p");
					if(_inherits((const char*) szValue, "Normal"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else
				{
					// <p style="<anything else!>"> ...

			        m_iBlockType = BT_NORMAL;
			      	m_pie->write("<p class=\"");
					_outputInheritanceLine((const char*) szValue);
					m_pie->write("\"");
					wasWritten = true;
				}
			}
		}
		else
		{
			for(UT_uint16 i = 0; i < m_iPrevListDepth; i++)
			{	// we're no longer in a list, close it
				if(m_utsListType.pop((void**) &popped) &&
					*popped == BT_NUMBEREDLIST)
				{
					m_pie->write("</li>\r\n</ol>\r\n");
				}
				else
				{
					m_pie->write("</li>\r\n</ul>\r\n");
				}
				DELETEP(popped);
			}
			m_iPrevListDepth = m_iListDepth = 0;

			// <p> with no style attribute ...

			m_iBlockType = BT_NORMAL;
			m_pie->write("<p");
			wasWritten = true;
		}

		/* Assumption: never get property set with block text, plain text. Probably true. */
		bool css = false;

		if ( m_iBlockType != BT_PLAINTEXT && m_iBlockType != BT_BLOCKTEXT &&
				(pAP->getProperty("text-align", szValue)
				|| pAP->getProperty("margin-bottom", szValue)
				|| pAP->getProperty("margin-top", szValue)
				|| pAP->getProperty("margin-left", szValue)
				|| pAP->getProperty("margin-right", szValue)
				|| pAP->getProperty("text-indent", szValue)
				|| pAP->getProperty("dom-dir", szValue)
))
		{
			if(pAP->getProperty("dom-dir", szValue))
			{
				m_pie->write(" dir=\"");
				m_pie->write((char*)szValue);
				m_pie->write("\"");
			}

			bool validProp = false;
			if(pAP->getProperty("text-align", szValue))
			{
				validProp = true;
				m_pie->write(" style=\"text-align: ");
				m_pie->write((char*)szValue);
				css = true;
			}
			if(pAP->getProperty("margin-bottom", szValue))
			{
				validProp = true;
				if (css)
					m_pie->write("; margin-bottom: ");
				else
					m_pie->write(" style=\"margin-bottom: ");
				m_pie->write((char*)szValue);
				css = true;
			}
			if(pAP->getProperty("margin-top", szValue))
			{
				validProp = true;
			    if (css)
				     m_pie->write("; margin-top: ");
				else
				     m_pie->write(" style=\"margin-top: ");
				m_pie->write((char*)szValue);
				css = true;
			}
			if(pAP->getProperty("margin-right", szValue))
			{
				validProp = true;
				if(css)
					m_pie->write("; margin-right: ");
				else
					m_pie->write(" style=\"margin-right: ");
				m_pie->write((char*)szValue);
				css = true;
			}
			/* NOTE: For both "margin-left" and "text-indent" for lists,
			   Abi's behaviour and HTML's do not match.
			   Furthermore, it seems like all blocks have a "margin-left"
			   and "text-indent", even if they are zero, which adds
			   significant clutter.  These are all manually taken care of
			   below.  I think that the computation of these attributes
			   needs to be rethought. - John */
			if(pAP->getProperty("margin-left", szValue)
				&& (!pAP->getAttribute("listid", szListID) ||
					0 == UT_strcmp(szListID, "0")) &&
					strstr(szValue, "0.0000") == 0)
			{	// HACK: extra cond ensures that we're not in a list
				validProp = true;
				if(css)
					m_pie->write("; margin-left: ");
				else
					m_pie->write(" style=\"margin-left: ");
				m_pie->write((char*)szValue);
				css = true;
			}
			if(pAP->getProperty("text-indent", szValue)
				&& (!pAP->getAttribute("listid", szListID) ||
					0 == UT_strcmp(szListID, "0")) &&
					strstr(szValue, "0.0000") == 0)
			{	// HACK: extra cond ensures that we're not in a list
				validProp = true;
				if(css)
					m_pie->write("; text-indent: ");
				else
					m_pie->write(" style=\"text-indent: ");
				m_pie->write((char*)szValue);
				css = true;
			}

			if(validProp)
			  m_pie->write("\"");
		}
	}
	else
	{

		// <p> with no style attribute, and no properties either

		m_iBlockType = BT_NORMAL;
		m_pie->write("<p");
		wasWritten = true;
	}
	if (wasWritten)
	{
		m_pie->write(">");
	}
	m_bInBlock = true;
}

void s_HTML_Listener::_openSection(PT_AttrPropIndex api)
{
	if (m_bFirstWrite)
	{
		_outputBegin(api);
	}

	m_pie->write("<div>\r\n");
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
	bool bDir = false;

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

			UT_return_if_fail(p || !pszDecor);
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

			UT_return_if_fail(p || !pszDecor);
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

			UT_return_if_fail(p || !pszDecor);
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

		if (span)
		{
			m_pie->write("\"");

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

void s_HTML_Listener::_outputInheritanceLine(const char* ClassName)
{
	PD_Style* pStyle = NULL;
	PD_Style* pBasedOn = NULL;
	const XML_Char* szName = NULL;

	if (m_pDocument->getStyle (ClassName, &pStyle)) {
		if(pStyle)
		{
			pBasedOn = pStyle->getBasedOn();
			if(pBasedOn)
			{
//
// The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME attribute within the
// style
//
				pBasedOn->getAttribute(PT_NAME_ATTRIBUTE_NAME, szName);

				UT_return_if_fail((szName));
				char * pName = removeWhiteSpace((const char*) szName);
				_outputInheritanceLine(pName);
				FREEP(pName);
				m_pie->write(" ");
			}
		}
	}

	ClassName = removeWhiteSpace(ClassName);
	m_pie->write(ClassName);
	FREEP(ClassName);
}

void s_HTML_Listener::_outputBegin(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	// we always encode as UTF-8
	if ( !m_bIs4 )
	{
	    m_pie->write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n");
	    m_pie->write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\r\n");
	}
	else
	{
	    m_pie->write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" \"http://www.w3.org/TR/REC-html40/loose.dtd\">\r\n");
	}

	m_pie->write("<!-- ================================================================================  -->\r\n");
	m_pie->write("<!-- This HTML file was created by AbiWord.                                            -->\r\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor.                                    -->\r\n");
	m_pie->write("<!-- You may obtain more information about AbiWord at www.abisource.com                -->\r\n");
	m_pie->write("<!-- ================================================================================  -->\r\n");
	m_pie->write("\r\n");

	if ( !m_bIs4 )
	{
	    m_pie->write("<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n");
	}
	else
	{
	    m_pie->write("<html>\r\n");
	}

	m_pie->write("<head>\r\n");

	// we always encode as UTF-8
	if ( !m_bIs4 )
	{
	    m_pie->write("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />\r\n");
	}
	else
	{
	    m_pie->write("<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" >\r\n");
	}

	m_pie->write("<title>");
	m_pie->write(m_pie->getFileName());
	m_pie->write("</title>\r\n");
	m_pie->write("<style type=\"text/css\">\r\n");
	if(bHaveProp && pAP)
	{							// global page styles refer to the <body> tag
		const XML_Char *szValue;
		PD_Style *pStyle = NULL;
		m_pDocument->getStyle("Normal", &pStyle);

		m_pie->write("body\r\n{\r\n\t");
		if(pStyle)				// Add normal styles to any descendent of the
								// body for global effect
		{
			const XML_Char *szName;
			for(UT_uint16 i = 0; i < pStyle->getPropertyCount(); i++)
			{
				pStyle->getNthProperty(i, szName, szValue);
				if(!is_CSS(static_cast<const char*>(szName)) ||
					strstr(szName, "margin"))	// ... don't include margins
					continue;

				m_pie->write(szName);
				m_pie->write(": ");
				if (UT_strcmp(szName, "font-family") == 0 &&
					(UT_strcmp(szValue, "serif") != 0 ||
					 UT_strcmp(szValue, "sans-serif") != 0 ||
					 UT_strcmp(szValue, "cursive") != 0 ||
					 UT_strcmp(szValue, "fantasy") != 0 ||
					 UT_strcmp(szValue, "monospace") != 0))
				{
					m_pie->write("\"");
					m_pie->write(szValue);
					m_pie->write("\"");
				} // only quote non-keyword family names
				else if (!UT_strcmp(szName, "color"))
				  {
				    if (!IS_TRANSPARENT_COLOR(szValue))
				      {
					m_pie->write("#");
					m_pie->write(szValue);
				      }
				  }
				else
					m_pie->write(szValue);
				m_pie->write(";\r\n\t");
			}
		}

		szValue = PP_evalProperty("background-color",
								  NULL, NULL, pAP, m_pDocument, true);
		if(!IS_TRANSPARENT_COLOR(szValue))
		  {
		    m_pie->write("background-color: ");
		    if (*szValue != '#')
		      m_pie->write("#");
		    m_pie->write(szValue);
		  }
		m_pie->write(";\r\n}\r\n\r\n@media print\r\n{\r\n\tbody\r\n\t{\r\n\t\t");

		szValue = PP_evalProperty("page-margin-top",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write("padding-top: ");
		m_pie->write(szValue);

		szValue = PP_evalProperty("page-margin-bottom",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write("; padding-bottom: ");
		m_pie->write(szValue);

		szValue = PP_evalProperty("page-margin-left",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write(";\r\n\t\tpadding-left: ");
		m_pie->write(szValue);

		szValue = PP_evalProperty("page-margin-right",
								  NULL, NULL, pAP, m_pDocument, true);
		m_pie->write("; padding-right: ");
		m_pie->write(szValue);

		m_pie->write(";\r\n\t}\r\n}\r\n\r\n");
	}

	const PD_Style* p_pds;
	const XML_Char* szName;
	const XML_Char* szValue;
	const PP_AttrProp * pAP_style = NULL;

	const XML_Char * szStyleName = NULL;

	for (size_t nthStyle = 0; m_pDocument->enumStyles (nthStyle, &szStyleName, &p_pds); nthStyle++)
	{
		PT_AttrPropIndex api = p_pds->getIndexAP();
		bool bHaveProp = m_pDocument->getAttrProp(api,&pAP_style);

		if(bHaveProp && pAP_style && p_pds->isUsed())
		{
			char * myStyleName = removeWhiteSpace ((const char *)szStyleName);

			if(UT_strcmp(myStyleName, "Heading1") == 0)
				m_pie->write("h1, ");
			else if(UT_strcmp(myStyleName, "Heading2") == 0)
				m_pie->write("h2, ");
			else if(UT_strcmp(myStyleName, "Heading3") == 0)
				m_pie->write("h3, ");
			else if(UT_strcmp(myStyleName, "BlockText") == 0)
				m_pie->write("blockquote, ");
			else if(UT_strcmp(myStyleName, "PlainText") == 0)
				m_pie->write("pre, ");
			else if(UT_strcmp(myStyleName, "Normal") == 0)
				m_pie->write("p, ");

			m_pie->write(".");			// generic class qualifier, CSS
			m_pie->write(myStyleName);
			m_pie->write("\r\n{");

			UT_uint32 i = 0, j = 0;
			while(pAP_style->getNthAttribute(i++, szName, szValue))
			{
				if(!is_CSS(static_cast<const char*>(szName)))
					continue;
				if(strstr(myStyleName, "List") && (
					strstr(szName, "margin") || UT_strcmp(szName, "text-indent") == 0))
					continue;
					// see line 770 of this file for reasoning behind skipping here

				m_pie->write("\r\n\t");
				m_pie->write(szName);
				m_pie->write(": ");
				m_pie->write(szValue);

				m_pie->write(";");
			}
			while(pAP_style->getNthProperty(j++, szName, szValue))
			{
				if(!is_CSS(static_cast<const char*>(szName)))
					continue;
				if(strstr(myStyleName, "List") && (
					strstr(szName, "margin") || UT_strcmp(szName, "text-indent") == 0))
					continue;
					// see line 770 of this file for reasoning behind skipping here

				m_pie->write("\r\n\t");
				m_pie->write(szName);
				m_pie->write(": ");
				if (UT_strcmp(szName, "font-family") == 0 &&
					(UT_strcmp(szValue, "serif") != 0 ||
					 UT_strcmp(szValue, "sans-serif") != 0 ||
					 UT_strcmp(szValue, "cursive") != 0 ||
					 UT_strcmp(szValue, "fantasy") != 0 ||
					 UT_strcmp(szValue, "monospace") != 0))
				{
					m_pie->write("\"");
					m_pie->write(szValue);
					m_pie->write("\"");
				}						// only quote non-keyword family names
				else
				{
				  if(strstr(szName, "color") && !IS_TRANSPARENT_COLOR(szValue))
				    {
				      m_pie->write("#");
				      m_pie->write(szValue);
				    }
				  else
				    {
				      m_pie->write(szValue);
				    }
				}

				m_pie->write(";");
			}
			FREEP(myStyleName);

			m_pie->write("\r\n}\r\n\r\n");
		}
	}

	m_pie->write("</style>\r\n");
	m_pie->write("</head>\r\n");
	m_pie->write("<body>");

	m_bFirstWrite = false;
}

s_HTML_Listener::s_HTML_Listener(PD_Document * pDocument,
				 IE_Exp_HTML * pie, bool is4):
  m_pDocument (pDocument), m_pie(pie), m_bInSection(false),
  m_bInBlock(false), m_bInSpan(false), m_bNextIsSpace(false),
  m_bWroteText(false), m_bFirstWrite(true), m_bIs4(is4),
  m_pAP_Span(0), m_iBlockType(0), m_iListDepth(0),
  m_iPrevListDepth(0), m_iImgCnt(0)
{
}

s_HTML_Listener::~s_HTML_Listener()
{
	_closeSpan();
	_closeTag();
	if(m_iListDepth > 0)
	{
		m_pie->write("</li>\r\n");
		if(m_iBlockType == BT_NUMBEREDLIST)
			m_pie->write("</ol>\r\n");
		else
			m_pie->write("</ul>\r\n");
	}
	_closeSection();
	_handleDataItems();

	UT_uint16 *popped;
	while(m_utsListType.pop((void**) &popped))
	{
		DELETEP(popped);
	}

	UT_VECTOR_FREEALL(char*, m_utvDataIDs);

	m_pie->write("</body>\r\n");
	m_pie->write("</html>\r\n");
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
				UT_ASSERT_NOT_REACHED();
				return false;
			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;

	default:
		UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_HTML_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
										   const PX_ChangeRecord * pcr,
										   PL_StruxFmtHandle * psfh)
{
	UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);
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
		UT_ASSERT_NOT_REACHED();
		return false;
	}
}

bool s_HTML_Listener::change(PL_StruxFmtHandle /*sfh*/,
									const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT_NOT_REACHED();						// this function is not used.
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
	UT_ASSERT_NOT_REACHED();						// this function is not used.
	return false;
}

bool s_HTML_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT_NOT_REACHED();
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
#ifndef LEGIONS
			UT_String_sprintf(fname, "%s_data", m_pie->getFileName());
#else
			fname = "images";
#endif
			UT_sint32 result = m_pDocument->getApp()->makeDirectory(fname.c_str(), 0750);
			if (result < 0)
			{
				UT_DEBUGMSG(("Failed to create directory\n"));
				/* we might want to return an error here, 
				 * but I don't think so. */
			}

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
