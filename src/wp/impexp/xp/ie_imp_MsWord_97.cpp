/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
 * Copyright (C) 2001 Tomas Frydrych
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "wv.h"

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_units.h"
#include "ut_math.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_EncodingManager.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Password.h"

#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"

#include "pd_Document.h"

#include "ie_imp_MsWord_97.h"
#include "ie_impGraphic.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#ifdef DEBUG
#define IE_IMP_MSWORD_DUMP
#include "ie_imp_MsWord_dump.h"
#undef IE_IMP_MSWORD_DUMP
#endif

#define X_CheckError(v) 		do { if (!(v)) return 1; } while (0)

// undef this to disable support for older images (<= Word95)
#define SUPPORTS_OLD_IMAGES 1

#include <fribidi/fribidi.h>

//
// Forward decls. to wv's callbacks
//
static int charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid);
static int specCharProc (wvParseStruct *ps, U16 eachchar, CHP* achp);
static int eleProc (wvParseStruct *ps, wvTag tag, void *props, int dirty);
static int docProc (wvParseStruct *ps, wvTag tag);

//
// DOC uses an unsigned int color index
//
typedef UT_uint32 Doc_Color_t;

//
// A mapping between Word's colors and Abi's RGB color scheme
//
static Doc_Color_t word_colors [][3] = {
	{0x00, 0x00, 0x00}, /* black */
	{0x00, 0x00, 0xff}, /* blue */
	{0x00, 0xff, 0xff}, /* cyan */
	{0x00, 0xff, 0x00}, /* green */
	{0xff, 0x00, 0xff}, /* magenta */
	{0xff, 0x00, 0x00}, /* red */
	{0xff, 0xff, 0x00}, /* yellow */
	{0xff, 0xff, 0xff}, /* white */
	{0x00, 0x00, 0x80}, /* dark blue */
	{0x00, 0x80, 0x80}, /* dark cyan */
	{0x00, 0x80, 0x00}, /* dark green */
	{0x80, 0x00, 0x80}, /* dark magenta */
	{0x80, 0x00, 0x00}, /* dark red */
	{0x80, 0x80, 0x00}, /* dark yellow */
	{0x80, 0x80, 0x80}, /* dark gray */
	{0xc0, 0xc0, 0xc0}, /* light gray */
};

//
// Field Ids that are useful later for mapping
//
typedef enum {
	F_TIME,
	F_DATE,
	F_EDITTIME,
	F_AUTHOR,
	F_PAGE,
	F_NUMCHARS,
	F_NUMPAGES,
	F_NUMWORDS,
	F_FILENAME,
	F_HYPERLINK,
	F_PAGEREF,
	F_EMBED,
	F_TOC,
	F_DateTimePicture,
	F_TOC_FROM_RANGE,
	F_OTHER
} Doc_Field_t;

//
// A mapping between DOC's field names and our given IDs
//
typedef struct
{
	char * m_name;
	Doc_Field_t m_id;
} Doc_Field_Mapping_t;

/*
 * This next bit of code enables us to import many of Word's fields
 */

static Doc_Field_Mapping_t s_Tokens[] =
{
	{"TIME",	   F_TIME},
	{"EDITTIME",   F_EDITTIME},
	{"DATE",	   F_DATE},
	{"date",	   F_DATE},
	{"\\@", 	   F_DateTimePicture},

	{"FILENAME",   F_FILENAME},
	{"\\filename", F_FILENAME},
	{"PAGE",	   F_PAGE},
	{"NUMCHARS",   F_NUMCHARS},
	{"NUMWORDS",   F_NUMWORDS},

	// these below aren't handled by AbiWord, but they're known about
	{"HYPERLINK",  F_HYPERLINK},
	{"PAGEREF",    F_PAGEREF},
	{"EMBED",	   F_EMBED},
	{"TOC", 	   F_TOC},
	{"\\o", 	   F_TOC_FROM_RANGE},
	{"AUTHOR",	   F_AUTHOR},

	{ "*",		   F_OTHER}
};

#define FieldMappingSize (sizeof(s_Tokens)/sizeof(s_Tokens[0]))

static Doc_Field_t
s_mapNameToField (const char * name)
{
	for (unsigned int k = 0; k < FieldMappingSize; k++)
	{
		if (!UT_strcmp(s_Tokens[k].m_name,name))
			return s_Tokens[k].m_id;
	}
	return F_OTHER;
}

#undef FieldMappingSize

static const char *
s_mapPageIdToString (UT_uint16 id)
{
	// TODO: make me way better when we determine code names

	switch (id)
	{
	case 0:
		return "Letter";
	case 9:
		return "A4";

	default:
		return 0;
	}
}

/*!
  Surprise, surprise, there are more list numerical formats than the 5 the
  MS documentation states happens to mention, so here I will put what I found
  out (latter we will move it to some better place)
*/
typedef enum
{
  WLNF_INVALID		   = -1,
  WLNF_EUROPEAN_ARABIC = 0,
  WLNF_UPPER_ROMAN	   = 1,
  WLNF_LOWER_ROMAN	   = 2,
  WLNF_UPPER_LETTER    = 3,
  WLNF_LOWER_LETTER    = 4,
  WLNF_ORDINAL		   = 5,
  WLNF_BULLETS		   = 23,
  WLNF_HEBREW_NUMBERS  = 45
} MSWordListIdType;

typedef struct{
  UT_uint32 listId;
  UT_uint32 level;
} ListIdLevelPair;

/*!
 * Map msword list enums back to abi's
 */
static const char *
s_mapDocToAbiListId (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "4";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "3";

	case WLNF_UPPER_LETTER: // upper letter
	  return "2";

	case WLNF_LOWER_LETTER: // lower letter
	  return "1";

	case WLNF_BULLETS: // bullet list
	  return "5";

	case WLNF_HEBREW_NUMBERS:
	  return "129";

	case WLNF_ORDINAL: // ordinal
	default:
	  return "0";
	}
}

/*!
 * Map msword list enum back to an abiword list delimiter tag
 */
static const char *
s_mapDocToAbiListDelim (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "%L.";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "%L.";

	case WLNF_UPPER_LETTER: // upper letter
	  return "%L)";

	case WLNF_LOWER_LETTER: // lower letter
	  return "%L)";

	case WLNF_BULLETS: // bullet list
	  return "%L";

	case WLNF_ORDINAL: // ordinal

	case WLNF_HEBREW_NUMBERS:

	default:
	  return "%L.";
	}
}

/*!
 * Map msword list enums back to abi's list styles
 */
static const char *
s_mapDocToAbiListStyle (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "Upper Roman List";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "Lower Roman List";

	case WLNF_UPPER_LETTER: // upper letter
	  return "Upper Case List";

	case WLNF_LOWER_LETTER: // lower letter
	  return "Lower Case List";

	case WLNF_BULLETS: // bullet list
	  return "Bullet List";

	case WLNF_ORDINAL: // ordinal
	default:
	  return "Numbered List";
	}
}

/*!
 * Map msword list enums back to abi's field font for that given style
 */
static const char *
s_fieldFontForListStyle (MSWordListIdType id)
{
  switch (id)
	{
	case WLNF_UPPER_ROMAN: // upper roman
	  return "NULL";

	case WLNF_LOWER_ROMAN: // lower roman
	  return "NULL";

	case WLNF_UPPER_LETTER: // upper letter
	  return "Times New Roman";

	case WLNF_LOWER_LETTER: // lower letter
	  return "Times New Roman";

	case WLNF_BULLETS: // bullet list
	  return "Symbol";

	case WLNF_ORDINAL: // ordinal
	default:
	  return "Times New Roman";
	}
}

/****************************************************************************/
/****************************************************************************/

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeContents (const char * szBuf,
												  UT_uint32 iNumbytes)
{
	char * magic	= 0;
	int magicoffset = 0;

	magic = "Microsoft Word 6.0 Document";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	magic = "Documento Microsoft Word 6";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp(szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	magic = "MSWordDoc";
	magicoffset = 2112;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return UT_CONFIDENCE_PERFECT;
		}
	}

	// ok, that didn't work, we'll try to dig through the OLE stream
	if (iNumbytes > 8)
	{
#if 1
	        // this code is too generic - also picks up .wri documents
		if (szBuf[0] == (char)0x31 && szBuf[1] == (char)0xbe &&
			szBuf[2] == (char)0 && szBuf[3] == (char)0)
		{
		  return UT_CONFIDENCE_SOSO; //POOR
		}

		// this identifies staroffice dox as well
		if (szBuf[0] == (char)0xd0 && szBuf[1] == (char)0xcf &&
			szBuf[2] == (char)0x11 && szBuf[3] == (char)0xe0 &&
			szBuf[4] == (char)0xa1 && szBuf[5] == (char)0xb1 &&
			szBuf[6] == (char)0x1a && szBuf[7] == (char)0xe1)
		{
		  return UT_CONFIDENCE_SOSO; // POOR
		}
#endif
		if (szBuf[0] == 'P' && szBuf[1] == 'O' &&
			szBuf[2] == '^' && szBuf[3] == 'Q' && szBuf[4] == '`')
		{
			return UT_CONFIDENCE_POOR;
		}
		if (szBuf[0] == (char)0xfe && szBuf[1] == (char)0x37 &&
			szBuf[2] == (char)0 && szBuf[3] == (char)0x23)
		{
			return UT_CONFIDENCE_POOR;
		}

		if (szBuf[0] == (char)0xdb && szBuf[1] == (char)0xa5 &&
			szBuf[2] == (char)0x2d && szBuf[3] == (char)0 &&
			szBuf[4] == (char)0 && szBuf[5] == (char)0)
		{
			return UT_CONFIDENCE_POOR;
		}
	}
	return UT_CONFIDENCE_ZILCH;
}

UT_Confidence_t IE_Imp_MsWord_97_Sniffer::recognizeSuffix (const char * szSuffix)
{
	// We recognize both word documents and their template versions
	if (!UT_stricmp(szSuffix,".doc") ||
			!UT_stricmp(szSuffix,".dot"))
	  return UT_CONFIDENCE_PERFECT;
	return UT_CONFIDENCE_ZILCH;
}

UT_Error IE_Imp_MsWord_97_Sniffer::constructImporter (PD_Document * pDocument,
													  IE_Imp ** ppie)
{
	IE_Imp_MsWord_97 * p = new IE_Imp_MsWord_97(pDocument);
	*ppie = p;
	return UT_OK;
}

bool	IE_Imp_MsWord_97_Sniffer::getDlgLabels (const char ** pszDesc,
												const char ** pszSuffixList,
												IEFileType * ft)
{
	*pszDesc = "Microsoft Word (.doc, .dot)";
	*pszSuffixList = "*.doc; *.dot";
	*ft = getFileType();
	return true;
}

/****************************************************************************/
/****************************************************************************/

// just buffer sizes, arbitrarily chosen
#define DOC_TEXTRUN_SIZE 2048
#define DOC_PROPBUFFER_SIZE 1024

IE_Imp_MsWord_97::~IE_Imp_MsWord_97()
{
	if(m_pBookmarks)
	{
		// free the names from the bookmarks
		for(UT_uint32 i = 0; i < m_iBookmarksCount; i++)
		{
			// make sure we do not delete any name twice
			if(m_pBookmarks[i].name && m_pBookmarks[i].start)
			{
			   delete[] m_pBookmarks[i].name;
			   m_pBookmarks[i].name = NULL;
			}
		}
		delete [] m_pBookmarks;
	}

	UT_VECTOR_PURGEALL(ListIdLevelPair *, m_vLists);
}

IE_Imp_MsWord_97::IE_Imp_MsWord_97(PD_Document * pDocument)
  : IE_Imp (pDocument),
	m_iImageCount (0),
	m_nSections(0),
	m_bSetPageSize(false),
	m_fieldWhich(NULL),
	m_fieldI(0),
	m_fieldDepth(0),
	m_fieldRet(0),
	m_fieldC(NULL),
	//m_fieldA(NULL),
	m_bIsLower(false),
	m_bInSect(false),
	m_bInPara(false),
	m_bPrevStrongCharRTL(false),
	m_iDocPosition(0),
	m_pBookmarks(NULL),
	m_iBookmarksCount(0),
	m_iMSWordListId(0)
{
  for(UT_uint32 i = 0; i < 9; i++)
	  m_iListIdIncrement[i] = 0;
}

/****************************************************************************/
/****************************************************************************/

#define ErrCleanupAndExit(code)  do {wvOLEFree (&ps); return(code);} while(0)

#define GetPassword() _getPassword ( getDoc()->getApp()->getLastFocussedFrame() )

#define ErrorMessage(x) do { XAP_Frame *_pFrame = getDoc()->getApp()->getLastFocussedFrame(); if ( _pFrame ) _errorMessage (_pFrame, (x)); } while (0)

static UT_String _getPassword (XAP_Frame * pFrame)
{
  UT_String password ( "" );

  if ( pFrame )
    {
      pFrame->raise ();

      XAP_DialogFactory * pDialogFactory
	= (XAP_DialogFactory *)(pFrame->getDialogFactory());

      XAP_Dialog_Password * pDlg = static_cast<XAP_Dialog_Password*>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PASSWORD));
      UT_return_val_if_fail(pDlg, password);

      pDlg->runModal (pFrame);

      XAP_Dialog_Password::tAnswer ans = pDlg->getAnswer();
      bool bOK = (ans == XAP_Dialog_Password::a_OK);

      if (bOK)
	password = pDlg->getPassword ();

      UT_DEBUGMSG(("Password is %s\n", password.c_str()));

      pDialogFactory->releaseDialog(pDlg);
    }

  return password;
}

static void _errorMessage (XAP_Frame * pFrame, int id)
{
  UT_return_if_fail(pFrame);

  const XAP_StringSet * pSS = XAP_App::getApp ()->getStringSet ();

  const char * text = pSS->getValue (id, pFrame->getApp()->getDefaultEncoding()).c_str();

  pFrame->showMessageBox (text,
			  XAP_Dialog_MessageBox::b_O,
			  XAP_Dialog_MessageBox::a_OK);
}

UT_Error IE_Imp_MsWord_97::importFile(const char * szFilename)
{
  wvParseStruct ps;

  int ret = wvInitParser (&ps, (char *)szFilename);
  const char * password = NULL;

  // HACK!!
  bool decrypted = false ;

  if (ret & 0x8000)		/* Password protected? */
    {
      UT_String pass = GetPassword();
      if ( pass.size () != 0 )
	password = pass.c_str();

      if ((ret & 0x7fff) == WORD8)
	{
	  ret = 0;
	  if (password == NULL)
	    {
	      ErrorMessage(AP_STRING_ID_WORD_PassRequired);
	      ErrCleanupAndExit(UT_IE_PROTECTED);
	    }
	  else
	    {
	      wvSetPassword (password, &ps);
	      if (wvDecrypt97 (&ps))
		{
		  ErrorMessage(AP_STRING_ID_WORD_PassInvalid);
		  ErrCleanupAndExit(UT_IE_PROTECTED);
		}
	      decrypted = true ;
	    }
	}
      else if (((ret & 0x7fff) == WORD7) || ((ret & 0x7fff) == WORD6))
	{
	  ret = 0;
	  if (password == NULL)
	    {
	      ErrorMessage(AP_STRING_ID_WORD_PassRequired);
	      ErrCleanupAndExit(UT_IE_PROTECTED);
	    }
	  else
	    {
	      wvSetPassword (password, &ps);
	      if (wvDecrypt95 (&ps))
		{
		  //("Incorrect Password\n"));
		  ErrCleanupAndExit(UT_IE_PROTECTED);
		}
	      decrypted = true ;
	    }
	}
    }

  if (ret)
    ErrCleanupAndExit(UT_IE_BOGUSDOCUMENT);

  // register ourself as the userData
  ps.userData = this;

  // register callbacks
  wvSetElementHandler (&ps, eleProc);
  wvSetCharHandler (&ps, charProc);
  wvSetSpecialCharHandler(&ps, specCharProc);
  wvSetDocumentHandler (&ps, docProc);

  UT_DEBUGMSG(("DOM: wvText\n"));

  wvText(&ps);

  UT_DEBUGMSG(("DOM: about to get summary information\n"));

  // now get the summary information, if available
  ret = wvQuerySupported (&ps.fib, NULL);

  // word 2 used an OLE like mechanism inside of a FILE*, but
  // good luck trying to ms_ole_summary_open something using that...
  if (WORD2 != ret)
    {
      MsOleSummary *summary = ms_ole_summary_open (ps.ole_file);
      if (summary)
	{
	  UT_DEBUGMSG(("DOM: getting summary information\n"));
	  
	  char *prop_str = NULL;
	  gboolean found = FALSE;
	  
	  // title
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_TITLE, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_TITLE, prop_str ) ;
	  
	  // subject
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_SUBJECT, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_SUBJECT, prop_str ) ;
	  
	  // author
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_AUTHOR, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_CREATOR, prop_str ) ;
	  
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_LASTAUTHOR, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_CONTRIBUTOR, prop_str ) ;
	  
	  // keywords
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_KEYWORDS, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_KEYWORDS, prop_str ) ;
	  
	  // comments
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_COMMENTS, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_DESCRIPTION, prop_str ) ;
	  
	  // category
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_CATEGORY, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_TYPE, prop_str ) ;
	  
	  // organization
	  prop_str = ms_ole_summary_get_string (summary, MS_OLE_SUMMARY_COMPANY, &found);
	  if (found && prop_str)
	    getDoc()->setMetaDataProp ( PD_META_KEY_PUBLISHER, prop_str ) ;
	  
	  ms_ole_summary_close (summary);
	}
    }

  UT_DEBUGMSG(("DOM: finished summary info\n"));

  // HACK - this will do until i sort out some global stream ugliness in wv
  if ( !decrypted && WORD2 != ret )
    wvOLEFree(&ps);

  // We can't be in a good state if we didn't add any sections!
  if (m_nSections == 0)
    return UT_IE_BOGUSDOCUMENT;

  return UT_OK;
}

void IE_Imp_MsWord_97::_flush ()
{

  if(!m_pTextRun.size())
	return;

  // we've got to ensure that we're inside of a section & paragraph
  if (!m_bInSect)
	{
	  // append a blank default section - assume it works
	  UT_DEBUGMSG(("#TF: _flush: appending default section\n"));
	  getDoc()->appendStrux(PTX_Section, NULL);
	  m_bInSect = true;
	  m_nSections++;
	}

  if(!m_bInPara)
	{
	  // append a blank defaul paragraph - assume it works
	  UT_DEBUGMSG(("#TF: _flush: appending default block\n"));
	  getDoc()->appendStrux(PTX_Block, NULL);
	  m_bInPara = true;
	}

	if (m_pTextRun.size())
	{
		if (!getDoc()->appendSpan(m_pTextRun.ucs4_str(), m_pTextRun.size()))
		{
			UT_DEBUGMSG(("DOM: error appending text run\n"));
			return;
		}
		m_pTextRun.clear ();
	}
}

void IE_Imp_MsWord_97::_appendChar (UT_UCSChar ch)
{
	if ( m_bIsLower )
	  ch = UT_UCS4_tolower ( ch );
	m_pTextRun += ch;
}

/****************************************************************************/
/****************************************************************************/

static int s_cmp_bookmarks_qsort(const void * a, const void * b)
{
	const bookmark * A = (const bookmark *) a;
	const bookmark * B = (const bookmark *) b;

	if(A->pos != B->pos)
		return (A->pos - B->pos);
	else
		// for bookmarks with identical position we want any start bookmarks to be
		// before end bookmarks.
		return ((UT_sint32)B->start - (UT_sint32)A->start);
}

static int s_cmp_bookmarks_bsearch(const void * a, const void * b)
{
	UT_uint32 A = *((const UT_uint32 *) a);
	const bookmark * B = (const bookmark *) b;

	return (A - B->pos);
}

XML_Char * IE_Imp_MsWord_97::_getBookmarkName(wvParseStruct * ps, UT_uint32 pos)
{
	XML_Char *str;
	UT_iconv_t ic_handle;
	// word bookmarks can be at most 30 characters, so make a reasonable buffer
	// for the utf-8 version
	char buff[200];
	char *buff_ptr = &buff[0];
	const char *in_ptr;
	size_t out_left = 200, in_left;

	if (!XAP_EncodingManager::get_instance()->cjk_locale()
	   &&(XAP_EncodingManager::get_instance()->try_nativeToU(0xa1) != 0xa1))
	{
		ic_handle = UT_iconv_open(XAP_EncodingManager::get_instance()->getNativeEncodingName(), "UCS-2");
	}
	else
	{
		// use utf-8
		ic_handle = UT_iconv_open("utf-8", "UCS-2LE");
	}

	if(ps->Sttbfbkmk.extendedflag == 0xFFFF)
	{
		// 16 bit stuff
		in_ptr = (const char *) ps->Sttbfbkmk.u16strings[pos];
		in_left = 2 * UT_UCS2_strlen((UT_UCS2Char*)ps->Sttbfbkmk.u16strings[pos]) + 2;
		UT_iconv( ic_handle, &in_ptr, &in_left, &buff_ptr,&out_left);
		str = new XML_Char[200 - out_left];
		strcpy(str, buff);
	}
	else
	{
		// 8 bit stuff
		// there is a bug in wv, and the table gets incorrectly retrieved
		// if it contains 8-bit strings
		if(ps->Sttbfbkmk.s8strings[pos])
		{
			UT_uint32 len = strlen(ps->Sttbfbkmk.s8strings[pos]);
			str = new XML_Char[len + 1];
			UT_uint32 i = 0;
			for(i = 0; i < len; i++)
				str[i] = ps->Sttbfbkmk.s8strings[pos][i];
			str[i] = 0;
		}
		else
			str = NULL;
	}

	return str;
}

int IE_Imp_MsWord_97::_docProc (wvParseStruct * ps, UT_uint32 tag)
{
	// flush out any pending character data
	this->_flush ();

	//
	// we currently don't do anything with these tags
	//

	switch ((wvTag)tag)
	{
	case DOCBEGIN:
		UT_uint32 i,j;

		if(m_pBookmarks)
		{
			for(i = 0; i < m_iBookmarksCount; i++)
			{
				if(m_pBookmarks[i].name && m_pBookmarks[i].start)
				{
					delete []m_pBookmarks[i].name;
					m_pBookmarks[i].name = NULL;
				}
			}
			delete [] m_pBookmarks;
		}
		BKF *bkf;
		BKL *bkl;
		U32 *posf, *posl, nobkf, nobkl;

		if(!wvGetBKF_PLCF (&bkf, &posf, &nobkf, ps->fib.fcPlcfbkf, ps->fib.lcbPlcfbkf, ps->tablefd))
		{
			m_iBookmarksCount = nobkf;
		}
		else
			m_iBookmarksCount = 0;

		if(!wvGetBKL_PLCF (&bkl, &posl, &nobkl, ps->fib.fcPlcfbkl, ps->fib.lcbPlcfbkl, ps->fib.fcPlcfbkf, ps->fib.lcbPlcfbkf, ps->tablefd))
		{
			m_iBookmarksCount += nobkl;
		}
		else
		{
			if(m_iBookmarksCount > 0)
			{
				//free the bkf and posf
				wvFree(bkf);
				wvFree(posf);
				m_iBookmarksCount = 0;
			}
		}
		UT_return_val_if_fail(nobkl == nobkf, 0);
		if(m_iBookmarksCount > 0)
		{
			m_pBookmarks = new bookmark[m_iBookmarksCount];
			UT_return_val_if_fail(m_pBookmarks, 0);
			for(i = 0; i < nobkf; i++)
			{
				m_pBookmarks[i].name = _getBookmarkName(ps, i);
				m_pBookmarks[i].pos  = posf[i];
				m_pBookmarks[i].start = true;
			}

			for(j = i; j < nobkl + i; j++)
			{
				// since the name is shared with the start of the bookmark,
				// we reuse it
				UT_sint32 iBkf = (UT_sint32) bkl[j-i].ibkf < 0 ? nobkl + (UT_sint32)bkl[j-i].ibkf : bkl[j-i].ibkf;
				m_pBookmarks[j].name = m_pBookmarks[iBkf].name;
				m_pBookmarks[j].pos  = posl[j - i];
				m_pBookmarks[j].start = false;
			}
			// free bkf, bkl, posf, posl
			wvFree(bkf);
			wvFree(bkl);
			wvFree(posf);
			wvFree(posl);

			//now sort the bookmarks by position
			qsort((void*)m_pBookmarks, m_iBookmarksCount, sizeof(bookmark), s_cmp_bookmarks_qsort);
#ifdef DEBUG
			for(UT_uint32 k = 0; k < m_iBookmarksCount; k++)
			{
				UT_DEBUGMSG(("Bookmark: name [%s], pos %d, start %d\n", m_pBookmarks[k].name,m_pBookmarks[k].pos,m_pBookmarks[k].start));
			}

#endif
		}

			break;
	case DOCEND:
	default:
		break;
	}

	return 0;
}

bool IE_Imp_MsWord_97::_insertBookmark(bookmark * bm)
{
	// first of all flush what is in the buffers
	this->_flush();
	bool error = false;

	const XML_Char* propsArray[5];
	propsArray[0] = (XML_Char *)"name";
	propsArray[1] = (XML_Char *)bm->name;
	propsArray[2] = (XML_Char *)"type";
	propsArray[4] = 0;

	if(bm->start)
		propsArray[3] = (XML_Char *)"start";
	else
		propsArray[3] = (XML_Char *)"end";


	if (!getDoc()->appendObject (PTO_Bookmark, propsArray))
	{
		UT_DEBUGMSG (("Could not append bookmark object\n"));
		error = true;
	}
	return error;
}

bool IE_Imp_MsWord_97::_insertBookmarkIfAppropriate()
{
	//now search for position m_iDocPosition in our bookmark list;
	bookmark * bm;
	if (m_iBookmarksCount == 0) {
		bm = (bookmark*) NULL;
	}
	else {
		bm = (bookmark*) bsearch((const void *) &m_iDocPosition,
				m_pBookmarks, m_iBookmarksCount, sizeof(bookmark),
				s_cmp_bookmarks_bsearch);
	}
	bool error = false;
	if(bm)
	{
	   // there is a bookmark at the current position
	   // first make sure the returned bookmark is the first one at this position
	   while(bm > m_pBookmarks && (bm - 1)->pos == m_iDocPosition)
		   bm--;

	   while(bm->pos == m_iDocPosition)
		  error |= _insertBookmark(bm++);
	}
	return error;
}

int IE_Imp_MsWord_97::_charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
	_insertBookmarkIfAppropriate();

	// convert incoming character to unicode
	if (chartype)
		eachchar = wvHandleCodePage(eachchar, lid);

	switch (eachchar)
	{

	case 11: // forced line break
		eachchar = UCS_LF;
		break;

	case 12: // page or section break
		this->_flush ();
		eachchar = UCS_FF;
		break;

	case 13: // end of paragraph
		m_iDocPosition++;
		return 0;

	case 14: // column break
		eachchar = UCS_VTAB;
		break;

	case 19: // field begin
		this->_flush ();
		ps->fieldstate++;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, chartype, lid);
		return 0;

	case 20: // field separator
		this->_fieldProc (ps, eachchar, chartype, lid);
		ps->fieldmiddle = 1;
		return 0;

	case 21: // field end
		ps->fieldstate--;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, chartype, lid);
		return 0;

	}

	// i'm not sure if this is needed any more
	// yes, it is, for instance hyperlinks need it
	if (ps->fieldstate)
	{
		xxx_UT_DEBUGMSG(("DOM: fieldstate\n"));
		if(this->_fieldProc (ps, eachchar, chartype, lid))
		{
			return 0;
		}
	}

	// take care of any oddities in Microsoft's character encoding
	if (chartype == 1 && eachchar == 146)
		eachchar = 39; // apostrophe

	// deal with the thorny problem of mirror characters
	if(m_bPrevStrongCharRTL)
	{
		FriBidiChar mirror_char;
		if(fribidi_get_mirror_char((FriBidiChar)eachchar, &mirror_char))
			eachchar = (U16)mirror_char;
	}

	//
	// Append the character to our character buffer
	//
	this->_appendChar ((UT_UCSChar) eachchar);
	m_iDocPosition++;

	FriBidiCharType cType = fribidi_get_type((FriBidiChar)eachchar);
	if(FRIBIDI_IS_STRONG(cType))
	{
		m_bPrevStrongCharRTL = FRIBIDI_IS_RTL(cType);
	}
	return 0;
}

int IE_Imp_MsWord_97::_specCharProc (wvParseStruct *ps, U16 eachchar, CHP *achp)
{
	Blip blip;
	long pos;
	FSPA * fspa;
	FDOA * fdoa;
#ifdef SUPPORTS_OLD_IMAGES
	wvStream *fil;
	PICF picf;
#endif

	//
	// This next bit of code is to handle fields
	//

	switch (eachchar)
	{

	case 19: // field begin
		this->_flush ();
		ps->fieldstate++;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	case 20: // field separator
		if (achp->fOle2)
		{
			UT_DEBUGMSG(("Field has an assocaited embedded OLE object\n"));
		}
		ps->fieldmiddle = 1;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	case 21: // field end
		ps->fieldstate--;
		ps->fieldmiddle = 0;
		this->_fieldProc (ps, eachchar, 0, 0x400);
		return 0;

	}

	/* it seems some fields characters slip through here which tricks
	 * the import into thinking it has an image with it really does
	 * not. this catches special characters in a field
	 */
	if (ps->fieldstate) {
		if (this->_fieldProc(ps, eachchar, 0, 0x400))
			return 0;
	}

	//
	// This next bit of code is to handle OLE2 embedded objects and images
	//

	switch (eachchar)
	{
	case 0x01: // Older ( < Word97) image, currently not handled very well

		if (achp->fOle2) {
			UT_DEBUGMSG(("embedded OLE2 component. currently unsupported"));
			return 0;
		}

		pos = wvStream_tell(ps->data);

#ifdef SUPPORTS_OLD_IMAGES
		UT_DEBUGMSG(("Pre W97 Image format.\n"));
		wvStream_goto(ps->data, achp->fcPic_fcObj_lTagObj);

		if (1 == wvGetPICF(wvQuerySupported(&ps->fib, NULL), &picf,
				   ps->data) && NULL != picf.rgb)
		  {
			fil = picf.rgb;

			if (wv0x01(&blip, fil, picf.lcb - picf.cbHeader))
			  {
                       this->_handleImage(&blip, picf.mx * picf.dxaGoal / 1000, picf.my * picf.dyaGoal / 1000);
			  }
			else
			  {
			UT_DEBUGMSG(("Dom: no graphic data\n"));
			  }
#else
			UT_DEBUGMSG(("DOM: 0x01 graphics support is disabled at the moment\n"));
#endif

			wvStream_goto(ps->data, pos);

			return 0;
		  }
		else
		  {
			UT_DEBUGMSG(("Couldn't import graphic!\n"));
			return 0;
		  }
	case 0x08: // Word 97, 2000, XP image

		if (wvQuerySupported(&ps->fib, NULL) >= WORD8) // sanity check
		{
			if (ps->nooffspa > 0)
			{

				fspa = wvGetFSPAFromCP(ps->currentcp, ps->fspa,
									   ps->fspapos, ps->nooffspa);

				if(!fspa)
				{
					UT_DEBUGMSG(("No fspa! Panic and Insanity Abounds!\n"));
					return 0;
				}

				if (wv0x08(&blip, fspa->spid, ps))
				{
					this->_handleImage(&blip, fspa->xaRight-fspa->xaLeft,
									   fspa->yaBottom-fspa->yaTop);
				}
				else
				{
					UT_DEBUGMSG(("Dom: no graphic data!\n"));
					return 0;
				}
			}
			else
			{
				xxx_UT_DEBUGMSG(("nooffspa was <= 0 -- ignoring"));
			}
		}
		else
		{
			UT_DEBUGMSG(("pre Word8 0x08 graphic -- unsupported at the moment"));
			fdoa = wvGetFDOAFromCP(ps->currentcp, NULL, ps->fdoapos,
								   ps->nooffdoa);

			// TODO: do something with the data in this fdoa someday...
		}

		return 0;
	}

	return 0;
}

int IE_Imp_MsWord_97::_beginComment(wvParseStruct *ps, UT_uint32 tag,
					void *props, int dirty)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}

int IE_Imp_MsWord_97::_endComment(wvParseStruct *ps, UT_uint32 tag,
				  void *props, int dirty)
{
  UT_DEBUGMSG(("DOM: begin comment\n"));
  return 0;
}


int IE_Imp_MsWord_97::_eleProc(wvParseStruct *ps, UT_uint32 tag,
							   void *props, int dirty)
{
	//
	// Marshall these off to the correct handlers
	//

	switch ((wvTag)tag)
	{

	case SECTIONBEGIN:
		return _beginSect (ps, tag, props, dirty);

	case SECTIONEND:
		return _endSect (ps, tag, props, dirty);

	case PARABEGIN:
		return _beginPara (ps, tag, props, dirty);

	case PARAEND:
		return _endPara (ps, tag, props, dirty);

	case CHARPROPBEGIN:
		return _beginChar (ps, tag, props, dirty);

	case CHARPROPEND:
		return _endChar (ps, tag, props, dirty);

	case COMMENTBEGIN:
	  return _beginComment (ps, tag, props, dirty);

	case COMMENTEND:
	  return _endComment (ps, tag, props, dirty);

	default:
	  UT_ASSERT_NOT_REACHED();

	}

	return 0;
}

/****************************************************************************/
/****************************************************************************/

int IE_Imp_MsWord_97::_beginSect (wvParseStruct *ps, UT_uint32 tag,
				  void *prop, int dirty)
{
	SEP * asep = static_cast <SEP *>(prop);

	XML_Char * propsArray[3];
	UT_String propBuffer;
	UT_String props;

	// flush any character runs
	this->_flush ();

	// page-margin-left
	UT_String_sprintf(propBuffer,
		"page-margin-left:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dxaLeft) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-right
	UT_String_sprintf(propBuffer,
		"page-margin-right:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dxaRight) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-top
	UT_String_sprintf(propBuffer,
		"page-margin-top:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaTop) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-bottom
	UT_String_sprintf(propBuffer,
		"page-margin-bottom:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaBottom) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-header
	UT_String_sprintf(propBuffer,
		"page-margin-header:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaHdrTop) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-footer
	UT_String_sprintf(propBuffer,
		"page-margin-footer:%s;",
		UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaHdrBottom) / 1440),
						  "1.4"));
	props += propBuffer;

	if(asep->fPgnRestart)
	  {
		// set to 1 when page numbering should be restarted at the beginning of this section
		props += "section-restart:1;";
	  }

	{
	  // user specified starting page number
	  UT_String_sprintf(propBuffer, "section-restart-value:%d;", asep->pgnStart);
	  props += propBuffer;
	}

	// columns
	if (asep->ccolM1) {
		// number of columns
		UT_String_sprintf(propBuffer,
				"columns:%d;", (asep->ccolM1+1));
		props += propBuffer;

		// columns gap
		UT_String_sprintf(propBuffer,
				"column-gap:%s;",
				UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dxaColumns) / 1440),
												  "1.4"));
		props += propBuffer;
	}

	// darw a vertical line between columns
	if (asep->fLBetween == 1)
	{
		props += "column-line:on;";
	}

	// space after section (gutter)
	UT_String_sprintf(propBuffer,
			"section-space-after:%s",
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dzaGutter) / 1440), "1.4"));
	props += propBuffer;

	//
	// TODO: headers/footers, section breaks
	//
//
// Sevior: Only do this ONCE!!! Abiword can only handle one page size.
//
	if(!m_bSetPageSize)
	{
		//
		// all of this data is related to Abi's <pagesize> tag
		//
		m_bSetPageSize = true;
		double page_width  = 0.0;
		double page_height = 0.0;
		double page_scale  = 1.0;

		if (asep->dmOrientPage == 1)
			getDoc()->m_docPageSize.setLandscape ();
		else
			getDoc()->m_docPageSize.setPortrait ();

		page_width = asep->xaPage / 1440.0;
		page_height = asep->yaPage / 1440.0;

		UT_DEBUGMSG(("DOM: pagesize: (landscape: %d) (width: %f) (height: %f) (paper-type: %d)\n",
					 asep->dmOrientPage, page_width, page_height, asep->dmPaperReq));

		const char * paper_name = s_mapPageIdToString (asep->dmPaperReq);

		if (paper_name)
		  {
			// we found a paper name, let's use it
			getDoc()->m_docPageSize.Set (paper_name);
		  }
		else
		  {
			getDoc()->m_docPageSize.Set ("Custom");
		  }

		// always use the passed size
		getDoc()->m_docPageSize.Set (page_width, page_height, DIM_IN);
		getDoc()->m_docPageSize.setScale(page_scale);
	}

	xxx_UT_DEBUGMSG (("DOM:SEVIOR the section properties are: '%s'\n", props.c_str()));

	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)props.c_str();
	propsArray[2] = 0;

	if (!getDoc()->appendStrux(PTX_Section, (const XML_Char **)propsArray))
	{
		UT_DEBUGMSG (("DOM: error appending section props!\n"));
		return 1;
	}

	// increment our section count
	m_bInSect = true;
	m_bInPara = false; // reset paragraph status
	m_nSections++;

	// TODO: we need to do some work on Headers/Footers

	/*
	 * break codes:
	 * 0 No break
	 * 1 New column
	 * 2 New page
	 * 3 Even page
	 * 4 Odd page
	 */

//	if (asep->bkc > 1 && m_nSections > 1) // don't apply on the 1st page
	if (m_nSections > 1) // don't apply on the 1st page
	{
		// new sections always need a block
		if (!getDoc()->appendStrux(PTX_Block, (const XML_Char **)NULL))
		{
			UT_DEBUGMSG (("DOM: error appending new block\n"));
			return 1;
		}
		m_bInPara = true;

		UT_UCSChar ucs = UCS_FF;
		switch (asep->bkc) {
		case 1:
			ucs = UCS_VTAB;
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 2:
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 3: // TODO: handle me better (not even)
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 4: // TODO: handle me better (not odd)
			X_CheckError(getDoc()->appendSpan(&ucs,1));
			break;

		case 0:
		default:
			break;
		}
	}

	return 0;
}

int IE_Imp_MsWord_97::_endSect (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	// if we're at the end of a section, we need to check for a section mark
	// at the end of our character stream and remove it (to prevent page breaks
	// between sections)
	if (m_pTextRun.size() &&
		m_pTextRun[m_pTextRun.size()-1] == UCS_FF)
	  {
		m_pTextRun[m_pTextRun.size()-1] = 0;
	  }
	m_bInSect = false;
	m_bInPara = false; // reset paragraph status
	return 0;
}

int IE_Imp_MsWord_97::_beginPara (wvParseStruct *ps, UT_uint32 tag,
								  void *prop, int dirty)
{
	PAP *apap = static_cast <PAP *>(prop);

	UT_String propBuffer;
	UT_String props;

	//
	// TODO: lists, exact line heights (and, eventually, tables)
	//

	// first, flush any character data in any open runs
	this->_flush ();

	xxx_UT_DEBUGMSG(("#TF: _beginPara: apap->fBidi %d\n",apap->fBidi));

	if(apap->fBidi == 1)
		m_bPrevStrongCharRTL = true;
	else
		m_bPrevStrongCharRTL = false;

	// DOM TODO: i think that this is right
	if (apap->fBidi == 1) {
		props += "dom-dir:rtl;";
	} else {
		props += "dom-dir:ltr;";
	}

	// paragraph alignment/justification
	switch(apap->jc)
	{
	case 0:
		props += "text-align:left;";
		break;
	case 1:
		props += "text-align:center;";
		break;
	case 2:
		props += "text-align:right;";
		break;
	case 3:
		props += "text-align:justify;";
		break;
	case 4:
		/* this type of justification is of unknown purpose and is
		 * undocumented , but it shows up in asian documents so someone
		 * should be able to tell me what it is someday
		 */
		props += "text-align:justify;";
		break;
	}

	// keep paragraph together?
	if (apap->fKeep) {
		props += "keep-together:yes;";
	}

	// keep with next paragraph?
	if (apap->fKeepFollow) {
		props += "keep-with-next:yes;";
	}

	// break before paragraph?
	if (apap->fPageBreakBefore)
	{
		// TODO: this should really set a property in
		// TODO: in the paragraph, instead; but this
		// TODO: gives a similar effect for now.
		UT_DEBUGMSG(("_beginPara: appending default block\n"));
		getDoc()->appendStrux(PTX_Block, NULL);
		UT_UCSChar ucs = UCS_FF;
		getDoc()->appendSpan(&ucs,1);
	}

	// widowed/orphaned lines
	if (!apap->fWidowControl) {
		// these AbiWord properties give the same effect
		props += "orphans:0;widows:0;";
	}

	// line spacing (single-spaced, double-spaced, etc.)
	if (apap->lspd.fMultLinespace) {
		UT_String_sprintf(propBuffer,
				"line-height:%s;",
				UT_convertToDimensionlessString( (((float)apap->lspd.dyaLine) / 240), "1.1"));
		props += propBuffer;
	} else {
		// TODO: handle exact line heights
	}

	//
	// margins
	//

	// margin-right
	if (apap->dxaRight) {
		UT_String_sprintf(propBuffer,
				"margin-right:%s;",
				UT_convertInchesToDimensionString(DIM_IN, (((float)apap->dxaRight) / 1440),
												  "1.4"));
		props += propBuffer;
	}

	// margin-left
	if (apap->dxaLeft) {
		UT_String_sprintf(propBuffer,
				"margin-left:%s;",
				UT_convertInchesToDimensionString(DIM_IN, (((float)apap->dxaLeft) / 1440),
												  "1.4"));
		props += propBuffer;
	}

	// margin-left first line (indent)
	if (apap->dxaLeft1) {
		UT_String_sprintf(propBuffer,
				"text-indent:%s;",
				UT_convertInchesToDimensionString(DIM_IN, (((float)apap->dxaLeft1) / 1440),
												  "1.4"));
		props += propBuffer;
	}

	// margin-top
	if (apap->dyaBefore) {
		UT_String_sprintf(propBuffer,
				"margin-top:%dpt;", (apap->dyaBefore / 20));
		props += propBuffer;
	}

	// margin-bottom
	if (apap->dyaAfter) {
		UT_String_sprintf(propBuffer,
				"margin-bottom:%dpt;", (apap->dyaAfter / 20));
	}

	// tab stops
	if (apap->itbdMac) {
		propBuffer += "tabstops:";

		for (int iTab = 0; iTab < apap->itbdMac; iTab++) {
			propBuffer += UT_String_sprintf(
							"%s/",
							UT_convertInchesToDimensionString(DIM_IN, (((float)apap->rgdxaTab[iTab])
												   / 1440), "1.4"));
			switch (apap->rgtbd[iTab].jc) {
			case 1:
				propBuffer += "C,";
				break;
			case 2:
				propBuffer += "R,";
				break;
			case 3:
				propBuffer += "D,";
				break;
			case 4:
				propBuffer += "B,";
				break;
			case 0:
			default:
				propBuffer += "L,";
				break;
			}
		}
		// replace final comma with a semi-colon
		propBuffer[propBuffer.size()-1] = ';';
		props += propBuffer;
	}

	UT_uint32 myListId = 0;
	LVLF * myLVLF = NULL;
	UT_String szListId;
	UT_String szParentId;
	UT_String szStartValue;
	UT_String szLevel;
	/*
	  The MS documentation on lists really sucks, but we've been able to decipher
	  some meaning from it and get simple lists to sorta work. This code mostly prints out
	  debug messages with useful information in them, but it will also append a list
	  and add a given paragraph to a given list
	*/

	if ( apap->ilfo && ps->lfo )
	{
	  // all lists have ilfo set
	  UT_DEBUGMSG(("list: ilvl %d, ilfo %d\n",apap->ilvl,apap->ilfo));	//ilvl is the list level
	  LVL * myLVL = NULL;
	  LFO * myLFO = NULL;
	  LST * myLST = NULL;
	  LFOLVL * myLFOLVL = NULL;

	  UT_sint32 myStartAt = -1;
	  U8 * mygPAPX = NULL;
	  U8 * mygCHPX = NULL;
	  XCHAR * myNumberStr = NULL;
	  UT_sint32 myNumberStr_count = 0;
	  UT_uint32 mygPAPX_count = 0, mygCHPX_count = 0;

	  PAPX myPAPX;
	  CHPX myCHPX;

	  // first, get the LFO, and then find the lfovl for this paragraph
	  myLFO = &ps->lfo[apap->ilfo - 1];

	  UT_uint32 i = 0, j = 0, k = 0;
	  while((UT_sint32)i < apap->ilfo - 1 && i < ps->nolfo)
		{
		  j += ps->lfo[i].clfolvl;
		  i++;
		}

	  // remember how many overrides are there for this record
	  k = ps->lfo[i].clfolvl;

	  // if there are any overrides, then see if one of them applies to this level
	  if(k && ps->lfolvl)
		{
		  i = 0;
		  while(i < k && ps->lfolvl[j].ilvl != apap->ilvl)
		{
		  j++;
		  i++;
		}

		  if(ps->lfolvl[--j].ilvl != apap->ilvl)
		{
		  UT_DEBUGMSG(("list: no LFOLVL found for this level (1)\n"));
		  myLFOLVL = NULL;
		}
		  else
		{
		  myLFOLVL = &ps->lfolvl[j];
		  UT_DEBUGMSG(("list: lfovl: iStartAt %d, fStartAt\n", myLFOLVL->iStartAt,myLFOLVL->fStartAt,myLFOLVL->fFormatting));
		  if(!myLFOLVL->fFormatting && myLFOLVL->fStartAt)
			myStartAt = myLFOLVL->iStartAt;
		}
		}
	  else
		{
		  UT_DEBUGMSG(("list: no LFOLVL found for this level (2)\n"));
		  myLFOLVL = NULL;
		}

	  // now that we might have the LFOLVL, let's see if we should use the LVL from the LFO
	  bool bNeedLST_LVL = (!myLFOLVL || !myLFOLVL->fStartAt || !myLFOLVL->fFormatting);
	  bool bLST_LVL_format = true;
	  if(myLFOLVL)
		{
		  // this branch has not been (thoroughly) debugged
		  // Abi bugs 2205 and 2393 exhibit this behavior
		  UT_DEBUGMSG(("list: using the LVL from LFO\n"));
		  myListId = myLFOLVL->iStartAt;
		  i = 0;
		  UT_DEBUGMSG(("list: number of LSTs %d, my lsid %d\n", ps->noofLST,myListId));
		  while(i < ps->noofLST && ps->lst[i].lstf.lsid != myListId)
		{
		  i++;
		  UT_DEBUGMSG(("list: lsid in LST %d\n", ps->lst[i-1].lstf.lsid));
		}

		  if(i == ps->noofLST || ps->lst[i].lstf.lsid != myListId)
		{
		  UT_DEBUGMSG(("error: could not locate LST entry\n"));
		  goto list_error;
		}

		  myLST = &ps->lst[i];
		  myLVL = &myLST->lvl[apap->ilvl];

		  // now we should have the LVL
		  UT_ASSERT(myLVL);

		  myLVLF = &myLVL->lvlf;
		  UT_ASSERT(myLVLF);

		  myStartAt = myLFOLVL->fStartAt ? (signed) myLVLF->iStartAt : -1;
		  
		  mygPAPX = myLFOLVL->fFormatting ? myLVL->grpprlPapx : NULL;
		  mygPAPX_count = myLFOLVL->fFormatting ? myLVLF->cbGrpprlPapx : 0;

		  // not sure about this, the CHPX applies to the number, so it might be
		  // that we should take this if the fStartAt is set -- the docs are not clear
		  mygCHPX = myLFOLVL->fFormatting ? myLVL->grpprlChpx : NULL;
		  mygCHPX_count = myLFOLVL->fFormatting ? myLVLF->cbGrpprlChpx : 0;

		  myNumberStr = myLFOLVL->fStartAt && myLVL->numbertext ? myLVL->numbertext + 1 : NULL;
		  myNumberStr_count = myNumberStr ? *(myLVL->numbertext) : 0;

		  if(myLFOLVL->fFormatting)
		bLST_LVL_format = false;

		}

	  if(bNeedLST_LVL)
		{
		  LVL * prevLVL = myLVL;
		  LVLF * prevLVLF = myLVLF;
		  myListId = myLFO->lsid;
		  UT_DEBUGMSG(("list: using the LVL from LST\n"));
		  i = 0;
		  UT_DEBUGMSG(("list: number of LSTs %d, my lsid %d\n", ps->noofLST,myListId));
		  while(i < ps->noofLST && ps->lst[i].lstf.lsid != myListId)
		{
		  i++;
		  xxx_UT_DEBUGMSG(("list: lsid in LST %d\n", ps->lst[i-1].lstf.lsid));
		}

		  if(i == ps->noofLST || ps->lst[i].lstf.lsid != myListId)
		{
		  UT_DEBUGMSG(("error: could not locate LST entry\n"));
		  goto list_error;
		}

		  myLST = &ps->lst[i];
		  myLVL = &myLST->lvl[apap->ilvl];

		  // now we should have the correct LVL
		  UT_ASSERT(myLVL);

		  myLVLF = &myLVL->lvlf;
		  UT_ASSERT(myLVLF);

		  // retrieve any stuff we need from here (i.e., only what we did not get from the LFO LVL)
		  myStartAt = myStartAt == -1 ? myLVLF->iStartAt : myStartAt;

		  mygPAPX_count = !mygPAPX ? myLVLF->cbGrpprlPapx : mygPAPX_count;
		  mygPAPX = !mygPAPX ? myLVL->grpprlPapx : mygPAPX;

		  mygCHPX_count = !mygCHPX ? myLVLF->cbGrpprlChpx : mygCHPX_count;
		  mygCHPX = !mygCHPX ? myLVL->grpprlChpx : mygCHPX;

		  myNumberStr_count = !myNumberStr && myLVL->numbertext ? *(myLVL->numbertext) : myNumberStr_count;
		  myNumberStr = !myNumberStr && myLVL->numbertext ? myLVL->numbertext + 1 : myNumberStr;


		  // if there was a valid LFO LVL record that pertained to formatting
		  // then we will set the myLVL and myLVLF variables back to this record
		  // so that it can be used
		  if(!bLST_LVL_format && prevLVL && prevLVLF)
		{
		  myLVL = prevLVL;
		  myLVLF = prevLVLF;
		}
		}

	  UT_DEBUGMSG(("list: number text len %d, papx len %d, chpx len%d\n",myNumberStr_count,mygPAPX_count,mygCHPX_count));
	  myPAPX.cb = mygPAPX_count;
	  myPAPX.grpprl = mygPAPX;
	  myPAPX.istd = 4095; // no style

	  myCHPX.cbGrpprl = mygCHPX_count;
	  myCHPX.grpprl = mygCHPX;
	  myCHPX.istd = 4095; // no style


	  // if we are in a new list, then do some clean up first and remember the list id
	  if(m_iMSWordListId != myListId)
	  {
		  m_iMSWordListId = myListId;

		  for(UT_uint32 i = 0; i < 9; i++)
			  m_iListIdIncrement[i] = 0;

		  UT_VECTOR_PURGEALL(ListIdLevelPair *, m_vLists);
		  m_vLists.clear();
	  }

	  // a hack -- see the note on myListId below
	  myListId += myLVLF->nfc;
	  myListId += apap->ilvl;

	  /*
		IMPORTANT now we have the list formatting sutff retrieved; it is found in several
		different places:
		apap->ilvl - the level of this list (0-8)

		myStartAt	- the value at which the numbering for this listshould start
		(i.e., the number of the first item on the list)

		myListId	- the id of this list, we need this to know to which list this
		paragraph belongs; unfortunately, there seem to be some cases where separate
		lists *share* the same id, for instance when two lists, of different formatting,
		are separated by only empty paragraphs. As a hack, I have added the format number
		to the list id, so gaining different id for different formattings (it is not foolproof,
		for if id1 + format1 == id2 + format2 then we get two lists joined, but the probability
		of that should be small). Further problem is that in AW, list id refers to the set of
		list elements on the same level, while in Word the id is that of the entire list. The
		easiest way to tranform the Word id to AW id is to add the level to the id, which
		is what has been done above

		PAPX		- the formatting information that needs to be added to the format
		of this list

		CHPX		- the formatting of the list number

		myNumberStr - the actual number string to display (XCHAR *); we probably need
		this to work out the number separator, since there does not seem
		to be any reference to this anywhere

		myNumberStr_count - length of the number string

		myLVLF->nfc - number format (see the enum below)

		myLVLF->jc	- number alignment [0: lft, 1: rght, 2: cntr]

		myLVLF->ixchFollow - what character stands between the number and the para
		[0:= tab, 1: spc, 2: none]

	  */
	  UT_DEBUGMSG(("list: id %d \n",myListId));
	  UT_DEBUGMSG(("list: iStartAt %d\n", myStartAt));
	  UT_DEBUGMSG(("list: lvlf: format %d\n",myLVLF->nfc)); // see the comment above for nfc values
	  UT_DEBUGMSG(("list: lvlf: number align %d [0: lft, 1: rght, 2: cntr]\n",myLVLF->jc));
	  UT_DEBUGMSG(("list: lvlf: ixchFollow %d [0:= tab, 1: spc, 2: none]\n",myLVLF->ixchFollow));

	  // If a given list id has already been defined, appending a new list with
	  // same values will have a harmless effect

	  // id, parentid, type, start value, list-delim, NULL
	  const XML_Char * list_atts[13];


	  // we will use this to keep track of how many entries of given level we have had
	  // every time we get here, we increase the counter for all levels lower that ours
	  // then we will add the counter for our level to myListId; this way subsections of
	  // the list separated by a higher level list entry will have different id's


	  for(j = apap->ilvl + 1; j < 9; j++)
		  m_iListIdIncrement[j]++;

	  myListId += m_iListIdIncrement[apap->ilvl];

	  // list id number
	  list_atts[0] = "id";
	  UT_String_sprintf(propBuffer, "%d", myListId);
	  szListId = propBuffer;
	  list_atts[1] = szListId.c_str();

	  // parent id
	  list_atts[2] = "parentid";

	  // we will search backward our list vector for the first entry that has a lower level than we
	  // and that will be our parent
	  UT_uint32 myParentID = 0;
	  for(UT_sint32 n = m_vLists.getItemCount(); n > 0; n--)
	  {
		  ListIdLevelPair * llp = (ListIdLevelPair *)(m_vLists.getNthItem(n - 1));
		  if(llp->level < apap->ilvl)
		  {
			  myParentID = llp->listId;
			  break;
		  }
	  }
	  UT_String_sprintf(propBuffer, "%d", myParentID);
	  szParentId = propBuffer;
	  list_atts[3] = szParentId.c_str();

	  // list type
	  list_atts[4] = "type";
	  list_atts[5] = s_mapDocToAbiListId ((MSWordListIdType)myLVLF->nfc);

	  // start value
	  list_atts[6] = "start-value";
	  UT_String_sprintf(propBuffer, "%d", myStartAt);
	  szStartValue = propBuffer;
	  list_atts[7] = szStartValue.c_str();

	  // list delimiter
	  list_atts[8] = "list-delim";
	  list_atts[9] = s_mapDocToAbiListDelim ((MSWordListIdType)myLVLF->nfc);

	  list_atts[10] = "level";
	  UT_String_sprintf(propBuffer, "%d", apap->ilvl + 1); // Word level starts at 0, Abi's at 1
	  szLevel = propBuffer;
	  list_atts[11] = szLevel.c_str();

	  // NULL
	  list_atts[12] = 0;

	  // now add this to our vector of lists
	  ListIdLevelPair * llp = new ListIdLevelPair;
	  llp->listId = myListId;
	  llp->level = apap->ilvl;
	  m_vLists.addItem((void*)llp);

	  getDoc()->appendList(list_atts);
	  UT_DEBUGMSG(("DOM: appended a list\n"));

	  // TODO: merge in list properties and such here with the variable 'props',
	  // such as list-style, field-font, ...

	  // start-value
	  props += "start-value:";
	  props += szStartValue;
	  props += ";";

	  // list style
	  props += "list-style:";
	  props += s_mapDocToAbiListStyle ((MSWordListIdType)myLVLF->nfc);
	  props += ";";

	  // field-font
	  props += "field-font:";
	  props += s_fieldFontForListStyle ((MSWordListIdType)myLVLF->nfc);
	  props += ";";
	} // end of list-related code

list_error:

	// remove the trailing semi-colon
	props [props.size()-1] = 0;

	xxx_UT_DEBUGMSG(("Dom: the paragraph properties are: '%s'\n",props.c_str()));

	//props, level, listid, parentid, style (TODO), NULL
	const XML_Char * propsArray[11];

	// props
	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)props.c_str();

	UT_uint32 i = 2;
	// level, or 0 for default, normal level
	if (myListId > 0)
	{
		propsArray[i++] = "level";
		propsArray[i++] = szLevel.c_str();
		propsArray[i++] = "listid";
		propsArray[i++] = szListId.c_str();
		propsArray[i++] = "parentid";
		propsArray[i++] = szParentId.c_str();
	}

	// NULL
	propsArray[i] = 0;

	if (!m_bInSect)
	{
		// check for should-be-impossible case
		UT_ASSERT_NOT_REACHED();
		getDoc()->appendStrux(PTX_Section, NULL);
		m_bInSect = true ;
	}
	
	if (!getDoc()->appendStrux(PTX_Block, (const XML_Char **)propsArray))
	{
		UT_DEBUGMSG(("DOM: error appending paragraph block\n"));
		return 1;
	}

	if (myListId > 0 && myLVLF)
	  {
		// TODO: honor more props
		const XML_Char *list_field_fmt[3];
		list_field_fmt[0] = "type";
		list_field_fmt[1] = "list_label";
		list_field_fmt[2] = 0;
		getDoc()->appendObject(PTO_Field, (const XML_Char**)list_field_fmt);

		// the character following the list label - 0=tab, 1=space, 2=none
		if ( myLVLF->ixchFollow == 0 ) // tab
		  {
		UT_UCSChar tab = UCS_TAB;
		getDoc()->appendSpan(&tab, 1);
		  }
		else if ( myLVLF->ixchFollow == 1 ) // space
		  {
		UT_UCSChar space = UCS_SPACE;
		getDoc()->appendSpan(&space, 1);
		  }
		// else none
	  }
	m_bInPara = true;
	return 0;
}

int IE_Imp_MsWord_97::_endPara (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	xxx_UT_DEBUGMSG(("#TF: _endPara\n"));
	// have to flush here, otherwise flushing later on will result in
	// an empty paragraph being inserted
	this->_flush ();
	m_bInPara = false;
	return 0;
}

int IE_Imp_MsWord_97::_beginChar (wvParseStruct *ps, UT_uint32 tag,
								  void *prop, int dirty)
{
	CHP *achp = static_cast <CHP *>(prop);

	XML_Char * propsArray[3];
	UT_String propBuffer;
	UT_String props;

	// set char tolower if fSmallCaps && fLowerCase
	if ( achp->fSmallCaps && achp->fLowerCase )
	  m_bIsLower = true;
	else
	  m_bIsLower = false;

	// flush any data in our character runs
	this->_flush ();

	// set language based the lid - TODO: do we want to handle -none- differently?
	props += "lang:";

	if (achp->fBidi)
		props += wvLIDToLangConverter (achp->lidBidi);
	else if (!ps->fib.fFarEast)
		props += wvLIDToLangConverter (achp->lidDefault);
	else
		props += wvLIDToLangConverter (achp->lidFE);
	props += ";";

	// decide best codepage based on the lid (as lang code above)
	UT_String codepage;
	if (achp->fBidi)
		codepage = wvLIDToCodePageConverter (achp->lidBidi);
	else if (!ps->fib.fFarEast)
		codepage = wvLIDToCodePageConverter (achp->lidDefault);
	else
		codepage = wvLIDToCodePageConverter (achp->lidFE);

	// watch out for codepage 0 = unicode
	if (codepage == "CP0")
		codepage = XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName();
	// if this is the first codepage we've seen, use it.
	// if we see more than one different codepage in a document, use unicode.
	if (!getDoc()->getEncodingName())
		getDoc()->setEncodingName(codepage.c_str());
	else if (getDoc()->getEncodingName() != codepage)
		getDoc()->setEncodingName(XAP_EncodingManager::get_instance()->getNativeUnicodeEncodingName());

	// after several hours of playing with this, I think I now undertand
	// how achp->fBidi works:
	// if it is 0, then treat the text as if it was LTR
	// if it is 1, then apply bidi algorithm to the text
	// effectively for 0 we should set dir-override to "ltr"
	// but that is very uggly, since most of the time we will be
	// applying this override to strong LTR characters that do not need
	// it. Instead we will use special value "nobidi", which will be processed
	// in our fp_TextRun, which will either remove it, or translate it to
	// LTR override, depending on the text.

	// unfortunately there were problems with the code in fp_TextRun, so
	// at the moment we will simply set "ltr"

	// if this segment of text is not bidi and it follows a segement that is
	// not bidi either, we will do nothing, but if the previous segement was
	// not bidi, we will set the nobidi value

	xxx_UT_DEBUGMSG(("#TF: _beginChar: [0x%x] achp->fBidi %d, m_bPrevStrongCharRTL %d\n",achp,achp->fBidi,m_bPrevStrongCharRTL));
	if (!achp->fBidi && m_bPrevStrongCharRTL)
		props += "dir-override:ltr;";
	// not entirely sure about this second branch, leave it out for now
#if 0
	else if (achp->fBidi && !m_bPrevStrongCharRTL)
		props += "dir-override:rtl;";
#endif

	// bold text
	bool fBold = (achp->fBidi ? achp->fBoldBidi : achp->fBold);
	if (fBold) {
		props += "font-weight:bold;";
	}

	// italic text
	bool fItalic = (achp->fBidi ? achp->fItalicBidi : achp->fItalic);
	if (fItalic) {
		props += "font-style:italic;";
	}

	// foreground color
	U8 ico = (achp->fBidi ? achp->icoBidi : achp->ico);
	if (ico) {
		UT_String_sprintf(propBuffer,
				"color:%02x%02x%02x;",
				word_colors[ico-1][0],
				word_colors[ico-1][1],
				word_colors[ico-1][2]);
		props += propBuffer;
	}

	// underline and strike-through
	if (achp->fStrike || achp->kul) {
		props += "text-decoration:";
		if (achp->fStrike && achp->kul) {
			props += "underline line-through;";
		} else if (achp->kul) {
			props += "underline;";
		} else {
			props += "line-through;";
		}
	}

	// background color
	if (achp->fHighlight) {
		UT_String_sprintf(propBuffer,
				"bgcolor:%02x%02x%02x;",
				word_colors[achp->icoHighlight-1][0],
				word_colors[achp->icoHighlight-1][1],
				word_colors[achp->icoHighlight-1][2]);
		props += propBuffer;
	}

	// superscript && subscript
	if (achp->iss == 1) {
		props += "text-position: superscript;";
	} else if (achp->iss == 2) {
		props += "text-position: subscript;";
	}

	// font size (hps is half-points)
	// I have seen a bidi doc that had hpsBidi == 0, and the actual size in hps
	U16 hps = (achp->fBidi &&  achp->hpsBidi ? achp->hpsBidi : achp->hps);
	UT_String_sprintf(propBuffer,
			"font-size:%dpt;", (int)(hps/2));
	props += propBuffer;

	// font family
	char *fname;

	// if the FarEast flag is set, use the FarEast font,
	// otherwise, we'll use the ASCII font.
	if (achp->fBidi) {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcBidi);
	} else if (!ps->fib.fFarEast) {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcAscii);
	} else {
		fname = wvGetFontnameFromCode(&ps->fonts, achp->ftcFE);

		if (strlen (fname) > 6)
			fname[6] = '\0';

		const char *f=XAP_EncodingManager::cjk_word_fontname_mapping.lookupByTarget(fname);

		if (f == fname)
		{
			FREEP (fname);
			fname = UT_strdup ("song");
		}
		else
		{
			FREEP (fname);
			fname = UT_strdup (f ? f : "helvetic");
		}
	}

	// there are times when we should use the third, Other font,
	// and the logic to know when somehow depends on the
	// character sets or encoding types? it's in the docs.

	UT_ASSERT_HARMLESS(fname != NULL);
	xxx_UT_DEBUGMSG(("font-family = %s\n", fname));

	props += "font-family:";

	if(fname)
		props += fname;
	else
		props += "Times New Roman";
	FREEP(fname);

	xxx_UT_DEBUGMSG(("DOM: character properties are: '%s'\n", props.c_str()));

	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)props.c_str();
	propsArray[2] = 0;

	// woah - major error here
	if(!m_bInSect)
	{
		UT_ASSERT_NOT_REACHED();
		getDoc()->appendStrux(PTX_Section, NULL);
		m_bInSect = true ;
	}

	if(!m_bInPara)
	{
		UT_ASSERT_NOT_REACHED();
		getDoc()->appendStrux(PTX_Block, NULL);
		m_bInPara = true ;
	}
	
	if (!getDoc()->appendFmt((const XML_Char **)propsArray))
	{
		UT_DEBUGMSG(("DOM: error appending character formatting\n"));
		return 1;
	}
	return 0;
}

int IE_Imp_MsWord_97::_endChar (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	// nothing is needed here
	return 0;
}

/****************************************************************************/
/****************************************************************************/

int IE_Imp_MsWord_97::_fieldProc (wvParseStruct *ps, U16 eachchar,
								  U8 chartype, U16 lid)
{
	xxx_UT_DEBUGMSG(("DOM: fieldProc: %c %x\n", (char)eachchar,
					 (int)eachchar));

	//
	// The majority of this code has just been ripped out of wv/field.c
	//

	if (eachchar == 0x13) // beginning of a field
	{
		//m_fieldA = 0;
		m_fieldRet = 1;
		if (m_fieldDepth == 0)
		{
			m_fieldWhich = m_command;
			m_command[0] = 0;
			m_argument[0] = 0;
			m_fieldI = 0;
		}
		m_fieldDepth++;
	}
	else if (eachchar == 0x14) // field trigger
	{
		if (m_fieldDepth == 1)
		{
			m_command[m_fieldI] = 0;
			m_iDocPosition += UT_UCS2_strlen(m_command) + 1; // +1 for the 0x14
			m_fieldC = wvWideStrToMB (m_command);
			if (this->_handleCommandField(m_fieldC))
				m_fieldRet = 1;
			else
				m_fieldRet = 0;

			wvFree(m_fieldC);
			m_fieldWhich = m_argument;
			m_fieldI = 0;
		}
	}

	if (m_fieldI >= FLD_SIZE)
	{
		UT_DEBUGMSG(("DOM: Something completely absurd in the fields implementation!\n"));
		UT_ASSERT_NOT_REACHED();
		return 1;
	}

	if (!m_fieldWhich) {
		UT_DEBUGMSG(("DOM: _fieldProc - 'which' is null\n"));
		UT_ASSERT_NOT_REACHED();
		return 1;
	}

	if (chartype)
		m_fieldWhich[m_fieldI] = wvHandleCodePage(eachchar, lid);
	else
		m_fieldWhich[m_fieldI] = eachchar;

	m_fieldI++;

	if (eachchar == 0x15) // end of field marker
	{
		m_fieldDepth--;
		if (m_fieldDepth == 0)
		{
			m_fieldWhich[m_fieldI] = 0;
			//I do not think we should convert this -- this is the field value
			//displayed in the document; in most cases we do not need it, as we
			//calulate it ourselves, but for instance for hyperlinks this is the
			//the text to which the link is tied
			//m_fieldA = wvWideStrToMB (m_argument);
			m_fieldC = wvWideStrToMB (m_command);
			_handleFieldEnd (m_fieldC);
			wvFree (m_fieldC);
		}
	}
	return m_fieldRet;
}

bool IE_Imp_MsWord_97::_handleFieldEnd (char *command)
{
  Doc_Field_t tokenIndex = F_OTHER;
	char *token;

	if (*command != 0x13)
	  {
	  UT_DEBUGMSG (("field did not begin with 0x13\n"));
	  return true;
	  }
	strtok (command, "\t, ");
	while ((token = strtok (NULL, "\t, ")))
	  {
	tokenIndex = s_mapNameToField (token);
	switch (tokenIndex)
	{
		case F_HYPERLINK:
		{
			token = strtok (NULL, "\"\" ");
			UT_return_val_if_fail(m_argument[0] == 0x14 && m_argument[m_fieldI - 1] == 0x15, false);
			m_argument[m_fieldI - 1] = 0;
			UT_UCS2Char * a = m_argument + 1;
			while(*a)
			{
				this->_appendChar(*a++);
				m_iDocPosition++;
				_insertBookmarkIfAppropriate();
			}
			this->_flush();
			getDoc()->appendObject(PTO_Hyperlink,NULL);
			// increase doc position for the 0x15
			m_iDocPosition++;
			break;
		}
		default:
		break;
		}
	  }
	return false;
}

bool IE_Imp_MsWord_97::_handleCommandField (char *command)
{
	Doc_Field_t tokenIndex = F_OTHER;
	char *token = NULL;

	xxx_UT_DEBUGMSG(("DOM: handleCommandField '%s'\n", command));

	const XML_Char* atts[5];
	atts[0] = "type";
	atts[1] = 0;
	atts[2] = 0;
	atts[3] = 0;
	atts[4] = 0;

	if (*command != 0x13)
	{
		UT_DEBUGMSG(("DOM: field did not begin with 0x13\n"));
		return true;
	}
	strtok(command, "\t, ");

	while ((token = strtok(NULL, "\t, ")))
	{
		tokenIndex = s_mapNameToField (token);

		switch (tokenIndex)
		{
		case F_EDITTIME:
			case F_TIME:
			atts[1] = "time";
			break;

		case F_DateTimePicture: // this isn't totally correct, but it's close
		case F_DATE:
			atts[1] = "date";
			break;

		case F_PAGE:
			atts[1] = "page_number";
			break;

		case F_NUMCHARS:
			atts[1] = "char_count";
			break;

		case F_NUMPAGES:
			atts[1] = "page_count";
			break;

		case F_NUMWORDS:
			atts[1] = "word_count";
			break;

		case F_FILENAME:
			atts[1] = "file_name";
			break;

		case F_PAGEREF:
			token = strtok (NULL, "\"\" ");
			atts[1] = "page_ref";
			atts[2] = "param";
			if(token)
				atts[3] = (const XML_Char *) token;
			else
				atts[3] = "no_bookmark_given";
			break;

		case F_HYPERLINK:
		  {
		const XML_Char *new_atts[3];
		token = strtok (NULL, "\"\" ");

		// hyperlink or hyperlink to bookmark
		new_atts[0] = "xlink:href";
		UT_String href;
		if ( !strcmp(token, "\\l") )
		  {
			token = strtok (NULL, "\"\" ");
			href = "#";
			href += token;
		  }
		else
		  {
			href = token;
		  }
		new_atts[1] = href.c_str();
		new_atts[2] = 0;
		this->_flush();
		getDoc()->appendObject(PTO_Hyperlink, new_atts);
		return true;
		  }

		default:
			// unhandled field type
			continue;
		}

		this->_flush();
		if (!getDoc()->appendObject (PTO_Field, (const XML_Char**)atts))
		{
			UT_DEBUGMSG(("Dom: couldn't append field (type = '%s')\n", atts[1]));
		}
	}

	return true;
}

typedef enum {
  MSWord_UnknownImage,
  MSWord_VectorImage,
  MSWord_RasterImage
} MSWord_ImageType;

static MSWord_ImageType s_determineImageType ( Blip * b )
{
  if ( !b )
	return MSWord_UnknownImage;

  switch ( b->type )
	{
	case msoblipEMF:
	case msoblipWMF:
	case msoblipPICT:
	  return MSWord_VectorImage;

	case msoblipJPEG:
	case msoblipPNG:
	case msoblipDIB:
	  return MSWord_RasterImage;

	case msoblipERROR:
	case msoblipUNKNOWN:
	default:
	  return MSWord_UnknownImage;
	}
}

UT_Error IE_Imp_MsWord_97::_handleImage (Blip * b, long width, long height)
{
	char * old_locale = "" ;
	const char * mimetype 	= UT_strdup ("image/png");
	IE_ImpGraphic * importer	= 0;
	FG_Graphic* pFG		= 0;
	UT_Error error		= UT_OK;
	UT_ByteBuf * buf		= 0;
	UT_ByteBuf * pictData 	= new UT_ByteBuf();

  // suck the data into the ByteBuffer

  MSWord_ImageType imgType = s_determineImageType ( b );

  wvStream *pwv;

  if ( imgType == MSWord_RasterImage )
	{
	  pwv = b->blip.bitmap.m_pvBits;
	}
  else if ( imgType == MSWord_VectorImage )
	{
	  pwv = b->blip.metafile.m_pvBits;
	}
  else
	{
	  UT_DEBUGMSG(("UNKNOWN IMAGE TYPE!!"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  return UT_ERROR;
	}

  size_t size = wvStream_size (pwv);
  char *data = new char[size];
  wvStream_rewind(pwv);
  wvStream_read(data,size,sizeof(char),pwv);
  pictData->append((UT_Byte*)data, size);
  delete [] data;

  UT_String propBuffer;
  UT_String propsName;

  if(!pictData->getPointer(0))
	  error =  UT_ERROR;
  else
	  error = IE_ImpGraphic::constructImporter (pictData, IEGFT_Unknown, &importer);

  if ((error != UT_OK) || !importer)
	{
	  UT_DEBUGMSG(("Could not create image importer object\n"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  goto Cleanup;
	}

  error = importer->importGraphic(pictData, &pFG);
  if ((error != UT_OK) || !pFG)
	{
	  UT_DEBUGMSG(("Could not import graphic\n"));
	  // pictData is already freed in ~FG_Graphic
	  //  DELETEP(pictData);
	  FREEP(mimetype);
	  goto Cleanup;
	}

  // TODO: can we get back a vector graphic?
  buf = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();

  if (!buf)
	{
	  // i don't think that this could ever happen, but...
	  UT_DEBUGMSG(("Could not convert to PNG\n"));
	  DELETEP(pictData);
	  FREEP(mimetype);
	  error = UT_ERROR;
	  goto Cleanup;
	}

  //
  // This next bit of code will set up our properties based on the image attributes
  //

  old_locale = setlocale(LC_NUMERIC, "C");
  UT_String_sprintf(propBuffer, "width:%fin; height:%fin",
					(double)width / (double)1440,
					(double)height / (double)1440);
  setlocale(LC_NUMERIC, old_locale);

  UT_String_sprintf(propsName, "image%d", m_iImageCount++);

  const XML_Char* propsArray[5];
  propsArray[0] = (XML_Char *)"props";
  propsArray[1] = (XML_Char *)propBuffer.c_str();
  propsArray[2] = (XML_Char *)"dataid";
  propsArray[3] = (XML_Char *)propsName.c_str();
  propsArray[4] = 0;

  if (!getDoc()->appendObject (PTO_Image, propsArray))
	{
	  UT_DEBUGMSG (("Could not create append object\n"));
	  error = UT_ERROR;
	  FREEP(mimetype);
	  goto Cleanup;
	}

  if (!getDoc()->createDataItem(propsName.c_str(), false,
				buf, (void*)mimetype, NULL))
	{
	  UT_DEBUGMSG (("Could not create data item\n"));
	  error = UT_ERROR;
	  // this is taken care of by createDataItem
	  //FREEP(mimetype);
	  goto Cleanup;
	}

 Cleanup:
  //DELETEP(pictData);
  //DELETEP(pFG);
  DELETEP(importer);

  // !!! must not free this; this is used by pd_Document !!!
  //FREEP(mimetype);

  //
  // Free any allocated data
  //
  return error;
}

/****************************************************************************/
/****************************************************************************/

//
// wv callbacks to marshall data back to our importer class
//

static int charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_charProc (ps, eachchar, chartype, lid);
}

static int specCharProc (wvParseStruct *ps, U16 eachchar, CHP* achp)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_specCharProc (ps, eachchar, achp);
}

static int eleProc (wvParseStruct *ps, wvTag tag, void *props, int dirty)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_eleProc (ps, tag, props, dirty);
}

static int docProc (wvParseStruct *ps, wvTag tag)
{
	IE_Imp_MsWord_97 * pDocReader = static_cast <IE_Imp_MsWord_97 *> (ps->userData);
	return pDocReader->_docProc (ps, tag);
}
