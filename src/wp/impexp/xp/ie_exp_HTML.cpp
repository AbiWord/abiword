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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_hash.h"
#include "ut_units.h"
#include "ut_wctomb.h"
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

#include "ut_debugmsg.h"
#include "ut_string_class.h"

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Exp_HTML_Sniffer * m_sniffer = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Exp_HTML_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	mi->name = "HTML Exporter";
	mi->desc = "Export HTML Documents";
	mi->version = "0.7.15";
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	UT_ASSERT (m_sniffer);

	IE_Exp::unregisterExporter (m_sniffer);
	if (!m_sniffer->unref())
	{
		m_sniffer = 0;
	}

	return 1;
}

ABI_FAR extern "C"
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return SUPPORTS_ABI_VERSION(major, minor, release);
}

#endif

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
	*pszDesc = "XHTML (.html, .htm, .xhtml)";
	*pszSuffixList = "*.html; *.htm; *.xhtml";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

IE_Exp_HTML::IE_Exp_HTML(PD_Document * pDocument)
	: IE_Exp(pDocument)
{
	m_error = 0;
	m_pListener = NULL;
}

IE_Exp_HTML::~IE_Exp_HTML()
{
}

/*****************************************************************/
/*****************************************************************/

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
						IE_Exp_HTML * pie);
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
	void				_convertFontSize(char* szDest, const char* pszFontSize);
	void				_convertColor(char* szDest, const char* pszColor);
	void				_storeStyles(void);
	
	PD_Document *		m_pDocument;
	IE_Exp_HTML *		m_pie;
	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bNextIsSpace;
	bool				m_bInList;
	bool				m_bWroteText;
	bool				m_bFirstWrite;
	const PP_AttrProp*	m_pAP_Span;
	UT_HashTable*		m_pStylesHash;

	// Need to look up proper type, and place to stick #defines...
  
	UT_uint16		m_iBlockType;	// BT_*
	UT_uint16		m_iListDepth;	// 0 corresponds to not in a list
        UT_uint16               m_iImgCnt;
	UT_Wctomb		m_wmctomb;
};

/*!	This function copies a string to a new string, removing all the white
	space in the process.  Note that this function allocates the new
	string (and so the caller must make sure to deallocate it).
 */

static char* removeWhiteSpace(const char * text)
{
	char* temp = static_cast<char *>(calloc(strlen(text)+1, sizeof(char)));
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

extern "C" { // for MRC compiler (Mac)
	static int s_str_compare (const void * a, const void * b)
	{
		const char * a1 = (const char *)a;
		const char * b1 = (const char *)b;
		UT_DEBUGMSG(("DOM: comparing %s && %s\n", a1, b1));
		return UT_strcmp (a1, b1);
	}
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
										"orphans", "text-align", 
										"text-decoration", "text-indent",
										"widows", "width"};

#if 0
	// TODO: why doesn't this work? make this work
	const char * prop = (const char *) bsearch (property, prop_list, 
												sizeof (prop_list)/sizeof(prop_list[0]),
												sizeof (char *), 
												s_str_compare);

	return ((prop != NULL) ? true : false);
#else
	#define PropListLen sizeof(prop_list)/sizeof(prop_list[0])

	for (UT_uint32 i = 0; i < PropListLen; i++) 
	{
		if (!UT_strcmp (property, prop_list[i]))
			return true;
	}
	return false;

	#undef PropListLen
#endif
}


void s_HTML_Listener::_closeSection(void)
{
	if (!m_bInSection)
	{
		return;
	}
	
	m_pie->write("</div>\n");
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
       	m_pie->write("</p>\n");
		if(!m_bWroteText)
		{
			m_pie->write("<br />\n");
		}
	}

	else if(m_iBlockType == BT_HEADING1)
		m_pie->write("</h1>\n");

	else if(m_iBlockType == BT_HEADING2)
		m_pie->write("</h2>\n");

	else if(m_iBlockType == BT_HEADING3)
		m_pie->write("</h3>\n");

	else if(m_iBlockType == BT_BLOCKTEXT)
		m_pie->write("</blockquote>\n");

	else if(m_iBlockType == BT_PLAINTEXT)
		m_pie->write("</pre>\n");

	else if(m_iBlockType == BT_NUMBEREDLIST || m_iBlockType == BT_BULLETLIST)
	{	/* do nothing, lists are handled differently, as they have multiple tags */ }

        // Add "catchall" for now

	else
	  m_pie->write("</p>\n");

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
		//const XML_Char * szLevel;
		const XML_Char * szListID;

		if (
		   (pAP->getAttribute("style", szValue))
		   )
		{
			char * value = removeWhiteSpace((char*) szValue);
			if(pAP->getAttribute("listid", szListID) &&
			   0 != UT_strcmp(szListID, "0"))
			{	// we're in a list
				if(!m_bInList)
				{
					if(0 != UT_strcmp(value, "BulletList"))
					{
						m_iBlockType = BT_NUMBEREDLIST;
						m_pie->write("<ol class=\"");
					}
					else 
					{
						m_iBlockType = BT_BULLETLIST;
						m_pie->write("<ul class=\"");
					}
					_outputInheritanceLine((const char*) value);
					m_pie->write("\">\n");
					m_bInList = true;
				}
				else
				{
					m_pie->write("</li>\n");
				}
				m_pie->write("<li");
				wasWritten = true;	
				DELETEPV(value);			
			}
			else 
			{
				if(m_bInList)
				{	// we're no longer in a list, close it
					if(m_iBlockType == BT_NUMBEREDLIST)
						m_pie->write("</li>\n</ol>\n");
					else if(m_iBlockType == BT_BULLETLIST)
						m_pie->write("</li>\n</ul>\n");
					m_bInList = false;
				}

				if(0 == UT_strcmp(szValue, "Heading1") ||
					_inherits((const char*) szValue, "Heading1")) 
				{
					// <p style="Heading 1"> ...

					m_iBlockType = BT_HEADING1;
					m_pie->write("\n<h1");
					if(_inherits((const char*) szValue, "Heading1"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(0 == UT_strcmp(szValue, "Heading2") ||
					_inherits((const char*) szValue, "Heading2")) 
				{
					// <p style="Heading 2"> ...

					m_iBlockType = BT_HEADING2;
					m_pie->write("\n<h2");
					if(_inherits((const char*) szValue, "Heading2"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(0 == UT_strcmp(szValue, "Heading3") ||
					_inherits((const char*) szValue, "Heading3")) 
				{
					// <p style="Heading 3"> ...

					m_iBlockType = BT_HEADING3;
					m_pie->write("\n<h3");
					if(_inherits((const char*) szValue, "Heading3"))
					{
						m_pie->write(" class=\"");
						_outputInheritanceLine((const char*) szValue);
						m_pie->write("\"");
					}
					wasWritten = true;
				}
				else if(0 == UT_strcmp(szValue, "BlockText") || 
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
				else if(0 == UT_strcmp(szValue, "PlainText") ||
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
				else 
				{
					// <p style="<anything else!>"> ...

			        m_iBlockType = BT_NORMAL;
			      	m_pie->write("<p class=\"");
					m_pie->write(szValue);
					m_pie->write("\"");
					wasWritten = true;
				}	
			}
		}
		else 
		{
			if(m_bInList)
			{	// we're no longer in a list, close it
				if(m_iBlockType == BT_NUMBEREDLIST)
					m_pie->write("</li>\n</ol>\n");
				else if(m_iBlockType == BT_BULLETLIST)
					m_pie->write("</li>\n</ul>\n");
				m_bInList = false;
			}
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
				|| pAP->getProperty("margin-top", szValue)) )
		{
			m_pie->write(" style=\"");

			if(pAP->getProperty("text-align", szValue))
			{
				m_pie->write("text-align: ");
				m_pie->write((char*)szValue);
				css = true;
			}
			if(pAP->getProperty("margin-bottom", szValue))
			{
			        if (css)
				     m_pie->write("; margin-bottom: ");
				else 
			             m_pie->write("margin-bottom: ");
				m_pie->write((char*)szValue);
				css = true;
			}
			if(pAP->getProperty("margin-top", szValue))
			{
			        if (css)
				     m_pie->write("; margin-top: ");
				else
				     m_pie->write("margin-top: ");
				m_pie->write((char*)szValue);
				css = true;
			}

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
	  m_pie->write(">");

	m_bInBlock = true;
}

void s_HTML_Listener::_openSection(PT_AttrPropIndex api)
{
	if (m_bFirstWrite)
	{
		_outputBegin(api);
	}

	m_pie->write("<div>\n");
}

void s_HTML_Listener::_convertColor(char* szDest, const char* pszColor)
{
	/*
	  TODO we might want to be a little more careful about this.
	  The proper HTML color is #rrggbb, which is basically the same
	  as what we use this.  HTML browsers are likely to be more
	  forgiving than we are, so this is probably not a big
	  problem.
	*/
	strcpy(szDest, pszColor);
}

void s_HTML_Listener::_convertFontSize(char* szDest, const char* pszFontSize)
{
	double fSizeInPoints = UT_convertToPoints(pszFontSize);

	/*
	  TODO we can probably come up with a mapping of font sizes that
	  is more accurate than the code below.  I just guessed.
	*/
	
	if (fSizeInPoints <= 7)
	{
		strcpy(szDest, "1");
	}
	else if (fSizeInPoints <= 10)
	{
		strcpy(szDest, "2");
	}
	else if (fSizeInPoints <= 12)
	{
		strcpy(szDest, "3");
	}
	else if (fSizeInPoints <= 16)
	{
		strcpy(szDest, "4");
	}
	else if (fSizeInPoints <= 24)
	{
		strcpy(szDest, "5");
	}
	else if (fSizeInPoints <= 36)
	{
		strcpy(szDest, "6");
	}
	else
	{
		strcpy(szDest, "7");
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
	
	m_bWroteText = true;
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);
	
	bool span = false;
	bool textD = false;
	
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
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("<sup>");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("<sub>");
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
					m_pie->write("<span style=\"color:#");	
					char szColor[16];
					_convertColor(szColor,(char*)pszColor);
					m_pie->write(szColor);
					span = true;
				}
				else 
				{
					m_pie->write("; color:#");	
					char szColor[16];
					_convertColor(szColor,(char*)pszColor);
					m_pie->write(szColor);
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
			
			char szSize[4];

			if (pszFontSize)
			{
				if (!span)
				{
					m_pie->write("<span style=\"font-size: ");	
					sprintf(szSize, "%f", UT_convertToPoints(pszFontSize));
					m_pie->write(szSize);
					m_pie->write("pt");
					span = true;
				}
				else 
				{
					m_pie->write("; font-size: ");	
					sprintf(szSize, "%f", UT_convertToPoints(pszFontSize));
					m_pie->write(szSize);
					m_pie->write("pt");
				}
			}
			
			if (pszBgColor)
			{
				if (!span)
				{
					m_pie->write("<span style=\"background: #");	
					char szColor[16];
					_convertColor(szColor,(char*)pszBgColor);
					m_pie->write(szColor);
					span = true;
				}
				else 
				{
					m_pie->write("; background: #");	
					char szColor[16];
					_convertColor(szColor,(char*)pszBgColor);
					m_pie->write(szColor);
				}
			}
		}
		
		char* szStyle = NULL;
		const XML_Char * pStyle;
		pAP->getAttribute("style", pStyle);
		if(pStyle)
		{
			szStyle = removeWhiteSpace((const char *)pStyle);
		}
		
		if (span)
		{
			m_pie->write("\"");
			if(szStyle)
			{
				m_pie->write(" class=\"");
				_outputInheritanceLine(szStyle);
				m_pie->write("\"");
			}
			m_pie->write(">");
		}
		else if(szStyle)
		{
			m_pie->write("<span class=\"");
			_outputInheritanceLine(szStyle);
			m_pie->write("\">");
		}
		else
		{
			m_pie->write("<span>");
		}
		DELETEPV(szStyle);
		
		m_bInSpan = true;
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
			)
		{
		  closeSpan = true;
		}

		if (pAP->getProperty("text-position", szValue))
		{
			if (!UT_strcmp("superscript", szValue))
			{
				m_pie->write("</sup>");
			}
			else if (!UT_strcmp("subscript", szValue))
			{
				m_pie->write("</sub>");
			}
		}

		if (
			(pAP->getProperty("text-decoration", szValue))
			&& UT_strcmp(szValue, "none")
			)
		{
		  closeSpan = true;
		}

		if (
			(pAP->getProperty("font-style", szValue))
			&& !UT_strcmp(szValue, "italic")
			)
		{
		  closeSpan = true;
		}
		
		if (
			(pAP->getProperty("font-weight", szValue))
			&& !UT_strcmp(szValue, "bold")
			)
		{
		  closeSpan = true;
		}

		if(pAP->getAttribute("style", szValue))
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
	m_bWroteText = true;

	UT_String sBuf;
	const UT_UCSChar * pData;

	if (!m_bInBlock)
	{
		return;
	}

	UT_ASSERT(sizeof(UT_Byte) == sizeof(char));

	for (pData=data; (pData<data+length); /**/)
	{
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
		  // tabs get treated as a single space
		  //
		  if(m_bNextIsSpace)
		    {
				sBuf += "&nbsp;";
				pData++;
		    }
		  else
		    {
		      // just tack on a single space to the textrun
				sBuf += " ";
				pData++;
		    }
		  break;

		case UCS_LF:			// LF -- representing a Forced-Line-Break
			sBuf += "<br />";
			pData++;
			break;

		case UCS_RQUOTE:				// Smart quotes get translated
			sBuf += "\'";				// back into normal quotes
			pData++;
			break;						// TODO: This handles apostrophes
										// (smart single right quotes)
										// what about the other types?
		default:
			if (*pData > 0x007f)
			{
				if(XAP_EncodingManager::get_instance()->isUnicodeLocale() || 
				   (XAP_EncodingManager::get_instance()->try_nativeToU(0xa1) == 0xa1))

				{
					XML_Char * pszUTF8 = UT_encodeUTF8char(*pData++);
					while (*pszUTF8)
					{
						sBuf += (char)*pszUTF8;
						pszUTF8++;
					}
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
						char localBuf[20];
						char * plocal = localBuf;
						sprintf(localBuf,"&#x%x;",*pData++);
						sBuf += plocal;
					}
					else
					{
						sBuf += (char)c;
						pData++;
					}
				}
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

bool s_HTML_Listener::_inherits(const char* style, const char* from)
{
	bool bret = false;
	UT_HashTable::HashValType p_uthe = m_pStylesHash->pick((UT_HashTable::HashKeyType)style);
	if(p_uthe)
	{
		PD_Style* pStyle = static_cast<PD_Style*>(p_uthe);
		char* szName = NULL;
		const XML_Char * pName = NULL;

		if(pStyle && pStyle->getBasedOn())
		{
			pStyle = pStyle->getBasedOn();
			pStyle->getAttribute(PT_NAME_ATTRIBUTE_NAME, 
								 pName);
			szName = removeWhiteSpace(pName);

			if(UT_strcmp(from, szName) == 0)
				bret = true;

			DELETEPV(szName);
		}
	}

	return bret;
}

void s_HTML_Listener::_outputInheritanceLine(const char* ClassName)
{
	UT_HashTable::HashValType p_uthe = m_pStylesHash->pick((UT_HashTable::HashKeyType)ClassName);
	PD_Style* pStyle = NULL;
	PD_Style* pBasedOn = NULL;
	const XML_Char* szName = NULL;

	if(p_uthe)
	{
		pStyle = static_cast<PD_Style*>(p_uthe);
	}
	if(pStyle)
	{
		pBasedOn = pStyle->getBasedOn();
		if(pBasedOn)
		{
			pBasedOn->getAttribute(PT_NAME_ATTRIBUTE_NAME, szName);

			UT_ASSERT((szName));
			char * pName = removeWhiteSpace((const char*) szName);
			_outputInheritanceLine(pName);
			DELETEPV(pName);
			m_pie->write(" ");
		}
	}

	m_pie->write(ClassName);
}

void s_HTML_Listener::_outputBegin(PT_AttrPropIndex api)
{
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	if (!XAP_EncodingManager::get_instance()->cjk_locale())
	{
	    m_pie->write("<?xml version=\"1.0\" encoding=\"");
	    m_pie->write(XAP_EncodingManager::get_instance()->getNativeEncodingName());
	    m_pie->write("\"?>\n");
	}
	else
	{
	    m_pie->write("<?xml version=\"1.0\"?>\n");
	};
	m_pie->write("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml/DTD/xhtml1-strict.dtd\">\n");

	m_pie->write("<!-- ================================================================================  -->\n");
	m_pie->write("<!-- This HTML file was created by AbiWord.                                            -->\n");
	m_pie->write("<!-- AbiWord is a free, Open Source word processor.                                    -->\n");
	m_pie->write("<!-- You may obtain more information about AbiWord at www.abisource.com                -->\n");
	m_pie->write("<!-- ================================================================================  -->\n");
	m_pie->write("\n");

	if (XAP_App::s_szBuild_ID && XAP_App::s_szBuild_ID[0])
	{
		m_pie->write("<!--         Build_ID          = ");
		m_pie->write(XAP_App::s_szBuild_ID);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_Version && XAP_App::s_szBuild_Version[0])
	{
		m_pie->write("<!--         Build_Version     = ");
		m_pie->write(XAP_App::s_szBuild_Version);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_Options && XAP_App::s_szBuild_Options[0])
	{
		m_pie->write("<!--         Build_Options     = ");
		m_pie->write(XAP_App::s_szBuild_Options);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_Target && XAP_App::s_szBuild_Target[0])
	{
		m_pie->write("<!--         Build_Target      = ");
		m_pie->write(XAP_App::s_szBuild_Target);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_CompileTime && XAP_App::s_szBuild_CompileTime[0])
	{
		m_pie->write("<!--         Build_CompileTime = ");
		m_pie->write(XAP_App::s_szBuild_CompileTime);
		m_pie->write(" -->\n");
	}
	if (XAP_App::s_szBuild_CompileDate && XAP_App::s_szBuild_CompileDate[0])
	{
		m_pie->write("<!--         Build_CompileDate = ");
		m_pie->write(XAP_App::s_szBuild_CompileDate);
		m_pie->write(" -->\n");
	}
	
	m_pie->write("\n");	
	m_pie->write("<html xmlns=\"http://www.w3.org/1999/xhtml\">\n");
	m_pie->write("<head>\n");
	m_pie->write("<meta http-equiv=\"content-type\" content=\"text/html; charset=");
	m_pie->write(XAP_EncodingManager::get_instance()->getNativeEncodingName());
	m_pie->write("\" />\n");
	m_pie->write("<title>");
	m_pie->write(m_pie->getFileName());
	m_pie->write("</title>\n");
	m_pie->write("<style type=\"text/css\">\n");
	m_pie->write("<!--\n");

	_storeStyles();

	PD_Style* p_pds;
	const XML_Char* szName;
	const XML_Char* szValue;
	const PP_AttrProp * pAP_style = NULL;

    UT_HashTable::UT_HashCursor c(m_pStylesHash);
	UT_HashTable::HashValType entry = c.first();

	while (true)
	{
		p_pds = static_cast<PD_Style*>(entry);
		UT_ASSERT((p_pds));

		PT_AttrPropIndex api = p_pds->getIndexAP();
		bool bHaveProp = m_pDocument->getAttrProp(api,&pAP_style);

		if(bHaveProp && pAP_style /*&& p_pds->isUsed()*/)
		{	
			/*	The isUsed() test above is commented out because
			 *	it's currently broken.  I'd like to use it when
			 *	it gets fixed, but all the functionality remains,
			 *	regardless.
			 */
			if(UT_strcmp(c.key(), "Heading1") == 0)
				m_pie->write("h1, ");
			else if(UT_strcmp(c.key(), "Heading2") == 0)
				m_pie->write("h2, ");
			else if(UT_strcmp(c.key(), "Heading3") == 0)
				m_pie->write("h3, ");
			else if(UT_strcmp(c.key(), "BlockText") == 0)
				m_pie->write("blockquote, ");
			else if(UT_strcmp(c.key(), "PlainText") == 0)
				m_pie->write("pre, ");
			else if(UT_strcmp(c.key(), "Normal") == 0)
				m_pie->write("p, ");

			m_pie->write(".");			// generic class qualifier, CSS
			m_pie->write(c.key());
			m_pie->write("\n{");

			UT_uint32 i = 0, j = 0;
			while(pAP_style->getNthAttribute(i++, szName, szValue))
			{
				if(!is_CSS(static_cast<const char*>(szName)))
					continue;
				
				m_pie->write("\n\t");
				m_pie->write(szName);
				m_pie->write(": ");
				m_pie->write(szValue);
				
				m_pie->write(";");
			}
			while(pAP_style->getNthProperty(j++, szName, szValue))
			{
				if(!is_CSS(static_cast<const char*>(szName)))
					continue;
				m_pie->write("\n\t");
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
					if(strstr(szName, "color"))
						m_pie->write("#");
					m_pie->write(szValue);
				}
				
				m_pie->write(";");
			}

			m_pie->write("\n}\n\n");
		}

		if (!c.more())
			break;
		entry = (UT_HashTable::HashValType)c.next();
	}
		
	m_pie->write("-->\n</style>\n");
	m_pie->write("</head>\n");
	m_pie->write("<body");
	if(bHaveProp && pAP)
	{								// global page styles go in the <body> tag
		const XML_Char * szValue;

			m_pie->write(" style=\"");

		szValue = PP_evalProperty("background-color",
			NULL, NULL, pAP, m_pDocument, true);
			m_pie->write("background-color: ");
			char color[16];
			_convertColor(color, szValue);
			if (*szValue != '#')
			  m_pie->write("#");
			m_pie->write(color);

		szValue = PP_evalProperty("page-margin-top",
			NULL, NULL, pAP, m_pDocument, true);
			m_pie->write(";\n    margin-top: ");
			m_pie->write(szValue);

		szValue = PP_evalProperty("page-margin-bottom",
			NULL, NULL, pAP, m_pDocument, true);
			m_pie->write("; margin-bottom: ");
			m_pie->write(szValue);

		szValue = PP_evalProperty("page-margin-left",
			NULL, NULL, pAP, m_pDocument, true);
			m_pie->write("; margin-left: ");
			m_pie->write(szValue);

		szValue = PP_evalProperty("page-margin-right",
			NULL, NULL, pAP, m_pDocument, true);
			m_pie->write("; margin-right: ");
			m_pie->write(szValue);

			m_pie->write("\"");
	}
	m_pie->write(">\n");

	m_bFirstWrite = false;
}

s_HTML_Listener::s_HTML_Listener(PD_Document * pDocument,
										 IE_Exp_HTML * pie)
{
	m_pDocument = pDocument;
	m_pie = pie;
	m_bInSection = false;
	m_bInBlock = false;
	m_bInSpan = false;
	m_bNextIsSpace = false;
	m_bInList = false;
	m_bWroteText = false;
	m_bFirstWrite = true;
	m_iListDepth = 0;
	m_iImgCnt = 0;
	m_pStylesHash = NULL;
}

s_HTML_Listener::~s_HTML_Listener()
{
	_closeSpan();
	_closeTag();
	if(m_bInList)
	{
		m_pie->write("</li>\n");
		if(m_iBlockType == BT_NUMBEREDLIST)
			m_pie->write("</ol>\n");
		else
			m_pie->write("</ul>\n");
	}
	_closeSection();
	_handleDataItems();
	
	DELETEP(m_pStylesHash);

	m_pie->write("</body>\n");
	m_pie->write("</html>\n");
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
			char buf[16];

			fd_Field* field;
			PT_AttrPropIndex api = pcr->getIndexAP();
			const PP_AttrProp * pAP = NULL;
			bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

			switch (pcro->getObjectType())
			{
			case PTO_Image:
				// TODO: differentiate between SVG and PNG
				// TODO: we do this in the img saving code
	                        sprintf(buf, "-%d.png", m_iImgCnt++);
				m_pie->write("<img alt=\"AbiWord Image");
				m_pie->write(buf);
				m_pie->write("\" src=\"");
				m_pie->write(m_pie->getFileName());
				m_pie->write(buf);
				m_pie->write("\" />\n");
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
				m_bInSection = false;
			}
		}
		else
		{
			m_bInSection = false;
		}
		
		return true;
	}

	case PTX_SectionHdrFtr:
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
				m_bInSection = false;
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
	m_pListener = new s_HTML_Listener(m_pDocument,this);
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	if (!m_pDocument->tellListener(static_cast<PL_Listener *>(m_pListener)))
		return UT_ERROR;
	delete m_pListener;

	m_pListener = NULL;
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}

/*****************************************************************/
/*****************************************************************/

void s_HTML_Listener::_handleDataItems(void)
{
	const char * szName;
   	const char * szMimeType;
	const UT_ByteBuf * pByteBuf;

	for (UT_uint32 k=0; (m_pDocument->enumDataItems(k,NULL,&szName,&pByteBuf,(void**)&szMimeType)); k++)
	{	  	  
	  FILE *fp;
	  char fname [1024]; // EVIL EVIL bad hardcoded buffer size
	  
	  if (!UT_strcmp(szMimeType, "image/svg-xml"))
	      sprintf(fname, "%s-%d.svg", m_pie->getFileName(), k);
	  if (!UT_strcmp(szMimeType, "text/mathml"))
	    sprintf(fname, "%s-%d.mathml", m_pie->getFileName(), k);
	  else // PNG Image
	    sprintf(fname, "%s-%d.png", m_pie->getFileName(), k);
	  
	  fp = fopen (fname, "wb+");
	  
	  if(!fp)
	    continue;
	  
	  int cnt = 0, len = pByteBuf->getLength();
	  
	  while (cnt < len)
	    {
	      xxx_UT_DEBUGMSG(("DOM: len: %d cnt: %d\n", len, cnt));
	      cnt += fwrite (pByteBuf->getPointer(cnt), sizeof(UT_Byte), len-cnt, fp);
	    }
	  
	  fclose(fp);
	}
	
	return;
}

/*!	This function gets a listing of all the styles from
 *	the document and stores them by their compressed name (without spaces)
 *	in the hash table m_pStylesHash.  This function allocates
 *	the hash table if it is NULL.
 */

void s_HTML_Listener::_storeStyles(void)
{
	const char* pszName;
	const PD_Style* pStyle;
	void* pData;
	size_t count = m_pDocument->getStyleCount();

	if(m_pStylesHash == NULL)
	{
		m_pStylesHash = new UT_HashTable(count);
	}

	for(int i = 0; m_pDocument->enumStyles(i, &pszName, &pStyle); i++)
	{
		pData = reinterpret_cast<void*>(const_cast<PD_Style*>(pStyle));
		char * szName = removeWhiteSpace(pszName);
		m_pStylesHash->insert(szName, pData);
		DELETEPV(szName);
	}

	return;
}
