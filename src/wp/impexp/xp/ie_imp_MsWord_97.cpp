/* AbiWord
 * Copyright (C) 1998-2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <dominicl@seas.upenn.edu>
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

#include "wv.h"
#include "ie_imp_MsWord_97.h"
#include "ie_impGraphic.h"
#include "xap_EncodingManager.h"

#include "ut_string_class.h"
#include "pd_Document.h"
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ut_units.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Frame.h"
#include "ap_Strings.h"

#define X_CheckError(v)			do { if (!(v)) return 1; } while (0)

//
// Just FYI, "dir" and "dom-dir" only get set if BIDI_ENABLED is set.
// Regardless of BIDI_ENABLED, if CHP::fBidi == 1, the BiDi versions
// Of the ico, hps, ftc, lid, etc... are used in place of their "normal"
// Counterparts
//

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
	{"TIME",       F_TIME},
	{"EDITTIME",   F_EDITTIME},
	{"DATE",       F_DATE},
	{"date",       F_DATE},
	{"\\@",        F_DateTimePicture},

	{"FILENAME",   F_FILENAME},
	{"\\filename", F_FILENAME},
	{"PAGE",       F_PAGE},
	{"NUMCHARS",   F_NUMCHARS},
	{"NUMWORDS",   F_NUMWORDS},

	// these below aren't handled by AbiWord, but they're known about
	{"HYPERLINK",  F_HYPERLINK},
	{"PAGEREF",    F_PAGEREF},
	{"EMBED",      F_EMBED},
	{"TOC",        F_TOC},
	{"\\o",        F_TOC_FROM_RANGE},
	{"AUTHOR",     F_AUTHOR},

	{ "*",         F_OTHER}
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

/****************************************************************************/
/****************************************************************************/

#ifdef ENABLE_PLUGINS

// completely generic code to allow this to be a plugin

#include "xap_Module.h"

#define SUPPORTS_ABI_VERSION(a,b,c) (((a==0)&&(b==7)&&(c==15)) ? 1 : 0)

// we use a reference-counted sniffer
static IE_Imp_MsWord_97_Sniffer * m_sniffer = 0;

ABI_FAR extern "C"
int abi_plugin_register (XAP_ModuleInfo * mi)
{

	if (!m_sniffer)
	{
		m_sniffer = new IE_Imp_MsWord_97_Sniffer ();
	}
	else
	{
		m_sniffer->ref();
	}

	UT_ASSERT (m_sniffer);

	mi->name    = "Microsoft Word (tm) Importer";
	mi->desc    = "Import Microsoft Word (tm) Documents";
	mi->version = "0.7.15";
	mi->author  = "Abi the Ant";
	mi->usage   = "No Usage";

	IE_Imp::registerImporter (m_sniffer);
	return 1;
}

ABI_FAR extern "C"
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name    = 0;
	mi->desc    = 0;
	mi->version = 0;
	mi->author  = 0;
	mi->usage   = 0;

	UT_ASSERT (m_sniffer);

	IE_Imp::unregisterImporter (m_sniffer);
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

#endif /* ENABLE_PLUGINS */

/****************************************************************************/
/****************************************************************************/

bool IE_Imp_MsWord_97_Sniffer::recognizeContents (const char * szBuf, 
												  UT_uint32 iNumbytes)
{
	char * magic    = 0;
	int magicoffset = 0;

	magic = "Microsoft Word 6.0 Document";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return true;
		}
	}

	magic = "Documento Microsoft Word 6";
	magicoffset = 2080;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp(szBuf + magicoffset, magic, strlen (magic)))
		{
			return true;
		}
	}

	magic = "MSWordDoc";
	magicoffset = 2112;
	if (iNumbytes > (magicoffset + strlen (magic)))
	{
		if (!strncmp (szBuf + magicoffset, magic, strlen (magic)))
		{
			return true;
		}
	}

	// ok, that didn't work, we'll try to dig through the OLE stream
	if (iNumbytes > 8)
	{
		if (szBuf[0] == (char)0x31 && szBuf[1] == (char)0xbe &&
			szBuf[2] == (char)0 && szBuf[3] == (char)0)
		{
			return true;
		}
		if (szBuf[0] == 'P' && szBuf[1] == 'O' &&
			szBuf[2] == '^' && szBuf[3] == 'Q' && szBuf[4] == '`')
		{
			return true;
		}
		if (szBuf[0] == (char)0xfe && szBuf[1] == (char)0x37 &&
			szBuf[2] == (char)0 && szBuf[3] == (char)0x23)
		{
			return true;
		}

		// OLE magic:
		if (szBuf[0] == (char)0xd0 && szBuf[1] == (char)0xcf &&
			szBuf[2] == (char)0x11 && szBuf[3] == (char)0xe0 &&
			szBuf[4] == (char)0xa1 && szBuf[5] == (char)0xb1 &&
			szBuf[6] == (char)0x1a && szBuf[7] == (char)0xe1)
		{
			return true;
		}
		if (szBuf[0] == (char)0xdb && szBuf[1] == (char)0xa5 &&
			szBuf[2] == (char)0x2d && szBuf[3] == (char)0 &&
			szBuf[4] == (char)0 && szBuf[5] == (char)0)
		{
			return true;
		}
	}
	return false;
}

bool IE_Imp_MsWord_97_Sniffer::recognizeSuffix (const char * szSuffix)
{
	// We recognize both word documents and their template versions
	return (!UT_stricmp(szSuffix,".doc") || 
			!UT_stricmp(szSuffix,".dot"));
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
}

IE_Imp_MsWord_97::IE_Imp_MsWord_97(PD_Document * pDocument)
	: IE_Imp (pDocument), m_iImageCount (0), m_nSections(0)
{
}

/****************************************************************************/
/****************************************************************************/

#define ErrCleanupAndExit(code)  do {wvOLEFree (); return (code); FREEP(password);} while(0)

// TODO: DOM: *actually define these*
#define GetPassword() _getPassword ( m_pDocument->getApp()->getLastFocussedFrame() )

#define ErrorMessage(x) do { XAP_Frame *_pFrame = m_pDocument->getApp()->getLastFocussedFrame(); _errorMessage (_pFrame, (x)); } while (0)

static char * _getPassword (XAP_Frame * pFrame)
{
  return NULL;
}

static void _errorMessage (XAP_Frame * pFrame, int id)
{
  const XAP_StringSet * pSS = XAP_App::getApp ()->getStringSet ();

  const char * text = pSS->getValue (id);

  pFrame->showMessageBox (text,
			  XAP_Dialog_MessageBox::b_O,
			  XAP_Dialog_MessageBox::a_OK);
}

UT_Error IE_Imp_MsWord_97::importFile(const char * szFilename)
{
	wvParseStruct ps;

	int ret = wvInitParser (&ps, (char *)szFilename);
	char * password = NULL;

	if (ret & 0x8000)		/* Password protected? */
	  {
	    password = GetPassword();

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
		  }
	      }

	    FREEP(password);
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
	
	wvText(&ps);	
	wvOLEFree();

	return UT_OK;
}

void IE_Imp_MsWord_97::pasteFromBuffer (PD_DocumentRange *, 
										unsigned char *, unsigned int, const char *)
{
	// nada
}

void IE_Imp_MsWord_97::_flush ()
{
	if (m_pTextRun.size())
	{
		if (!m_pDocument->appendSpan(m_pTextRun.ucs_str(), m_pTextRun.size()))
		{
			UT_DEBUGMSG(("DOM: error appending text run\n"));
			return;
		}
		m_pTextRun.clear ();
	}
}

void IE_Imp_MsWord_97::_appendChar (UT_UCSChar ch)
{
    m_pTextRun += ch;
}

/****************************************************************************/
/****************************************************************************/

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
	case DOCEND:	
	default:
		break;
	}

	return 0;
}

int IE_Imp_MsWord_97::_charProc (wvParseStruct *ps, U16 eachchar, U8 chartype, U16 lid)
{
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

	// TODO: i'm not sure if this is needed any more
	if (ps->fieldstate)
	{
		xxx_UT_DEBUGMSG(("DOM: fieldstate\n"));
		if(this->_fieldProc (ps, eachchar, chartype, lid))
			return 0;
	}

	// take care of any oddities in Microsoft's character encoding
	if (chartype == 1 && eachchar == 146) 
		eachchar = 39; // apostrophe

	//
	// Append the character to our character buffer
	//
	this->_appendChar ((UT_UCSChar) eachchar);
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
		wvStream_goto(ps->data, achp->fcPic_fcObj_lTagObj);
		
		wvGetPICF(wvQuerySupported(&ps->fib, NULL), &picf, ps->data);		
		fil = picf.rgb;
		
		if (wv0x01(&blip, fil, picf.lcb - picf.cbHeader))
		{
			this->_handleImage(&blip, picf.dxaGoal, picf.dyaGoal);
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

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

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
	XML_Char propBuffer [DOC_PROPBUFFER_SIZE];
	UT_String props;

	// flush any character runs
	this->_flush ();
		
	// page-margin-left
	sprintf(propBuffer,
			"page-margin-left:%s;", 
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dxaLeft) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-right
	sprintf(propBuffer,
			"page-margin-right:%s;", 
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dxaRight) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-top
	sprintf(propBuffer,
			"page-margin-top:%s;", 
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaTop) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-bottom
	sprintf(propBuffer,
			"page-margin-bottom:%s;", 
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaBottom) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-header
	sprintf(propBuffer,
			"parge-margin-header:%s;",
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaHdrTop) / 1440), "1.4"));
	props += propBuffer;

	// page-margin-footer
	sprintf(propBuffer,
			"parge-margin-footer:%s;",
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dyaHdrBottom) / 1440), 
											  "1.4"));
	props += propBuffer;

	// columns
	if (asep->ccolM1) {
		// number of columns
		sprintf(propBuffer,
				"columns:%d;", (asep->ccolM1+1));
		props += propBuffer;

		// columns gap
		sprintf(propBuffer,
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
	sprintf(propBuffer,
			"section-space-after:%s",
			UT_convertInchesToDimensionString(DIM_IN, (((float)asep->dzaGutter) / 1440), "1.4"));
	props += propBuffer;

	//
	// TODO: headers/footers, section breaks
	//

	{
		//
		// all of this data is related to Abi's <pagesize> tag
		//
		double page_width  = 0.0;
		double page_height = 0.0;
		double page_scale  = 1.0;

		// TODO: paper type is currently unhandled
		const char * paper_name = 0;

		if (asep->dmOrientPage == 1)
			m_pDocument->m_docPageSize.setLandscape ();
		else
			m_pDocument->m_docPageSize.setPortrait ();

		page_width = asep->xaPage / 1440.0;
		page_height = asep->yaPage / 1440.0;

		UT_DEBUGMSG(("DOM: pagesize: (landscape: %d) (width: %f) (height: %f) (paper-type: %d)\n",
					 asep->dmOrientPage, page_width, page_height, asep->dmPaperReq));

		paper_name = s_mapPageIdToString (asep->dmPaperReq);

		if (paper_name) {
			// we found a paper name
			m_pDocument->m_docPageSize.Set (paper_name);
		}
		else {
			// this can cause us to SEGV, so I'm not sure if we even want to set it...
			// TODO: make more mappings for s_MapPageIdToString as I discover them
			m_pDocument->m_docPageSize.Set (page_width, page_height, fp_PageSize::inch);
		}
		m_pDocument->m_docPageSize.setScale(page_scale);
	}

	xxx_UT_DEBUGMSG (("DOM: the section properties are: '%s'\n", props.c_str()));

	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)props.c_str();
	propsArray[2] = 0;

	if (!m_pDocument->appendStrux(PTX_Section, (const XML_Char **)propsArray))
	{
		UT_DEBUGMSG (("DOM: error appending section props!\n"));
		return 1;
	}

	// increment our section count
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

	if (asep->bkc > 1 && m_nSections > 1) // don't apply on the 1st page
	{
		// new section
		if (!m_pDocument->appendStrux(PTX_Block, (const XML_Char **)NULL))
		{
			UT_DEBUGMSG (("DOM: error appending new block\n"));
			return 1;
		}

		UT_UCSChar ucs = UCS_FF;
		switch (asep->bkc) {
		case 1: 
			ucs = UCS_VTAB;
			X_CheckError(m_pDocument->appendSpan(&ucs,1)); 
			break;
			
		case 2:
			X_CheckError(m_pDocument->appendSpan(&ucs,1)); 
			break;
			
		case 3: // TODO: handle me better (not even)
			X_CheckError(m_pDocument->appendSpan(&ucs,1)); 
			break;
			
		case 4: // TODO: handle me better (not odd)
			X_CheckError(m_pDocument->appendSpan(&ucs,1)); 
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
	return 0;
}

int IE_Imp_MsWord_97::_beginPara (wvParseStruct *ps, UT_uint32 tag,
								  void *prop, int dirty)
{
	PAP *apap = static_cast <PAP *>(prop);

	XML_Char * propsArray[3];
	XML_Char propBuffer [DOC_PROPBUFFER_SIZE];
	UT_String props;

	//
	// TODO: lists, exact line heights (and, eventually, tables)
	//

	// first, flush any character data in any open runs
	this->_flush ();

#ifdef BIDI_ENABLED

	// DOM TODO: i think that this is right
	if (apap->fBidi == 1) {
		props += "dom-dir:rtl;";
	} else {
		props += "dom-dir:ltr;";
	}

#endif

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
		UT_UCSChar ucs = UCS_FF;
		m_pDocument->appendSpan(&ucs,1);
	}

	// widowed/orphaned lines
	if (!apap->fWidowControl) {
		// these AbiWord properties give the same effect
		props += "orphans:0;widows:0;";
	}

	// line spacing (single-spaced, double-spaced, etc.)
	if (apap->lspd.fMultLinespace) {
		sprintf(propBuffer,
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
		sprintf(propBuffer,
				"margin-right:%s;", 
				UT_convertInchesToDimensionString(DIM_IN, (((float)apap->dxaRight) / 1440), 
												  "1.4"));
		props += propBuffer;
	}

	// margin-left
	if (apap->dxaLeft) {
		sprintf(propBuffer,
				"margin-left:%s;", 
				UT_convertInchesToDimensionString(DIM_IN, (((float)apap->dxaLeft) / 1440), 
												  "1.4"));
		props += propBuffer;
	}

	// margin-left first line (indent)
	if (apap->dxaLeft1) {
		sprintf(propBuffer,
				"text-indent:%s;", 
				UT_convertInchesToDimensionString(DIM_IN, (((float)apap->dxaLeft1) / 1440), 
												  "1.4"));
		props += propBuffer;
	}

	// margin-top
	if (apap->dyaBefore) {
		sprintf(propBuffer,
				"margin-top:%dpt;", (apap->dyaBefore / 20));
		props += propBuffer;
	}

	// margin-bottom
	if (apap->dyaAfter) {
		sprintf(propBuffer,
				"margin-bottom:%dpt;", (apap->dyaAfter / 20));
	}

	// tab stops
	if (apap->itbdMac) {
		strcpy(propBuffer, "tabstops:");

		for (int iTab = 0; iTab < apap->itbdMac; iTab++) {
			sprintf(propBuffer + strlen(propBuffer),
					"%s/",
					UT_convertInchesToDimensionString(DIM_IN, (((float)apap->rgdxaTab[iTab]) 
															   / 1440), "1.4"));
			switch (apap->rgtbd[iTab].jc) {
			case 1:
				strcat(propBuffer, "C,");
				break;
			case 2:
				strcat(propBuffer, "R,");
				break;
			case 3:
				strcat(propBuffer, "D,");
				break;	
			case 4:
				strcat(propBuffer, "B,");
				break;
			case 0:
			default:
				strcat(propBuffer, "L,");
				break;
			}
		}
		// replace final comma with a semi-colon
		propBuffer[strlen(propBuffer)-1] = ';';
		props += propBuffer;
	}

	// remove the trailing semi-colon
	props [props.size()-1] = 0;

	xxx_UT_DEBUGMSG(("Dom: the paragraph properties are: '%s'\n",props.c_str()));
	
	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)props.c_str();
	propsArray[2] = 0;
	
	if (!m_pDocument->appendStrux(PTX_Block, (const XML_Char **)propsArray))
	{
		UT_DEBUGMSG(("DOM: error appending paragraph block\n"));
		return 1;
	}
	return 0;
}

int IE_Imp_MsWord_97::_endPara (wvParseStruct *ps, UT_uint32 tag,
								void *prop, int dirty)
{
	// nothing is needed here
	return 0;
}

int IE_Imp_MsWord_97::_beginChar (wvParseStruct *ps, UT_uint32 tag,
								  void *prop, int dirty)
{
	CHP *achp = static_cast <CHP *>(prop);

	XML_Char * propsArray[3];
	XML_Char propBuffer [DOC_PROPBUFFER_SIZE];
	UT_String props;

	//
	// TODO: set char tolower if fSmallCaps && fLowerCase, possibly some list stuff
	//

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

#ifdef BIDI_ENABLED

	// Our textrun automatically sets "dir" for us. We just need too keep
	// LTR runs separate from RTL runs

	if (achp->fBidi) {
		// TODO: do we need to do anything?
	}

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
		sprintf(propBuffer, 
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
		sprintf(propBuffer, 
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
	U16 hps = (achp->fBidi ? achp->hpsBidi : achp->hps);
	sprintf(propBuffer, 
			"font-size:%dpt;", (hps/2));
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
	
	UT_ASSERT(fname != NULL);
	xxx_UT_DEBUGMSG(("font-family = %s\n", fname));
		
	props += "font-family:";
	props += fname;
	FREEP(fname);

	xxx_UT_DEBUGMSG(("DOM: character properties are: '%s'\n", props.c_str()));
	
	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)props.c_str();
	propsArray[2] = 0;
	if (!m_pDocument->appendFmt((const XML_Char **)propsArray))
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

	static U16 *which = 0;
	static int i = 0, depth = 0;
	char *a = 0;
	static char *c = 0;
	static int ret = 0;
	
	if (eachchar == 0x13) // beginning of a field
	{
	    a = 0;
	    ret = 1;
	    if (depth == 0)
		{
			which = m_command;
			m_command[0] = 0;
			m_argument[0] = 0;
			i = 0;
		}
	    depth++;
	}
	else if (eachchar == 0x14) // field trigger
	{
	    if (depth == 1)
		{
			m_command[i] = 0;
			c = wvWideStrToMB (m_command);
			if (this->_handleCommandField(c))
				ret = 1;
			else
				ret = 0;
			
			xxx_UT_DEBUGMSG(("DOM: Field: command %s, ret is %d\n", 
							 wvWideStrToMB(command), ret));
			wvFree(c);
			which = m_argument;
			i = 0;
		}
	}
	
	if (i >= FLD_SIZE)
	{
	    UT_DEBUGMSG(("DOM: Something completely absurd in the fields implementation!\n"));
	    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	    return 1;
	}

	if (!which) {
		UT_DEBUGMSG(("DOM: _fieldProc - 'which' is null\n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return 1;
	}
	
	if (chartype)
		which[i] = wvHandleCodePage(eachchar, lid);
	else
		which[i] = eachchar;
	
	i++;
	
	if (eachchar == 0x15) // end of field marker
	{
	    depth--;
	    if (depth == 0)
		{
			which[i] = 0;
		}
	}
	return ret;
}

bool IE_Imp_MsWord_97::_handleCommandField (char *command)
{
	Doc_Field_t tokenIndex = F_OTHER;
	char *token = NULL;
	
	xxx_UT_DEBUGMSG(("DOM: handleCommandField '%s'\n", command));
	
	const XML_Char* atts[3];
	atts[0] = "type";
	atts[1] = 0;
	atts[2] = 0;
	
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

	    default:
			// unhandled field type
			continue;
	    }

		if (!m_pDocument->appendObject (PTO_Field, (const XML_Char**)atts))
		{
			UT_DEBUGMSG(("Dom: couldn't append field (type = '%s')\n", atts[1]));
		}
	}
	
	return true;
}

UT_Error IE_Imp_MsWord_97::_handleImage (Blip * b, long width, long height)
{
	const char * mimetype     = 0;
	
	UT_ByteBuf * pBBPNG       = 0;
	UT_ByteBuf * buf          = 0;
	IE_ImpGraphic * converter = 0;
	UT_Error err              = UT_OK;
	
	switch(b->type) 
	{
	// currently handled image types
	case msoblipDIB:
	case msoblipPNG:
		mimetype = UT_strdup("image/png"); // this will get freed for us elsewhere
		break;

	// currently unhandled image types
	case msoblipWMF:
	case msoblipEMF:
	case msoblipPICT:
	case msoblipJPEG:
	default:
		return UT_ERROR;
	}
	
	buf = new UT_ByteBuf();

	// suck the data into the ByteBuffer

#if 0
	// TODO: make this call work
	buf->insertFromFile (0, (FILE *)(b->blip.bitmap.m_pvBits));
#else
	int data = 0;

	while (EOF != (data = getc((FILE*)(b->blip.bitmap.m_pvBits))))
		buf->append((UT_Byte*)&data, 1);
#endif
	
	if(b->type == msoblipDIB) {
		// this is just a BMP file, so we'll use the BMP image importer
		// to convert it to a PNG for us.
		if ((err = IE_ImpGraphic::constructImporter("", IEGFT_DIB, &converter)) != UT_OK)
		{
			UT_DEBUGMSG(("Could not construct importer for BMP\n"));
			DELETEP(buf);
			return err;
		}
	}
	
	//
	// This next bit of code will set up our properties based on the image attributes
	//

	XML_Char propBuffer[128];
	propBuffer[0] = 0;
	sprintf(propBuffer, "width:%fin; height:%fin", 
			(double)width / (double)1440, 
			(double)height / (double)1440);
	
	XML_Char propsName[32];
	propsName[0] = 0;
	sprintf(propsName, "image%d", m_iImageCount++);
	
	const XML_Char* propsArray[5];
	propsArray[0] = (XML_Char *)"props";
	propsArray[1] = (XML_Char *)propBuffer;
	propsArray[2] = (XML_Char *)"dataid";
	propsArray[3] = (XML_Char *)propsName;
	propsArray[4] = 0;

	if (converter == NULL) // we never converted from BMP->PNG
	{
		pBBPNG = buf;
	}
	else 
	{
		if (!converter->convertGraphic(buf, &pBBPNG))
		{
			UT_DEBUGMSG (("Could not convert from BMP to PNG\n"));
			DELETEP(buf);
			DELETEP(converter);
			return UT_ERROR;
		}
	}
	
	if (!m_pDocument->appendObject (PTO_Image, propsArray))
	{
		UT_DEBUGMSG (("Could not append object\n"));
		DELETEP(buf);

		if (converter)
		{
			DELETEP(pBBPNG);
			DELETEP(converter);
		}

		return UT_ERROR;
	}

	if (!m_pDocument->createDataItem((char*)propsName, false,
									 pBBPNG, (void*)mimetype, NULL))
	{
		UT_DEBUGMSG (("Could not create data item\n"));

		DELETEP(buf);
		if (converter)
		{
			DELETEP(pBBPNG);
			DELETEP(converter);
		}
		return UT_ERROR;
	}
	
	//
	// Free any allocated data
	//

	DELETEP(converter);
	return UT_OK;
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
