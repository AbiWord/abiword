/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <locale.h>
#include <ctype.h>

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_types.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_debugmsg.h"
#include "ut_map.h"
#include "ut_pair.h"
#include "ut_path.h"

#include "xap_EncodingManager.h"

#include "pd_Style.h"
#include "pd_Document.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

#include "fd_Field.h"
#include "fl_AutoNum.h"
#include "fp_PageSize.h"

#include "ap_Prefs.h"

#include "ie_exp_XSL-FO.h"

/*****************************************************************/
/*****************************************************************/

// A little class to help export lists

class ListHelper
{
public:
	explicit ListHelper(UT_uint32 id = 0)
		: m_pan(0),
		  m_iNextNb(0),
		  m_iInc(0)
	{
		setIdList(id);
	}

	void setIdList(UT_uint32 id)
	{
		UT_Map::Iterator it(myLists.find(reinterpret_cast<UT_Map::key_t> (id)));

		if (it.is_valid())
		{
			m_pan = static_cast<const fl_AutoNum*> (static_cast<const UT_Pair*> (it.value())->second());
			UT_ASSERT(m_pan);
			m_iNextNb = m_pan->getStartValue32();

			if (m_pan->getType() >= BULLETED_LIST)
				m_iInc = 0;
			else
				m_iInc = 1;
		}
	}

	UT_String getNextLabel()
	{
		UT_ASSERT(m_pan);
		UT_uint32 next_nb = m_iNextNb;
		m_iNextNb += m_iInc;
		// TODO: I'm laking pre and post text
		return int2label(next_nb);
	}
	
    static void addList(fl_AutoNum* pAutoNum) 
	{
		myLists.insert(reinterpret_cast<void*> (pAutoNum->getID()), pAutoNum);
	}

private:
	UT_String int2label(UT_uint32 i)
	{
		return UT_String_sprintf ("%lu", static_cast<unsigned long>(i));
	}

	const fl_AutoNum* m_pan;
	UT_uint32 m_iNextNb;
	UT_uint32 m_iInc;
    static UT_Map myLists;
};

UT_Map  ListHelper::myLists;

/************************************************************/

#define TT_OTHER               0 // ? Tag not recognized (not an error, though)

#define TT_ROOT		           1 // <root> Document main/first tag
#define TT_FLOW                2 // <flow>
#define TT_BLOCK               3 // <block>
#define TT_INLINE              4 // <inline>

#define TT_LIST_BLOCK          5 // <list-block>
#define TT_LIST_ITEM           6 // <list-item>
#define TT_LIST_ITEM_LABEL     7 // <list-item-label>
#define TT_LIST_ITEM_BODY      8 // <list-item-body>

#define TT_EXTERNAL_GRAPHIC   12 // <external-graphic>
#define TT_STATIC_CONTENT     13 // <static-content>
#define TT_LAYOUT_MASTER_SET  14 // <layout-master-set>
#define TT_SIMPLE_PAGE_MASTER 15 // <simple-page-master>
#define TT_REGION_BODY        16 // <region-body>
#define TT_PAGE_SEQUENCE      17 // <page-sequence>

/************************************************************/

class s_XSL_FO_Listener : public PL_Listener
{
public:
	s_XSL_FO_Listener(PD_Document * pDocument,
					  IE_Exp_XSL_FO * pie);
	virtual ~s_XSL_FO_Listener();

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
	void                _handlePageSize(PT_AttrPropIndex api);
	void				_writeImage (const UT_ByteBuf * pByteBuf,
									 const UT_String & imagedir,
									 const UT_String & filename);
	void				_handleImage(PT_AttrPropIndex api);
	void				_handleLists();
	void				_handleField(PT_AttrPropIndex api);
	void                _outputData(const UT_UCSChar * data, UT_uint32 length);

	void				_closeSection();
	void				_closeBlock();
	void				_closeSpan();
	void				_openBlock(PT_AttrPropIndex api);
	void				_openSection(PT_AttrPropIndex api);
	void				_openSpan(PT_AttrPropIndex api);

	enum WhiteSpace
	{
		ws_None = 0,
		ws_Pre  = 1,
		ws_Post = 2,
		ws_Both = 3
	};

	/* these may use m_utf8_0
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

	void			textTrusted (const UT_UTF8String & text);
	void			textUntrusted (const char * text);

	/* temporary strings; use with extreme caution
	 */
	UT_UTF8String	m_utf8_0; // low-level
	UT_UTF8String	m_utf8_xmlns;
	UT_UTF8String	m_utf8_span;

	UT_Stack		m_tagStack;

private:
	PD_Document *		m_pDocument;
	IE_Exp_XSL_FO *	    m_pie;
	ListHelper			m_List;

	bool				m_bInSection;
	bool				m_bInBlock;
	bool				m_bInSpan;
	bool				m_bFirstWrite;
	int                 m_iImgCnt;
};

/*****************************************************************/
/*****************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

ABI_PLUGIN_DECLARE("XSL-FO")

// we use a reference-counted sniffer
static IE_Exp_XSL_FO_Sniffer * m_sniffer = 0;

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
		m_sniffer = new IE_Exp_XSL_FO_Sniffer ();
	else
		m_sniffer->ref();

	mi->name = "XSL-FO Exporter";
	mi->desc = "Export XSL-FO Documents";
	mi->version = ABI_VERSION_STRING;
	mi->author = "Abi the Ant";
	mi->usage = "No Usage";

	IE_Exp::registerExporter (m_sniffer);
	return 1;
}

ABI_FAR_CALL
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
		m_sniffer = 0;

	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
								 UT_uint32 release)
{
	return 1;
}

#endif

/*****************************************************************/
/*****************************************************************/

IE_Exp_XSL_FO::IE_Exp_XSL_FO(PD_Document * pDocument)
	: IE_Exp(pDocument),
	  m_pListener(0),
	  m_error(UT_OK)
{
}

IE_Exp_XSL_FO::~IE_Exp_XSL_FO()
{
}

/*****************************************************************/
/*****************************************************************/

bool IE_Exp_XSL_FO_Sniffer::recognizeSuffix(const char * szSuffix)
{
	return (!UT_stricmp(szSuffix, ".fo"));
}

UT_Error IE_Exp_XSL_FO_Sniffer::constructExporter(PD_Document * pDocument,
											   IE_Exp ** ppie)
{
	IE_Exp_XSL_FO * p = new IE_Exp_XSL_FO(pDocument);
	*ppie = p;
	return UT_OK;
}

bool IE_Exp_XSL_FO_Sniffer::getDlgLabels(const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEFileType * ft)
{
	*pszDesc = "XSL-FO (.fo)";
	*pszSuffixList = "*.fo";
	*ft = getFileType();
	return true;
}

/*****************************************************************/
/*****************************************************************/

UT_Error IE_Exp_XSL_FO::_writeDocument()
{
	m_pListener = new s_XSL_FO_Listener(getDoc(),this);
	if (!m_pListener)
		return UT_IE_NOMEMORY;
	if (!getDoc()->tellListener(static_cast<PL_Listener *>(m_pListener)))
		return UT_ERROR;
	delete m_pListener;

	m_pListener = NULL;
	
	return ((m_error) ? UT_IE_COULDNOTWRITE : UT_OK);
}  

/*****************************************************************/
/*****************************************************************/

void s_XSL_FO_Listener::tagNewIndent (UT_uint32 extra)
{
	m_utf8_0 = "";

	UT_uint32 depth = m_tagStack.getDepth () + extra;

	UT_uint32 i;  // MSVC DOES NOT SUPPORT CURRENT for SCOPING RULES!!!
	for (i = 0; i < (depth >> 3); i++) m_utf8_0 += "\t";
	for (i = 0; i < (depth &  7); i++) m_utf8_0 += " ";
}

void s_XSL_FO_Listener::tagOpenClose (const UT_UTF8String & content, bool suppress,
									  WhiteSpace ws)
{
	if (ws & ws_Pre)
		tagNewIndent ();
	else
		m_utf8_0 = "";

	m_utf8_0 += "<";

	if (!m_utf8_xmlns.empty ())
		{
			m_utf8_0 += m_utf8_xmlns;
			m_utf8_0 += ":";
		}
	m_utf8_0 += content;
	if (suppress)
		m_utf8_0 += ">";
	else
		m_utf8_0 += " />";

	if (ws & ws_Post) m_utf8_0 += "\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_XSL_FO_Listener::tagOpen (UT_uint32 tagID, const UT_UTF8String & content,
								 WhiteSpace ws)
{
	if (ws & ws_Pre)
		tagNewIndent ();
	else
		m_utf8_0 = "";

	m_utf8_0 += "<";

	if (!m_utf8_xmlns.empty ())
		{
			m_utf8_0 += m_utf8_xmlns;
			m_utf8_0 += ":";
		}
	m_utf8_0 += content;
	m_utf8_0 += ">";

	if (ws & ws_Post) m_utf8_0 += "\r\n";

	m_pie->write (m_utf8_0.utf8_str ());

	void * vptr = reinterpret_cast<void *>(tagID);
	m_tagStack.push (vptr);
}

void s_XSL_FO_Listener::tagClose (UT_uint32 tagID, const UT_UTF8String & content,
								  WhiteSpace ws)
{
	tagClose (tagID);

	if (ws & ws_Pre)
		tagNewIndent ();
	else
		m_utf8_0 = "";

	m_utf8_0 += "</";

	if (!m_utf8_xmlns.empty ())
		{
			m_utf8_0 += m_utf8_xmlns;
			m_utf8_0 += ":";
		}
	m_utf8_0 += content;
	m_utf8_0 += ">";

	if (ws & ws_Post) m_utf8_0 += "\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_XSL_FO_Listener::tagClose (UT_uint32 tagID)
{
	void * vptr = 0;
	m_tagStack.pop (&vptr);

	if (reinterpret_cast<UT_uint32>(vptr) == tagID) return;

	UT_DEBUGMSG(("WARNING: possible tag mis-match in XSL-FO output!\n"));
}

UT_uint32 s_XSL_FO_Listener::tagTop ()
{
	void * vptr = 0;
	if (m_tagStack.viewTop (&vptr)) return reinterpret_cast<UT_uint32>(vptr);
	return 0;
}

void s_XSL_FO_Listener::tagPI (const char * target, const UT_UTF8String & content)
{
	tagNewIndent ();

	m_utf8_0 += "<?";
	m_utf8_0 += target;
	m_utf8_0 += " ";
	m_utf8_0 += content;
	m_utf8_0 += "?>\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_XSL_FO_Listener::tagComment (const UT_UTF8String & content)
{
	tagNewIndent ();

	m_utf8_0 += "<!-- ";
	m_utf8_0 += content;
	m_utf8_0 += " -->\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_XSL_FO_Listener::tagCommentOpen ()
{
	tagNewIndent ();

	m_utf8_0 += "<!--\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_XSL_FO_Listener::tagCommentClose ()
{
	tagNewIndent (2);

	m_utf8_0 += "-->\r\n";

	m_pie->write (m_utf8_0.utf8_str ());
}

void s_XSL_FO_Listener::textTrusted (const UT_UTF8String & text)
{
	m_pie->write (text.utf8_str ());
}

void s_XSL_FO_Listener::textUntrusted (const char * text)
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

/*****************************************************************/
/*****************************************************************/

static const char * s_hdr[3] = {
	" This XSL-FO document was created by AbiWord        ",
	" AbiWord is a free, open source word processor      ",
	" See http://www.abisource.com/ for more information "
};

s_XSL_FO_Listener::s_XSL_FO_Listener(PD_Document * pDocument,
									 IE_Exp_XSL_FO * pie)
	: m_pDocument(pDocument),
	  m_pie (pie),
	  m_List(),
	  m_bInSection(false),
	  m_bInBlock(false),
	  m_bInSpan(false),
	  m_bFirstWrite(true),
	  m_iImgCnt(0)
{
	UT_UTF8String content("version=\"1.0\"");
	tagPI ("xml", content);

#ifndef XSL_FO_OMIT_NAMESPACE
	m_utf8_xmlns = "fo";
	content = "root xmlns:fo=\"http://www.w3.org/1999/XSL/Format\"";
#else
	content = "root xmlns=\"http://www.w3.org/1999/XSL/Format\"";
#endif
	tagOpen (TT_ROOT, content);

	for (int i = 0; i < 3; i++)
		{
			content = s_hdr[i];
			tagComment (content);
		}

	_handleLists();
}

s_XSL_FO_Listener::~s_XSL_FO_Listener()
{
	_closeSection();

	UT_UTF8String content("root");
	tagClose (TT_ROOT, content);
}

void s_XSL_FO_Listener::_handleLists()
{
	fl_AutoNum* pAutoNum;

	for (UT_uint32 k = 0; m_pDocument->enumLists(k, &pAutoNum); ++k)
	{
		if (pAutoNum->isEmpty() == true)
			continue;

		ListHelper::addList(pAutoNum);
	}
}

void s_XSL_FO_Listener::_handleField(PT_AttrPropIndex api)
{
	const PP_AttrProp* pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api, &pAP);
	
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue = 0;
		if (pAP->getAttribute("type", szValue))
		{
			if (szValue[0] == 'l' && strcmp((const char*) szValue, "list_label") == 0)
			{
				UT_UTF8String content("list-item-label end-indent=\"label-end()\"");
				tagOpen (TT_LIST_ITEM_LABEL, content, ws_Pre);

				/* content = "block";
				 * tagOpen (TT_BLOCK, content, ws_None);
				 */
				textUntrusted (m_List.getNextLabel().c_str());

				/* tagClose (TT_BLOCK, content, ws_None);
				 */
				content = "list-item-label";
				tagClose (TT_LIST_ITEM_LABEL, content, ws_Post);
			}
		}
	}
}

/* dataid   is the raw string with the data ID
 * imagedir is the name of the directory in which we'll write the image
 * filename is the name of the file to which we'll write the image
 * url      is the URL which we'll use
 */
void s_XSL_FO_Listener::_writeImage (const UT_ByteBuf * pByteBuf,
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

void s_XSL_FO_Listener::_handleImage (PT_AttrPropIndex api)
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

	UT_UTF8String content("external-graphic");

	content += " src=\"";
	content += url;
	content += "\"";

	const XML_Char * szWidth  = 0;
	const XML_Char * szHeight = 0;

	pAP->getProperty ("width",  szWidth);
	pAP->getProperty ("height", szHeight);

	char buf[16];

	if (szWidth)
		{
			sprintf (buf, "%d", (int) UT_convertToDimension (szWidth, DIM_PX));
			content += " width=\"";
			content += buf;
			content += "\"";
		}
	if(szHeight)
		{
			sprintf (buf, "%d", (int) UT_convertToDimension (szHeight, DIM_PX));
			content += " height=\"";
			content += buf;
			content += "\"";
		}

	tagOpenClose (content, false, ws_None);
}

bool s_XSL_FO_Listener::populate(PL_StruxFmtHandle /*sfh*/,
								 const PX_ChangeRecord * pcr)
{
	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrs = 
				static_cast<const PX_ChangeRecord_Span *> (pcr);

			PT_AttrPropIndex api = pcr->getIndexAP();
			_openSpan (api);
			
			PT_BufIndex bi = pcrs->getBufIndex();
			_outputData(m_pDocument->getPointer(bi),pcrs->getLength());

			return true;
		}

	case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);

			switch (pcro->getObjectType())
			{
			case PTO_Image:
			{
				_handleImage (pcr->getIndexAP());
				return true;
			}

			case PTO_Field:
			{
				_handleField(pcr->getIndexAP());
				return true;
			}

				// todo: support these
			case PTO_Hyperlink:
			case PTO_Bookmark:
			  return true;

			default:
			{
				UT_ASSERT(0);
				return false;
			}

			}
		}

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return true;
		
	default:
		UT_ASSERT(0);
		return false;
	}
}

bool s_XSL_FO_Listener::populateStrux(PL_StruxDocHandle /*sdh*/,
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
		_closeBlock();
		
//		<fo:static-content flow-name="xsl-region-before">
//		<fo:block>
//		</fo:block>
//		</fo:static-content> 

		
		// TODO???
		return true;
	}
	
	case PTX_Block:
	{
		_openBlock(pcr->getIndexAP());
		return true;
	}
	
	default:
	{
		UT_ASSERT(0);
		return false;
	}

	}
}

bool s_XSL_FO_Listener::signal(UT_uint32 /* iSignal */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return false;
}

bool s_XSL_FO_Listener::change(PL_StruxFmtHandle /*sfh*/,
							   const PX_ChangeRecord * /*pcr*/)
{
	UT_ASSERT(0);						// this function is not used.
	return false;
}

bool s_XSL_FO_Listener::insertStrux(PL_StruxFmtHandle /*sfh*/,
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

/*****************************************************************/
/*****************************************************************/

static UT_String
purgeSpaces(const char* st)
{
	UT_String retval;

	while (*st != '\0')
	{
		if (*st != ' ')
			retval += *st++;
		else
			++st;
	}

	return retval;
}

void s_XSL_FO_Listener::_handlePageSize(PT_AttrPropIndex api)
{
  //
  // Code to write out the PageSize Definitions to disk
  // 
	const PP_AttrProp * pAP = NULL;
	bool bHaveProp = m_pDocument->getAttrProp(api,&pAP);

	char * old_locale = setlocale (LC_NUMERIC, "C");

	UT_UTF8String content("layout-master-set");
	tagOpen (TT_LAYOUT_MASTER_SET, content);

	content = "simple-page-master";

	// query and output properties
	// todo - validate these and make sure they all make sense
	if (bHaveProp && pAP)
	{
		const XML_Char * szValue = 0;

		szValue = PP_evalProperty("page-margin-top",
								  NULL, NULL, pAP, m_pDocument, true);
		if (szValue)
		{
			content += " margin-top=\"";
			content += szValue;
			content += "\"";
		}

		szValue = PP_evalProperty("page-margin-bottom",
								  NULL, NULL, pAP, m_pDocument, true);
		if (szValue)
		{
			content += " margin-bottom=\"";
			content += szValue;
			content += "\"";
		}

		szValue = PP_evalProperty("page-margin-left",
								  NULL, NULL, pAP, m_pDocument, true);
		if (szValue)
		{
			content += " margin-left=\"";
			content += szValue;
			content += "\"";
		}

		szValue = PP_evalProperty("page-margin-right",
								  NULL, NULL, pAP, m_pDocument, true);
		if (szValue)
		{
			content += " margin-right=\"";
			content += szValue;
			content += "\"";
		}
		
		UT_Dimension docUnit = m_pDocument->m_docPageSize.getDims(); 
		UT_String page_dim;

		page_dim = UT_String_sprintf("%g", m_pDocument->m_docPageSize.Width(docUnit));
		content += " page-width=\"";
		content += page_dim.c_str ();
		content += UT_dimensionName (docUnit);
		content += "\"";

		page_dim = UT_String_sprintf("%g", m_pDocument->m_docPageSize.Height(docUnit));
		content += " page-height=\"";
		content += page_dim.c_str ();
		content += UT_dimensionName (docUnit);
		content += "\"";
	}

	content += " master-name=\"first\"";
	tagOpen (TT_SIMPLE_PAGE_MASTER, content, ws_Pre);

	content = "region-body";
	tagOpenClose (content, false, ws_None);

	content = "simple-page-master";
	tagClose (TT_SIMPLE_PAGE_MASTER, content, ws_Post);

	content = "layout-master-set";
	tagClose (TT_LAYOUT_MASTER_SET, content);

	setlocale (LC_NUMERIC, old_locale);

	m_bFirstWrite = false;
	return;
}

void s_XSL_FO_Listener::_openSection(PT_AttrPropIndex api)
{
	if (m_bFirstWrite) _handlePageSize (api);

	UT_UTF8String content("page-sequence master-reference=\"first\"");
	tagOpen (TT_PAGE_SEQUENCE, content);

	content = "flow flow-name=\"xsl-region-body\"";
	tagOpen (TT_FLOW, content);

	m_bInSection = true;
}

void s_XSL_FO_Listener::_closeSection()
{
	if (!m_bInSection) return;

	if (m_bInBlock) _closeBlock ();

	UT_UTF8String content("flow");
	tagClose (TT_FLOW, content);

	content = "page-sequence";
	tagClose (TT_PAGE_SEQUENCE, content);

	m_bInSection = false;
}

void s_XSL_FO_Listener::_openBlock(PT_AttrPropIndex api)
{
	if (!m_bInSection) return;

	UT_UTF8String content;

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = m_pDocument->getAttrProp (api, &pAP);

	const XML_Char * szValue = 0;

	UT_uint32 tagID;
	WhiteSpace ws;

	if (pAP && pAP->getAttribute ("listid", szValue))
	{
		if (tagTop () == TT_LIST_ITEM_BODY)
		{
			content = "list-item-body";
			tagClose (TT_LIST_ITEM_BODY, content, ws_Post);
		}
		if (tagTop () == TT_LIST_ITEM)
		{
			content = "list-item";
			tagClose (TT_LIST_ITEM, content);
		}
		if (tagTop () != TT_LIST_BLOCK)
		{
			_closeBlock();

			content = "list-block";
			tagOpen (TT_LIST_BLOCK, content);
		}

		tagID = TT_LIST_ITEM;
		ws = ws_Both;
		content = "list-item";

		UT_uint32 listid = (UT_uint32) atoi ((const char *) szValue);
		m_List.setIdList (listid);
	}
	else
	{
		_closeBlock ();

		tagID = TT_BLOCK;
		ws = ws_Pre;
		content = "block";
	}

	// query and output properties
	// todo - validate these and make sure they all make sense

	if (bHaveProp && pAP)
	{
		char buf[2];
		buf[1] = 0;

		if (pAP->getProperty("bgcolor", szValue))
		{
			content += " background-color=\"";

			if (*szValue >= '0' && *szValue <= '9')
			{
				buf[0] = '#';
				content += buf;
			}
			content += szValue;
			content += "\"";
		}

		if (pAP->getProperty("color", szValue))
		{
			content += " color=\"";

			if (*szValue >= '0' && *szValue <= '9')
			{
				buf[0] = '#';
				content += buf;
			}
			content += szValue;
			content += "\"";
		}

		if (pAP->getProperty("lang", szValue))
		{
			content += " language=\"";
			content += szValue;
			content += "\"";
		}

		if (pAP->getProperty("font-size", szValue)) // TODO: ??
		{
			content += " font-size=\"";
			content += purgeSpaces((const char *)szValue).c_str();
			content += "\"";
		}

#ifdef PROPERTY
#undef PROPERTY
#endif
#define PROPERTY(N) \
		if (pAP->getProperty(N, szValue)) \
		{ \
			content += " "N"=\""; \
			content += szValue; \
			content += "\""; \
		}

		PROPERTY("font-family");
		PROPERTY("font-weight");
		PROPERTY("font-style");
		PROPERTY("font-stretch");
		PROPERTY("keep-together");
		PROPERTY("keep-with-next");
		PROPERTY("line-height");
		PROPERTY("margin-bottom");
		PROPERTY("margin-top");
		PROPERTY("margin-left");
		PROPERTY("margin-right");
		PROPERTY("text-align");
		PROPERTY("widows");

#undef PROPERTY
	}
	tagOpen (tagID, content, ws);

	m_utf8_span = "";

	m_bInBlock = true;
}

void s_XSL_FO_Listener::_closeBlock()
{
	if (!m_bInBlock) return;

	if (m_bInSpan) _closeSpan ();

	UT_UTF8String content;

	if (tagTop () == TT_LIST_ITEM_BODY)
	{
		content = "list-item-body";
		tagClose (TT_LIST_ITEM_BODY, content, ws_Post);
	}
	if (tagTop () == TT_LIST_ITEM)
	{
		content = "list-item";
		tagClose (TT_LIST_ITEM, content);
	}

	UT_uint32 tagID;
	WhiteSpace ws;

	if (tagTop () == TT_BLOCK)
	{
		tagID = TT_BLOCK;
		ws = ws_Post;
		content = "block";
	}
	else if (tagTop () == TT_LIST_BLOCK)
	{
		tagID = TT_LIST_BLOCK;
		ws = ws_Both;
		content = "list-block";
	}
	else
	{
		UT_DEBUGMSG(("s_XSL_FO_Listener::_closeBlock: close tag mis-match!\n"));
		return;
	}

	tagClose (tagID, content, ws);

	m_bInBlock = false;
}

void s_XSL_FO_Listener::_openSpan(PT_AttrPropIndex api)
{
	if (!m_bInBlock) return;

	UT_UTF8String content;

	if (tagTop () == TT_LIST_ITEM)
	{
		content = "list-item-body";
		tagOpen (TT_LIST_ITEM_BODY, content, ws_Pre);
	}

	content = "inline";
	bool bInSpan = false;

	const PP_AttrProp * pAP = 0;
	bool bHaveProp = (api ? (m_pDocument->getAttrProp (api, &pAP)) : false);

	// query and output properties
	if (bHaveProp && pAP)
	{
		char buf[2];
		buf[1] = 0;

		const XML_Char * szValue = 0;

		if (pAP->getProperty("bgcolor", szValue))
		{
			content += " background-color=\"";

			if (*szValue >= '0' && *szValue <= '9')
			{
				buf[0] = '#';
				content += buf;
			}
			content += szValue;
			content += "\"";

			bInSpan = true;
		}

		if (pAP->getProperty("color", szValue))
		{
			content += " color=\"";

			if (*szValue >= '0' && *szValue <= '9')
			{
				buf[0] = '#';
				content += buf;
			}
			content += szValue;
			content += "\"";

			bInSpan = true;
		}

		if (pAP->getProperty("lang", szValue))
		{
			content += " language=\"";
			content += szValue;
			content += "\"";

			bInSpan = true;
		}
		
		if (pAP->getProperty("font-size", szValue))
		{
			content += " font-size=\"";
			content += purgeSpaces((const char *)szValue).c_str();
			content += "\"";

			bInSpan = true;
		}		

#ifdef PROPERTY
#undef PROPERTY
#endif
#define PROPERTY(N) \
		if (pAP->getProperty(N, szValue)) \
		{ \
			content += " "N"=\""; \
			content += szValue; \
			content += "\""; \
			bInSpan = true; \
		}

		PROPERTY("font-family");
		PROPERTY("font-weight");
		PROPERTY("font-style");
		PROPERTY("font-stretch");
		PROPERTY("keep-together");
		PROPERTY("keep-with-next");
		PROPERTY("text-decoration");

#undef PROPERTY
	}

	if (bInSpan)
		{
			if (m_bInSpan)
				{
					if (m_utf8_span == content) return; // this span same as last...
					_closeSpan ();
				}
			m_utf8_span = content;

			tagOpen (TT_INLINE, m_utf8_span, ws_None);
			m_bInSpan = true;
		}
	else if (m_bInSpan) _closeSpan ();
}

void s_XSL_FO_Listener::_closeSpan()
{
	if (!m_bInSpan) return;

	UT_UTF8String content("inline");
	tagClose (TT_INLINE, content, ws_None);

	m_bInSpan = false;
}

/*****************************************************************/
/*****************************************************************/

void s_XSL_FO_Listener::_outputData(const UT_UCSChar * data, UT_uint32 length)
{
	if (!m_bInBlock) return;

	UT_UTF8String content;

	if (tagTop () == TT_LIST_ITEM)
	{
		content = "list-item-body";
		tagOpen (TT_LIST_ITEM_BODY, content, ws_Pre);
	}

	const UT_UCSChar * ucs2ptr = data;
	for (UT_uint32 i = 0; i < length; i++)
	{
		switch (*ucs2ptr)
		{
		case UCS_LF:   // LF   -- representing a Forced-Line-Break
		case UCS_VTAB: // VTAB -- representing a Forced-Column-Break
		case UCS_FF:   // FF   -- representing a Forced-Page-Break
			// TODO
			break;
			
		default:
			if ((*ucs2ptr & 0x007f) == *ucs2ptr) // ASCII
			{
				char c = static_cast<char>(*ucs2ptr & 0x007f);

				switch (c)
				{
				case '<':
					content += "&lt;";
					break;
				case '>':
					content += "&gt;";
					break;
				case '&':
					content += "&amp;";
					break;
				default:
					content.append (ucs2ptr, 1);
					break;
				}
			}
			else content.append (ucs2ptr, 1); // !ASCII, just append... ??
			break;
		}
		ucs2ptr++;
	}
	if (content.byteLength ()) textTrusted (content);
}
