/* -*- mode: C++; tab-width: 4; c-basic-offset: 4;  indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002,2003 Tomas Frydrych
 * Copyright (C) 2013-2016 Hubert Figuiere
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_base64.h"
#include "ut_misc.h"
#include "ut_rand.h"
#include "ut_uuid.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"
#include "xad_Document.h"
#include "xap_Strings.h"
#include "pt_PieceTable.h"
#include "pl_Listener.h"
#include "ie_imp.h"
#include "ie_exp.h"
#include "pf_Frag_Strux.h"
#include "pp_Property.h"
#include "pd_Style.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_FmtMark.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "pf_Frag.h"
#include "pd_Iterator.h"
#include "fd_Field.h"
#include "po_Bookmark.h"
#include "fl_AutoNum.h"
#include "xap_Frame.h"
#include "xap_App.h"
#include "xap_Prefs.h"
#include "ap_Prefs.h"
#include "ap_Strings.h"
#include "ut_units.h"
#include "ut_string_class.h"
#include "ut_sleep.h"
#include "ut_path.h"
#include "ut_locale.h"
#include "pp_Author.h"

// these are needed because of the exportGetVisDirectionAtPosition() mechanism
#include "fp_Run.h"
#include "fl_BlockLayout.h"
#include "fl_DocListener.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"

#include "ut_go_file.h"
#include "ut_std_string.h"

/*! 
 * Helper class to import Page Referenced Images
 */

ImagePage::ImagePage(UT_UTF8String & sImageId, UT_sint32 iPage, double xInch, double yInch, const char * pzProps) : m_sImageId(sImageId), m_iPage(iPage),m_xInch(xInch),m_yInch(yInch)
{
	m_sProps = pzProps;
}

ImagePage::~ImagePage(void)
{
}

const UT_UTF8String * ImagePage::getImageId(void) const
{
	return &m_sImageId;
}

UT_sint32 ImagePage::getPageNo(void) const
{
	return m_iPage;
}

double ImagePage::getXInch(void) const
{
	return m_xInch;
}

double ImagePage::getYInch(void) const
{
	return m_yInch;
}

const	UT_UTF8String * ImagePage::getProps(void) const
{
	return &m_sProps;
}


/*!
 * Helpder class to import Page Referenced TextBoxes
 */
TextboxPage::TextboxPage(UT_sint32 iPage, double xInch, double yInch,const char * pzProps, UT_ByteBuf & sContent) : m_iPage(iPage),m_xInch(xInch),m_yInch(yInch)
{
	m_sProps = pzProps;
	m_sContent.append(sContent.getPointer(0),sContent.getLength());
}
TextboxPage::~TextboxPage(void)
{

}
const UT_ByteBuf * TextboxPage::getContent(void) const
{
	return &m_sContent;
}
UT_sint32 TextboxPage::getPageNo(void) const
{
	return m_iPage;
}
double TextboxPage::getXInch(void) const
{
	return m_xInch;
}
double TextboxPage::getYInch(void) const
{
	return m_yInch;
}
const UT_UTF8String * TextboxPage::getProps(void) const
{
	return &m_sProps;
}


// our currently used DTD
#define ABIWORD_FILEFORMAT_VERSION "1.1"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct _dataItemPair
{
	UT_ByteBuf* pBuf;
	const void*	pToken;
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// perhaps this should be a magic "unknown" or "NULL" value,
// but now we just depend on save() never being called without
// a previous saveAs() (which specifies a type)
PD_Document::PD_Document()
	: AD_Document(),
	  m_docPageSize("A4"),
	  m_ballowListUpdates(false),
	  m_pPieceTable(0),
      m_hDocumentRDF( new PD_DocumentRDF( this )),
	  m_lastOpenedType(IEFT_Bogus), // used to be: IE_Imp::fileTypeForSuffix(".abw"))
	  m_lastSavedAsType(IEFT_Bogus), // used to be: IE_Exp::fileTypeForSuffix(".abw")
	  m_bDoingPaste(false),
	  m_bAllowInsertPointChange(true),
	  m_bRedrawHappenning(false),
	  m_bLoading(false),
	  m_bLockedStyles(false),        // same as lockStyles(false)
	  m_indexAP(0xffffffff),
	  m_bDontImmediatelyLayout(false),
	  m_iLastDirMarker(0),
	  m_pVDBl(NULL),
	  m_pVDRun(NULL),
	  m_iVDLastPos(0xffffffff),
	  m_iNewHdrHeight(0), 
	  m_iNewFtrHeight(0),
	  m_bMarginChangeOnly(false),
	  m_bVDND(false),
	  m_iCRCounter(0),
	  m_iUpdateCount(0),
	  m_bIgnoreSignals(false),
	  m_bCoalescingMask(false),
	  m_bShowAuthors(true),
	  m_bExportAuthorAtts(false), //should be false by default. Set true to test
	  m_iMyAuthorInt(-1),
	  m_iLastAuthorInt(-1),
	  m_iStruxCount(0)
{
	XAP_App::getApp()->getPrefs()->getPrefsValueBool(AP_PREF_KEY_LockStyles,&m_bLockedStyles);
	UT_ASSERT(isOrigUUID());
#ifdef PT_TEST
	m_pDoc = this;
#endif

	const gchar *name = g_get_real_name();
	if(strcmp(name, "Unknown") == 0)
		name = g_get_user_name();
	gchar *utf8name = g_locale_to_utf8(name, -1, NULL, NULL, NULL);
	if (utf8name != NULL)
	{
		m_sUserName = utf8name;
		g_free(utf8name);
	}
	else
	{
		m_sUserName = "Unknown";
	}
}

PD_Document::~PD_Document()
{
	// ideally all connections would have been removed BEFORE
	// we ever reach this destructor (for example by disconnecting
	// listeners in the frame before deleting the document); this
	// will do for now though
	removeConnections();

	if (m_pPieceTable)
		delete m_pPieceTable;

	_destroyDataItemData();

	UT_VECTOR_PURGEALL(fl_AutoNum*, m_vecLists);

	UT_VECTOR_PURGEALL(pp_Author *, m_vecAuthors);
	UT_VECTOR_PURGEALL(ImagePage *, m_pPendingImagePage);
	UT_VECTOR_PURGEALL(TextboxPage *, m_pPendingTextboxPage);
	// we do not purge the contents of m_vecListeners
	// since these are not owned by us.

	// TODO: delete the key/data pairs
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void PD_Document::setMetaDataProp ( const std::string & key,
									const std::string & value )
{
	m_metaDataMap[key] = value;

	const PP_PropertyVector atts = {
		PT_DOCPROP_ATTRIBUTE_NAME,"metadata"
	};
	const PP_PropertyVector props = {
		key, value
	};
	createAndSendDocPropCR(atts,props);
}

UT_sint32  PD_Document::getNextCRNumber(void)
{
	m_iCRCounter++;
	return m_iCRCounter;
}

bool PD_Document::getMetaDataProp (const std::string & key, std::string & outProp) const
{
	auto iter = m_metaDataMap.find(key);
	bool found = (iter != m_metaDataMap.end());

	if (found && !iter->second.empty()) {
		outProp = iter->second;
	} else {
		outProp = "";
	}

	return found;
}

// RIVERA TODO not working and may not be needed
void PD_Document::setAnnotationProp ( const std::string & /*key*/,
									  const std::string & /*value*/ )
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return; // TODO something!
}
bool PD_Document::getAnnotationProp (const std::string & /*key*/, std::string & outProp) const
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	bool found = true;//false;
	outProp = "Dummy value";

	return found;
}


std::string PD_Document::getMailMergeField(const std::string & key) const
{
	auto iter = m_mailMergeMap.find(key);
	if(iter != m_mailMergeMap.end()) {
		return iter->second;
	}
	return "";
}

bool PD_Document::mailMergeFieldExists(const std::string & key) const
{
	return (m_mailMergeMap.find(key) != m_mailMergeMap.end());
}

void PD_Document::setMailMergeField(const std::string & key,
									const std::string & value)
{
	m_mailMergeMap[key] = value;
}

void PD_Document::clearMailMergeMap()
{
	m_mailMergeMap.clear();
}

void PD_Document::setMarginChangeOnly(bool b)
{
	m_bMarginChangeOnly = b;
}

bool PD_Document::isMarginChangeOnly(void) const
{
	return m_bMarginChangeOnly;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


void PD_Document::removeCaret(const std::string& sCaretID)
{
	UT_GenericVector<AV_View *> vecViews;
	getAllViews(&vecViews);
	UT_sint32 i = 0;
	for(i = 0; i<vecViews.getItemCount(); i++)
	{
		FV_View * pView = static_cast<FV_View *>(vecViews.getNthItem(i));
		pView->removeCaret(sCaretID);
	}
}

/////////////////////////////////////////////////////////////////

void PD_Document::addPageReferencedImage(UT_UTF8String & sImageId, UT_sint32 iPage, double xInch, double yInch, const char * pzProps)
{
	m_pPendingImagePage.addItem(new ImagePage(sImageId, iPage, xInch, yInch, pzProps));
}

void PD_Document::addPageReferencedTextbox(UT_ByteBuf & sContent,UT_sint32 iPage, double xInch, double yInch,const char * pzProps)
{
	m_pPendingTextboxPage.addItem(new TextboxPage(iPage, xInch,yInch,pzProps, sContent));
}

ImagePage * PD_Document::getNthImagePage(UT_sint32 iImagePage)
{
	if(iImagePage < m_pPendingImagePage.getItemCount())
	{
		return m_pPendingImagePage.getNthItem(iImagePage);
	}
	return NULL;
}

TextboxPage * PD_Document::getNthTextboxPage(UT_sint32 iTextboxPage)
{
	if(iTextboxPage < m_pPendingTextboxPage.getItemCount())
	{
		return m_pPendingTextboxPage.getNthItem(iTextboxPage);
	}
	return NULL;
}

void PD_Document::clearAllPendingObjects(void)
{
	UT_VECTOR_PURGEALL(ImagePage *, m_pPendingImagePage);
	UT_VECTOR_PURGEALL(TextboxPage *, m_pPendingTextboxPage);
	m_pPendingImagePage.clear();
	m_pPendingTextboxPage.clear();
}


UT_sint32 PD_Document::getNumAuthors() const
{
	return m_vecAuthors.getItemCount();
}

pp_Author *  PD_Document::getNthAuthor(UT_sint32 i) const
{
	return m_vecAuthors.getNthItem(i);
}

pp_Author *  PD_Document::addAuthor(UT_sint32 iAuthor)
{
	UT_DEBUGMSG(("creating author with int %d \n",iAuthor));
	m_vecAuthors.addItem(new pp_Author(this, iAuthor));
	return 	m_vecAuthors.getNthItem(m_vecAuthors.getItemCount()-1);
}

/** private method share by send*AuthorCR()
 *  \param attrName the value of the PT_DOCPROP_ATTRIBUTE_NAME attribute
 */
bool PD_Document::_sendAuthorCR(const char *attrName, pp_Author *pAuthor)
{
	const PP_PropertyVector atts = {
		PT_DOCPROP_ATTRIBUTE_NAME, attrName
	};
	PP_PropertyVector props;

	_buildAuthorProps(pAuthor, props);
	UT_return_val_if_fail(!props.empty(), false);
	return createAndSendDocPropCR(atts, props);
}

bool PD_Document::sendAddAuthorCR(pp_Author * pAuthor)
{
	return _sendAuthorCR("addauthor", pAuthor);
}


bool PD_Document::sendChangeAuthorCR(pp_Author * pAuthor)
{
	return _sendAuthorCR("changeauthor", pAuthor);
}

bool PD_Document::_buildAuthorProps(pp_Author * pAuthor, PP_PropertyVector & props)
{
	const PP_AttrProp * pAP = pAuthor->getAttrProp();
	UT_uint32 iCnt = pAP->getPropertyCount();
	props.clear();
	UT_DEBUGMSG(("_buildAuthorProps getAuthorInt %d \n",pAuthor->getAuthorInt()));
	props.push_back("id");
	props.push_back(UT_std_string_sprintf("%d",pAuthor->getAuthorInt()));
	const gchar * szName = NULL;
	const gchar * szValue = NULL;

	for (UT_uint32 i = 0; i < iCnt; i++) {
		pAP->getNthProperty(i, szName, szValue);
		if (*szValue) {
			props.push_back(szName);
			props.push_back(szValue);
		}
	}
	return true;
}

UT_sint32 PD_Document::findFirstFreeAuthorInt(void) const
{
	UT_sint32 i= 0;
	for(i=0;i<1000;i++)
	{
		if(getAuthorByInt(i) == NULL)
			break;
	}
	return i;
}
pp_Author * PD_Document::getAuthorByInt(UT_sint32 i) const
{
	UT_sint32 j = 0;
	for(j=0; j< m_vecAuthors.getItemCount(); j++)
	{
		if(m_vecAuthors.getNthItem(j)->getAuthorInt() == i)
			return m_vecAuthors.getNthItem(j);
	}
	return NULL;
}
/*!
 * True if the Author attributes should be exported.
 */
bool  PD_Document::isExportAuthorAtts(void) const
{
	return m_bExportAuthorAtts;
}

void  PD_Document::setExportAuthorAtts(bool bAuthor)
{
	m_bExportAuthorAtts = bAuthor;
}
/*!
 * Returns the integer mapping for this session
 */
UT_sint32 PD_Document::getMyAuthorInt(void) const
{
	return m_iMyAuthorInt;
}

void PD_Document::setMyAuthorInt(UT_sint32 i)
{
	m_iMyAuthorInt = i;	
}
/*!
 * Returns the most recently received author int
 */
UT_sint32 PD_Document::getLastAuthorInt(void) const
{
	return m_iLastAuthorInt;
}

/*!
 * Add the current documents UUID as the author to the attribute list if
 * the author attribute is not present.
 * Returns true if author attribute is already present. false if we add it.
 */
bool PD_Document:: addAuthorAttributeIfBlank(PP_PropertyVector & atts)
{
	// Set Author attribute
	const std::string & author = PP_getAttribute(PT_AUTHOR_NAME, atts);
	if (!author.empty()) {
		m_iLastAuthorInt = stoi(author);
		return true;
	}
	if(getMyAuthorInt() == -1)
	{
		UT_sint32 k = findFirstFreeAuthorInt();
		setMyAuthorInt(k);
		m_iLastAuthorInt = k;
		pp_Author * pA = addAuthor(k);
		sendAddAuthorCR(pA);
	}

	m_iLastAuthorInt = getMyAuthorInt();
	atts.push_back(PT_AUTHOR_NAME);
	atts.push_back(UT_std_string_sprintf("%d", getMyAuthorInt()));

	return false;
}

void PD_Document::setShowAuthors(bool bAuthors)
{
	bool bChanged = (bAuthors != m_bShowAuthors);
	m_bShowAuthors = bAuthors;
	//
	// Could do this with a listner but that might screw up other stuff
	//
	if(bChanged)
	{
		UT_GenericVector<AV_View *> vecViews;
		getAllViews(&vecViews);
		UT_sint32 i = 0;
		for(i = 0; i<vecViews.getItemCount(); i++)
		{
			FV_View * pView = static_cast<FV_View *>(vecViews.getNthItem(i));
			FL_DocLayout * pL = pView->getLayout();
			pL->refreshRunProperties();
			pView->updateScreen(false ); // redraw the whole thing
		}
	}

}

/*!
 * Add the current documents UUID as the author to the ppAttrProps 
 * if the attribute is not set.
 * returns true if author is set
 */
bool PD_Document::addAuthorAttributeIfBlank( PP_AttrProp *&p_AttrProp)
{
	xxx_UT_DEBUGMSG(("Doing addAuthorAttributeIfBlank PAP \n"));
	std::string sNum;
	if(getMyAuthorInt() == -1)
	{
		UT_sint32 k = findFirstFreeAuthorInt();
		setMyAuthorInt(k);
		pp_Author * pA = addAuthor(k);
		sendAddAuthorCR(pA);
	}
	sNum = UT_std_string_sprintf("%d",getMyAuthorInt());
	m_iLastAuthorInt = getMyAuthorInt();
	if(!p_AttrProp)
	{
		static PP_AttrProp p;
		p.setAttribute(PT_AUTHOR_NAME,sNum.c_str());
		p_AttrProp = &p;
		return false;
	}
	const gchar * sz = NULL;
	if(p_AttrProp->getAttribute(PT_AUTHOR_NAME,sz))
	{
		xxx_UT_DEBUGMSG(("Found athor att %s \n",sz));
		if(sz)
		{
			m_iLastAuthorInt = atoi(sz);
			return true;
		}
	}
	p_AttrProp->setAttribute(PT_AUTHOR_NAME,sNum.c_str());
	return false;
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

static std::string UT_filenameToUri(const std::string & rhs) {
	char * uri = UT_go_filename_to_uri (rhs.c_str());
	if(!uri) {
		return "";
	}
	std::string res(uri);
	g_free (uri);

	return res;
}

static void buildTemplateList(std::string *template_list, const std::string & base)
{
	UT_LocaleInfo locale(UT_LocaleInfo::system());

	std::string lang (locale.getLanguage());
	std::string terr (locale.getTerritory());

	/* try *6* combinations of the form:
	   1) /templates/normal.awt-en_US
	   2) /templates/normal.awt-en
	   3) /templates/normal.awt
	   4) /templates/normal.awt-en_US
	   5) /templates/normal.awt-en
	   6) /templates/normal.awt
	*/

	std::string user_template_base (XAP_App::getApp()->getUserPrivateDirectory());
#if defined(_WIN32)
	user_template_base += UT_std_string_sprintf("\\templates\\%s", base.c_str());
#else
	user_template_base += UT_std_string_sprintf("/templates/%s", base.c_str());
#endif
	std::string global_template_base (XAP_App::getApp()->getAbiSuiteLibDir());
#if defined(_WIN32)
	global_template_base += UT_std_string_sprintf("\\templates\\%s", base.c_str());
#else
	global_template_base += UT_std_string_sprintf("/templates/%s", base.c_str());
#endif

	template_list[0] = UT_std_string_sprintf ("%s-%s_%s", user_template_base.c_str(), lang.c_str(), terr.c_str());
	template_list[1] = UT_std_string_sprintf ("%s-%s", user_template_base.c_str(), lang.c_str());
	template_list[2] = user_template_base;

	if (!XAP_App::getApp()->findAbiSuiteLibFile(template_list[5],base.c_str(),"templates"))
		template_list[5] = global_template_base; // always try to load global normal.awt last

	std::string xbase = base;

	xbase += "-";
	xbase += lang;

	if (!XAP_App::getApp()->findAbiSuiteLibFile(template_list[4],xbase.c_str(),"templates"))
		template_list[4] = UT_std_string_sprintf ("%s-%s", global_template_base.c_str(), lang.c_str());

	xbase += "_";
	xbase += terr;

	if (!XAP_App::getApp()->findAbiSuiteLibFile(template_list[3],xbase.c_str(),"templates"))
		template_list[3] = UT_std_string_sprintf ("%s-%s_%s", global_template_base.c_str(), lang.c_str(), terr.c_str());

	for(int i = 0; i < 6; i++) {
		template_list[i] = UT_filenameToUri(template_list[i]);
	}
}

UT_Error PD_Document::importFile(const char * szFilename, int ieft,
								 bool markClean, bool bImportStylesFirst,
								 const char* impProps)
{
	return _importFile(szFilename, ieft, markClean, bImportStylesFirst, true, impProps);
}

UT_Error PD_Document::importFile(GsfInput * input, int ieft,
								 bool markClean, bool bImportStylesFirst,
								 const char* impProps)
{
	return _importFile(input, ieft, markClean, bImportStylesFirst, true, impProps);
}

UT_Error PD_Document::_importFile(const char * szFilename, int ieft,
								  bool markClean, bool bImportStylesFirst,
								  bool bIsImportFile, const char* impProps)
{
	GsfInput * input;

	input = UT_go_file_open(szFilename, NULL);
	if (!input)
	{
		UT_DEBUGMSG(("PD_Document::importFile -- invalid filename\n"));
		return UT_INVALIDFILENAME;
	}	

	UT_Error result = _importFile(input, ieft, markClean, bImportStylesFirst, bIsImportFile, impProps);

	g_object_unref (G_OBJECT (input));

	return result;
}

void PD_Document::updateStatus(void)
{
	m_iStruxCount++;
	UT_sint32 updateRate =100;
	UT_sint32 iStruxDiv = m_iStruxCount/updateRate;
	if(iStruxDiv*updateRate == m_iStruxCount)
	{
		xxx_UT_DEBUGMSG(("UpdateStatus StruxCount %d \n",m_iStruxCount));
		XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
		if(pFrame)
			pFrame->nullUpdate();
		AP_StatusBar * pBar = getStatusBar();
		if(pFrame && pBar)
		{
			const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
			UT_UTF8String msg (pSS->getValue(XAP_STRING_ID_MSG_ParagraphsImported));
			UT_UTF8String msg2;
			UT_UTF8String_sprintf(msg2," %d",m_iStruxCount);
			msg += msg2;
			pBar->setStatusMessage(static_cast<const gchar *>(msg.utf8_str()));
			pBar->setStatusProgressValue(m_iStruxCount);
		}
	}
}

UT_Error PD_Document::_importFile(GsfInput * input, int ieft,
								  bool markClean, bool bImportStylesFirst,
								  bool bIsImportFile, const char* impProps)
{
	if (!input)
	{
		UT_DEBUGMSG(("PD_Document::importFile -- invalid filename\n"));
		return UT_INVALIDFILENAME;
	}

	const char * szFilename = gsf_input_name (input);
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	if(pFrame)
	{
		pFrame->nullUpdate();
	}
    AP_StatusBar * pStatusBar = getStatusBar();
	if(pFrame && pStatusBar)
	{
		//
		// Show a pulsing status bar since we don't know how big the document
		// is.
		//
		pStatusBar->setStatusProgressType(0,100,PROGRESS_INDEFINATE);
		pStatusBar->showProgressBar();
		pFrame->nullUpdate();
	}	

	m_pPieceTable = new pt_PieceTable(this);
	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::importFile -- could not construct piece table\n"));
		return UT_NOPIECETABLE;
	}

	m_bLoading = true;
	m_pPieceTable->setPieceTableState(PTS_Loading);

	UT_Error errorCode;

    UT_DEBUGMSG(("PD_Document::_importFile... name:%s\n",szFilename));
    errorCode = m_hDocumentRDF->setupWithPieceTable();
    if( errorCode != UT_OK )
    {
        return errorCode;
    }
    // m_hDocumentRDF->runMilestone2Test();
    
	if (bImportStylesFirst) {
		std::string template_list[6];

		buildTemplateList (template_list, "normal.awt");

		bool success = false;
		for (UT_uint32 i = 0; i < 6 && !success; i++)
			success = (importStyles(template_list[i].c_str(), ieft, true) == UT_OK);

		// don't worry if this fails
	}


	// set standard document properties and attributes, such as dtd,
	// lang, dom-dir, etc., which the importer can then overwite this
	// also initializes m_indexAP
	m_indexAP = 0xffffffff;
	setAttrProp(PP_NOPROPS);

	if(bIsImportFile)
		{
			IEFileType savedAsType;
			errorCode = IE_Imp::loadFile (this, input, static_cast<IEFileType>(ieft), impProps, &savedAsType);
		}
	else
		{
			errorCode = IE_Imp::loadFile(this, input, static_cast<IEFileType>(ieft), impProps, &m_lastOpenedType);
			_syncFileTypes(false);

			if (getFilename().empty())
				_setFilename(szFilename);
		}

	if (!UT_IS_IE_SUCCESS(errorCode))
	{
		UT_DEBUGMSG(("PD_Document::importFile -- could not import file\n"));
		DELETEP(m_pPieceTable);
		return errorCode;
	}
	repairDoc();
	
	m_bLoading = false;

	setLastOpenedTime(time(NULL));
	
	// get document-wide settings from the AP
	const PP_AttrProp * pAP = getAttrProp();
	
	if(pAP)
	{
		const gchar * pA = NULL;

		// TODO this should probably be stored as an attribute of the
		// styles section rather then the whole doc ...
		if(pAP->getAttribute("styles", pA))
		{
			m_bLockedStyles = !(strcmp(pA, "locked"));
		}

		if(pAP->getAttribute("xid-max", pA))
		{
			UT_uint32 i = (UT_uint32)atoi(pA);
			m_pPieceTable->setXIDThreshold(i);
		}
	}

	m_pPieceTable->setPieceTableState(PTS_Editing);
	updateFields();

	if(markClean)
		_setClean();
	else
	  	_setForceDirty(true); // force this to be dirty
	//	m_pPieceTable->getFragments().verifyDoc();


	// show warning if document contains revisions and they are hidden
	// from view ...
	bool bHidden = (isMarkRevisions() && (getHighestRevisionId() <= getShowRevisionId()));
	bHidden |= (!isMarkRevisions() && !isShowRevisions() && getRevisions().getItemCount());

	// note: the GsfInput could be a memory stream, and thus we have could have no filename yet
	if(pFrame && szFilename && (strstr(szFilename, "normal.awt") == NULL))
		XAP_App::getApp()->getPrefs()->addRecent(szFilename);

	if(pFrame && bHidden)
	{
		pFrame->showMessageBox(AP_STRING_ID_MSG_HiddenRevisions, 
						       XAP_Dialog_MessageBox::b_O, 
							   XAP_Dialog_MessageBox::a_OK);
	}
	UT_ASSERT(isOrigUUID());
	if(pFrame && pStatusBar)
	{
		pStatusBar->hideProgressBar();
		pFrame->nullUpdate();
	}	

	return errorCode;
}

AP_StatusBar *  PD_Document::getStatusBar(void)
{
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	if(pFrame)
	{
		AP_FrameData * pData =  static_cast<AP_FrameData *>(pFrame->getFrameData());	
		if(pData)
			return static_cast<AP_StatusBar *>(pData->m_pStatusBar);
	}
	return NULL;
}

UT_Error PD_Document::createRawDocument(void)
{
	m_pPieceTable = new pt_PieceTable(this);
	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::readFromFile -- could not construct piece table\n"));
		return UT_NOPIECETABLE;
	}

	m_pPieceTable->setPieceTableState(PTS_Loading);

	{
		std::string template_list[6];
		
		buildTemplateList (template_list, "normal.awt");

		bool success = false;
		int ieft = IEFT_Unknown;
		for (UT_uint32 i = 0; i < 6 && !success; i++)
			success = (importStyles(template_list[i].c_str(), ieft, true) == UT_OK);

		// don't worry if this fails
	}

	// set standard document properties and attributes, such as dtd, lang,
	// dom-dir, etc., which the importer can then overwite
	// this also initializes m_indexAP
	m_indexAP = 0xffffffff;
	setAttrProp(PP_NOPROPS);
	UT_ASSERT(isOrigUUID());

    UT_Error errorCode = m_hDocumentRDF->setupWithPieceTable();
    if( errorCode != UT_OK )
    {
        return errorCode;
    }
    
	return UT_OK;
}

void PD_Document::finishRawCreation(void)
{
	repairDoc();
	m_pPieceTable->setPieceTableState(PTS_Editing);
	updateFields();
	_setClean();							// mark the document as not-dirty
	//	m_pPieceTable->getFragments().verifyDoc();
}

UT_Error PD_Document::readFromFile(const char * szFilename, int ieft,
								   const char * impProps)
{
	return _importFile(szFilename, ieft, true, true, false, impProps);
}

UT_Error PD_Document::readFromFile(GsfInput *input, int ieft,
								   const char * impProps)
{
	return _importFile(input, ieft, true, true, false, impProps);
}

UT_Error PD_Document::importStyles(const char * szFilename, int ieft, bool bDocProps)
{
	if (!szFilename || !*szFilename)
	{
		UT_DEBUGMSG(("PD_Document::importStyles -- invalid filename\n"));
		return UT_INVALIDFILENAME;
	}

	if ( !UT_isRegularFile(szFilename) )
	{
	  UT_DEBUGMSG (("PD_Document::importStyles -- file is not plain file\n"));
	  return UT_INVALIDFILENAME;
	}

	if (!m_pPieceTable)
	{
		UT_DEBUGMSG(("PD_Document::importStyles -- could not construct piece table\n"));
		return UT_NOPIECETABLE;
	}

	IE_Imp * pie = NULL;
	UT_Error errorCode;

	// don't use IE_Imp::loadFile () here, because of the setLoadStylesOnly below. it doesn't do us much good anyway
	errorCode = IE_Imp::constructImporter(this, szFilename, static_cast<IEFileType>(ieft), &pie);
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::importStyles -- could not construct importer\n"));
		return errorCode;
	}

	if(!pie->supportsLoadStylesOnly())
	{
		UT_DEBUGMSG(("PD_Document::importStyles -- import of styles-only not supported\n"));
		return UT_IE_IMPSTYLEUNSUPPORTED;
	}
	
	pie->setLoadStylesOnly(true);
	pie->setLoadDocProps(bDocProps);
	errorCode = pie->importFile(szFilename);
	delete pie;

	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::importStyles -- could not import file\n"));
		return errorCode;
	}

	// need to update anything that uses styles ...
	//
	// this is rather cumbersome, but did not see a simpler way of
	// doing this (perhaps we should consider some way of invalidating
	// styles: a style could carry a time stamp and each element would
	// also carry a timestamp reflecting when its atributes were last
	// refreshed; in this case if style stamp > element stamp, element
	// would reformat) Tomas, June 7, 2003
	
	UT_GenericVector<PD_Style*> vStyles;
	getAllUsedStyles(&vStyles);
	for(UT_sint32 i = 0; i < vStyles.getItemCount();i++)
	{
		PD_Style * pStyle = vStyles.getNthItem(i);

		if(pStyle)
			updateDocForStyleChange(pStyle->getName(),!pStyle->isCharStyle());
	}

	return UT_OK;	
}

UT_Error PD_Document::newDocument(void)
{
	std::string template_list[6];

	buildTemplateList(template_list, "normal.awt");

	bool success = false;

	for (UT_uint32 i = 0; i < 6 && !success; i++) 
		success = (importFile (template_list[i].c_str(), IEFT_Unknown, true, false) == UT_OK);

	if (!success) {
			m_pPieceTable = new pt_PieceTable(this);
			if (!m_pPieceTable)
				return UT_NOPIECETABLE;

			m_pPieceTable->setPieceTableState(PTS_Loading);

			// add just enough structure to empty document so we can edit
			appendStrux(PTX_Section, PP_NOPROPS);
			appendStrux(PTX_Block, PP_NOPROPS);

			// set standard document properties, such as dtd, lang,
			// dom-dir, etc. (some of the code that used to be here is
			// now in the setAttrProp() function, since it is shared
			// both by new documents and documents being loaded from disk
			// this also initializes m_indexAP
			m_indexAP = 0xffffffff;
			setAttrProp(PP_NOPROPS);

			m_pPieceTable->setPieceTableState(PTS_Editing);
	}
	//	m_pPieceTable->getFragments().verifyDoc();

	setDocVersion(0);
	setEditTime(0);
	setLastOpenedTime(time(NULL));

	// set document metadata from context
	setMetaDataProp(PD_META_KEY_CREATOR, m_sUserName);
    
	// mark the document as not-dirty
	_setClean();
	UT_ASSERT(isOrigUUID());

	return UT_OK;
}

UT_Error PD_Document::saveAs(GsfOutput *output, int ieft, bool cpy, const char * expProps)
{
	return _saveAs(output, ieft, cpy, expProps);
}

UT_Error PD_Document::_saveAs(const char * szFilename, int ieft,
							 const char * expProps)
{
  return _saveAs(szFilename, ieft, true, expProps);
}

UT_Error PD_Document::_saveAs(const char * szFilename, int ieft, bool cpy,
							  const char * expProps)
{
	IE_Exp * pie = NULL;
	UT_Error errorCode;
	IEFileType newFileType;

	errorCode = IE_Exp::constructExporter(this, szFilename, static_cast<IEFileType>(ieft), &pie, &newFileType);
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not construct exporter\n"));
		return UT_SAVE_EXPORTERROR;
	}
	if (expProps && strlen(expProps))
		pie->setProps (expProps);

	if (cpy && !XAP_App::getApp()->getPrefs()->isIgnoreRecent())
	{
		m_lastSavedAsType = newFileType;
		_syncFileTypes(true);
	}
	if(!XAP_App::getApp()->getPrefs()->isIgnoreRecent())
	{
		// order of these calls matters
		_adjustHistoryOnSave();
		
		// see if revisions table is still needed ...
		purgeRevisionTable();
	}
	
	errorCode = pie->writeFile(szFilename);
	delete pie;

	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not write file\n"));
		return (errorCode == UT_SAVE_CANCELLED) ? UT_SAVE_CANCELLED : UT_SAVE_WRITEERROR;
	}

	if (cpy && !XAP_App::getApp()->getPrefs()->isIgnoreRecent()) // we want to make the current settings persistent
	{
	    // no file name currently set - make this filename the filename
	    // stored for the doc
	    if (!szFilename)
			return UT_SAVE_OTHERERROR;
	    _setFilename(szFilename);
	    _setClean(); // only mark as clean if we're saving under a new name
		signalListeners(PD_SIGNAL_DOCNAME_CHANGED);	
	}

	if (szFilename)
		XAP_App::getApp()->getPrefs()->addRecent(szFilename);

	return UT_OK;
}

UT_Error PD_Document::_saveAs(GsfOutput *output, int ieft, bool cpy, const char * expProps)
{
	UT_return_val_if_fail(output, UT_SAVE_NAMEERROR);

	const char * szFilename = gsf_output_name(output);

	IE_Exp * pie = NULL;
	UT_Error errorCode;
	IEFileType newFileType;

	errorCode = IE_Exp::constructExporter(this, output, static_cast<IEFileType>(ieft), &pie, &newFileType);
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not construct exporter\n"));
		return UT_SAVE_EXPORTERROR;
	}
	if (expProps && strlen(expProps))
		pie->setProps (expProps);

	if (cpy && !XAP_App::getApp()->getPrefs()->isIgnoreRecent())
	{
		m_lastSavedAsType = newFileType;
		_syncFileTypes(true);
	}
	if(!XAP_App::getApp()->getPrefs()->isIgnoreRecent())
	{
		// order of these calls matters
		_adjustHistoryOnSave();
		
		// see if revisions table is still needed ...
		purgeRevisionTable();
	}
	
	errorCode = pie->writeFile(output);
	delete pie;

	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not write file\n"));
		return (errorCode == UT_SAVE_CANCELLED) ? UT_SAVE_CANCELLED : UT_SAVE_WRITEERROR;
	}

	if (cpy && !XAP_App::getApp()->getPrefs()->isIgnoreRecent()) // we want to make the current settings persistent
	{
	    // no file name currently set - make this filename the filename
	    // stored for the doc
	    if (!szFilename)
			return UT_SAVE_OTHERERROR;
	    _setFilename(szFilename);
	    _setClean(); // only mark as clean if we're saving under a new name
		signalListeners(PD_SIGNAL_DOCNAME_CHANGED);	
	}

	if (szFilename)
		XAP_App::getApp()->getPrefs()->addRecent(szFilename);

	return UT_OK;
}

UT_Error PD_Document::_save(void)
{
	if (getFilename().empty())
		return UT_SAVE_NAMEERROR;
	if (m_lastSavedAsType == IEFT_Unknown)
		return UT_EXTENSIONERROR;

	IE_Exp * pie = NULL;
	UT_Error errorCode;

	errorCode = IE_Exp::constructExporter(this, getFilename().c_str(),
                                          m_lastSavedAsType, &pie);
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not construct exporter\n"));
		return UT_SAVE_EXPORTERROR;
	}

	_syncFileTypes(true);

	_adjustHistoryOnSave();

	// see if revisions table is still needed ...
	purgeRevisionTable();
	
	errorCode = pie->writeFile(getFilename().c_str());
	delete pie;
	if (errorCode)
	{
		UT_DEBUGMSG(("PD_Document::Save -- could not write file\n"));
		return (errorCode == UT_SAVE_CANCELLED) ? UT_SAVE_CANCELLED : UT_SAVE_WRITEERROR;
	}

	_setClean();
	return UT_OK;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool PD_Document::isDirty(void) const
{
	return m_pPieceTable->isDirty() || isForcedDirty();
}

void PD_Document::_setClean(void)
{
	m_pPieceTable->setClean();
	_setForceDirty(false);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool	PD_Document::insertObject(PT_DocPosition dpos,
								  PTObjectType pto,
								  const PP_PropertyVector & attributes,
								  const PP_PropertyVector & properties)
{
	return insertObject(dpos, pto, attributes, properties, nullptr);
}

bool	PD_Document::insertObject(PT_DocPosition dpos,
								  PTObjectType pto,
								  const PP_PropertyVector & attributes,
								  const PP_PropertyVector & properties, fd_Field ** pField)
{
	if(isDoingTheDo())
	{
		return false;
	}
	PP_PropertyVector newattrs(attributes);
	addAuthorAttributeIfBlank(newattrs);
	if (pField) {
		pf_Frag_Object * pfo = NULL;
		bool b = m_pPieceTable->insertObject(dpos, pto, newattrs, properties, &pfo);
		*pField = pfo->getField();
		return b;
	}
	return m_pPieceTable->insertObject(dpos, pto, newattrs, properties);
}

bool PD_Document::insertSpan( PT_DocPosition dpos,
                              const std::string& s,
                              PP_AttrProp *p_AttrProp )
{
	UT_UCS4String t( s );
    return insertSpan( dpos, t.ucs4_str(), t.length(), p_AttrProp );
}


/*!
 * Note that the text will be set to exactly the properties of given by
 *  p_AttrProp.
 * If pAttrProp is set to NULL, the text will be set to exactly
 * the properties of the style of the current paragraph.
 */
bool PD_Document::insertSpan(PT_DocPosition dpos, const UT_UCSChar * pbuf,
							 UT_uint32 length, PP_AttrProp *p_AttrProp,
							 UT_uint32 *insertedSpanLength)
{
	if(isDoingTheDo())
	{
		return false;
	}
	addAuthorAttributeIfBlank(p_AttrProp);
	if(p_AttrProp)
	{
		m_pPieceTable->insertFmtMark(PTC_SetExactly, dpos, p_AttrProp);
	}
#if DEBUG
#if 1
	UT_uint32 ii = 0;
	std::string sStr;
	for(ii=0; ii<length;ii++)
	{
		sStr += static_cast<const char>(pbuf[ii]);
	}
	UT_DEBUGMSG(("PD_Document Insert span |%s| pos %d \n",sStr.c_str(),dpos));
#endif
#endif
	// REMOVE UNDESIRABLE CHARACTERS ...
	// we will remove all LRO, RLO, LRE, RLE, and PDF characters
	// * at the moment we do not handle LRE/RLE
	// * we replace LRO/RLO with our dir-override property

	PT_DocPosition cur_pos = dpos;
	PP_AttrProp AP;

	// we want to reset m_iLastDirMarker (which is in a state left
	// over from the last insert/append operation)
	m_iLastDirMarker = 0;
	
	bool result = true;
	const UT_UCS4Char * pStart = pbuf;
	UT_sint32 newLength = length;

	for(const UT_UCS4Char * p = pbuf; p < pbuf + length; p++)
	{
		switch(*p)
		{
			case UCS_LRO:
				if((p - pStart) > 0)
				{
					result &= m_pPieceTable->insertSpan(cur_pos, pStart, p - pStart);
					cur_pos += p - pStart;
				}
				
				AP.setProperty("dir-override", "ltr");
				result &= m_pPieceTable->insertFmtMark(PTC_AddFmt, cur_pos, &AP);
				pStart = p + 1;
				m_iLastDirMarker = *p;
				newLength--;
				break;
				
			case UCS_RLO:
				if((p - pStart) > 0)
				{
					result &= m_pPieceTable->insertSpan(cur_pos, pStart, p - pStart);
					cur_pos += p - pStart;
				}
				
				AP.setProperty("dir-override", "rtl");
				result &= m_pPieceTable->insertFmtMark(PTC_AddFmt, cur_pos, &AP);
				pStart = p + 1;
				m_iLastDirMarker = *p;
				newLength--;
				break;
				
			case UCS_PDF:
				if((p - pStart) > 0)
				{
					result &= m_pPieceTable->insertSpan(cur_pos, pStart, p - pStart);
					cur_pos += p - pStart;
				}
				
				if((m_iLastDirMarker == UCS_RLO) || (m_iLastDirMarker == UCS_LRO))
				{
					AP.setProperty("dir-override", "");
					result &= m_pPieceTable->insertFmtMark(PTC_RemoveFmt, cur_pos, &AP);
				}

				pStart = p + 1;
				m_iLastDirMarker = *p;
				newLength--;
				break;
				
			case UCS_LRE:
			case UCS_RLE:
				if((p - pStart) > 0)
				{
					result &= m_pPieceTable->insertSpan(cur_pos, pStart, p - pStart);
					cur_pos += p - pStart;
				}
				
				pStart = p + 1;
				m_iLastDirMarker = *p;
				newLength--;
				break;
		}
	}
	
	// A length of zero can occur if one of the already-handled characters
	// in the above switch comprises the entire span.
	if((length - (pStart - pbuf)) > 0)
		result &= m_pPieceTable->insertSpan(cur_pos, pStart, length - (pStart - pbuf));

	if (insertedSpanLength)
	{
		*insertedSpanLength = (newLength >= 0) ? newLength:0;
	}
	return result;
}

bool PD_Document::deleteSpan(PT_DocPosition dpos1,
							 PT_DocPosition dpos2,
							 PP_AttrProp *p_AttrProp_Before,
							 UT_uint32 &iRealDeleteCount,
							 bool bDeleteTableStruxes)
{
	if(isDoingTheDo())
	{
		return false;
	}
	return m_pPieceTable->deleteSpanWithTable(dpos1, dpos2, p_AttrProp_Before,iRealDeleteCount, bDeleteTableStruxes );
}

bool PD_Document::changeSpanFmt(PTChangeFmt ptc,
								PT_DocPosition dpos1,
								PT_DocPosition dpos2,
								const PP_PropertyVector & attributes,
								const PP_PropertyVector & properties)
{
	if(isDoingTheDo())
	{
		return false;
	}
	bool f;
	deferNotifications();
	PP_PropertyVector attsAuthor =	attributes;
	addAuthorAttributeIfBlank(attsAuthor);
	f = m_pPieceTable->changeSpanFmt(ptc, dpos1, dpos2, attsAuthor, properties);
	processDeferredNotifications();
	return f;
}

bool PD_Document::insertStrux(PT_DocPosition dpos,
							  PTStruxType pts, pf_Frag_Strux ** ppfs_ret)
{
	if(isDoingTheDo())
	{
		return false;
	}
	return m_pPieceTable->insertStrux(dpos,pts,ppfs_ret);
}

bool PD_Document::insertStrux(PT_DocPosition dpos, PTStruxType pts,
							  const PP_PropertyVector & attributes,
							  const PP_PropertyVector & properties,
							  pf_Frag_Strux ** ppfs_ret)
{
	if(isDoingTheDo())
	{
		return false;
	}

	PP_PropertyVector attr(attributes);
	addAuthorAttributeIfBlank(attr);

	return m_pPieceTable->insertStrux(dpos, pts, attr, properties, ppfs_ret);
}

/*!
 * This method deletes the HdrFtr strux pointed to by sdh
 */
void PD_Document::deleteHdrFtrStrux(pf_Frag_Strux* sdh)
{
	pf_Frag_Strux * pfs_hdrftr = sdh;
	UT_return_if_fail (pfs_hdrftr->getType()  == pf_Frag::PFT_Strux);
	m_pPieceTable->deleteHdrFtrStrux(pfs_hdrftr);
}

bool PD_Document::changeStruxFmt(PTChangeFmt ptc,
								 PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const PP_PropertyVector & attributes,
								 const PP_PropertyVector & properties,
								 PTStruxType pts)
{
	if(isDoingTheDo())
	{
		return false;
	}
	return m_pPieceTable->changeStruxFmt(ptc,dpos1,dpos2,attributes,properties,pts);
}


bool PD_Document::changeStruxFmtNoUndo(PTChangeFmt ptc,
								 pf_Frag_Strux* sdh,
								 const PP_PropertyVector & attributes,
								 const PP_PropertyVector & properties)
{
	pf_Frag_Strux * pfs = sdh;
	UT_return_val_if_fail (pfs->getType() == pf_Frag::PFT_Strux,false);
	return m_pPieceTable->changeStruxFmtNoUndo(ptc,pfs,attributes,properties);
}


/*!
 * This method changes *all* the strux fragments in the specified region.
 */
bool PD_Document::changeStruxFmt(PTChangeFmt ptc,
								 PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const PP_PropertyVector & attributes,
								 const PP_PropertyVector & properties)
{
	if(isDoingTheDo())
	{
		return false;
	}
	return m_pPieceTable->changeStruxFmt(ptc, dpos1, dpos2, attributes, properties);
}

/*!
 * This Method is used to change just the parentID of each strux in a list
 * without updating the fl_Layouts.
 */
bool PD_Document::changeStruxForLists(pf_Frag_Strux* sdh, const char * pszParentID)
{
	return m_pPieceTable->changeStruxForLists(sdh, pszParentID);
}

bool PD_Document::insertFmtMark(PTChangeFmt ptc, PT_DocPosition dpos, PP_AttrProp *p_AttrProp)
{
	if(isDoingTheDo())
	{
		return false;
	}
	return m_pPieceTable->insertFmtMark(ptc, dpos, p_AttrProp);
}

bool  PD_Document::deleteFmtMark( PT_DocPosition dpos)
{
	return m_pPieceTable->deleteFmtMark(dpos);
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool PD_Document::appendStrux(PTStruxType pts, const PP_PropertyVector & attributes, pf_Frag_Strux ** ppfs_ret)
{
	UT_return_val_if_fail (m_pPieceTable, false);

	// can only be used while loading the document
//
// Update frames during load.
//
	if(pts == PTX_EndCell)
	{
		checkForSuspect();
	}
	else if(pts == PTX_Section)
	{
		checkForSuspect();
	}
	updateStatus();
	return m_pPieceTable->appendStrux(pts,attributes,ppfs_ret);
}

/*!
    appends given fmt to the last strux in document
*/
bool PD_Document::appendLastStruxFmt(PTStruxType pts, const PP_PropertyVector & attributes, const PP_PropertyVector & props,
									 bool bSkipEmbededSections)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	updateStatus();
	return m_pPieceTable->appendLastStruxFmt(pts,attributes,props,bSkipEmbededSections);
}

bool PD_Document::appendLastStruxFmt(PTStruxType pts, const PP_PropertyVector & attributes, const std::string & props,
									 bool bSkipEmbededSections)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	updateStatus();
	return m_pPieceTable->appendLastStruxFmt(pts,attributes,props,bSkipEmbededSections);
}

bool PD_Document::changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pts,
									 const PP_PropertyVector & attributes, const PP_PropertyVector & props,
									 bool bSkipEmbededSections)
{
	UT_return_val_if_fail (m_pPieceTable, false);

	return m_pPieceTable->changeLastStruxFmtNoUndo(dpos, pts,attributes,props,bSkipEmbededSections);
}

bool PD_Document::changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pts,
										   const PP_PropertyVector & attributes, const std::string & props,
									 bool bSkipEmbededSections)
{
	UT_return_val_if_fail (m_pPieceTable, false);

	return m_pPieceTable->changeLastStruxFmtNoUndo(dpos, pts,attributes,props,bSkipEmbededSections);
}

bool PD_Document::appendStruxFmt(pf_Frag_Strux * pfs, const PP_PropertyVector & attributes)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	updateStatus();
	return m_pPieceTable->appendStruxFmt(pfs,attributes);
}

/*!
 * Scan the vector of suspect frags and add blocks if they're needed.
 * Returns true if there are changes to the document. 
 */
bool PD_Document::repairDoc(void)
{
	pf_Frag * pf = NULL;
	pf_Frag_Strux * pfs = NULL;
	bool bRepaired = false;
	//
	// First check there is *some* content.
	//
	pf = m_pPieceTable->getFragments().getFirst();
	if(!pf)
	{
		appendStrux(PTX_Section, PP_NOPROPS);
		appendStrux(PTX_Block, PP_NOPROPS);
		return true;
	}
	// Now check if the document starts with a section
	pf = m_pPieceTable->getFragments().getFirst();
	if(pf->getType() != pf_Frag::PFT_Strux)
	{
		insertStruxBeforeFrag(pf, PTX_Section, PP_NOPROPS);
		insertStruxBeforeFrag(pf, PTX_Block, PP_NOPROPS);
		bRepaired = true;
	}
	pf = m_pPieceTable->getFragments().getFirst();
	pfs = static_cast<pf_Frag_Strux *>(pf);
	if(pfs->getStruxType() != PTX_Section)
	{
		insertStruxBeforeFrag(pf, PTX_Section, PP_NOPROPS);
		insertStruxBeforeFrag(pf, PTX_Block, PP_NOPROPS);
		bRepaired = true;
	}

	checkForSuspect(); // Look at last frag. If it's an endtable we need a block
	UT_sint32 i = 0;
	for(i=0; i< m_vecSuspectFrags.getItemCount(); i++)
	{
		pf = m_vecSuspectFrags.getNthItem(i);
		UT_DEBUGMSG(("Suspect frag %d pointer %p \n",i,pf));
		if(pf->getType() == pf_Frag::PFT_Strux)
		{
			pfs = static_cast<pf_Frag_Strux *>(pf);
			if((pfs->getStruxType() != PTX_Block) && (pfs->getStruxType() != PTX_EndFootnote) && (pfs->getStruxType() != PTX_EndEndnote)  && (pfs->getStruxType() != PTX_EndAnnotation) )
			{
				pf_Frag * pfNext = pf->getNext();
				if(pfNext && ((pfNext->getType() ==  pf_Frag::PFT_Text) || (pfNext->getType() ==  pf_Frag::PFT_Object) || (pfNext->getType() ==  pf_Frag::PFT_FmtMark)))
				{
					//
					// Insert a block afterwards!
					//
					insertStruxBeforeFrag(pfNext, PTX_Block, PP_NOPROPS);
					bRepaired = true;
				}
				else if(pfNext && (pfs->getStruxType() == PTX_SectionCell) && (pfNext->getType() == pf_Frag::PFT_Strux) )
				{
					pf_Frag_Strux * pfNexts = static_cast<pf_Frag_Strux *>(pfNext);
					if(pfNexts->getStruxType() == PTX_EndCell)
					{
					//
					// Insert a block afterwards!
					//
						insertStruxBeforeFrag(pfNext, PTX_Block, PP_NOPROPS);
						bRepaired = true;
					}
				}
				else if(pfNext && (pfs->getStruxType() == PTX_EndTable) && ((pfNext->getType() == pf_Frag::PFT_Strux) || (pfNext == m_pPieceTable->getFragments().getLast())))
			    {
					if(pfNext == m_pPieceTable->getFragments().getLast())
					{
					//
					// Insert a block afterwards!
					//
						insertStruxBeforeFrag(pfNext, PTX_Block, PP_NOPROPS);
						bRepaired = true;
					}
					else
					{
						pf_Frag_Strux * pfNexts = static_cast<pf_Frag_Strux *>(pfNext);
						if(pfNexts->getStruxType() == PTX_Section)
						{
							//
							// Insert a block afterwards!
							//
							insertStruxBeforeFrag(pfNext, PTX_Block, PP_NOPROPS);
							bRepaired = true;
						}
						
					}
				}
				else if(pfs->getStruxType() == PTX_EndTable && pfNext == NULL)
				{
					appendStrux(PTX_Block, PP_NOPROPS);
				}
			}
		}
	}
	//
	// Now Obtain a list of sections/headers and footers.
	// remove unreferenced headers/footer
	// Remove references to headers and footers in sections that are not
	// Present.
	// Remove repeated HdrFtr's
	//
	UT_GenericVector<pf_Frag_Strux *> vecSections;
	UT_GenericVector<pf_Frag_Strux *> vecHdrFtrs;
	UT_GenericVector<pf_Frag_Strux *> vecTables;
	pf = m_pPieceTable->getFragments().getFirst();
	while(pf)
	{
		if(pf->getType() == pf_Frag::PFT_Strux)
		{
			pfs = static_cast<pf_Frag_Strux *>(pf);
			if(pfs->getStruxType() == PTX_Section)
			{
				vecSections.addItem(pfs);
			}
			else if(pfs->getStruxType() == PTX_SectionHdrFtr)
			{
				vecHdrFtrs.addItem(pfs);
			}
			else if(pfs->getStruxType() == PTX_SectionTable)
			{
				vecTables.addItem(pfs);
			}
			else if(pfs->getStruxType() == PTX_EndTable)
			{
				vecTables.addItem(pfs);
			}
		}
		pf = pf->getNext();
	}
	//
	// Look for bare tables struxes. Delete them if we find one
	//
	for(i=0; i< vecTables.getItemCount(); i++)
	{
		pfs = vecTables.getNthItem(i);
		bRepaired = bRepaired | _checkAndFixTable(pfs);
	}
	//
	// Fix section matching of HdrFtrs
	//
	for(i = 0; i< vecSections.getItemCount(); i++)
	{
		pfs = vecSections.getNthItem(i);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"header",&vecHdrFtrs);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"header-even",&vecHdrFtrs);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"header-first",&vecHdrFtrs);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"header-last",&vecHdrFtrs);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"footer",&vecHdrFtrs);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"footer-even",&vecHdrFtrs);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"footer-first",&vecHdrFtrs);
		bRepaired = bRepaired | _pruneSectionAPI(pfs,"footer-last",&vecHdrFtrs);
	}
	for(i = 0; i< vecHdrFtrs.getItemCount(); i++)
	{
		pfs = vecHdrFtrs.getNthItem(i);
		if(!_matchSection(pfs,&vecSections))
		{
			//
			// Now we have to delete this whole Header/Footer
			//
			_removeHdrFtr(pfs);
			bRepaired = true;
			vecHdrFtrs.deleteNthItem(i);
			i--;
		}
	}
	//
	// Now remove repeated HdrFtr's ie Header/Footers with identical ID's
	//
	for(i = 0; i< vecHdrFtrs.getItemCount(); i++)
	{
		pfs = vecHdrFtrs.getNthItem(i);
		if(!_removeRepeatedHdrFtr(pfs,&vecHdrFtrs,i+1))
		{
			bRepaired = true;
		}
	}
	//
	// Check that no section is empty. Add block if necessary
	//
	for(i = 0; i < vecSections.getItemCount(); i++)
	{
		pfs = vecSections.getNthItem(i);
		pf_Frag * pfsNext = pfs->getNext();
		if (!pfsNext)
		{
			appendStrux(PTX_Block, PP_NOPROPS);
			bRepaired = true;
		}
		else if ((pfsNext->getType() == pf_Frag::PFT_Strux) &&
				 (static_cast<pf_Frag_Strux *>(pfsNext)->getStruxType() != PTX_Block) &&
				 (static_cast<pf_Frag_Strux *>(pfsNext)->getStruxType() != PTX_SectionTable))
		{
			insertStruxBeforeFrag(pfsNext, PTX_Block, PP_NOPROPS);
			bRepaired = true;
		}
	}

	for(i = 0; i < vecHdrFtrs.getItemCount(); i++)
	{
		pfs = vecHdrFtrs.getNthItem(i);
		pf_Frag * pfsNext = pfs->getNext();
		if (!pfsNext)
		{
			appendStrux(PTX_Block, PP_NOPROPS);
			bRepaired = true;
		}
		else if ((pfsNext->getType() == pf_Frag::PFT_Strux) &&
				 (static_cast<pf_Frag_Strux *>(pfsNext)->getStruxType() != PTX_Block) &&
				 (static_cast<pf_Frag_Strux *>(pfsNext)->getStruxType() != PTX_SectionTable))
		{
			insertStruxBeforeFrag(pfsNext, PTX_Block, PP_NOPROPS);
			bRepaired = true;
		}
	}

	//
	// Now repair text and objects which aren't enclosed in a paragraph
	//
	pf = m_pPieceTable->getFragments().getFirst();
	bool bGotBlock = false;
	while(pf)
	{
		if(pf->getType() == pf_Frag::PFT_Strux)
		{
			pfs = static_cast<pf_Frag_Strux *>(pf);
			if((pfs->getStruxType() == PTX_Block) || (m_pPieceTable->isEndFootnote(pfs))) 
			{
				bGotBlock = true;
			}
			else
			{
				bGotBlock = false;
			}
		}
		else if(!bGotBlock && (pf->getType() !=  pf_Frag::PFT_EndOfDoc))
		{
			// BUG! Content not in a block. Insert one now
			insertStruxBeforeFrag(pf, PTX_Block, PP_NOPROPS);
			bGotBlock = true;
			bRepaired = true;
		}
		pf = pf->getNext();
	}
	return !bRepaired;
}

/*!
 * Look for a Hdr/Ftr with exactly the same identification as that of the 
 * input strux.
 * If we find a match delete the HdrFtr
 */
bool PD_Document::_removeRepeatedHdrFtr(pf_Frag_Strux * pfs ,UT_GenericVector<pf_Frag_Strux *> * vecHdrFtrs,UT_sint32 iStart)
{
	const char * pszMyHdrFtr = NULL;
	const char * pszMyID = NULL;
	const char * pszThisID = NULL;
	const char * pszThisHdrFtr = NULL;
	UT_sint32 i=0;
	pf_Frag_Strux * pfsS = NULL;
	getAttributeFromSDH(pfs,false,0,"type",&pszMyHdrFtr);
	getAttributeFromSDH(pfs,false,0,"id",&pszMyID);
	if(pszMyHdrFtr && *pszMyHdrFtr && pszMyID && *pszMyID)
	{
		for(i = iStart; i<vecHdrFtrs->getItemCount(); i++)
		{
			pfsS = vecHdrFtrs->getNthItem(i);
			getAttributeFromSDH(pfsS,false,0,"type",&pszThisHdrFtr);
			getAttributeFromSDH(pfsS,false,0,"id",&pszThisID);
			if(pszThisHdrFtr && *pszThisHdrFtr && pszThisID && *pszThisID)
			{
				if((strcmp(pszMyHdrFtr,pszThisHdrFtr) == 0) &&
				   (strcmp(pszMyID,pszThisID) == 0))
				{
					_removeHdrFtr(pfsS);
					vecHdrFtrs->deleteNthItem(i);
				}
			}

		}
	}
	return false;
}


/*!
 * This method looks to see if we have a table strux without a matching cell
 * or an endtable without a matching endcell preceding it
 * If we do it deletes the table/endtable.
 */
bool PD_Document::_checkAndFixTable(pf_Frag_Strux * pfs)
{
	pf_Frag * pf =NULL;
	pf_Frag_Strux * pfsn = NULL;
	if(pfs->getStruxType() == PTX_SectionTable)
	{
		pf = pfs->getNext();
		if(!pf)
		{
			m_pPieceTable->deleteFragNoUpdate(pfs);
			return true;
		}
		else if(pf->getType() != pf_Frag::PFT_Strux)
		{
			m_pPieceTable->deleteFragNoUpdate(pfs);
			return true;
		}
		pfsn = static_cast<pf_Frag_Strux *>(pf);
		if(pfsn->getStruxType() !=  PTX_SectionCell)
		{
			m_pPieceTable->deleteFragNoUpdate(pfs);
			return true;
		}
	}
	else if(pfs->getStruxType() == PTX_EndTable)
	{
		pf = pfs->getPrev();
		if(!pf)
		{
			m_pPieceTable->deleteFragNoUpdate(pfs);
			return true;
		}
		else if(pf->getType() != pf_Frag::PFT_Strux)
		{
			m_pPieceTable->deleteFragNoUpdate(pfs);
			return true;
		}
		pfsn = static_cast<pf_Frag_Strux *>(pf);
		if(pfsn->getStruxType() !=  PTX_EndCell)
		{
			m_pPieceTable->deleteFragNoUpdate(pfs);
			return true;
		}
	}
	return false;
}


/*!
 * This scans the HdrFtrs to see if there is a match for the header/footer type
 * in the section strux pfs.
 * Return true of a prune happened
 */
bool PD_Document::_pruneSectionAPI(pf_Frag_Strux * pfs,const char * szHType, UT_GenericVector<pf_Frag_Strux *> *vecHdrFtrs)
{
	const char * pszHdrFtr = NULL;
	const char * pszHFID = NULL;
	const char * pszID = NULL;
	UT_sint32 i = 0;
	getAttributeFromSDH(pfs,false,0,szHType,&pszID);
	if(!pszID)
		return false;
	if(!(*pszID))
		return false;
	for(i= 0; i< vecHdrFtrs->getItemCount(); i++)
	{
		pf_Frag_Strux * pfsS = vecHdrFtrs->getNthItem(i);
		getAttributeFromSDH(pfsS,false,0,"type",&pszHdrFtr);
		if(pszHdrFtr && *pszHdrFtr)
		{
			if(strcmp(szHType,pszHdrFtr) == 0)
			{
				getAttributeFromSDH(pfsS,false,0,"id",&pszHFID);
				if(pszHFID && *pszHFID)
				{
					if(strcmp(pszHFID,pszID) == 0)
					{
						return false;
					} 
				}
			}
		}
	}
	//
	// No matching HdrFtr was found. Remove the property.
	//
	const PP_PropertyVector atts = {
		szHType, pszID
	};
	UT_DEBUGMSG(("Pruning HdrFtr %s ID %s From section \n",szHType,pszID));
	m_pPieceTable->changeStruxFormatNoUpdate(PTC_RemoveFmt, pfs, atts);
	return true;
}

/*!
 * Remove the HdrFtr starting at pfs. No changeRecords are cre created. 
 * Only used for document repair during import.
 */
bool PD_Document::_removeHdrFtr(pf_Frag_Strux * pfs)
{
	UT_DEBUGMSG(("Removing HdrFtr %p \n",pfs));
	pf_Frag * pf = NULL;
	pf_Frag * pfNext = NULL;
	pfNext = pfs->getNext();
	pf = static_cast<pf_Frag *>(pfs);
	while(pf )
	{
		m_pPieceTable->deleteFragNoUpdate(pf);
		pf = pfNext;
		if(pf)
		{
			pfNext = pf->getNext();
			if(pf->getType() == pf_Frag::PFT_Strux)
			{
				pfs = static_cast<pf_Frag_Strux *>(pf);
				if(pfs->getStruxType() == PTX_SectionHdrFtr)
					break;
			}
		}
	} 
	return true;
}

/*!
 * pfs point to a header footer section. This Method returns true if there
 * is a section that has a reference to it's HdrFtr type and id
 */
bool PD_Document::_matchSection(pf_Frag_Strux * pfs, UT_GenericVector<pf_Frag_Strux *> *vecSections)
{
	const char * pszHdrFtr = NULL;
	const char * pszHFID = NULL;
	const char * pszID = NULL;
	UT_sint32 i = 0;
	getAttributeFromSDH(pfs,false,0,"type",&pszHdrFtr);
	if(!pszHdrFtr)
		return false;
	if(!(*pszHdrFtr))
		return false;
	getAttributeFromSDH(pfs,false,0,"id",&pszHFID);
	if(!pszHFID)
		return false;
	if(!(*pszHFID))
		return false;
	for(i= 0; i< vecSections->getItemCount(); i++)
	{
		pf_Frag_Strux * pfsS = vecSections->getNthItem(i);
		getAttributeFromSDH(pfsS,false,0,pszHdrFtr,&pszID);
		if(pszID && *pszID)
		{
			if(strcmp(pszID,pszHFID) == 0)
			{
				return true;
			}
		}
	}
	return false;
}

/*!
 * This method is called after appendspan, appendObject, appendfmtMark and
 * checks to see if there is an invalid strux just before. If it see one, it
 * marks the strux as suspect for verification after the load is over.
 * Really useful for importers.
 */
bool PD_Document::checkForSuspect(void)
{
	pf_Frag * pf = getLastFrag();
	if(pf == NULL)
	{
		return true;
	}
	if(pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if((pfs->getStruxType() != PTX_Block) && (pfs->getStruxType() != PTX_EndFootnote) && (pfs->getStruxType() != PTX_EndEndnote) && (pfs->getStruxType() != PTX_EndAnnotation) )
		{
			//
			// Append a block!
			//
			m_vecSuspectFrags.addItem(pf);
			return true;
		}
		
	}
	return true;
}

bool PD_Document::appendFmt(const PP_PropertyVector & vecAttributes)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	checkForSuspect();

	// can only be used while loading the document

	return m_pPieceTable->appendFmt(vecAttributes);
}

bool PD_Document::appendSpan(const UT_UCSChar * pbuf, UT_uint32 length)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	checkForSuspect();

	// can only be used while loading the document

	// REMOVE UNDESIRABLE CHARACTERS ...
	// we will remove all LRO, RLO, LRE, RLE, and PDF characters
	// * at the moment we do not handle LRE/RLE
	// * we replace LRO/RLO with our dir-override property

	PP_PropertyVector attrs = {
		"props", ""
	};

	bool result = true;
	const UT_UCS4Char * pStart = pbuf;

	for(const UT_UCS4Char * p = pbuf; p < pbuf + length; p++)
	{
		switch(*p)
		{
			case UCS_LRO:
				if((p - pStart) > 0)
					result &= m_pPieceTable->appendSpan(pStart,p - pStart);

				attrs[1] = "dir-override:ltr";
				result &= m_pPieceTable->appendFmt(attrs);
				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;

			case UCS_RLO:
				if((p - pStart) > 0)
					result &= m_pPieceTable->appendSpan(pStart,p - pStart);

				attrs[1] = "dir-override:rtl";
				result &= m_pPieceTable->appendFmt(attrs);

				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;

			case UCS_PDF:
				if((p - pStart) > 0)
					result &= m_pPieceTable->appendSpan(pStart,p - pStart);

				if((m_iLastDirMarker == UCS_RLO) || (m_iLastDirMarker == UCS_LRO))
				{
					attrs[1] = "dir-override:";
					result &= m_pPieceTable->appendFmt(attrs);
				}
				
				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;
				
			case UCS_LRE:
			case UCS_RLE:
				if((p - pStart) > 0)
					result &= m_pPieceTable->appendSpan(pStart,p - pStart);

				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;
		}
	}

	if(length - (pStart-pbuf))
		{
#if DEBUG
#if 0
	UT_uint32 ii = 0;
	std::string sStr;
	for(ii=0; ii<(length -(pStart-pbuf));ii++)
	{
		sStr += static_cast<const char>(pStart[ii]);
	}
	UT_DEBUGMSG(("Append span %s \n",sStr.c_str()));
#endif
#endif

			result &= m_pPieceTable->appendSpan(pStart,length - (pStart-pbuf));
		}
	return result;
}

bool PD_Document::appendObject(PTObjectType pto, const PP_PropertyVector & attributes)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	checkForSuspect();

	// can only be used while loading the document

	return m_pPieceTable->appendObject(pto,attributes);
}

bool PD_Document::appendFmtMark(void)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	checkForSuspect();

	// can only be used while loading the document

	return m_pPieceTable->appendFmtMark();
}

/*!
 * This method returns the value associated with attribute szAttribute
 * at picetable strux given by sdh.
 * NB: attributes and props are view-specific because of revision attributes
 * 
 \param  pf_Frag_Strux* sdh (pf_Frag_Strux) where we want to find the value
 \param  bool bShowRevisions -- revisions setting for the view (FV_View::isShowRevisions())
 \param  UT_uint32 iRevisionLevel -- the revision level of the view (FV_View::getRevisionLevel())
 \param const char * szAttribute the attribute we're looking for.
 \param const char ** pszValue the value of the attribute.
 \returns true if the attribute was present at the sdh

 Don't FREEP *pszRetValue!!!
*/
bool PD_Document::getAttributeFromSDH(pf_Frag_Strux* sdh, bool bShowRevisions, UT_uint32 iRevisionLevel,
									  const char * szAttribute, const char ** pszRetValue)
{
	const pf_Frag_Strux * pfStrux = static_cast<const pf_Frag_Strux *>(sdh);
	PT_AttrPropIndex indexAP = pfStrux->getIndexAP();
	const PP_AttrProp * pAP = NULL;
	const gchar * pszValue = NULL;

	bool bHiddenRevision = false;
	getAttrProp(indexAP, &pAP,NULL,bShowRevisions,iRevisionLevel,bHiddenRevision);
	
	UT_return_val_if_fail (pAP, false);
	(pAP)->getAttribute(szAttribute, pszValue);
	if(pszValue == NULL)
	{
		*pszRetValue = NULL;
		return false;
	}
	*pszRetValue = pszValue;
	return true;
}

/*!
 * Get API fromthe supplied StruxDocHandle
 *
 * NB: this method does not take into account revisions settings; you either have to further process
 *     the AP at the index using the explodeRevisions() methods or you can retrieve specific props
 *     and attrs using getPropertyFromSDH() and getAttributeFromSDH().
 */
PT_AttrPropIndex PD_Document::getAPIFromSDH( pf_Frag_Strux* sdh)
{
	const pf_Frag_Strux * pfStrux = sdh;
	return pfStrux->getIndexAP();
}

/*!
 * This method returns the value associated with attribute szProperty
 * at picetable strux given by sdh.
 * NB: attributes and props are view-specific because of revision attributes
 * 
 \param  pf_Frag_Strux* sdh (pf_Frag_Strux) where we want to find the value
 \param  bool bShowRevisions -- revisions setting for the view (FV_View::isShowRevisions())
 \param  UT_uint32 iRevisionLevel -- the revision level of the view (FV_View::getRevisionLevel())
 \param const char * szProperty the Property we're looking for.
 \param const char ** pszValue the value of the property.
 \returns true if the property was present at the sdh

 Don't FREEP *pszRetValue!!!
*/
bool PD_Document::getPropertyFromSDH(const pf_Frag_Strux* sdh, bool bShowRevisions, UT_uint32 iRevisionLevel,
									 const char * szProperty, const char ** pszRetValue) const
{
	const pf_Frag_Strux * pfStrux = static_cast<const pf_Frag_Strux *>(sdh);
	PT_AttrPropIndex indexAP = pfStrux->getIndexAP();
	const PP_AttrProp * pAP = NULL;
	const gchar * pszValue = NULL;

	bool bHiddenRevision = false;

	getAttrProp(indexAP, &pAP,NULL,bShowRevisions,iRevisionLevel,bHiddenRevision);
	
	UT_return_val_if_fail (pAP, false);
	(pAP)->getProperty(szProperty, pszValue);
	
	if(pszValue == NULL)
	{
		*pszRetValue = NULL;
		return false;
	}
	*pszRetValue = pszValue;
	return true;
}

/*!
 * This medthod modifies the attributes of a section strux without
 * generating a change record. Use with extreme care!!
 */
bool  PD_Document::changeStruxAttsNoUpdate(pf_Frag_Strux* sdh, const char * attr, const char * attvalue)
{
	pf_Frag_Strux * pfStrux = sdh;
	UT_return_val_if_fail (pfStrux, false);
	return m_pPieceTable->changeSectionAttsNoUpdate(pfStrux, attr, attvalue);
}


/*!
 * This method inserts a strux of type pts immediately before the sdh given.
 * Attributes of the strux can be optionally passed. This method does not throw
 * a change record and should only be used under exceptional circumstances to 
 * repair the piecetable during loading. It was necessary to import RTF tables.
 */
bool PD_Document::insertStruxNoUpdateBefore(pf_Frag_Strux* sdh, PTStruxType pts, const PP_PropertyVector & attributes )
{
#if 0
	pf_Frag_Strux * pfStrux = sdh;
	T_ASSERT(pfStrux->getStruxType() != PTX_Section);
#endif
	return m_pPieceTable->insertStruxNoUpdateBefore(sdh, pts, attributes);
}

/*!
 * This method examines the frag immediately before the given sdh and decides
 * if it matches the strux type given.
 */
bool PD_Document::isStruxBeforeThis(pf_Frag_Strux* sdh,  PTStruxType pts)
{
	const pf_Frag_Strux * pfs = static_cast<const pf_Frag_Strux *>(sdh);
	pf_Frag * pfb = pfs->getPrev();
	if(pfb->getType() != pf_Frag::PFT_Strux)
		return false;
	pf_Frag_Strux * pfsb = static_cast<pf_Frag_Strux *>(pfb);
	if(pfsb->getStruxType() == pts)
		return true;
	return false;
}

 /*!
 * Create a changerecord object and broadcast it to all the listeners.
 * If bsave is true save the CR in th eunod stack.
 */
bool PD_Document::createAndSendCR(PT_DocPosition dpos, UT_sint32 iType,bool bSave,UT_Byte iGlob)
{
	return m_pPieceTable->createAndSendCR(dpos,iType,bSave,iGlob);
}

/*!
 * method used to import document property changes
 */
bool PD_Document::changeDocPropeties(const PP_PropertyVector & pAtts, const PP_PropertyVector & pProps)
{
	PP_AttrProp  AP;
	if(!pAtts.empty()) {
		AP.setAttributes(pAtts);
	}
	if(!pProps.empty()) {
		AP.setProperties(pProps);
	}
	const gchar * szValue=NULL;
	bool b= AP.getAttribute( PT_DOCPROP_ATTRIBUTE_NAME,szValue);
	if(!b || (szValue == NULL))
		return false;
	gchar * szLCValue = g_utf8_strdown (szValue, -1);
	if(strcmp(szLCValue,"revision") == 0)
    {
		const gchar * szID=NULL;
		const gchar * szDesc=NULL;
		const gchar * szTime=NULL;
		const gchar * szVersion=NULL;
		AP.getAttribute(PT_REVISION_ATTRIBUTE_NAME,szID);
		AP.getAttribute(PT_REVISION_DESC_ATTRIBUTE_NAME,szDesc);
		AP.getAttribute(PT_REVISION_TIME_ATTRIBUTE_NAME,szTime);
		AP.getAttribute(PT_REVISION_VERSION_ATTRIBUTE_NAME,szVersion);
		UT_DEBUGMSG(("Received revision ID %s szDesc %s time %s ver %s \n",szID,szDesc,szTime,szVersion));
		UT_uint32 id = atoi(szID);
		UT_UTF8String sDesc = szDesc; 
		time_t iTime = atoi(szTime);
		UT_uint32 iVer = atoi(szVersion);
		UT_UCS4Char * pD = NULL;
		UT_uint32 iLen = sDesc.ucs4_str().size();
		pD = new UT_UCS4Char [iLen+1];
		UT_UCS4_strncpy(pD,sDesc.ucs4_str().ucs4_str(),iLen);
		pD[iLen] = 0;
		AD_Document::addRevision(id,pD,iTime,iVer, false);
	}
	else if(strcmp(szLCValue,"pagesize") == 0)
    {
#if 0 // some debug code. XXX remove this
		UT_sint32 i = 0;
		UT_DEBUGMSG(("pagesize docprop received \n"));
		const gchar * szP = pProps ? pProps[i] : NULL;
		while(szP != NULL)
		{
			UT_DEBUGMSG(("property %s value %s \n",pProps[i],pProps[i+1]));
			i += 2;
			szP = pProps[i];
		}
#endif
		setPageSizeFromFile(pProps);
	}
	else if(strcmp(szLCValue,"metadata") == 0)
    {
		UT_DEBUGMSG(("metadata docprop received \n"));
		ASSERT_PV_SIZE(pProps);
		for (auto iter = pProps.cbegin(); iter != pProps.cend(); iter += 2) {
			const std::string & sName = *iter;
			const std::string & sValue = *(iter + 1);
			UT_DEBUGMSG(("property %s value %s \n", sName.c_str(), sValue.c_str()));
			setMetaDataProp(sName, sValue);
		}

	}
	else if(strcmp(szLCValue,"addauthor") == 0)
	{
		const gchar * szInt=NULL;
		AP.getProperty("id",szInt);
		UT_DEBUGMSG(("addauthor docprop CR received int %s \n",szInt));
		if(szInt)
		{
			UT_sint32 iAuthor = atoi(szInt);
			pp_Author * pA = addAuthor(iAuthor);
			UT_uint32 j = 0;
			const gchar * szName = NULL;
			szValue = NULL;
			PP_AttrProp * pAP = pA->getAttrProp();
			while(AP.getNthProperty(j++,szName,szValue))
			{
				if(strcmp(szName,"id") == 0)
					continue;
				if(*szValue)
					pAP->setProperty(szName,szValue);
			}
			sendAddAuthorCR(pA);
		}
	}
	else if(strcmp(szLCValue,"changeauthor") == 0)
	{
		const gchar * szInt=NULL;
		pp_Author * pA = NULL;
		if(AP.getProperty("id",szInt) && szInt && *szInt)
	    {
			UT_sint32 iAuthor = atoi(szInt);
			pA = getAuthorByInt(iAuthor);
		}
		if(pA)
		{
			PP_AttrProp * pAP = pA->getAttrProp();
			UT_uint32 j = 0;
			const gchar * szName = NULL;
			while(AP.getNthProperty(j++,szName,szValue))
			{
				if(strcmp(szName,"id") == 0)
					continue;
				if(*szValue)
					pAP->setProperty(szName,szValue);
			}
			sendChangeAuthorCR(pA);
		}
	}
	g_free (szLCValue);
	return true;
}

/* XXX remove this when PP_PropertyVector can be made visible from xad.
 * And change the base pure virtual signature. */
bool PD_Document::createAndSendDocPropCR( const gchar ** pAtts,const gchar ** pProps )
{
	return m_pPieceTable->createAndSendDocPropCR(PP_std_copyProps(pAtts),
												 PP_std_copyProps(pProps));
}


/*!
 * This method creates DocProp Change Record and broadcasts it to the listeners
 */
bool PD_Document::createAndSendDocPropCR( const PP_PropertyVector & pAtts, const PP_PropertyVector & pProps )
{
	return m_pPieceTable->createAndSendDocPropCR(pAtts,pProps);
}

/*!
 * This method deletes a strux of the type specified at the position
 * requested.
 * if bRecordChange is fale no change record is recorded.
 * This method was created soled for the use of AbiCollab.
  * Use with extreme care. Should only be needed by AbiCollab
 */
bool PD_Document::deleteStrux(PT_DocPosition dpos,
							  PTStruxType /*pts*/,
							  bool bRecordChange)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	m_pPieceTable->getFragFromPosition(dpos,&pf,&pOffset);
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf == NULL)
		return false;
	pf_Frag_Strux* sdh = NULL;
	if(pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		sdh = static_cast<pf_Frag_Strux*>(pfs);
	}
	else
	{
		return false;
	}
	if(!bRecordChange)
	{
		return m_pPieceTable->deleteStruxNoUpdate(sdh);
	}
	if(getStruxPosition(sdh) != dpos)
		return false;
	return m_pPieceTable->deleteStruxWithNotify(sdh);
}

/*!
 * This method deletes a strux without throwing a change record.
 * sdh is the StruxDocHandle that gets deleted..
 * Use with extreme care. Should only be used for document import.
 */
bool PD_Document::deleteStruxNoUpdate(pf_Frag_Strux* sdh)
{
	return m_pPieceTable->deleteStruxNoUpdate(sdh);
}

/*!
 * This method deletes a frag without throwing a change record.
 * pf is the frag that gets deleted..
 * Use with extreme care. Should only be used for document import.
 */
bool PD_Document::deleteFragNoUpdate(pf_Frag * pf)
{
	return m_pPieceTable->deleteFragNoUpdate(pf);
}

/*!
 * Returns true if it is legal to insert a hperlink at this position. Looks to see if there is
 * An open hyperlink or annotation upstream.
 */
bool   PD_Document::isInsertHyperLinkValid(PT_DocPosition pos) const
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	while(pf && (pf->getType() != pf_Frag::PFT_Strux) )
	{
		if(pf->getType() == pf_Frag::PFT_Object)
		{
			pf_Frag_Object * pfo = static_cast<pf_Frag_Object *>(pf);
			if((pfo->getObjectType() != PTO_Hyperlink)
               && (pfo->getObjectType() != PTO_Annotation)
               && (pfo->getObjectType() != PTO_RDFAnchor) )
			{
				pf = pf->getPrev();
			}
			else
			{
				PT_AttrPropIndex iAP = pf->	getIndexAP();
				const PP_AttrProp * pAP = NULL;
				m_pPieceTable->getAttrProp(iAP,&pAP);
				UT_return_val_if_fail (pAP, false);
				const gchar * pszXlink = NULL;
				(pAP)->getAttribute(PT_HYPERLINK_TARGET_NAME,pszXlink);
				if(pszXlink)
				{
					return false;
				}
				(pAP)->getAttribute(PT_ANNOTATION_NUMBER,pszXlink);
				if(pszXlink)
				{
					return false;
				}
				(pAP)->getAttribute(PT_RDF_XMLID,pszXlink);
				if(pszXlink)
				{
					return false;
				}
				return true;
			}
		}
		else
		{
			pf = pf->getPrev();
		}
	}
	if(!pf)
		return false;
	if(pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_Block)
		{
			return true;
		}
	}
	return false;
}
/*!
 * This method returns the last pf_Frag_Strux as a pf_Frag_Strux* before the end of the piecetable.
 */
const pf_Frag_Strux*  PD_Document::getLastSectionSDH(void) const
{
	const pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	const pf_Frag_Strux * pfSecLast = NULL;
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
		     const pf_Frag_Strux * pfSec = static_cast<const pf_Frag_Strux *>(currentFrag);
		     if(pfSec->getStruxType() == PTX_Section)
		     {
				 pfSecLast = pfSec;
			 }
		}
		currentFrag = currentFrag->getNext();
	}
	return pfSecLast;
}

/*!
 * This method returns the last pf_Frag_Strux as a pf_Frag_Strux* before the end of the piecetable.
 */
pf_Frag_Strux*  PD_Document::getLastSectionMutableSDH(void)
{
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	pf_Frag_Strux * pfSecLast = NULL;
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
		     pf_Frag_Strux * pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
		     if(pfSec->getStruxType() == PTX_Section)
		     {
				 pfSecLast = pfSec;
			 }
		}
		currentFrag = currentFrag->getNext();
	}
	return pfSecLast;
}


/*!
 * This method returns the last pf_Frag_Strux as a pf_Frag_Strux* 
 * before the end of the piecetable.
 */
pf_Frag_Strux*  PD_Document::getLastStruxOfType(PTStruxType pts )
{
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getLast();
	pf_Frag_Strux * pfSecLast = NULL;
	bool bFound = false;
	UT_sint32 nest = 0;
	pf_Frag_Strux * pfSec = NULL;
	if(pts == PTX_SectionTable)
		nest = 1;
	if(currentFrag->getType()  == pf_Frag::PFT_Strux)
	{
		pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
		if(pfSec->getStruxType() == PTX_EndTable)
			nest--;
	}
	while (!bFound && currentFrag!=m_pPieceTable->getFragments().getFirst())
	{
		UT_return_val_if_fail (currentFrag,0);
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
		     pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
			 if(pts != PTX_EndTable)
			 { 
				 if(pfSec->getStruxType() == PTX_EndTable)
					 nest++;
				 if(pfSec->getStruxType() == PTX_SectionTable)
 					 nest--;
			 }
		     if((pfSec->getStruxType() == pts) && (nest == 0))
		     {
				 pfSecLast = pfSec;
				 bFound = true;
			 }
		}
		currentFrag = currentFrag->getPrev();
	}
	return pfSecLast;
}


/*!
 * This method scans the document to check that the id of a header/footer
 *  section actually exists in a section somewhere in the document.
 */
bool PD_Document::verifySectionID(const gchar * pszId)
{
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		PT_AttrPropIndex indexAP = 0;
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
		     pf_Frag_Strux * pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
		     if(pfSec->getStruxType() == PTX_Section)
		     {
				 indexAP = currentFrag->getIndexAP();
				 const PP_AttrProp * pAP = NULL;
				 m_pPieceTable->getAttrProp(indexAP,&pAP);
				 UT_return_val_if_fail (pAP,false);
				 const gchar * pszIDName = NULL;
				 (pAP)->getAttribute("header", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;
				 (pAP)->getAttribute("header-first", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;
				 (pAP)->getAttribute("header-last", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;
				 (pAP)->getAttribute("header-even", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;
				 (pAP)->getAttribute("footer", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;
				 (pAP)->getAttribute("footer-first", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;
				 (pAP)->getAttribute("footer-last", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;
				 (pAP)->getAttribute("footer-even", pszIDName);
				 if(pszIDName && strcmp(pszIDName,pszId) == 0)
					 return true;

				 // the id could also be hidden in a revision attribute ...
				 const gchar * pszRevisionAttr = NULL;
				 
				 if((pAP)->getAttribute("revision", pszRevisionAttr))
				 {
					 PP_RevisionAttr RA(pszRevisionAttr);

					 for(UT_uint32 i = 0; i < RA.getRevisionsCount(); ++i)
					 {
						 const PP_Revision * pRev = RA.getNthRevision(i);
						 if(!pRev)
						 {
							 UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
							 continue;
						 }

						 (pRev)->getAttribute("header", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
						 (pRev)->getAttribute("header-first", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
						 (pRev)->getAttribute("header-last", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
						 (pRev)->getAttribute("header-even", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
						 (pRev)->getAttribute("footer", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
						 (pRev)->getAttribute("footer-first", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
						 (pRev)->getAttribute("footer-last", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
						 (pRev)->getAttribute("footer-even", pszIDName);
						 if(pszIDName && strcmp(pszIDName,pszId) == 0)
							 return true;
					 }
				 }
		     }
		}
//
// Get Next frag in the table.
//
		currentFrag = currentFrag->getNext();
	}
	return false;
}



/*!
 * This method scans the document to look for a HdrFtr strux.
\param const char * pszHdrFtr The particular attribute that identifies the
                               strux as "header" "footer" "header-even" etc.
\param const char * pszHdrFtrID the unique string to match with Docsection.
\returns a pf_Frag_Strux* of the matching frag or NULL if none found.
 */
pf_Frag_Strux* PD_Document::findHdrFtrStrux(const gchar * pszHdrFtr,
											const gchar * pszHdrFtrID)
{
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		PT_AttrPropIndex indexAP = 0;
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
		     pf_Frag_Strux * pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
		     if(pfSec->getStruxType() == PTX_SectionHdrFtr)
		     {
				 indexAP = pfSec->getIndexAP();
				 const PP_AttrProp * pAP = NULL;
				 m_pPieceTable->getAttrProp(indexAP,&pAP);
				 UT_return_val_if_fail (pAP, NULL);
				 const gchar * pszIDName = NULL;
				 const gchar * pszHeaderName = NULL;
				 (pAP)->getAttribute(PT_TYPE_ATTRIBUTE_NAME, pszHeaderName);
				 (pAP)->getAttribute(PT_ID_ATTRIBUTE_NAME, pszIDName);
				 if(pszIDName && pszHeaderName && (strcmp(pszIDName,pszHdrFtrID) == 0) && (strcmp(pszHeaderName,pszHdrFtr) == 0))
					 return static_cast<pf_Frag_Strux*>(pfSec) ;
			 }
		}
//
// Get Next frag in the table.
//
		currentFrag = currentFrag->getNext();
	}
	return NULL;
}


/*!
 * This method returns the offset to a an embedded strux 
 * And a pointer to the embedded strux found.
 * If no emebedded strux is found in the block we return -1 ans NULL
 */ 
UT_sint32 PD_Document::getEmbeddedOffset(pf_Frag_Strux* sdh, PT_DocPosition posoff, pf_Frag_Strux* & sdhEmbedded)
{
	pf_Frag_Strux * pfs = sdh;
	UT_return_val_if_fail (pfs->getStruxType() == PTX_Block,-1);
	pf_Frag * pf = pfs;
	pf = pf->getNext();
	PT_DocPosition pos = m_pPieceTable->getStruxPosition(sdh) + posoff;
	while(pf && m_pPieceTable->getFragPosition(pf) + pf->getLength() <= pos)
	{
		pf = pf->getNext();
	}
	if(pf == NULL)
	{
		sdhEmbedded = NULL;
		return -1;
	}
	while(pf && pf->getType() != pf_Frag::PFT_Strux)
	{
		pf = pf ->getNext();
	}
	if(pf == NULL)
	{
		sdhEmbedded = NULL;
		return -1;
	}
	if(!m_pPieceTable->isFootnote(pf))
    {
		sdhEmbedded = NULL;
		return -1;
	}
	pf_Frag_Strux * pfsNew = static_cast<pf_Frag_Strux *>(pf);
	pos  = m_pPieceTable->getFragPosition(pf);
	UT_sint32 diff = static_cast<UT_sint32>(pos) - static_cast<UT_sint32>(m_pPieceTable->getFragPosition(pfs));
	sdhEmbedded = pfsNew;
	return diff;
}

bool PD_Document::hasEmbedStruxOfTypeInRange(PT_DocPosition posStart, PT_DocPosition posEnd, 
											 PTStruxType iType) const
{
	UT_return_val_if_fail(posStart < posEnd,false);
	if ((iType != PTX_SectionFootnote) && (iType != PTX_SectionEndnote) &&
		(iType != PTX_SectionAnnotation))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	return m_pPieceTable->hasEmbedStruxOfTypeInRange(posStart,posEnd,iType);
}


/*!
 * This method returns true if there is a Footnote strux at exactly this 
 * position.
 */
bool PD_Document::isFootnoteAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	while(pf && (pf->getLength() == 0))
    {
		pf = pf->getPrev();
    }
	bool b = m_pPieceTable->isFootnote(pf);
	if(b)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_SectionTOC)
		{
			return false;
		}
	}
	return b;
}


/*!
 * This method returns true if there is a TOC or endTOC strux at exactly this 
 * position.
 */
bool PD_Document::isTOCAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	while(pf && (pf->getLength() == 0))
    {
		pf = pf->getPrev();
    }
	bool b = (pf && (pf->getType() == pf_Frag::PFT_Strux));
	if(b)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_SectionTOC)
		{
			return true;
		}
	}
	if(b)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_EndTOC)
		{
			return true;
		}
	}
	return false;
}


/*!
 * This method returns true if there is an EndFootnote strux at exactly this 
 * position.
 */
bool PD_Document::isEndFootnoteAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	while(pf && (pf->getLength() == 0))
	{
		pf = pf->getPrev();
	}
	if(!pf || pf->getPos() < pos)
	{
		return false;
	}
	bool b = m_pPieceTable->isEndFootnote(pf);
	if(b)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_EndTOC)
		{
			return false;
		}
	}
	return b;
}


/*!
 * This method returns true if there is a frame strux at exactly this 
 * position.
 */
bool PD_Document::isFrameAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_SectionFrame)
		{
			return true;
		}
	}
	return false;
}



/*!
 * This method returns true if there is an endFrame strux at exactly this 
 * position.
 */
bool PD_Document::isEndFrameAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_EndFrame)
		{
			return true;
		}
	}
	return false;
}


/*!
 * This method returns true if there is a HdrFtr strux at exactly this 
 * position.
 */
bool PD_Document::isHdrFtrAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_SectionHdrFtr)
		{
			return true;
		}
	}
	return false;
}



/*!
 * This method returns true if there is a Section strux at exactly this 
 * position.
 */
bool PD_Document::isSectionAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_Section)
		{
			return true;
		}
	}
	return false;
}


/*!
 * This method returns true if there is a Block strux at exactly this 
 * position.
 */
bool PD_Document::isBlockAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_Block)
		{
			return true;
		}
	}
	return false;
}


//============================================================================
// Table Medthods
//===========================================================================

/*!
 * This method returns true if there is a table strux at exactly this 
 * position.
 */
bool PD_Document::isTableAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_SectionTable)
		{
			return true;
		}
	}
	return false;
}


/*!
 * This method returns true if there is an EndTable strux at exactly this 
 * position.
 */
bool PD_Document::isEndTableAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_EndTable)
		{
			return true;
		}
	}
	return false;
}


/*!
 * This method returns true if there is a cell strux at exactly this 
 * position.
 */
bool PD_Document::isCellAtPos(PT_DocPosition pos)
{
	PT_BlockOffset pOffset;
	pf_Frag * pf = NULL;
	/*bool bRes = */m_pPieceTable->getFragFromPosition(pos,&pf,&pOffset);
	if(!pf)
		return false;
	while(pf && pf->getLength() == 0)
		pf = pf->getPrev();
	if(pf && pf->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if(pfs->getStruxType() == PTX_SectionCell)
		{
			return true;
		}
	}
	return false;
}

/*!
 * This method returns the end table strux associated with the table strux tableSDH
 * Returns NULL on failure to find it.
 */
pf_Frag_Strux* PD_Document::getEndTableStruxFromTableSDH(pf_Frag_Strux* tableSDH)
{
	pf_Frag * currentFrag = tableSDH;
	currentFrag = currentFrag->getNext();
	UT_sint32 depth =0;
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
			if(pfSec->getStruxType() == PTX_SectionTable)
				depth++;
			else if(pfSec->getStruxType() == PTX_EndTable)
			{
				if(depth == 0)
				{
					return pfSec;
				}
				else
					depth--;
			}
		}
//
// Get Next frag in the table.
//
		currentFrag = currentFrag->getNext();
	}
	return NULL;
}
/*!
 * This method returns the end cell strux associated with the cell strux cellSDH
 * Returns NULL on failure to find it.
 */
pf_Frag_Strux* PD_Document::getEndCellStruxFromCellSDH(pf_Frag_Strux* cellSDH)
{
	pf_Frag * currentFrag = cellSDH;
	currentFrag = currentFrag->getNext();
	while (currentFrag && currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfSec = static_cast<pf_Frag_Strux*>(currentFrag);
			if(pfSec->getStruxType() == PTX_SectionTable)
			{
				pf_Frag_Strux* endTab = getEndTableStruxFromTableSDH(pfSec);
				currentFrag = endTab;
			}
			else if(pfSec->getStruxType() == PTX_EndCell )
			{
				return pfSec;
			}
			else if(pfSec->getStruxType() == PTX_SectionCell)
			{
				return NULL;
			}
			else if(pfSec->getStruxType() == PTX_EndTable)
			{
				return NULL;
			}
		}
//
// Get Next frag in the table.
//
		if(currentFrag)
		{
			currentFrag = currentFrag->getNext();
		}
	}
	return NULL;
}

/*!
 * This method returns the end table strux associated with the table strux tableSDH
 * Returns NULL on failure to find it.
 */
pf_Frag_Strux* PD_Document::getEndTableStruxFromTablePos(PT_DocPosition tablePos)
{
	pf_Frag_Strux* tableSDH = NULL;
	pf_Frag_Strux* EndTableSDH = NULL;
	bool bRes =	getStruxOfTypeFromPosition(tablePos, PTX_SectionTable, &tableSDH);
	if(!bRes)
		return NULL;
	EndTableSDH = getEndTableStruxFromTableSDH(tableSDH);
	return EndTableSDH;
}

/*!
 * The method returns the number of rows and columns in table pointed to by tableSDH
\param pf_Frag_Strux* tableSDH SDH of the table in question
\param UT_sint32 * numRows pointer to the number of rows returned
\param UT_sint32 * numCols pointer to the number of cols returned
*/
bool PD_Document::getRowsColsFromTableSDH(pf_Frag_Strux* tableSDH, bool bShowRevisions, UT_uint32 iRevisionLevel,
										  UT_sint32 * numRows, UT_sint32 * numCols)
{
	UT_sint32 iRight = 0;
    UT_sint32 iBot = 0;
	const char * szRight = NULL;
	const char * szBot = NULL;
	pf_Frag_Strux* cellSDH;
	*numRows = 0;
	*numCols = 0;
//
// Do the scan
//
	pf_Frag * currentFrag = tableSDH;
	currentFrag = currentFrag->getNext();
	while (currentFrag && currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
			if(pfSec->getStruxType() == PTX_SectionTable)
			{
//
// skip to the end of this nested table
//
				pf_Frag_Strux* endSDH = getEndTableStruxFromTableSDH(pfSec);
				pfSec = endSDH;
			}
			else if(pfSec->getStruxType() == PTX_EndTable)
				return true;
			else if(pfSec->getStruxType() == PTX_SectionCell)
			{
				cellSDH = pfSec;
				UT_DebugOnly<bool> bres = getPropertyFromSDH(cellSDH,bShowRevisions, iRevisionLevel,"right-attach",&szRight);
				UT_ASSERT(bres);
				if(szRight && *szRight)
					iRight = atoi(szRight);
				bres = getPropertyFromSDH(cellSDH,bShowRevisions, iRevisionLevel,"bot-attach",&szBot);
				UT_ASSERT(bres);
				if(szBot && *szBot)
					iBot = atoi(szBot);

				if(*numCols < iRight)
					*numCols = iRight;
				if(*numRows < iBot)
					*numRows = iBot;
			}
			currentFrag = pfSec;
		}
		if(currentFrag)
			currentFrag = currentFrag->getNext();
	}
	return false;
}

void  PD_Document::miniDump(pf_Frag_Strux* sdh, UT_sint32 nstruxes)
{
#ifdef DEBUG
	UT_sint32 i=0;
	const pf_Frag_Strux * pfs = sdh;
	const pf_Frag * pf = pfs;
	for(i=0; pfs && (i< nstruxes); i++)
	{
		pf = pf->getPrev();
		while(pf && pf->getType() != pf_Frag::PFT_Strux)
			pf = pf->getPrev();
		pfs = static_cast<const pf_Frag_Strux *>(pf);
	}
	if(pfs == NULL)
	{
		pf =  m_pPieceTable->getFragments().getFirst();
		while(pf && (pf->getType() != pf_Frag::PFT_Strux))
			pf = pf->getNext();
		if(pf)
			pfs = static_cast<const pf_Frag_Strux *>(pf);
	}
	for(i=0; pfs && (i< 2*nstruxes); i++)
	{
		pf = pfs;
		pfs = static_cast<const pf_Frag_Strux *>(pf);
		const char * szStrux = NULL;
		if(pfs->getStruxType() == PTX_Block)
			szStrux = "Block";
		else if(pfs->getStruxType() == PTX_SectionTable)
			szStrux = "Table";
		else if(pfs->getStruxType() == PTX_SectionCell)
			szStrux = "Cell";
		else if(pfs->getStruxType() == PTX_EndTable)
			szStrux = "End Table";
		else if(pfs->getStruxType() == PTX_EndCell)
			szStrux = "End Cell";
		else if(pfs->getStruxType() == PTX_SectionFootnote)
			szStrux = "Footnote";
		else if(pfs->getStruxType() == PTX_EndFootnote)
			szStrux = "End Footnote";
		else if(pfs->getStruxType() == PTX_SectionAnnotation)
			szStrux = "Annotation";
		else if(pfs->getStruxType() == PTX_EndAnnotation)
			szStrux = "End Annotation";
		else if(pfs->getStruxType() == PTX_SectionEndnote)
			szStrux = "Endnote";
		else if(pfs->getStruxType() == PTX_EndEndnote)
			szStrux = "End Endnote";
		else if(pfs->getStruxType() == PTX_Section)
			szStrux = "Section";
		else if(pfs->getStruxType() == PTX_SectionFrame)
			szStrux = "Frame";
		else if(pfs->getStruxType() == PTX_EndFrame)
			szStrux = "EndFrame";
		else
			szStrux = "Other Strux";
		if(i< nstruxes)
		{
			UT_DEBUGMSG(("MiniDump Before Frag %p Type %s \n",pfs,szStrux));
		}
		else if(i > nstruxes)
		{
			UT_DEBUGMSG(("MiniDump After Frag %p Type %s \n",pfs,szStrux));
		}
		if(pfs == static_cast<const pf_Frag_Strux *>(sdh))
		{
			UT_DEBUGMSG(("MiniDump Actual Frag %p Type %s \n",pfs,szStrux));
		}
		const char * szLeft=NULL;
		const char * szRight=NULL;
		const char * szTop=NULL;
		const char * szBot = NULL;
		getPropertyFromSDH(pfs,true, PD_MAX_REVISION,"left-attach",&szLeft);
		getPropertyFromSDH(pfs,true, PD_MAX_REVISION,"right-attach",&szRight);
		getPropertyFromSDH(pfs,true, PD_MAX_REVISION,"top-attach",&szTop);
		getPropertyFromSDH(pfs,true, PD_MAX_REVISION,"bot-attach",&szBot);
		if(szLeft != NULL)
		{
			UT_DEBUGMSG(("left-attach %s right-attach %s top-attach %s bot-attach %s \n",szLeft,szRight,szTop,szBot));
		}
		pf = pf->getNext();
		while(pf && pf->getType() != pf_Frag::PFT_Strux)
		{
			UT_DEBUGMSG(("MiniDump: Other Frag %p of Type %d \n",pf,pf->getType()));
			pf = pf->getNext();
		}
		if(pf)
			pfs= static_cast<const pf_Frag_Strux *>(pf);
	}
#else
	UT_UNUSED(sdh);
	UT_UNUSED(nstruxes);
#endif
}
		
bool
PD_Document::dumpDoc( const char* msg, PT_DocPosition currentpos, PT_DocPosition endpos )
{
    return m_pPieceTable->dumpDoc( msg, currentpos, endpos );
}

/*!
 * The method returns the SDH of the cell at the location given by (rows,columns) in table 
 * pointed to by tableSDH. Returns NULL if the requested location is not contained in the
 * cell.
\param pf_Frag_Strux* tableSDH SDH of the table in question
\param UT_sint32 row row location.
\param UT_sint32 col column location
*/

pf_Frag_Strux* PD_Document::getCellSDHFromRowCol(pf_Frag_Strux* tableSDH,
													bool bShowRevisions, UT_uint32 iRevisionLevel,
													UT_sint32 row, 
													UT_sint32 col)
{
	UT_sint32 Top,Left,Bot,Right;
	const char * szLeft = NULL;
	const char * szTop = NULL;
	const char * szRight = NULL;
	const char * szBot = NULL;
	pf_Frag_Strux* cellSDH;
//
// Do the scan
//
	pf_Frag * currentFrag = tableSDH;

	UT_return_val_if_fail(currentFrag != NULL, NULL);

	currentFrag = currentFrag->getNext();
	while (currentFrag && currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_val_if_fail (currentFrag,0);
		if(currentFrag->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfSec = static_cast<pf_Frag_Strux *>(currentFrag);
			if(pfSec->getStruxType() == PTX_SectionTable)
			{
//
// skip to the end of this nested table
//
				pfSec = getEndTableStruxFromTableSDH(pfSec);
			}
			else if(pfSec->getStruxType() == PTX_EndTable)
			{
//
// end of table before the requested cell was found. Return NULL
//
				return NULL;
			}
			else if(pfSec->getStruxType() == PTX_SectionCell)
			{
				cellSDH = pfSec;
				Left = -1;
				Top = -1;
				Right = -1;
				Bot = -1;
				UT_DebugOnly<bool> bres = getPropertyFromSDH(cellSDH,bShowRevisions,iRevisionLevel,"left-attach",&szLeft);
				UT_ASSERT(bres);
				if(szLeft && *szLeft)
					Left = atoi(szLeft);
				bres = getPropertyFromSDH(cellSDH,bShowRevisions,iRevisionLevel,"top-attach",&szTop);
				UT_ASSERT(bres);
				if(szTop && *szTop)
					Top = atoi(szTop);
				bres = getPropertyFromSDH(cellSDH,bShowRevisions,iRevisionLevel,"right-attach",&szRight);
				UT_ASSERT(bres);
				if(szRight && *szRight)
					Right = atoi(szRight);
				bres = getPropertyFromSDH(cellSDH,bShowRevisions,iRevisionLevel,"bot-attach",&szBot);
				UT_ASSERT(bres);
				if(szBot && *szBot)
					Bot = atoi(szBot);
				if( (Top <= row) && (row < Bot) && (Left <= col) && (Right > col))
				{
					return pfSec;
				}
			}
			currentFrag = pfSec;
		}
		if(currentFrag)
			currentFrag = currentFrag->getNext();
	}
	return NULL;
}

//===================================================================================
/*!
 * This method scans the document for all styles used in the document, including
 * styles in the basedon heiracy and the followedby list
 *
 */
void PD_Document::getAllUsedStyles(UT_GenericVector <PD_Style*>* pVecStyles)
{
	UT_sint32 i = 0;
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	PD_Style * pStyle = NULL;
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		UT_return_if_fail (currentFrag);
//
// get indexAP
// get PT_STYLE_ATTRIBUTE_NAME
// if it matches style name or is contained in a basedon name or followedby
//
//
// All this code is used to find if this frag has a style in it.
//
		PT_AttrPropIndex indexAP = 0;
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
			indexAP = static_cast<pf_Frag_Strux *>(currentFrag)->getIndexAP();
		else if(currentFrag->getType()  == pf_Frag::PFT_Text)
			indexAP = static_cast<pf_Frag_Text *>(currentFrag)->getIndexAP();
		else if(currentFrag->getType()  == pf_Frag::PFT_Object)
			indexAP = static_cast<pf_Frag_Object *>(currentFrag)->getIndexAP();
		else if(currentFrag->getType()  == pf_Frag::PFT_FmtMark)
			indexAP = static_cast<pf_Frag_FmtMark *>(currentFrag)->getIndexAP();
		const PP_AttrProp * pAP = NULL;
		m_pPieceTable->getAttrProp(indexAP,&pAP);
		UT_return_if_fail (pAP);
		const gchar * pszStyleName = NULL;
		(pAP)->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyleName);
//
// We've found a style...
//
		if(pszStyleName != NULL)
		{
			m_pPieceTable->getStyle(pszStyleName,&pStyle);
			UT_return_if_fail (pStyle);
			if(pStyle)
			{
				if(pVecStyles->findItem(pStyle) < 0)
					pVecStyles->addItem(pStyle);
				PD_Style * pBasedOn = pStyle->getBasedOn();
				i = 0;
				while(pBasedOn != NULL && i <  pp_BASEDON_DEPTH_LIMIT)
				{
					if(pVecStyles->findItem(pBasedOn) < 0)
						pVecStyles->addItem(pBasedOn);
					i++;
					pBasedOn = pBasedOn->getBasedOn();
				}
				PD_Style * pFollowedBy = pStyle->getFollowedBy();
				if(pFollowedBy && (pVecStyles->findItem(pFollowedBy) < 0))
					pVecStyles->addItem(pFollowedBy);
			}
		}
//
// Get Next frag in the table.
//
		currentFrag = currentFrag->getNext();
	}
//
// Done!
//
}


struct prevStuff
{
private:
	pf_Frag::PFType fragType;
	pf_Frag_Strux * lastFragStrux;
	PT_AttrPropIndex indexAPFrag;
	pf_Frag * thisFrag;
	PT_DocPosition thisPos;
	PT_DocPosition thisStruxPos;
	UT_uint32 fragLength;
	bool bChangeIndexAP;
	friend bool PD_Document::removeStyle(const gchar * pszName);
};

/*!
 * This method removes the style of name pszName from the styles definition and removes
 * all instances of it from the document including the basedon heiracy and the
 * followed-by sequences.
 */
bool PD_Document::removeStyle(const gchar * pszName)
{
	UT_return_val_if_fail (m_pPieceTable, false);
//
// First replace all occurances of pszName with "Normal"
//
	PD_Style * pNormal = NULL;
	PD_Style * pNuke = NULL;
	m_pPieceTable->getStyle(pszName,&pNuke);
	UT_return_val_if_fail (pNuke, false);
	pNormal = pNuke->getBasedOn();
	const gchar * szBack = NULL;
	if(pNormal == NULL)
	{
		m_pPieceTable->getStyle("Normal",&pNormal);
		szBack = "None";
	}
	else
	{
//
// The name of the style is stored in the PT_NAME_ATTRIBUTE_NAME attribute within the
// style
//
		pNormal->getAttribute(PT_NAME_ATTRIBUTE_NAME, szBack);
	}
	UT_return_val_if_fail (szBack, false);
	UT_return_val_if_fail (pNormal, false);
	PT_AttrPropIndex indexNormal = pNormal->getIndexAP();

//
// Now scan through the document finding all instances of pszName as either
// the style or the basedon style or the followed by style. Replace these with
// "normal"
//
	UT_GenericVector<prevStuff *> vFrag;

	PT_DocPosition pos = 0;
	pf_Frag_Strux * pfs = NULL;
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	UT_return_val_if_fail (currentFrag,false);
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
//
// get indexAP
// get PT_STYLE_ATTRIBUTE_NAME
// if it matches style name or is contained in a basedon name or followedby
//
//
// All this code is used to find if this strux has our style in it
//
		PT_AttrPropIndex indexAP = 0;
		if(currentFrag->getType()  == pf_Frag::PFT_Strux)
		{
			pfs = static_cast<pf_Frag_Strux *>(currentFrag);
			indexAP = static_cast<pf_Frag_Strux *>(currentFrag)->getIndexAP();
		}
		else if(currentFrag->getType()  == pf_Frag::PFT_Text)
		{
			indexAP = static_cast<pf_Frag_Text *>(currentFrag)->getIndexAP();
		}
		else if(currentFrag->getType()  == pf_Frag::PFT_Object)
		{
			indexAP = static_cast<pf_Frag_Object *>(currentFrag)->getIndexAP();
		}
		else if(currentFrag->getType()  == pf_Frag::PFT_FmtMark)
		{
			indexAP = static_cast<pf_Frag_FmtMark *>(currentFrag)->getIndexAP();
		}
		const PP_AttrProp * pAP = NULL;
		m_pPieceTable->getAttrProp(indexAP,&pAP);
		UT_return_val_if_fail (pAP, false);
		const gchar * pszStyleName = NULL;
		(pAP)->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyleName);
//
// It does so remember this frag and set the old indexAP to Normal
//
		if(pszStyleName != NULL && strcmp(pszStyleName,pszName)==0)
		{
			prevStuff *  pStuff = new prevStuff;
			pf_Frag::PFType cType = currentFrag->getType();
			pStuff->fragType = cType;
			pStuff->thisFrag = currentFrag;
			pStuff->indexAPFrag = indexAP;
			pStuff->lastFragStrux = pfs;
			pStuff->thisPos = pos;
			pStuff->thisStruxPos = pos;
			pStuff->fragLength = currentFrag->getLength();
			pStuff->bChangeIndexAP = true;
			vFrag.addItem(pStuff);
//
// OK set this frag's indexAP to that of basedon of our deleted style or
// Normal.
//
			if(pf_Frag::PFT_Strux == cType)
				static_cast<pf_Frag_Strux *>(currentFrag)->setIndexAP(indexNormal);
			else if(pf_Frag::PFT_Text == cType)
				static_cast<pf_Frag_Text *>(currentFrag)->setIndexAP(indexNormal);
			else if(pf_Frag::PFT_Object == cType)
				static_cast<pf_Frag_Object *>(currentFrag)->setIndexAP(indexNormal);
			else if(pf_Frag::PFT_FmtMark == cType)
				static_cast<pf_Frag_FmtMark *>(currentFrag)->setIndexAP(indexNormal);
		}
//
// Now recursively search to see if has our style in the basedon list
//
		else if(pszStyleName != NULL)
		{
			PD_Style * cStyle = NULL;
			m_pPieceTable->getStyle(pszStyleName,&cStyle);
			UT_ASSERT_HARMLESS(cStyle);
			if(!cStyle)
				break;
			PD_Style * pBasedOn = cStyle->getBasedOn();
			PD_Style * pFollowedBy = cStyle->getFollowedBy();
			UT_uint32 i =0;
			for(i=0; (i < pp_BASEDON_DEPTH_LIMIT) && (pBasedOn != NULL) && (pBasedOn!= pNuke); i++)
			{
				pBasedOn = pBasedOn->getBasedOn();
			}
			if(pBasedOn == pNuke)
			{
				prevStuff *  pStuff = new prevStuff;
				pStuff->fragType = currentFrag->getType();
				pStuff->thisFrag = currentFrag;
				pStuff->indexAPFrag = indexAP;
				pStuff->lastFragStrux = pfs;
				pStuff->thisPos = pos;
				pStuff->thisStruxPos = pos;
				pStuff->fragLength = currentFrag->getLength();
				pStuff->bChangeIndexAP = false;
				vFrag.addItem(pStuff);
			}
//
// Look if followedBy points to our style
//
			else if(pFollowedBy == pNuke)
			{
				prevStuff *  pStuff = new prevStuff;
				pStuff->fragType = currentFrag->getType();
				pStuff->thisFrag = currentFrag;
				pStuff->indexAPFrag = indexAP;
				pStuff->lastFragStrux = pfs;
				pStuff->thisPos = pos;
				pStuff->thisStruxPos = pos;
				pStuff->fragLength = currentFrag->getLength();
				pStuff->bChangeIndexAP = false;
				vFrag.addItem(pStuff);
			}
		}
		pos = pos + currentFrag->getLength();
		currentFrag = currentFrag->getNext();
	}
//
// Now replace all pointers to this style in basedon or followedby
// with Normal
//
	UT_uint32 nstyles = getStyleCount();
	UT_uint32 i;
	UT_GenericVector<PD_Style*> * pStyles = NULL;
	enumStyles(pStyles);
	UT_return_val_if_fail( pStyles, false );
	
	for(i=0; i< nstyles;i++)
	{
		// enumStyles(i, &szCstyle,&cStyle);
		PD_Style * pStyle = pStyles->getNthItem(i);
		UT_return_val_if_fail( pStyle, false );
		
		bool bDoBasedOn = false;
		bool bDoFollowedby = false;
		if(pStyle->getBasedOn() == pNuke)
		{
			bDoBasedOn = true;
		}
		if(pStyle->getFollowedBy() == pNuke)
		{
			bDoFollowedby = true;
		}
		PP_PropertyVector nAtts;

		if( bDoBasedOn && bDoFollowedby)
		{
			nAtts.push_back("basedon");
			nAtts.push_back(szBack);
			nAtts.push_back("followedby");
			nAtts.push_back("Current Settings");
		}
		else if ( bDoBasedOn && ! bDoFollowedby)
		{
			nAtts.push_back("basedon");
			nAtts.push_back(szBack);
		}
		else if ( !bDoBasedOn && bDoFollowedby)
		{
			nAtts.push_back("followedby");
			nAtts.push_back("Current Settings");
		}
		if( bDoBasedOn || bDoFollowedby)
		{
			pStyle->addAttributes(nAtts);
		}
	}

	delete pStyles;
//
// OK Now remove the style
//
	m_pPieceTable->removeStyle(pszName);
//
// Alright now we replace all the instances of fragSrux using the style to be
// deleted.
//
	UT_sint32 countChanges = vFrag.getItemCount();
	UT_sint32 j;
	pf_Frag * pfsLast = NULL;
	PX_ChangeRecord * pcr = NULL;
	for(j = 0; j<countChanges; j++)
	{
		prevStuff * pStuff = static_cast<prevStuff *>(vFrag.getNthItem(j));
		if(pStuff->fragType == pf_Frag::PFT_Strux)
		{
			pfsLast = pStuff->thisFrag;
			if(pStuff->bChangeIndexAP)
			{
				pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangeStrux,pStuff->thisPos,indexNormal,
										  pfsLast->getXID());
				notifyListeners(pStuff->lastFragStrux, pcr);
				delete pcr;
			}
			else
			{
				pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangeStrux,pStuff->thisPos,pStuff->indexAPFrag,
										  pfsLast->getXID());
				notifyListeners(pStuff->lastFragStrux, pcr);
				delete pcr;
			}
		}
		else
		{
			if(pStuff->lastFragStrux != pfsLast)
			{
				pfsLast = pStuff->lastFragStrux;
				if(pStuff->bChangeIndexAP)
				{
					pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangeStrux,pStuff->thisPos,indexNormal,
											  pfsLast->getXID());
					notifyListeners(pStuff->lastFragStrux, pcr);
					delete pcr;
				}
				else
				{
					PT_AttrPropIndex indexLastAP = static_cast<pf_Frag_Strux *>(pfsLast)->getIndexAP();
					pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangeStrux,pStuff->thisPos,indexLastAP,
											  pfsLast->getXID());
					notifyListeners(pStuff->lastFragStrux, pcr);
					delete pcr;
				}
			}
		}
	}
//  		else if(bisCharStyle)
//  		{
//  			UT_uint32 blockoffset = (UT_uint32) (pStuff->thisPos - pStuff->thisStruxPos -1);
//  			pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pStuff->thisFrag);
//  			PX_ChangeRecord_SpanChange * pcr =
//  				new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,
//  											   pStuff->thisPos,
//  											   pStuff->indexAPFrag,
//  											   indexNormal,
//  											   m_pPieceTable->getVarSet().getBufIndex(pft->getBufIndex(),0),
//  											   pStuff->fragLength,
//  											   blockoffset);
//  			notifyListeners(pStuff->lastFragStrux, pcr);
//  			delete pcr;
//  		}
//  	}
	if(countChanges > 0)
	{
		UT_VECTOR_PURGEALL(prevStuff *,vFrag);
	}
//
// Now reformat the entire document
//
//	signalListeners(PD_SIGNAL_REFORMAT_LAYOUT);
	return true;
}

bool PD_Document::appendStyle(const PP_PropertyVector & attributes)
{
	UT_return_val_if_fail (m_pPieceTable, false);

	// can only be used while loading the document

	return m_pPieceTable->appendStyle(attributes);
}

size_t PD_Document::getStyleCount(void)
{
  UT_return_val_if_fail (m_pPieceTable, false);

  return m_pPieceTable->getStyleCount();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool PD_Document::tellListener(PL_Listener* pListener)
{
	UT_return_val_if_fail (pListener,false);
	UT_return_val_if_fail (m_pPieceTable,false);

	return m_pPieceTable->tellListener(pListener);
}

bool PD_Document::tellListenerSubset( PL_Listener* pListener,
                                      PD_DocumentRange * pDocRange,
                                      PL_ListenerCoupleCloser* closer )
{
	UT_return_val_if_fail (pListener, false);
	UT_return_val_if_fail (m_pPieceTable, false);
	UT_return_val_if_fail (pDocRange && pDocRange->m_pDoc==this, false);

	return m_pPieceTable->tellListenerSubset(pListener,pDocRange,closer);
}

bool PD_Document::addListener(PL_Listener * pListener,
								 PL_ListenerId * pListenerId)
{
	UT_sint32 kLimit = m_vecListeners.getItemCount();
	UT_sint32 k=0;

	// see if we can recycle a cell in the vector.

	for (k=0; k<kLimit; k++)
		if (m_vecListeners.getNthItem(k) == 0)
		{
			m_vecListeners.setNthItem(k,pListener,NULL);
			goto ClaimThisK;
		}

	// otherwise, extend the vector for it.

	if (m_vecListeners.addItem(pListener,&k) != 0)
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;				// could not add item to vector
	}
  ClaimThisK:

	// propagate the listener to the PieceTable and
	// let it do its thing.
	UT_return_val_if_fail (m_pPieceTable, false);

	// give our vector index back to the caller as a "Listener Id".

	*pListenerId = k;
	UT_return_val_if_fail (pListener, false);
	m_pPieceTable->addListener(pListener,k);
	return true;
}

bool PD_Document::removeListener(PL_ListenerId listenerId)
{
	xxx_UT_DEBUGMSG(("Removing lid %d from document %x \n",listenerId,this));
	bool res = (m_vecListeners.setNthItem(listenerId,NULL,NULL) == 0);

	// clear out all format handles that this listener has created
	pf_Frag* pFrag = m_pPieceTable->getFragments().getFirst();
	for (; pFrag; pFrag = pFrag->getNext())
	{
		if (pFrag->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux* pFS = static_cast<pf_Frag_Strux*>(pFrag);
			pFS->setFmtHandle(listenerId, NULL);
		}
	}

	return res;
}

bool PD_Document::signalListeners(UT_uint32 iSignal) const
{
	if(m_bIgnoreSignals)
		return true;
	if(iSignal == PD_SIGNAL_UPDATE_LAYOUT)
	{
			m_iUpdateCount++;
	}
	else
	{
			m_iUpdateCount = 0;
	}
	if(m_iUpdateCount > 1)
  	{
			return true;
	}
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).

	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = m_vecListeners.getNthItem(lid);
		if (pListener)
		{
			pListener->signal(iSignal);
		}
	}

	return true;
}

/*!
 * Remove all AbiCollab connections. We should do this before the document is destructed.
 */
void PD_Document::removeConnections(void)
{
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();
	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = static_cast<PL_Listener *>(m_vecListeners.getNthItem(lid));
		if (pListener)
		{
			if(pListener->getType() >= PTL_CollabExport)
			{
				static_cast<PL_DocChangeListener *>(pListener)->removeDocument();
				removeListener(lid);
			}
		}
	}
}


/*!
 * Change all AbiCollab connections to point to the new document.
 */
void PD_Document::changeConnectedDocument(PD_Document * pDoc)
{
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();
	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = static_cast<PL_Listener *>(m_vecListeners.getNthItem(lid));
		if (pListener)
		{
			if(pListener->getType() >= PTL_CollabExport )
			{
				static_cast<PL_DocChangeListener *>(pListener)->setNewDocument(pDoc);
				removeListener(lid);
			}
		}
	}
}

std::list<AV_View*> PD_Document::getAllViews() const
{
    UT_GenericVector<AV_View *> t;
    getAllViews( &t );
    std::list<AV_View*> ret;
    for( int i=0; i < t.size(); ++i )
        ret.push_back( (AV_View*)t[i] );
    return ret;
}


/*!
 * return a vector of all the views attached to this document.
 */
void PD_Document::getAllViews(UT_GenericVector<AV_View *> * vecViews) const
{
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).

	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = static_cast<PL_Listener *>(m_vecListeners.getNthItem(lid));
		if (pListener)
		{
			if(pListener->getType() == PTL_DocLayout)
				{
					fl_DocListener * pLayoutList = static_cast<fl_DocListener *>(pListener);
					const FL_DocLayout * pLayout = pLayoutList->getLayout();
					if(pLayout != NULL)
					{
						AV_View * pView = reinterpret_cast<AV_View *>(pLayout->getView());
						if(pView != NULL)
						 {
							 vecViews->addItem(pView);
						 }
					}
				}
		}
	}
}

bool PD_Document::notifyListeners(const pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const
{
	// notify listeners of a change.

#ifdef PT_TEST
	//pcr->__dump();
#endif
	m_iUpdateCount = 0;
	if(pcr->getDocument() == NULL)
	{
	        pcr->setDocument(this);
			pcr->setCRNumber();
	}
	else if(pcr->getCRNumber() == 0)
	{
			pcr->setCRNumber();
	}
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listners (for listeners which have been
	// removed (views that went away)).

	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = static_cast<PL_Listener *>(m_vecListeners.getNthItem(lid));
		if (pListener)
		{
			fl_ContainerLayout* sfh = 0;
			if (pfs && (pListener->getType() < PTL_CollabExport))
				sfh = pfs->getFmtHandle(lid);

			// some pt elements have no corresponding layout elements (for example a
			// hdr/ftr section that was deleted in revisions mode)
			if(sfh && (pListener->getType() < PTL_CollabExport ))
				pListener->change(sfh,pcr);
			else if(pListener->getType() >= PTL_CollabExport)
				pListener->change(NULL,pcr);	
		}
	}

	return true;
}

void PD_Document::deferNotifications(void)
{
	// notify listeners to defer notifications.

#ifdef PT_TEST
	//pcr->__dump();
#endif

	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listeners (for listeners which have been
	// removed (views that went away)).

	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = static_cast<PL_Listener *>(m_vecListeners.getNthItem(lid));
		if (pListener)
		{
			pListener->deferNotifications();
		}
	}
}

UT_sint32 PD_Document::getAdjustmentForCR(const PX_ChangeRecord * pcr) const
{
	UT_sint32 iAdj = 0;
	switch(pcr->getType())
	{
		case PX_ChangeRecord::PXT_GlobMarker:
			break;
		case PX_ChangeRecord::PXT_InsertSpan:
			{
				const PX_ChangeRecord_SpanChange * pcrc = static_cast<const PX_ChangeRecord_SpanChange *> (pcr);
				UT_uint32 iLen = pcrc->getLength();
				iAdj = iLen;
			}
			break;
		case PX_ChangeRecord::PXT_DeleteSpan:
			{
				const PX_ChangeRecord_SpanChange * pcrc = static_cast<const PX_ChangeRecord_SpanChange *> (pcr);
				UT_uint32 iLen = pcrc->getLength();
				iAdj = -iLen;
			}
			break;
		case PX_ChangeRecord::PXT_ChangeSpan:
			break;
		case PX_ChangeRecord::PXT_InsertStrux:
			iAdj = 1;
			break;
		case PX_ChangeRecord::PXT_DeleteStrux:
			iAdj = -1;
			break;
		case PX_ChangeRecord::PXT_ChangeStrux:
			break;
		case PX_ChangeRecord::PXT_InsertObject:
			iAdj =  1;
			break;
		case PX_ChangeRecord::PXT_DeleteObject:
			iAdj = -1;
			break;
		case PX_ChangeRecord::PXT_ChangeObject:
			break;
		case PX_ChangeRecord::PXT_InsertFmtMark:
			break;
		case PX_ChangeRecord::PXT_DeleteFmtMark:
			break; 
		case PX_ChangeRecord::PXT_ChangeFmtMark:
			break;
		case PX_ChangeRecord::PXT_ChangePoint:
			break; 
		case PX_ChangeRecord::PXT_ListUpdate:
			break; 
		case PX_ChangeRecord::PXT_StopList:
			break; 
		case PX_ChangeRecord::PXT_UpdateField:
			break;
		case PX_ChangeRecord::PXT_RemoveList:
			break;
		case PX_ChangeRecord::PXT_UpdateLayout:
			break;
		case PX_ChangeRecord::PXT_AddStyle:
			break;
		case PX_ChangeRecord::PXT_RemoveStyle:
			break;
		case PX_ChangeRecord::PXT_CreateDataItem:
			break;
		case PX_ChangeRecord::PXT_ChangeDocProp:
			break;
		default:
			break;
	}
	return iAdj;
}

void PD_Document::processDeferredNotifications(void)
{
	// notify listeners to process any deferred notifications.

#ifdef PT_TEST
	//pcr->__dump();
#endif

	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listeners (for listeners which have been
	// removed (views that went away)).

	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = m_vecListeners.getNthItem(lid);
		if (pListener)
		{
			pListener->processDeferredNotifications();
		}
	}
}




fl_ContainerLayout* PD_Document::getNthFmtHandle(pf_Frag_Strux* sdh, UT_uint32 n)
{
	const pf_Frag_Strux * pfs = static_cast<const pf_Frag_Strux *>(sdh);
	UT_uint32 nListen = m_vecListeners.getItemCount();
	if(n >= nListen)
		return NULL;
	PL_ListenerId lid = static_cast<PL_ListenerId>(n);
	fl_ContainerLayout* sfh = pfs->getFmtHandle(lid);
	return sfh;
}

static void s_BindHandles(pf_Frag_Strux* sdhNew,
						  PL_ListenerId lid,
						  fl_ContainerLayout* sfhNew)
{
	UT_return_if_fail (sdhNew);
	UT_return_if_fail (sfhNew);

	pf_Frag_Strux * pfsNew = sdhNew;
	UT_DEBUGMSG(("Set Format handle number %d of strux %p to format %p \n",lid,pfsNew,sfhNew));
	pfsNew->setFmtHandle(lid,sfhNew);
}

bool PD_Document::notifyListeners(const pf_Frag_Strux * pfs,
									 pf_Frag_Strux * pfsNew,
									 const PX_ChangeRecord * pcr) const
{
	// notify listeners of a new strux.  this is slightly
	// different from the other one because we need to exchange
	// handles with the listener for the new strux.

#ifdef PT_TEST
	//pcr->__dump();
#endif
	m_iUpdateCount = 0;
	if(pcr->getDocument() == NULL)
	{
	        pcr->setDocument(this);
			pcr->setCRNumber();
	}
	else if(pcr->getCRNumber() == 0)
	{
			pcr->setCRNumber();
	}

	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();

	// for each listener in our vector, we send a notification.
	// we step over null listeners (for listeners which have been
	// removed (views that went away)).

	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = m_vecListeners.getNthItem(lid);
		if (pListener)
		{
			pf_Frag_Strux* sdhNew = static_cast<pf_Frag_Strux*>(pfsNew);
			fl_ContainerLayout* sfh = NULL;
			if(pListener->getType() < PTL_CollabExport)
				sfh = pfs->getFmtHandle(lid);
			if (pListener->insertStrux(sfh,pcr,sdhNew,lid,s_BindHandles))
			{
				// verify that the listener used our callback
				if(pListener->getType() < PTL_CollabExport)
				{
					UT_ASSERT_HARMLESS(pfsNew->getFmtHandle(lid));
				}
			}
		}
	}

	return true;
}

/*!
 * Return true if the document has an abicollab connection
 */
bool PD_Document::isConnected(void)
{
	PL_ListenerId lid;
	PL_ListenerId lidCount = m_vecListeners.getItemCount();
	for (lid=0; lid<lidCount; lid++)
	{
		PL_Listener * pListener = m_vecListeners.getNthItem(lid);
		if (pListener)
		{
			if(pListener->getType() >= PTL_CollabExport)
			{
				return true;
			}
		}
	}
	return false;

}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/*!
    If the input pAP contains revision attribute, this function
    returns AP in which the revision attribute has been inflated into
    actual properties and attributes (the return AP lives in the PT varset,
    so it is not to be modified, deleted, etc., but the caller)
    
    bShow indicates whether revisions are shown or hidden (view - dependent)
    iId is the id of revision to be shown (view - dependent)

    on return bHiddenRevision indicates if the element associated with
    pAP is to be hidden or visible
*/
const PP_AttrProp * PD_Document::explodeRevisions(PP_RevisionAttr *& pRevisions, const PP_AttrProp * pAP,
												  bool bShow, UT_uint32 iId, bool &bHiddenRevision) const
{
	PP_AttrProp * pNewAP = NULL;
	const gchar* pRevision = NULL;
	bHiddenRevision = false;
	
	bool bMark = isMarkRevisions();
	
	if(pAP && pAP->getAttribute("revision", pRevision))
	{
		if(!pRevisions)
			pRevisions = new PP_RevisionAttr(pRevision);

		UT_return_val_if_fail(pRevisions, NULL);
		
		//first we need to ascertain if this revision is visible
		bool bDeleted = false;

		const PP_Revision * pRev;
		UT_uint32 i = 0;
		UT_uint32 iMinId;

		pRev = pRevisions->getLastRevision();
		UT_return_val_if_fail(pRev, NULL);
		
		UT_uint32 iMaxId = pRev->getId();

		if(!bMark && !bShow && iId == 0)
		{
			// revisions are not to be shown, and the document to be
			// shown in the state before the first revision, i.e.,
			// additions are to be hidden, fmt changes ignored, and
			// deletions will be visible

			// see if the first revision is an addition ...
			i = 1;
			do
			{
				pRev = pRevisions->getRevisionWithId(i, iMinId);

				if(!pRev)
				{
					UT_DEBUGMSG(("PD_Document::inflateRevisions: iMinId %d\n", iMinId));
					
					if(iMinId == PD_MAX_REVISION)
					{
						UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
						return NULL;
					}

					// jump directly to the first revision ...
					i = iMinId;
				}
			}
			while(!pRev && i <= iMaxId);
			
				
			if(  (pRev->getType() == PP_REVISION_ADDITION)
			   ||(pRev->getType() == PP_REVISION_ADDITION_AND_FMT))
			{
				bHiddenRevision = true;
				return NULL;
			}

			bHiddenRevision = false;
			return NULL;
		}
		
		if((bMark || !bShow) && iId != 0)
		{
			// revisions not to be shown, but document to be presented
			// as it looks after the revision iId
			// UT_ASSERT( bMark || iId == PD_MAX_REVISION );
			
			UT_uint32 iMyMaxId = bMark ? UT_MIN(iId,iMaxId) : iMaxId;

			// we need to loop through subsequent revisions,
			// working out the their cumulative effect
			for(i = 1; i <= iMyMaxId; i++)
			{
				pRev = pRevisions->getRevisionWithId(i,iMinId);

				if(!pRev)
				{
					if(iMinId == PD_MAX_REVISION)
					{
						UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
						break;
					}

					// advance i so that we do not waste our time, -1
					// because of i++ in loop
					i = iMinId - 1;
					continue;
				}
			
			
				if(  (pRev->getType() == PP_REVISION_FMT_CHANGE && !bDeleted)
					 ||(pRev->getType() == PP_REVISION_ADDITION_AND_FMT))
				{
					// create copy of span AP and then set all props contained
					// in our revision;
					if(!pNewAP)
					{
						pNewAP = new PP_AttrProp;
						UT_return_val_if_fail(pNewAP,NULL);
				
						(*pNewAP) = *pAP;
						(*pNewAP) = *pRev;
					}
					else
					{
						// add fmt to our AP
						pNewAP->setAttributes(pRev->getAttributes());
						pNewAP->setProperties(pRev->getProperties());
					}
				}
				else if(pRev->getType() == PP_REVISION_DELETION)
				{
					// deletion means resetting all previous fmt
					// changes
					if(pNewAP)
					{
						delete pNewAP;
						pNewAP = NULL;
					}

					bDeleted = true;
				}
				else if(pRev->getType() == PP_REVISION_ADDITION)
				{
					bDeleted = false;
				}
			} // for

			if(bDeleted)
			{
				bHiddenRevision = true;
			}
			else
			{
				bHiddenRevision = false;
			}

			if(!bMark || iId == PD_MAX_REVISION)
			{
				if(pNewAP)
				{
					// explode style, prune and store the AP
					pNewAP->explodeStyle(this);
					pNewAP->prune();
					pNewAP->markReadOnly();
					
					PT_AttrPropIndex api;
					UT_return_val_if_fail(getPieceTable()->getVarSet().addIfUniqueAP(pNewAP, &api), NULL);
					pAP->setRevisedIndex(api,iId,bShow,bMark,bHiddenRevision);

					// the above might have resulted in the deletion
					// of pNewAP -- retrieve it by the index
					getAttrProp(api, const_cast<const PP_AttrProp **>(&pNewAP));
				}
				
				return pNewAP;
			}
			
			// if we are in Mark mode, we need to process the last
			// revision ... 
		}
		else if(!pRevisions->isVisible(iId))
		{
			// we are to show revisions with id <= iId
			bHiddenRevision = true;
			UT_ASSERT_HARMLESS(!pNewAP);
			return NULL;
		}

		//next step is to find any fmt changes, layering them as
		//subsequent revisions come
		if(bMark && iId != 0)
		{
			// we are in Mark mode and only interested in the last
			// revision; the loop below will run only once
			i = UT_MIN(iId+1,iMaxId);
		}
		else
		{
			i = 1;
		}
		

		for(i = 1; i <= iMaxId; i++)
		{
			pRev = pRevisions->getRevisionWithId(i,iMinId);

			if(!pRev)
			{
				if(iMinId == PD_MAX_REVISION)
				{
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
					break;
				}

				// advance i so that we do not waste our time, -1
				// because of i++ in loop
				i = iMinId - 1;
				continue;
			}
			
			
			if(  (pRev->getType() == PP_REVISION_FMT_CHANGE && !bDeleted)
				 ||(pRev->getType() == PP_REVISION_ADDITION_AND_FMT))
			{
				// create copy of span AP and then set all props contained
				// in our revision;
				if(!pNewAP)
				{
					pNewAP = new PP_AttrProp;
					UT_return_val_if_fail(pNewAP, NULL);
				
					(*pNewAP) = *pAP;
					(*pNewAP) = *pRev;
					bDeleted = false;
				}
				else
				{
					// add fmt to our AP
					pNewAP->setAttributes(pRev->getAttributes());
					pNewAP->setProperties(pRev->getProperties());
					bDeleted = false;
				}
			}
		} // for
	} // if "revision"

	if(pNewAP)
	{
		// explode style, prune and store the AP
		pNewAP->explodeStyle(this);
		pNewAP->prune();
		pNewAP->markReadOnly();
					
		PT_AttrPropIndex api;
		UT_return_val_if_fail(getPieceTable()->getVarSet().addIfUniqueAP(pNewAP, &api), NULL);
		pAP->setRevisedIndex(api,iId,bShow,bMark,bHiddenRevision);

		// the above might have resulted in the deletion
		// of pNewAP -- retrieve it by the index
		getAttrProp(api, const_cast<const PP_AttrProp**>(&pNewAP));
	}
				
	return pNewAP;
}

bool PD_Document::getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const
{
	return m_pPieceTable->getAttrProp(indexAP,ppAP);
}

const UT_UCSChar * PD_Document::getPointer(PT_BufIndex bi) const
{
	// the pointer that we return is NOT a zero-terminated
	// string.  the caller is responsible for knowing how
	// long the data is within the span/fragment.

	return m_pPieceTable->getPointer(bi);
}

bool PD_Document::getBlockBuf(pf_Frag_Strux* sdh, UT_GrowBuf * pgb) const
{
	return m_pPieceTable->getBlockBuf(sdh,pgb);
}

bool PD_Document::getBounds(bool bEnd, PT_DocPosition & docPos) const
{
	return m_pPieceTable->getBounds(bEnd,docPos);
}

PT_DocPosition PD_Document::getStruxPosition(pf_Frag_Strux* sdh) const
{
	return m_pPieceTable->getStruxPosition(sdh);
}

bool PD_Document::getSpanAttrProp(pf_Frag_Strux* sdh, UT_uint32 offset, bool bLeftSide,
									 const PP_AttrProp ** ppAP) const
{
	return m_pPieceTable->getSpanAttrProp(sdh,offset,bLeftSide,ppAP);
}

/*!
 * Return strux type of the StruxDocHandle
 */
PTStruxType PD_Document::getStruxType(pf_Frag_Strux* sdh) const
{
	UT_return_val_if_fail( sdh,(PTStruxType)0 );
	const pf_Frag * pf = static_cast<const pf_Frag *>(sdh);
	UT_return_val_if_fail (pf->getType() == pf_Frag::PFT_Strux,(PTStruxType)0);
	const pf_Frag_Strux * pfs = static_cast<const pf_Frag_Strux *> (pf);
	return pfs->getStruxType();
}

/*!
 * Returns true if the document as any math or SVG runs within it.
 */
bool PD_Document::hasMath(void)
{
	pf_Frag *  pf = getPieceTable()->getFragments().getFirst();
	while(pf)
	{
		if(pf->getType() == pf_Frag::PFT_Object)
		{
			pf_Frag_Object * po = (pf_Frag_Object*) pf;
			if(po->getObjectType() == PTO_Math)
			{
				return true;
			}
		}
		pf = pf->getNext();
	}
	return false;
}

pf_Frag * PD_Document::findBookmark(const char * pName, bool bEnd, pf_Frag * pfStart)
{
	if(!pfStart)
	{
		pfStart = getPieceTable()->getFragments().getFirst();
	}

	UT_return_val_if_fail(pfStart, NULL);

	pf_Frag * pf = pfStart;
	while(pf)
	{
		if(pf->getType() == pf_Frag::PFT_Object)
		{
			pf_Frag_Object * po = (pf_Frag_Object*) pf;
			if(po->getObjectType() == PTO_Bookmark)
			{
				po_Bookmark * pb = po->getBookmark();
				if(!pb)
				{
					UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
					pf = pf->getNext();
					continue;
				}

				if(  (!bEnd && pb->getBookmarkType() == po_Bookmark::POBOOKMARK_START)
				   ||( bEnd && pb->getBookmarkType() == po_Bookmark::POBOOKMARK_END))
				{
					if(0 == strcmp(pName, pb->getName()))
					   return pf;
				}
			}
		}

		pf = pf->getNext();
	}

	return NULL;
}


po_Bookmark * PD_Document::getBookmark(pf_Frag_Strux* sdh, UT_uint32 offset)
{
	const pf_Frag * pf = static_cast<const pf_Frag *>(sdh);
	UT_return_val_if_fail (pf->getType() == pf_Frag::PFT_Strux, NULL);
	const pf_Frag_Strux * pfsBlock = static_cast<const pf_Frag_Strux *> (pf);
	UT_return_val_if_fail (pfsBlock->getStruxType() == PTX_Block, NULL);

	UT_uint32 cumOffset = 0;
	pf_Frag_Object * pfo = NULL;
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		cumOffset += pfTemp->getLength();
		if (offset < cumOffset)
		{
			switch (pfTemp->getType())
			{
				case pf_Frag::PFT_Object:
					pfo = static_cast<pf_Frag_Object *> (pfTemp);
					return pfo->getBookmark();
				default:
					return NULL;
			}
		}

	}
	return NULL;
}

bool PD_Document::getField(pf_Frag_Strux* sdh, UT_uint32 offset,
                               fd_Field * & pField)
{

	const pf_Frag * pf = static_cast<const pf_Frag *>(sdh);
	UT_return_val_if_fail (pf->getType() == pf_Frag::PFT_Strux, false);
	const pf_Frag_Strux * pfsBlock = static_cast<const pf_Frag_Strux *> (pf);
	UT_return_val_if_fail (pfsBlock->getStruxType() == PTX_Block, false);

	UT_uint32 cumOffset = 0;
	pf_Frag_Text * pft = NULL;
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		cumOffset += pfTemp->getLength();
		if (offset < cumOffset)
		{
			switch (pfTemp->getType())
			{
			case pf_Frag::PFT_Text:
			case pf_Frag::PFT_Object:
				pft = static_cast<pf_Frag_Text *> (pfTemp);
				pField = pft->getField();
				return true; // break out of loop
				break;
			default:
				return false;
				break;
			}
		}

	}
	return false;
}

bool PD_Document::getStruxFromPosition(PL_ListenerId listenerId,
										  PT_DocPosition docPos,
										  fl_ContainerLayout* * psfh) const
{
	return m_pPieceTable->getStruxFromPosition(listenerId,docPos,psfh);
}

bool PD_Document::getStruxOfTypeFromPosition(PL_ListenerId listenerId,
												PT_DocPosition docPos,
												PTStruxType pts,
												fl_ContainerLayout* * psfh) const
{
	return m_pPieceTable->getStruxOfTypeFromPosition(listenerId,docPos,pts,psfh);
}

pf_Frag_Strux* PD_Document::getBlockFromPosition( PT_DocPosition pos ) const
{
    return m_pPieceTable->getBlockFromPosition( pos );
}


///
///  return the SDH of the last strux of the given type
/// immediately prior to the given absolute document position.
/// This sdh is actually a (void *) pointer to a pf_Frag_Strux
///
bool PD_Document::getStruxOfTypeFromPosition(PT_DocPosition docPos,
												PTStruxType pts,
												pf_Frag_Strux* * sdh) const
{
	return m_pPieceTable->getStruxOfTypeFromPosition(docPos,pts, sdh);
}

///
/// Return the sdh of type pts immediately prior to sdh
///
bool PD_Document::getPrevStruxOfType(pf_Frag_Strux* sdh,PTStruxType pts,
					pf_Frag_Strux* * prevsdh)
{
	pf_Frag* pfs = sdh;
	UT_return_val_if_fail (pfs, false);
	pfs = pfs->getPrev();
	for (pf_Frag * pf=pfs; (pf); pf=pf->getPrev())
		if (pf->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfsTemp = static_cast<pf_Frag_Strux *>(pf);
			if (pfsTemp->getStruxType() == pts)	// did we find it
			{
				*prevsdh = pfsTemp;
				return true;
			}
		}

	// did not find it.

	return false;
}


///
///get the next strux after the strux given. Skip embedded strux's
///
bool PD_Document::getNextStrux(pf_Frag_Strux* sdh,
							   pf_Frag_Strux* * nextsdh)
{
	pf_Frag * pfs = sdh;
	UT_return_val_if_fail (pfs, false);
	pfs = pfs->getNext();
	UT_sint32 iEmbedDepth = 0;
	for (pf_Frag * pf=pfs; (pf); pf=pf->getNext())
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfsTemp = static_cast<pf_Frag_Strux *>(pf);
			if(iEmbedDepth <= 0 && !m_pPieceTable->isFootnote(pf) &&
									 !m_pPieceTable->isEndFootnote(pf))
			{
				*nextsdh = pfsTemp;
				return true;
			}
			else if(m_pPieceTable->isFootnote(pf))
			{
				iEmbedDepth++;
			}
			else if(m_pPieceTable->isEndFootnote(pf))
			{
				iEmbedDepth--;
			}
		}
	}
	// did not find it.

	return false;
}

pf_Frag * PD_Document::getFragFromPosition(PT_DocPosition docPos) const
{
	pf_Frag * pf = 0;
	m_pPieceTable->getFragFromPosition(docPos,&pf,0);
	return pf;
}

///
/// Return the sdh of type pts immediately after sdh
///
bool PD_Document::getNextStruxOfType(pf_Frag_Strux* sdh,PTStruxType pts,
					pf_Frag_Strux* * nextsdh)
{
	pf_Frag * pfs = sdh;
	UT_return_val_if_fail (pfs, false);
	pfs = pfs->getNext();
	UT_sint32 iNest = 0;
	for (pf_Frag * pf=pfs; (pf); pf=pf->getNext())
		if (pf->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfsTemp = static_cast<pf_Frag_Strux *>(pf);
			if((pfsTemp->getStruxType() == PTX_SectionTable) && (pts != PTX_SectionTable))
			{
				iNest++;
				continue;
			}
			if((iNest > 0) && (pfsTemp->getStruxType() == PTX_EndTable))
			{
				iNest--;
				continue;
			}
			if(iNest > 0)
			{
				continue;
			}
			if (pfsTemp->getStruxType() == pts)	// did we find it
			{
				*nextsdh = pfsTemp;
				return true;
			}
		}

	// did not find it.

	return false;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void PD_Document::beginUserAtomicGlob(void)
{
	m_pPieceTable->beginUserAtomicGlob();
}

void PD_Document::endUserAtomicGlob(void)
{
	m_pPieceTable->endUserAtomicGlob();
}

UT_uint32 PD_Document::undoCount(bool bUndo) const
{
  return m_pPieceTable->undoCount(bUndo);
}

bool PD_Document::canDo(bool bUndo) const
{
	return m_pPieceTable->canDo(bUndo);
}

bool PD_Document::undoCmd(UT_uint32 repeatCount)
{
	UT_sint32 sRepeatCount = static_cast<UT_uint32>(repeatCount);
	while (sRepeatCount > 0)
	{
		UT_uint32 inCount = undoCount(true);
		if (!m_pPieceTable->undoCmd())
			return false;
		sRepeatCount -= inCount - undoCount(true);
	}
	return true;
}

bool PD_Document::redoCmd(UT_uint32 repeatCount)
{
	while (repeatCount--)
		if (!m_pPieceTable->redoCmd())
			return false;
	return true;
}


bool  PD_Document::isDoingTheDo(void) const
{
	return m_pPieceTable->isDoingTheDo();
}

///////////////////////////////////////////////////////////////////
// DataItems represent opaque (and probably binary) data found in
// the data-section of the document.  These are used, for example,
// to store the actual data of an image.  The inline image tag has
// a reference to a DataItem.
// mime_type is the associated mime type for the blob.
bool PD_Document::createDataItem(const char * szName, bool bBase64, 
                                 const UT_ByteBuf * pByteBuf,
                                 const std::string & mime_type,
                                 PD_DataItemHandle* ppHandle)
{
	PD_DataItemHandle pPair = NULL;

	UT_return_val_if_fail (pByteBuf, false);

	// verify unique name
	UT_DEBUGMSG(("Create data item name %s \n",szName));
	if (getDataItemDataByName(szName,NULL,NULL,NULL) == true)
    {
        UT_DEBUGMSG(("Data item %s already exists! \n",szName));
		return false;
    }
	// set the actual DataItem's data using the contents of the ByteBuf.
	// we must copy it if we want to keep it.  bBase64 is TRUE if the
	// data is Base64 encoded.

	std::unique_ptr<UT_ByteBuf> pNew(new UT_ByteBuf());

	if (bBase64)
	{
		if (!UT_Base64Decode(pNew.get(), pByteBuf)) {
			return false;
		}
	}
	else
	{
		if (!pNew->ins(0,pByteBuf->getPointer(0),pByteBuf->getLength())) {
			return false;
		}
	}

	pPair = new _dataItemPair();
	if (!pPair) {
		return false;
	}

	pPair->pBuf = pNew.release();
	pPair->pToken = g_strdup(mime_type.c_str());
	m_hashDataItems.insert(std::make_pair(szName, pPair));

	// give them back a handle if they want one

	if (ppHandle)
	{
		hash_data_items_t::iterator iter = m_hashDataItems.find(szName);
		UT_return_val_if_fail (iter != m_hashDataItems.end(), false);
		*ppHandle = iter->second;
	}
	{
		const PP_PropertyVector attributes = {
			PT_DATAITEM_ATTRIBUTE_NAME, szName
		};
		PT_AttrPropIndex iAP= 0;
		m_pPieceTable->getVarSet().storeAP(attributes, &iAP);
		PX_ChangeRecord * pcr =  new PX_ChangeRecord(PX_ChangeRecord::PXT_CreateDataItem,0,iAP,getXID());
		UT_DEBUGMSG(("indexAP %d \n",iAP)); 
		notifyListeners(NULL, pcr);
		delete pcr;
	}
	return true;
}

/*!
 * Replace the contents of the pre-existing data item with this new
 * data item (pByteBuf). Used when updating a preview of an embedded object.
 */
bool PD_Document::replaceDataItem(const char * szName, const UT_ByteBuf * pByteBuf)
{
	// verify data item exists

	hash_data_items_t::iterator iter = m_hashDataItems.find(szName);
	if (iter == m_hashDataItems.end()) {
		return false;
	}

	_dataItemPair* pPair = iter->second;
	UT_return_val_if_fail (pPair, false);

	UT_return_val_if_fail (pByteBuf, false);

	UT_ByteBuf * pOldBuf =  pPair->pBuf;
	pOldBuf->truncate(0);
	if (!pOldBuf->ins(0,pByteBuf->getPointer(0),pByteBuf->getLength()))
		return false;

	return true;
}

bool PD_Document::getDataItemDataByName(const char * szName,
										   const UT_ByteBuf ** ppByteBuf,
                                        std::string * pMimeType,
										   PD_DataItemHandle* ppHandle) const
{
	UT_DEBUGMSG(("Look for %s \n",szName));
	UT_return_val_if_fail (szName && *szName, false);

	
	hash_data_items_t::const_iterator iter = m_hashDataItems.find(szName);
	if (iter == m_hashDataItems.end()) {
		return false;
	}

	_dataItemPair* pPair = iter->second;
	UT_DEBUGMSG(("Found data item name %s buf %p \n",szName,pPair->pBuf));

	if (ppByteBuf)
	{
		*ppByteBuf = pPair->pBuf;
	}

	if (pMimeType)
	{
		*pMimeType = (const char *)pPair->pToken;
	}

	if (ppHandle)
	{
		*ppHandle = pPair;
	}

	return true;
}

/*! This function accepts a data ID and assigns the file extension of the corresponding data item.
	 \param szDataID The incoming data ID to look up
	 \param sExt The extension string that is populated on success
	 \param bDot A boolean to determine whether the extension string will be prefixed with a dot ('.').
	 Defaults to true.
	 \return Returns true only if the data item is found _and_ an extension is assigned.
*/

bool PD_Document::getDataItemFileExtension(const char *szDataID, std::string &sExt, bool bDot) const
{
	UT_return_val_if_fail(szDataID && *szDataID, false);

    std::string mimeType;
 
	if(getDataItemDataByName(szDataID, NULL, &mimeType, NULL))
	{
		if(mimeType.empty())
			return false;

		if(mimeType == "image/png")
		{
			sExt = (bDot ? "." : "");
			sExt += "png";
			return true;
		}
		if(mimeType == "image/jpeg")
		{
			sExt = (bDot ? "." : "");
			sExt += "jpg";
			return true;
		}
		else if(mimeType ==  "image/svg+xml")
		{
			sExt = (bDot ? "." : "");
			sExt += "svg";
			return true;	
		}
		else
		{
			UT_DEBUGMSG(("getDataItemFileExtension(): unhandled/ignored mime type: %s\n", mimeType.c_str()));
		}
	}

	return false;
}


bool PD_Document::setDataItemToken(PD_DataItemHandle pHandle,
									  void* pToken) const
{
	UT_return_val_if_fail (pHandle, false);

	_dataItemPair* pPair = pHandle;
	UT_return_val_if_fail (pPair, false);

	pPair->pToken = pToken;

	return true;
}

bool PD_Document::getDataItemData(PD_DataItemHandle pHandle,
									 const char ** pszName,
									 const UT_ByteBuf ** ppByteBuf,
									 const void** ppToken) const
{
	UT_return_val_if_fail (pHandle,false);

	_dataItemPair* pPair = pHandle;

	if (ppByteBuf)
	{
		*ppByteBuf = pPair->pBuf;
	}

	if (ppToken)
	{
		*ppToken = pPair->pToken;
	}

	if (pszName)
	{
		UT_ASSERT_HARMLESS(UT_TODO);
		*pszName = 0;
		//*pszName = pHashEntry->pszLeft;
	}

	return true;
}

bool PD_Document::enumDataItems(UT_uint32 k,
                                PD_DataItemHandle* ppHandle, const char ** pszName, 
								const UT_ByteBuf ** ppByteBuf, std::string * pMimeType) const
{
	// return the kth data item.

	UT_uint32 kLimit = m_hashDataItems.size();
	if (k >= kLimit)
		return false;

	UT_uint32 i = 0;
	hash_data_items_t::const_iterator iter;
	for (iter = m_hashDataItems.begin();
		 iter != m_hashDataItems.end() && i < k; ++i, ++iter) {

		// noop
	}

	if (ppHandle && iter != m_hashDataItems.end()) {
		*ppHandle = iter->second;
	}

	const struct _dataItemPair* pPair = iter->second;
	UT_return_val_if_fail (pPair, false);

	if (ppByteBuf)
	{
		*ppByteBuf = pPair->pBuf;
	}

	if (pMimeType)
	{
		*pMimeType = (const char *)pPair->pToken;
	}

	if (pszName)
	{
		*pszName = iter->first.c_str();
	}

	return true;
}

void PD_Document::_destroyDataItemData(void)
{
	if (m_hashDataItems.empty())
		return;

	hash_data_items_t::iterator iter;

	for (iter = m_hashDataItems.begin(); iter != m_hashDataItems.end(); ++iter) {
		xxx_UT_DEBUGMSG(("DOM: destroying data item\n"));
		_dataItemPair* pPair = iter->second;
		UT_return_if_fail (pPair);
		delete pPair->pBuf;
		FREEP(pPair->pToken);
		delete pPair;
	}
	m_hashDataItems.clear();
}

/*!
  Synchronize the last opened/last saves filetypes.
 \param bReadLastSavedAsType True to write last opened and read last
           saved type; otherwise, write last saved type from last opened type.

 There are actually two filetypes - one for importers and one for
 exporters.  This function tries to synchronize the one to the other.
*/
bool PD_Document::_syncFileTypes(bool bReadSaveWriteOpen)
{
	const char *szSuffixes;

	// used to operate on description. now operates on suffixes

	if (bReadSaveWriteOpen)
	  szSuffixes = IE_Exp::suffixesForFileType(m_lastSavedAsType);
	else
	  szSuffixes = IE_Imp::suffixesForFileType(m_lastOpenedType);

	if (!szSuffixes)
	  return false;

	IEFileType ieft;
	if (bReadSaveWriteOpen)
	{
		ieft = IE_Imp::fileTypeForSuffixes(szSuffixes);
		m_lastOpenedType = ieft;
	}
	else
	{
		ieft = IE_Exp::fileTypeForSuffixes(szSuffixes);
		m_lastSavedAsType = ieft;
	}

	if (ieft == IEFT_Unknown || ieft == IEFT_Bogus)
	{
		//UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////
// Styles represent named collections of formatting properties.

const char * PD_Document::getDefaultStyle() const
{
    return "Normal";
}

bool PD_Document::getStyle(const char * szName, PD_Style ** ppStyle) const
{
	return m_pPieceTable->getStyle(szName, ppStyle);
}

bool PD_Document::enumStyles(UT_uint32 k,
								const char ** pszName, const PD_Style ** ppStyle) const
{
	return m_pPieceTable->enumStyles(k, pszName, ppStyle);
}

bool PD_Document::enumStyles(UT_GenericVector<PD_Style*> * & pStyles) const
{
	return m_pPieceTable->enumStyles(pStyles);
}

bool PD_Document::getStyleProperty(const char * szStyleName, const char * szPropertyName, const char *& szPropertyValue)
{
	PD_Style * pS;
	PD_Style ** ppS = &pS;
	if(!m_pPieceTable->getStyle(szStyleName, ppS))
		return false;

	return (*ppS)->getProperty(szPropertyName, szPropertyValue);
}

bool	PD_Document::addStyleProperty(const char * szStyleName, const char * szPropertyName, const char * szPropertyValue)
{
	PD_Style * pS;
	PD_Style ** ppS = &pS;
	if(!m_pPieceTable->getStyle(szStyleName, ppS))
		return false;

	return (*ppS)->addProperty(szPropertyName, szPropertyValue);
}

bool	PD_Document::addStyleProperties(const gchar * szStyleName, const PP_PropertyVector & pProperties)
{
	PD_Style * pS;
	PD_Style ** ppS = &pS;
	if(!m_pPieceTable->getStyle(szStyleName, ppS))
		return false;
	if(!(*ppS)->addProperties(pProperties))
		return false;
	return updateDocForStyleChange(szStyleName,!(*ppS)->isCharStyle());
}

/*!
 * This methods changes the attributes of a style (basedon,followedby)
 *
\param szStyleName the const gchar * name of the style
\param pAttribs The list of attributes of the updated style.
*/
bool	PD_Document::addStyleAttributes(const gchar * szStyleName, const PP_PropertyVector & pAttribs)
{
	PD_Style * pS;
	PD_Style ** ppS = &pS;
	if(!m_pPieceTable->getStyle(szStyleName, ppS))
		return false;
	if(!(*ppS)->addAttributes(pAttribs))
		return false;
//
// These functions just set the new member variable pointers in the class
//
	(*ppS)->getBasedOn();
	(*ppS)->getFollowedBy();
	return updateDocForStyleChange(szStyleName,!(*ppS)->isCharStyle());
}

/*!
 * The method returns the style defined in a sdh. If there is no style it returns
 * NULL
 */
PD_Style * PD_Document::getStyleFromSDH( pf_Frag_Strux* sdh)
{
	const pf_Frag_Strux * pfs = sdh;
	PT_AttrPropIndex indexAP = pfs->getIndexAP();
	const PP_AttrProp * pAP = NULL;
	m_pPieceTable->getAttrProp(indexAP,&pAP);
	UT_return_val_if_fail (pAP, NULL);
	const gchar * pszStyleName = NULL;
	(pAP)->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyleName);
	if(pszStyleName == NULL  || strcmp(pszStyleName,"Current Settings") == 0 || strcmp(pszStyleName,"None") == 0)
	{
		return NULL;
	}
	PD_Style * pStyle = NULL;
	if(!m_pPieceTable->getStyle(pszStyleName, &pStyle))
	{
		return NULL;
	}
	return pStyle;
}

/*!
 * Find previous style of type numbered heading or basedon numbered heading
\param sdh The StruxDocHandle of the fragment where we start to look from.
\returns PD_Style of the first Numbered Heading, otherwise NULL
*/
pf_Frag_Strux* PD_Document::getPrevNumberedHeadingStyle(pf_Frag_Strux* sdh)
{
	pf_Frag * pf = sdh;
	bool bFound = false;
	pf = pf->getPrev();
	PD_Style * pStyle = NULL;
	pf_Frag_Strux* foundSDH = NULL;
	PD_Style * pBasedOn = NULL;
	const char * szStyleName = NULL;
	while(pf && !bFound)
	{
		if(pf->getType() == pf_Frag::PFT_Strux)
		{
			foundSDH = static_cast<pf_Frag_Strux*>(pf);
			pStyle = getStyleFromSDH(foundSDH);
			if(pStyle != NULL)
			{
				szStyleName = pStyle->getName();
				if(strstr(szStyleName,"Numbered Heading") != 0)
				{
					bFound = true;
					break;
				}
				pBasedOn  = pStyle->getBasedOn();
				UT_uint32 i = 0;
				while(pBasedOn != NULL && i < 10 && !bFound)
				{
					if(strstr(pBasedOn->getName(),"Numbered Heading") != 0)
					{
						bFound = true;
					}
					else
					{
						pBasedOn = pBasedOn->getBasedOn();
					}
				}
				if(bFound)
				{
					break;
				}
			}
		}
//
// Should not need the if. It's in for defensive programming.
//
		if(!bFound)
		{
			pf = pf->getPrev();
		}
	}
	if(!bFound)
	{
		return NULL;
	}
	return foundSDH;
}



//
/*!
 * This methods changes the attributes /properties of a style (basedon,followedby)
 * plus the properties. We have to save the indexAP of the pre-existing style
 * and broadcast it out witht e change records.
 *
\param szStyleName the const gchar * name of the style
\param attribs The list of attributes/properties of the updated style.
*/
bool	PD_Document::setAllStyleAttributes(const gchar * szStyleName, const PP_PropertyVector & attribs)
{
	PD_Style * pS;
	PD_Style ** ppS = &pS;
	if(!m_pPieceTable->getStyle(szStyleName, ppS))
		return false;
//
// Sevior May need this code
//	PT_AttrPropIndex oldindexAp = (*pss)->getIndexAP();
	if(!(*ppS)->setAllAttributes(attribs))
		return false;
//
// These functions just set the new member variable pointers in the class
//
	(*ppS)->getBasedOn();
	(*ppS)->getFollowedBy();
	return updateDocForStyleChange(szStyleName,!(*ppS)->isCharStyle());
}

/*!
 * This method scans the document backwards for a struc with the style name szStyle in it.
\param pStyle a pointer to style to be scanned for.
\param pos the document position to start from.
\return the sdh of the strux found.
*/
pf_Frag_Strux* PD_Document::findPreviousStyleStrux(const gchar * szStyle, PT_DocPosition pos)
{
	pf_Frag_Strux* sdh = NULL;
	getStruxOfTypeFromPosition(pos,PTX_Block, &sdh);
	pf_Frag_Strux * pfs = NULL;
	pf_Frag * currentFrag = sdh;
	bool bFound = false;
    while (currentFrag && currentFrag != m_pPieceTable->getFragments().getFirst() && !bFound)
	{
		if (currentFrag->getType()==pf_Frag::PFT_Strux)
		{
//
// All this code is used to find if this strux has our style in it
//
			pfs = static_cast<pf_Frag_Strux *> (currentFrag);
			PT_AttrPropIndex indexAP = pfs->getIndexAP();
			const PP_AttrProp * pAP = NULL;
			m_pPieceTable->getAttrProp(indexAP,&pAP);
			UT_return_val_if_fail (pAP,0);
			const gchar * pszStyleName = NULL;
			(pAP)->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyleName);
			if(pszStyleName != NULL && strcmp(pszStyleName,szStyle)==0)
			{
				bFound = true;
			}
		}
		if(!bFound)
		{
			currentFrag = currentFrag->getPrev();
		}
	}
	if(bFound)
	{
		sdh = static_cast<pf_Frag_Strux*>(currentFrag);
	}
	else
	{
		sdh = NULL;
	}
	return sdh;
}

/*!
 * This method scans the document forwards for a strux with the style name
 * szStyle in it.
\param pStyle a pointer to style to be scanned for.
\param pos the document position to start from.
\return the sdh of the strux found.
*/
pf_Frag_Strux* PD_Document::findForwardStyleStrux(const gchar * szStyle, PT_DocPosition pos)
{
	pf_Frag_Strux* sdh = NULL;
	getStruxOfTypeFromPosition(pos,PTX_Block, &sdh);
	pf_Frag_Strux * pfs = NULL;
	pf_Frag * currentFrag = sdh;
	bool bFound = false;
    while (currentFrag != m_pPieceTable->getFragments().getLast() && !bFound)
	{
		if (currentFrag->getType()==pf_Frag::PFT_Strux)
		{
//
// All this code is used to find if this strux has our style in it
//
			pfs = static_cast<pf_Frag_Strux *> (currentFrag);
			PT_AttrPropIndex indexAP = pfs->getIndexAP();
			const PP_AttrProp * pAP = NULL;
			m_pPieceTable->getAttrProp(indexAP,&pAP);
			UT_return_val_if_fail (pAP, 0);
			const gchar * pszStyleName = NULL;
			(pAP)->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyleName);
			if(pszStyleName != NULL && strcmp(pszStyleName,szStyle)==0)
			{
				bFound = true;
			}
		}
		if(!bFound)
		{
			currentFrag = currentFrag->getNext();
		}
	}
	if(bFound)
	{
		sdh = static_cast<pf_Frag_Strux*>(currentFrag);
	}
	else
	{
		sdh = NULL;
	}
	return sdh;
}


/*!
 * This method loops through the entire document updating each location
 * where the style exists.
\param szStyle the name of style that has changed.
\param isParaStyle true if the style is a paragraph type.
*/
bool   PD_Document::updateDocForStyleChange(const gchar * szStyle,
											bool isParaStyle)
{
	PT_DocPosition pos = 0;
	PT_DocPosition posLastStrux = 0;
	pf_Frag_Strux * pfs = NULL;
	PD_Style * pStyle = NULL;
	m_pPieceTable->getStyle(szStyle,&pStyle);
	UT_return_val_if_fail (pStyle, false);
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	UT_return_val_if_fail (currentFrag, false);
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
//
// get indexAP
// get PT_STYLE_ATTRIBUTE_NAME
// if it matches style name do a notify listeners call.
		if(isParaStyle)
		{
			if (currentFrag->getType()==pf_Frag::PFT_Strux)
			{
//
// All this code is used to find if this strux has our style in it
//
				pfs = static_cast<pf_Frag_Strux *> (currentFrag);
				PT_AttrPropIndex indexAP = pfs->getIndexAP();
				const PP_AttrProp * pAP = NULL;
				m_pPieceTable->getAttrProp(indexAP,&pAP);
				UT_return_val_if_fail (pAP, false);
				const gchar * pszStyleName = NULL;
				(pAP)->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyleName);
				bool bUpdate = false;
//
// It does so signal all the layouts to update themselves for the new definition
// of the style.
//
				if(pszStyleName != NULL && strcmp(pszStyleName,szStyle)==0)
				{
					bUpdate = true;
				}
				else if(pfs->getStruxType() == 	PTX_SectionTOC)
				{
					bUpdate = true; // FIXME should be more fine grained.
				}
//
// Look if the style in the basedon ancestory is our style
//
				else if (pszStyleName != NULL)
				{
					PD_Style * cStyle = NULL;
					m_pPieceTable->getStyle(pszStyleName,&cStyle);
					UT_ASSERT_HARMLESS(cStyle);
					if(cStyle)
					{
						PD_Style * pBasedOn = cStyle->getBasedOn();
						UT_uint32 i =0;
						for(i=0; (i < pp_BASEDON_DEPTH_LIMIT) && (pBasedOn != NULL) && (pBasedOn!= pStyle); i++)
						{
							pBasedOn = pBasedOn->getBasedOn();
						}
						if(pBasedOn == pStyle)
						{
							bUpdate = true;
						}
					}
				}
				if(bUpdate)
				{
					PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangeStrux,pos,indexAP,
																pfs->getXID());
					notifyListeners(pfs, pcr);
					delete pcr;
				}
			}
		}
//
// Character type
//
		else
		{
//
// Need the most recent frag_strux to find the block containing our text span
//
			if (currentFrag->getType()==pf_Frag::PFT_Strux)
			{
				pfs = static_cast<pf_Frag_Strux *> (currentFrag);
				posLastStrux = pos;
			}
			if (currentFrag->getType()==pf_Frag::PFT_Text)
			{
//
// All this code is used to find if this Text Frag has our style in it
//
				pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (currentFrag);
				PT_AttrPropIndex indexAP = pft->getIndexAP();
				const PP_AttrProp * pAP = NULL;
				m_pPieceTable->getAttrProp(indexAP,&pAP);
				UT_return_val_if_fail (pAP, false);
				const gchar * pszStyleName = NULL;
				(pAP)->getAttribute(PT_STYLE_ATTRIBUTE_NAME, pszStyleName);

//
// It does so signal all the layouts to update themselves for the new definition
// of the style.
//
				if(pszStyleName != NULL && strcmp(pszStyleName,szStyle)==0)
				{
					UT_uint32 blockoffset = (UT_uint32) (pos - posLastStrux -1);
					PX_ChangeRecord_SpanChange * pcr = new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,
																					  pos,indexAP,indexAP,
																					  m_pPieceTable->getVarSet().getBufIndex(pft->getBufIndex(),0) ,
																					  currentFrag->getLength(),
																					  blockoffset, false);
					notifyListeners(pfs, pcr);
					delete pcr;
				}
			}
		}
		pos += currentFrag->getLength();
		currentFrag = currentFrag->getNext();
	}
	return true;
}


/*!
 * This method updates all the layouts associated with the document.
*/
void  PD_Document::updateAllLayoutsInDoc( pf_Frag_Strux* sdh)
{
	const pf_Frag_Strux * pfs = sdh;
	PT_AttrPropIndex indexAP = pfs->getIndexAP();
	PT_DocPosition pos = getStruxPosition(sdh);
	PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangeStrux,
												pos,indexAP,pfs->getXID());
	notifyListeners(pfs, pcr);
	delete pcr;
}

//////////////////////////////////////////////////////////////////

void PD_Document::clearIfAtFmtMark (PT_DocPosition dpos)
{
	m_pPieceTable->clearIfAtFmtMark(dpos);
}

bool PD_Document::updateFields(void)
{
	//
	// Turn off Insertion point motion during this general update
	//
	setDontChangeInsPoint();
	pf_Frag * currentFrag = m_pPieceTable->getFragments().getFirst();
	UT_return_val_if_fail (currentFrag,false);
	while (currentFrag!=m_pPieceTable->getFragments().getLast())
	{
		if (currentFrag->getType()==pf_Frag::PFT_Object)
		{
			pf_Frag_Object * pfo = static_cast<pf_Frag_Object *>
				(currentFrag);
			if (pfo->getObjectType()==PTO_Field)
			{
				UT_return_val_if_fail (pfo->getField(), false);
				pfo->getField()->update();
			}
		}
		currentFrag = currentFrag->getNext();
	}
	//
	// Restore insertion point motion
	//
	allowChangeInsPoint();
	return true;
}

void PD_Document::setDontChangeInsPoint(void)
{
	if(m_bLoading)
	{
		UT_DEBUGMSG(("Illegal request to not change insertion Point!!! \n"));
        m_bAllowInsertPointChange = true;
		return;
	}
	m_bAllowInsertPointChange = false;
}

void PD_Document::allowChangeInsPoint(void)
{
        m_bAllowInsertPointChange = true;
}

bool PD_Document::getAllowChangeInsPoint(void) const
{
        return m_bAllowInsertPointChange;
}

////////////////////////////////////////////////////////////////////////////////
// Step towards full thread safety

void PD_Document::notifyPieceTableChangeStart(void)
{
//
// Wait for all redraws to finish before starting.
//
	UT_uint32 i = 0;
	while(m_bRedrawHappenning && i < 10000)
	{
		UT_usleep(100); // wait 100 microseonds
		i++;
	}
	if(i>0)
	{
		UT_DEBUGMSG(("!!!!Waited %d microseconds for redraw to finish \n",i*100));
	}
	m_bRedrawHappenning = false;
	_setPieceTableChanging(true);
//
// Invalidate visible direction cache variables. PieceTable manipulations of
// any sort can screw these.
//
	m_pVDBl = NULL;
	m_pVDRun = NULL;
	m_iVDLastPos = 0;
}

void PD_Document::notifyPieceTableChangeEnd(void)
{
        _setPieceTableChanging(false);
}

void PD_Document::invalidateCache(void)
{
	m_pVDBl = NULL;
	m_pVDRun = NULL;
	m_iVDLastPos = 0;
}

////////////////////////////////////////////////////////////////
// List Vector Functions



fl_AutoNum * PD_Document::getListByID(UT_uint32 id) const
{
	UT_uint16 i = 0;
	UT_sint32 cnt = 0;
	fl_AutoNum * pAutoNum;

	cnt = m_vecLists.getItemCount();
	if ( cnt <= 0)
		return static_cast<fl_AutoNum *>(NULL);
	UT_return_val_if_fail (m_vecLists.getFirstItem(), NULL);

	while (i<cnt)
	{
		pAutoNum = m_vecLists[i];
		if (pAutoNum->getID() == id)
			return pAutoNum;
		i++;
	}

	return static_cast<fl_AutoNum *>(NULL);
}

bool PD_Document::enumLists(UT_uint32 k, fl_AutoNum ** pAutoNum)
{
	UT_uint32 kLimit = m_vecLists.getItemCount();
	if (k >= kLimit)
		return false;

	if (pAutoNum)
		*pAutoNum = m_vecLists[k];

	return true;
}

fl_AutoNum * PD_Document::getNthList(UT_uint32 i) const
{
	return m_vecLists[i];
}

UT_uint32 PD_Document::getListsCount(void) const
{
	return m_vecLists.getItemCount();
}

void PD_Document::addList(fl_AutoNum * pAutoNum)
{
	UT_uint32 id = pAutoNum->getID();
	UT_uint32 i;
	UT_uint32 numlists = m_vecLists.getItemCount();
	for(i=0; i < numlists; i++)
	{
		fl_AutoNum * pAuto = m_vecLists.getNthItem(i);
		if(pAuto->getID() == id)
			break;
	}
	if(i >= numlists)
		m_vecLists.addItem(pAutoNum);
}

void PD_Document::listUpdate(pf_Frag_Strux* sdh )
{
	//
	// Notify all views of a listupdate
	//
	UT_return_if_fail (sdh);
	const pf_Frag_Strux * pfs = sdh;
	PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
	PT_DocPosition pos = getStruxPosition(sdh);
	const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_ListUpdate,pos,pAppIndex,pfs->getXID());
	notifyListeners(pfs, pcr);
	delete pcr;
}


void PD_Document::StopList(pf_Frag_Strux* sdh )
{
	//
	// Notify all views of a stoplist
	//
	setHasListStopped(false);
	const pf_Frag_Strux * pfs = sdh;
	PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
	PT_DocPosition pos = getStruxPosition(sdh);
	const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_StopList,pos,pAppIndex,pfs->getXID());
	notifyListeners(pfs, pcr);
	delete pcr;
	setHasListStopped(false);
}


bool PD_Document::appendList(const PP_PropertyVector & attributes)
{
	const std::string *szID = NULL;
	const std::string *szPid = NULL;
	const std::string *szType = NULL;
	const std::string *szStart = NULL;
	const std::string *szDelim = NULL;
	std::string szDec;
	UT_uint32 id, parent_id, start;
	FL_ListType type;

	for (auto iter = attributes.cbegin();
		 iter != attributes.cend(); ++iter) {

		const std::string & key = *iter;
		++iter;
		if (iter == attributes.cend()) {
			break;
		}
		if (key == "id") {
			szID = &(*iter);
		} else if (key == "parentid") {
			szPid = &(*iter);
		} else if (key == "type") {
			szType = &(*iter);
		} else if (key == "start-value") {
			szStart = &(*iter);
		} else if (key == "list-delim") {
			szDelim = &(*iter);
		} else if (key == "list-decimal") {
			szDec = *iter;
		}
	}

	if(!szID || !szPid || !szType || !szStart || !szDelim) {
		return false;
	}
	if(szDec.empty()) {
		szDec = ".";
	}
	id = stoi(*szID);
	UT_uint32 i;
	UT_uint32 numlists = m_vecLists.getItemCount();
	for(i=0; i < numlists; i++)
	{
		fl_AutoNum * pAuto = m_vecLists.getNthItem(i);
		if(pAuto->getID() == id)
			break;
	}
	if(i < numlists)
		return true; // List is already present
	parent_id = stoi(*szPid);
	type = static_cast<FL_ListType>(stoi(*szType));
	start = stoi(*szStart);

	// this is bad design -- layout items should not be created by the document, only by the view
	// (the props and attrs of layout items are view-specific due to possible revisions settings !!!)
	fl_AutoNum * pAutoNum = new fl_AutoNum(id, parent_id, type, start, szDelim->c_str(), szDec.c_str(), this, NULL);
	addList(pAutoNum);

	return true;
}

bool PD_Document::areListUpdatesAllowed(void)
{
        return m_ballowListUpdates;
}

void PD_Document::disableListUpdates(void)
{
        m_ballowListUpdates = false;
}

void PD_Document::enableListUpdates(void)
{
        m_ballowListUpdates = true;
}


void PD_Document::updateDirtyLists(void)
{
	UT_uint32 iNumLists = m_vecLists.getItemCount();
	UT_uint32 i;
	fl_AutoNum * pAutoNum;
	bool bDirtyList = false;
	for(i=0; i< iNumLists; i++)
	{
		pAutoNum = m_vecLists.getNthItem(i);
		if(pAutoNum->isEmpty() || (pAutoNum->getDoc() != this))
		{
			delete pAutoNum;
			m_vecLists.deleteNthItem(i);
			iNumLists--;
			i--;
		}
	}
	for(i=0; i< iNumLists; i++)
	{
		pAutoNum = m_vecLists.getNthItem(i);
		if(pAutoNum->isDirty() == true)
		{
			pAutoNum->update(0);
			bDirtyList = true;
		}
	}
	if(bDirtyList)
	{
		for(i=0; i< iNumLists; i++)
		{
			pAutoNum = m_vecLists.getNthItem(i);
			pAutoNum->fixHierarchy();
			pAutoNum->findAndSetParentItem();
		}
	}
}


bool PD_Document::fixListHierarchy(void)
{
	UT_uint32 iNumLists = m_vecLists.getItemCount();
	fl_AutoNum * pAutoNum;

	if (iNumLists == 0)
	{
		return false;
	}
	else
	{
            // Some documents may contain empty lists
            // that appear as a result of importing ODT file which contains
            // nested lists without paragraphs. To get rid of them we should
            // delete all lists that are defined but doesn't contain any items
            std::vector<unsigned int> itemsToRemove;
            for (UT_uint32 i = 0; i < iNumLists; i++)
            {
                    pAutoNum = m_vecLists.getNthItem(i);
                    if (pAutoNum->getFirstItem() == NULL)
                    {
                        itemsToRemove.push_back(i);
                    } 
                    else
                    {
                        pAutoNum->fixHierarchy();
                    }
            }
            while(!itemsToRemove.empty())
            {
                m_vecLists.deleteNthItem(itemsToRemove.back());
                itemsToRemove.pop_back();
            }
	    
            return true;
	}
}

void PD_Document::removeList(fl_AutoNum * pAutoNum, pf_Frag_Strux* sdh )
{
	UT_return_if_fail (pAutoNum);
	UT_sint32 ndx = m_vecLists.findItem(pAutoNum);
	UT_return_if_fail (ndx >= 0);
	//
	// Notify all views of a remove List
	//
	const pf_Frag_Strux * pfs = sdh;
	PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
	PT_DocPosition pos = getStruxPosition(sdh);
	const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_RemoveList,pos,pAppIndex,pfs->getXID());
	notifyListeners(pfs, pcr);
	delete pcr;
	m_vecLists.deleteNthItem(ndx);
}

void  PD_Document::setDoingPaste(void)
{
         m_bDoingPaste = true;
}


void  PD_Document::clearDoingPaste(void)
{
         m_bDoingPaste = false;
}

bool  PD_Document::isDoingPaste(void)
{
         return m_bDoingPaste;
}

bool PD_Document::convertPercentToInches(const char * szPercent, UT_UTF8String & sInches)
{
	double width = m_docPageSize.Width(DIM_IN);
	const pf_Frag_Strux* sdhSec = getLastSectionSDH();
	const char * szLeftMargin = NULL;
	const char * szRightMargin = NULL;

	// TODO -- probably needs to get revision settings from some view ...
	getPropertyFromSDH(sdhSec,true,PD_MAX_REVISION,"page-margin-left",&szLeftMargin);
	getPropertyFromSDH(sdhSec,true,PD_MAX_REVISION,"page-margin-right",&szRightMargin);
	if(szLeftMargin == NULL)
	{
		szLeftMargin = "0.5in";
	}
	if(szRightMargin == NULL)
	{
		szRightMargin = "0.5in";
	}
	double dLeft = UT_convertToInches(szLeftMargin);
	double dRight = UT_convertToInches(szRightMargin);
	width = width - dLeft - dRight;
	UT_String sVal = szPercent;
	sInches = UT_convertInchesToDimensionString(DIM_IN,width);
	return true;
}


bool PD_Document:: setPageSizeFromFile(const PP_PropertyVector & props)
{

	bool b =  m_docPageSize.Set(props);
	UT_DEBUGMSG(("SetPageSize m_bLoading %d \n",m_bLoading));
	if(!m_bLoading)
	{
		const PP_PropertyVector atts = {
			PT_DOCPROP_ATTRIBUTE_NAME,"pagesize"
		};
		UT_DEBUGMSG(("Sending page size CR \n"));
		b &= createAndSendDocPropCR(atts, props);
	}
	return b;
}

void PD_Document::addBookmark(const gchar * pName)
{
	m_vBookmarkNames.push_back(pName);
}

void PD_Document::removeBookmark(const gchar * pName)
{
	std::vector<std::string>::iterator iter = m_vBookmarkNames.begin();
	for( ; iter != m_vBookmarkNames.end() ; ++iter)
	{
		if(*iter == pName)
		{
			m_vBookmarkNames.erase(iter);
			break;
		}
	}
}

/*! Returns true if pName doesn't correspond to a
 *  currently existing bookmark. */
bool PD_Document::isBookmarkUnique(const gchar * pName) const
{
	std::vector<std::string>::const_iterator iter = m_vBookmarkNames.begin();
	for( ; iter != m_vBookmarkNames.end() ; ++iter)
	{
		if(*iter == pName)
		{
			return false;
		}
	}

	return true;
}

/*! Returns true if pName looks like a relative link, rather than a
 *  bookmark.

 *  Current heuristic: if pName contains a ., then it's a rel link;
 * otherwise it's a bookmark. */
bool PD_Document::isBookmarkRelativeLink(const gchar * pName) const
{
	UT_ASSERT_HARMLESS(sizeof(char) == sizeof(gchar));
	return strchr(static_cast<const char *>(pName), '.') != NULL;
}

//////////////////////////////////////////////////////////////////
// document-level properties

#define VARSET m_pPieceTable->getVarSet()

const PP_AttrProp * PD_Document::getAttrProp() const
{
	return VARSET.getAP(m_indexAP);
}

/*!
    Sets document attributes and properties
    can only be used while loading documents
    
    \param const gchar ** ppAttr: array of attribute/value pairs

	    if ppAttr == NULL and m_indexAP == 0xffffffff, the function
    	creates a new AP and sets it to the default values hardcoded
    	in it

        if ppAttr == NULL and m_indexAP != 0xffffffff, the function
        does nothing

        if ppAttr != NULL the function overlays passed attributes over
        the existing attributes (creating a new AP first if necessary)

    When initialising document attributes and props, we need to set
    m_indexAP to 0xffffffff and then call setAttributes(NULL).

    Importers should just call setAttributes(NULL) in the
    initialisation stage, this ensures that default values are set
    without overwriting existing values if those were set by the
    caller of the importer.

    Tomas, Dec 6, 2003
*/
bool PD_Document::setAttrProp(const PP_PropertyVector & ppAttr)
{
	// this method can only be used while loading  ...
	if(m_pPieceTable->getPieceTableState() != PTS_Loading)
	{
		UT_return_val_if_fail(0,false);
	}

	bool bRet = true;

	if(m_indexAP == 0xffffffff)
	{
		// AP not initialised, do so and set standard document attrs
		// and properties

		// first create an empty AP by passing NULL to storeAP
		// cast needed to disambiguate function signature
		bRet = VARSET.storeAP(PP_NOPROPS, &m_indexAP);

		if(!bRet)
			return false;

		// now set standard attributes
		PP_PropertyVector attr = {
			"xmlns", "http://www.abisource.com/awml.dtd",
			"xml:space", "preserve",
			"xmlns:awml", "http://www.abisource.com/awml.dtd",
			"xmlns:xlink", "http://www.w3.org/1999/xlink",
			"xmlns:svg", "http://www.w3.org/2000/svg",
			"xmlns:fo",	"http://www.w3.org/1999/XSL/Format",
			"xmlns:math", "http://www.w3.org/1998/Math/MathML",
			"xmlns:dc",	"http://purl.org/dc/elements/1.1/",
			"xmlns:ct", "http://www.abisource.com/changetracking.dtd",
			"fileformat", ABIWORD_FILEFORMAT_VERSION
		};

		if (XAP_App::s_szBuild_Version && XAP_App::s_szBuild_Version[0])
		{
			attr.push_back("version");
			attr.push_back(XAP_App::s_szBuild_Version);
		}

		bRet =  setAttributes(attr);

		if(!bRet)
			return false;

		// now set default properties, starting with dominant
		// direction
		bool bRTL = false;
		XAP_App::getApp()->getPrefs()->getPrefsValueBool(AP_PREF_KEY_DefaultDirectionRtl,&bRTL);

		PP_PropertyVector props = {
			"dom-dir", bRTL ? "rtl" : "ltr"
		};

		UT_DEBUGMSG(( "pd_Document::setAttrProp: setting dom-dir to %s\n", props[1].c_str()));
		bRet = setProperties(props);

		if(!bRet)
			return false;

		// if there is a default language in the preferences, set it
		UT_LocaleInfo locale;

		std::string lang(locale.getLanguage());
		if (locale.getTerritory().size()) {
			lang += "-";
			lang += locale.getTerritory();
		}

		props[0] = "lang";
		props[1] = lang;
		bRet = setProperties(props);

		if(!bRet)
			return false;
		
		// Yes, we have to set default properties for all document-level items, because
		// some piece of code (exporter, plugin) may want to get the value of that default,
		// not unitialized memory.  When a hashing solution is factored out of the PT,
		// it may be tempting to return NULLs.  Not good enough either.
		// I'm going to ask Dom the preferred way to make this rather more concise. -MG
		//
		// Actually, we do not set these because of uninitialised memory; you never get a
		// uninitialised memory from the the PP_AttrProp chain; nor do we set these
		// because we cannot return NULLs. We set these, because without them we cannot
		// lay the document out, and it is much better to have the defaults gathered in
		// one place than having all kinds of fallback values hardcoded all over the
		// place. Tomas
		
		// Update: Surely there is a way to make the getProperty mechanisms smarter, to
		// provide valid and accurate information on request (lazy-evaluation /
		// late-binding), because this superfluous storage sucks, and actually (in
		// concept) adds ambiguity by virtue of the fact that the means by which these
		// were set is not known or stored, and hence other pieces of code while capable
		// of following WYSIWYG, are not able to do otherwise with knowledge of whether
		// the user explicitly requested these properties to be set to these values or
		// they're just this way by virtue of AbiWord insisting on setting the default
		// upon initialization of any and every pd_Document.  This is bad for external
		// document storage and processing solutions, not to mention plugins that AbiWord
		// may ship.  Keep in mind, this is NOT the only place we have to do this.  Even
		// individual struxes within the document have to have their properties
		// initialized as it stands now. -MG

		// This storage is not superfluous, I have already explained that. Also, the attrs
		// and props in here fall into two separate groups. The document-only stuff (like
		// the various xml attributes), and attributes and properties that are part of the
		// resolution mechanism: when looking for property value, it is resolved through a
		// chain: spanAP - blockAP - sectionAP - documentAP - hardcoded defaults (the
		// hardcodes defaults are in PP_Property.cpp).  Struxes, etc., do not have any
		// properties as such, and do not have to have them initialised; they simply have
		// a reference to an PP_AttrProp instance, which can contain any number of
		// attributes/props, or none. If you use the getProperty() mechanism, you are
		// simply asking about resolution of a given property; if you want to know where
		// that property came from, it can be achieved by stepping down the chain (and the
		// PP_EvaluateProperty() function could easily be extended to return this info if
		// you really need it).
		//
		// There might be some value in knowing which properties were set manually by the
		// user, but I am not sure it is at all necessary. As the chain is, each level
		// should only contain attributes and properties set manually, since everything
		// else is inherited from the level below. There is currently a problem with some
		// code that sets individual attributes and properties without asking about their
		// relationship to the lower levels of the chain -- properties that resolve to the
		// same values as the chain below should be removed, not explicitely set. This
		// should be fixed up, but that has nothing to do with this code. Tomas

			// Endnotes
		props[0] = "document-endnote-type";
		props[1] = "numeric";
		if(!setProperties(props)) return false;
		props[0] = "document-endnote-place-enddoc";
		props[1] = "1";
		if(!setProperties(props)) return false;
		props[0] = "document-endnote-place-endsection";
		props[1] = "0";
		if(!setProperties(props)) return false;
		props[0] = "document-endnote-initial";
		props[1] = "1";
		if(!setProperties(props)) return false;
		props[0] = "document-endnote-restart-section";
		props[1] = "0";
		if(!setProperties(props)) return false;
			// Footnotes
		props[0] = "document-footnote-type";
		props[1] = "numeric";
		if(!setProperties(props)) return false;
		props[0] = "document-footnote-initial";
		props[1] = "1";
		if(!setProperties(props)) return false;
		props[0] = "document-footnote-restart-page";
		props[1] = "0";
		if(!setProperties(props)) return false;
		props[0] = "document-footnote-restart-section";
		props[1] = "0";
		if(!setProperties(props)) return false;

		// now overlay the attribs we were passed ...
		bRet = setAttributes(ppAttr);
	}
	else if(ppAttr.empty())
	{
		// we already have an AP, and have nothing to add to it
		return true;
	}
	else
	{
		// have an AP and given something to add to it
		// first, we need to take care of the top-xid attribute
		const std::string & pXID = PP_getAttribute("top-xid", ppAttr);
		if(!pXID.empty())
		{
			UT_uint32 iXID = stoi(pXID);
			m_pPieceTable->setXIDThreshold(iXID);
		}

		bRet = VARSET.mergeAP(PTC_AddFmt, m_indexAP, ppAttr,
                              PP_NOPROPS, &m_indexAP, this);
	}

	return bRet;
}

bool PD_Document::setAttributes(const PP_PropertyVector & ppAttr)
{
	return VARSET.mergeAP(PTC_AddFmt, m_indexAP, ppAttr, PP_NOPROPS, &m_indexAP, this);
}


bool PD_Document::setProperties(const PP_PropertyVector & ppProps)
{
	return VARSET.mergeAP(PTC_AddFmt, m_indexAP, PP_NOPROPS, ppProps, &m_indexAP, this);
}

#undef VARSET

void PD_Document::lockStyles(bool b)
{
	PP_PropertyVector attr = {
		"styles", b ? "locked" : "unlocked"
	};

	setAttributes(attr);
	m_bLockedStyles = b;
}

/*!
    Some exporters (RTF) need to know the visual direction at each
    position in the document as it is being exported. The problem is
    that visual direction is a property of the layout not of the
    document itself (I shall not make any comments about badly
    designed file formats here!). Since our document is not directly
    aware of any of its layouts, we have to find a listener for
    FL_DocLayout that is registered with this document (it does not
    matter if there are more FL_DocLayout listeners registered, the
    visual direction will be same for all, so we grab the first one),
    and from the listener we can get access to the layout, down to the
    runs which carry the information that we need.  Tomas, May 3, 2003
 */
bool PD_Document::exportGetVisDirectionAtPos(PT_DocPosition pos, UT_BidiCharType &type)
{
	if(m_bLoading)
		return true;

	if(pos == m_iVDLastPos && m_pVDRun)
	{
		// we have all the info we need cached, so just use it
		type = m_pVDRun->getVisDirection();
		return true;
	}
	else if(pos < m_iVDLastPos)
	{
		// this is the worst-case scenario, we have to start from the
		// beginning
		m_iVDLastPos = pos;
		if(!_exportInitVisDirection(pos))
			return false;
	}
	else
	{
		// we can continue from where we left of the last time
		m_iVDLastPos = pos;
		if(!_exportFindVisDirectionRunAtPos(pos))
			return false;
	}
	
	// make sure nothing has gone wrong here ...
	UT_return_val_if_fail(m_pVDRun, false);
	
	type = m_pVDRun->getVisDirection();
	return true;
}

bool PD_Document::_exportInitVisDirection(PT_DocPosition pos)
{
	if(m_bLoading)
		return true;
	m_pVDBl = NULL;
	m_pVDRun = NULL;

	// find the first DocLayout listener
	UT_uint32 count = m_vecListeners.getItemCount();
    fl_DocListener* pDocListener = NULL;
	
	for(UT_uint32 i = 0; i < count; i++)
	{
		PL_Listener * pL = (PL_Listener *) m_vecListeners.getNthItem(i);
		if(pL && pL->getType() == PTL_DocLayout)
		{
			pDocListener = (fl_DocListener*) pL;
			break;
		}
	}

	UT_return_val_if_fail(pDocListener, false);

	const FL_DocLayout * pDL = pDocListener->getLayout();
	UT_return_val_if_fail(pDL, false);

	
	m_pVDBl = pDL->findBlockAtPosition(pos);
	UT_return_val_if_fail(m_pVDBl, false);

	UT_uint32 iOffset = pos - m_pVDBl->getPosition();
	m_pVDRun = m_pVDBl->findRunAtOffset(iOffset);
	UT_return_val_if_fail(m_pVDRun, false);
	return true;
}

bool PD_Document::_exportFindVisDirectionRunAtPos(PT_DocPosition pos)
{
	// this is similar to the above, except we will first try to use
	// the cached info

	if(m_pVDBl && m_pVDRun)
	{
		UT_uint32 iOffset = pos - m_pVDBl->getPosition();

		//first see if the cached run matches (this will often be the
		//case since this we typicaly crawl over the document position
		//by position
		if(m_pVDRun->getBlockOffset() <= iOffset
		   && (m_pVDRun->getBlockOffset() + m_pVDRun->getLength()) > iOffset)
		{
			return true;
		}

		// now try to use the present block and any blocks that are
		// chained with it
		const fl_BlockLayout * pBL        = m_pVDBl;
		fp_Run *         pRunResult = NULL;

		while (1)
		{
			UT_sint32 iOffset2 = pos - pBL->getPosition();

			if(iOffset2 < 0)
				break;
			
			pRunResult = pBL->findRunAtOffset((UT_uint32)iOffset2);

			if(pRunResult)
				break;
			
			const fl_ContainerLayout * pCL = pBL->getNext();
			
			if(pCL && pCL->getContainerType() == FL_CONTAINER_BLOCK)
				pBL = reinterpret_cast<const fl_BlockLayout*>(pCL);
			else
				break;
		}

		if(pRunResult)
		{
			m_pVDRun = pRunResult;
			m_pVDBl = pBL;
			return true;
		}
	}

	// if we got so far the offset is past the present
	// block-chain, i.e., in a different section, we start from
	// the beginning
	return _exportInitVisDirection(pos);
}

bool PD_Document::insertStruxBeforeFrag(pf_Frag * pF, PTStruxType pts,
										const PP_PropertyVector & attributes,
										pf_Frag_Strux ** ppfs_ret)
{
	UT_return_val_if_fail (m_pPieceTable, false);

	// can only be used while loading the document

	if(pts == PTX_EndCell)
	{
		pf_Frag * pPrevFrag = pF->getPrev();
		if(pPrevFrag && pPrevFrag->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pPrevFrag);
			if(pfs->getStruxType() == PTX_SectionCell)
			{
				m_vecSuspectFrags.addItem(pPrevFrag);
			}
		}
	}
	updateStatus();
	return m_pPieceTable->insertStruxBeforeFrag(pF,pts,attributes,ppfs_ret);
}

bool PD_Document::insertSpanBeforeFrag(pf_Frag * pF, const UT_UCSChar * pbuf, UT_uint32 length)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	if(pF->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pF);
		if((pfs->getStruxType() != PTX_Block) && (pfs->getStruxType() != PTX_EndFootnote) && (pfs->getStruxType() != PTX_EndEndnote) && (pfs->getStruxType() != PTX_EndAnnotation) && (pfs->getStruxType() != PTX_EndCell) )
		{
			//
			// Append a block!
			//
			m_vecSuspectFrags.addItem(pF);
			return true;
		}
	}

	// can only be used while loading the document

	// REMOVE UNDESIRABLE CHARACTERS ...
	// we will remove all LRO, RLO, LRE, RLE, and PDF characters
	// * at the moment we do not handle LRE/RLE
	// * we replace LRO/RLO with our dir-override property

	PP_PropertyVector attrs = {
		"props", ""
	};

	bool result = true;
	const UT_UCS4Char * pStart = pbuf;

	for(const UT_UCS4Char * p = pbuf; p < pbuf + length; p++)
	{
		switch(*p)
		{
			case UCS_LRO:
				if((p - pStart) > 0)
					result &= m_pPieceTable->insertSpanBeforeFrag(pF,pStart,p - pStart);

				attrs[1] = "dir-override:ltr";
				result &= m_pPieceTable->appendFmt(attrs);
				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;

			case UCS_RLO:
				if((p - pStart) > 0)
					result &= m_pPieceTable->insertSpanBeforeFrag(pF,pStart,p - pStart);

				attrs[1] = "dir-override:rtl";
				result &= m_pPieceTable->appendFmt(attrs);

				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;

			case UCS_PDF:
				if((p - pStart) > 0)
					result &= m_pPieceTable->insertSpanBeforeFrag(pF,pStart,p - pStart);

				if((m_iLastDirMarker == UCS_RLO) || (m_iLastDirMarker == UCS_LRO))
				{
					attrs[1] = "dir-override:";
					result &= m_pPieceTable->appendFmt(attrs);
				}

				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;

			case UCS_LRE:
			case UCS_RLE:
				if((p - pStart) > 0)
					result &= m_pPieceTable->insertSpanBeforeFrag(pF,pStart,p - pStart);

				pStart = p + 1;
				m_iLastDirMarker = *p;
				break;
		}
	}

	result &= m_pPieceTable->insertSpanBeforeFrag(pF,pStart,length - (pStart-pbuf));
	return result;
}

bool PD_Document::insertObjectBeforeFrag(pf_Frag * pF, PTObjectType pto,
										 const PP_PropertyVector & attributes)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	if(pF->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pF);
		if((pfs->getStruxType() != PTX_Block) && (pfs->getStruxType() != PTX_EndFootnote) && (pfs->getStruxType() != PTX_EndEndnote)  && (pfs->getStruxType() != PTX_EndAnnotation) )
		{
			//
			// Append a block!
			//
			m_vecSuspectFrags.addItem(pF);
			return true;
		}
	}

	// can only be used while loading the document

	return m_pPieceTable->insertObjectBeforeFrag(pF,pto,attributes);
}

bool PD_Document::insertFmtMarkBeforeFrag(pf_Frag * pF)
{
	UT_return_val_if_fail (m_pPieceTable, false);
	if(pF->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pF);
		if((pfs->getStruxType() != PTX_Block) && (pfs->getStruxType() != PTX_EndFootnote) && (pfs->getStruxType() != PTX_EndEndnote) && (pfs->getStruxType() != PTX_EndAnnotation) )
		{
			//
			// Append a block!
			//
			m_vecSuspectFrags.addItem(pF);
			return true;
		}
	}

	// can only be used while loading the document

	return m_pPieceTable->insertFmtMarkBeforeFrag(pF);
}

bool PD_Document::changeStruxFormatNoUpdate(PTChangeFmt ptc, pf_Frag_Strux* sdh, const PP_PropertyVector & attributes)
{
	return m_pPieceTable->changeStruxFormatNoUpdate(ptc ,sdh,attributes);
}


/*!
 * Change the attributes of an object without generating a Change Record.
 * Use with extreme care.
 */
bool PD_Document::changeObjectFormatNoUpdate(PTChangeFmt ptc, pf_Frag_Object* odh, const PP_PropertyVector & attributes, const PP_PropertyVector & properties )
{
	pf_Frag_Object * pfo = odh;
	return m_pPieceTable->changeObjectFormatNoUpdate(ptc, pfo, attributes, properties);
}

/*!
 * Return Attribute Property Index associated with the pf_Frag_Object pointed
 * to by odh
 */
PT_AttrPropIndex  PD_Document::getAPIFromSOH(pf_Frag_Object* odh)
{
	pf_Frag_Object * pfo = odh;
	return pfo->getIndexAP();
}	

bool PD_Document::insertFmtMarkBeforeFrag(pf_Frag * pF, const PP_PropertyVector & attributes)
{
	UT_return_val_if_fail (m_pPieceTable, false);

	// can only be used while loading the document

	return m_pPieceTable->insertFmtMarkBeforeFrag(pF,attributes);
}

pf_Frag * PD_Document::findFragOfType(pf_Frag::PFType type, UT_sint32 iSubtype, pf_Frag * pfStart) const
{
	UT_return_val_if_fail(m_pPieceTable,NULL);

	pf_Frag * pf = pfStart;
	
	if(!pf)
		pf = m_pPieceTable->getFragments().getFirst();

	UT_return_val_if_fail(pf, NULL);

	while(pf)
	{
		bool bBreak = true;
		if(pf->getType() == type)
		{
			if(iSubtype < 0)
				break;

			switch(type)
			{
				// fragments with no subtypes
				case pf_Frag::PFT_Text:
				case pf_Frag::PFT_EndOfDoc:
				case pf_Frag::PFT_FmtMark:
					break;

				case pf_Frag::PFT_Object:
					{
						const pf_Frag_Object * pfo = static_cast<const pf_Frag_Object*>(pf);
						if((UT_sint32)pfo->getObjectType() != iSubtype)
							bBreak = false;
					}
					break;
					
				case pf_Frag::PFT_Strux:
					{
						const pf_Frag_Strux * pfs = static_cast<const pf_Frag_Strux*>(pf);
						if((UT_sint32)pfs->getStruxType() != iSubtype)
							bBreak = false;
					}
					break;

				default: 
					UT_ASSERT_HARMLESS(UT_NOT_REACHED);
			}

			if(bBreak)
				break;
		}

		pf = pf->getNext();
	}

	return pf;
}

pf_Frag * PD_Document::getLastFrag() const
{
	UT_return_val_if_fail(m_pPieceTable,NULL);
	return m_pPieceTable->getFragments().getLast();
}


/*!
    force the document into being dirty and signal this to our listeners
*/
void PD_Document::forceDirty()
{
	if(!isDirty())
	{
		_setForceDirty(true);

		// now notify listeners ...
		// this is necessary so that the save command is available after
		// operations that only change m_bForcedDirty
		signalListeners(PD_SIGNAL_DOCDIRTY_CHANGED);
	}
}


/*!
    Returns true if the stylesheets of both documents are identical
*/
bool PD_Document::areDocumentStylesheetsEqual(const AD_Document &D) const
{
	if(D.getType() != ADDOCUMENT_ABIWORD)
		return false;

	PD_Document &d = (PD_Document &)D;
	UT_return_val_if_fail(m_pPieceTable || d.m_pPieceTable, false);

	const std::map<std::string,PD_Style*> & hS1 = m_pPieceTable->getAllStyles();
	const std::map<std::string,PD_Style*> & hS2 = d.m_pPieceTable->getAllStyles();

	if(hS1.size() != hS2.size())
		return false;

	UT_StringPtrMap hFmtMap;

	for(std::map<std::string,PD_Style*>::const_iterator iter = hS1.begin();
		iter != hS1.end(); ++iter)
	{
		const PD_Style * pS1, * pS2;
		const std::string &key = iter->first;

		pS1 = iter->second;

		std::map<std::string,PD_Style*>::const_iterator iter2 = hS2.find(key);
		if (iter2 == hS2.end())
			return false;

		pS2 = iter2->second;

		PT_AttrPropIndex ap1 = pS1->getIndexAP();
		PT_AttrPropIndex ap2 = pS2->getIndexAP();

		// because the indexes are into different piecetables, we
		// have to expand them
		const PP_AttrProp * pAP1;
		const PP_AttrProp * pAP2;

		m_pPieceTable->getAttrProp(ap1, &pAP1);
		d.m_pPieceTable->getAttrProp(ap2, &pAP2);

		UT_return_val_if_fail(pAP1 && pAP2, false);

		// must print all digits to make this unambigous
		std::string s = UT_std_string_sprintf("%08x%08x", ap1, ap2);
		bool bAreSame = hFmtMap.contains(s,NULL);
		
		if(!bAreSame)
		{
			if(!pAP1->isEquivalent(pAP2))
			{
				return false;
			}
			else
			{
				hFmtMap.insert(s,NULL);
			}
		}
	}
	
	return true;
}


/*!
    carries out the actual change in PieceTable; called by
    acceptRejectRevision() and rejectAllHigherRevisions()

    this method operates on a fragment at a time, but if it
    results in deletion from PT, more fragments might be deleted
*/
bool PD_Document::_acceptRejectRevision(bool bReject, UT_uint32 iStart, UT_uint32 iEnd,
										const PP_Revision * pRev,
										PP_RevisionAttr &RevAttr, pf_Frag * pf,
										bool & bDeleted)
{
	UT_return_val_if_fail(pf && pRev, false);
	bDeleted = false;

	UT_uint32 iRealDeleteCount;
	const std::string rev = "revision";
	PP_PropertyVector ppAttr = {
		rev, ""
	};

	PP_PropertyVector ppProps;
	PP_PropertyVector ppAttr2;
	bool bRet = true;
	UT_uint32 i;

	// if the fragment is a strux that has a corresponding end element
	// and we will be deleting itwe have to expand the deletion to the
	// end of that element
	UT_uint32 iEndDelete = iEnd;
	PP_RevisionType iRevType = pRev->getType();

	if(pf->getType() == pf_Frag::PFT_Strux &&
	   (   (bReject &&  (iRevType == PP_REVISION_ADDITION_AND_FMT || iRevType == PP_REVISION_ADDITION))
		|| (!bReject && (iRevType == PP_REVISION_DELETION))))
	{
		pf_Frag_Strux * pfs = (pf_Frag_Strux*)pf;
		PTStruxType pst = PTX_Block;
		
		switch(pfs->getStruxType())
		{
			case PTX_SectionEndnote:
				pst = PTX_EndEndnote; break;
			case PTX_SectionTable:
				pst = PTX_EndTable; break;
			case PTX_SectionCell:
				pst = PTX_EndCell; break;
			case PTX_SectionFootnote:
				pst = PTX_EndFootnote; break;
			case PTX_SectionAnnotation:
				pst = PTX_EndAnnotation; break;
		    case PTX_SectionMarginnote:
				pst = PTX_EndMarginnote; break;
			case PTX_SectionFrame:
				pst = PTX_EndFrame; break;
			case PTX_SectionTOC:
				pst = PTX_EndTOC; break;

			default: ; // do nothing 
		}

		if(pst != PTX_Block)
		{
			pf_Frag * pf2 = pf->getNext();
			while(pf2)
			{
				iEndDelete += pf2->getLength();
				if(pf2->getType() == pf_Frag::PFT_Strux)
				{
					pf_Frag_Strux * pfs2 = (pf_Frag_Strux*)pf2;
					if(pfs2->getStruxType() == pst)
						break;
				}

				pf2 = pf2->getNext();
			}
		}
	}
	
	if(bReject)
	{
		switch(iRevType)
		{
			case PP_REVISION_ADDITION:
			case PP_REVISION_ADDITION_AND_FMT:
				{
					// delete this fragment
					bDeleted = true;

					// since we need real delete, we need to step out
					// of rev. marking mode for a moment ...
					bool bMark = isMarkRevisions();
					_setMarkRevisions(false);
					bRet = deleteSpan(iStart,iEndDelete,NULL,iRealDeleteCount);
					_setMarkRevisions(bMark);

					if(!bRet)
						bDeleted = false;
					
					return bRet;
				}
					
			case PP_REVISION_DELETION:
				// remove the revision (and any higher ones) from the attribute
				RevAttr.removeAllHigherOrEqualIds(pRev->getId());
				pRev = NULL;
				
				ppAttr[0] = rev;
				ppAttr[1] = RevAttr.getXMLstring();

				if(pf->getType() == pf_Frag::PFT_Strux)
				{
					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);

					// the changeStrux function tries to locate the strux which _contains_ the
					// position we pass into it; however, iStart is the doc position of the actual
					// strux, so we have to skip over the strux
					return changeStruxFmt(PTC_AddFmt, iStart + 1, iEnd, ppAttr, PP_NOPROPS, pfs->getStruxType());
				}
				else
					return changeSpanFmt(PTC_AddFmt, iStart, iEnd, ppAttr, PP_NOPROPS);

			case PP_REVISION_FMT_CHANGE:
				// need to set a new revision attribute
				// first remove current revision from pRevAttr
				RevAttr.removeAllHigherOrEqualIds(pRev->getId());
				pRev = NULL;
				
				ppAttr[0] = rev;
				ppAttr[1] = RevAttr.getXMLstring();

				if(pf->getType() == pf_Frag::PFT_Strux)
				{
					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);					
					// the changeStrux function tries to locate the strux which _contains_ the
					// position we pass into it; however, iStart is the doc position of the actual
					// strux, so we have to skip over the strux
					bRet &= changeStruxFmt(PTC_AddFmt, iStart + 1, iEnd, ppAttr, ppProps, pfs->getStruxType());
				}
				else
					bRet &= changeSpanFmt(PTC_AddFmt, iStart, iEnd, ppAttr, ppProps);

				return bRet;


			default:
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				return false;
		}
	}
	else
	{
		switch(iRevType)
		{
			case PP_REVISION_ADDITION:
				// simply remove the revision attribute
				if(pf->getType() == pf_Frag::PFT_Strux)
				{
					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);					
					// the changeStrux function tries to locate the strux which _contains_ the
					// position we pass into it; however, iStart is the doc position of the actual
					// strux, so we have to skip over the strux
					return changeStruxFmt(PTC_RemoveFmt, iStart + 1, iEnd, ppAttr, PP_NOPROPS, pfs->getStruxType());
				}
				else
					return changeSpanFmt(PTC_RemoveFmt, iStart, iEnd, ppAttr, PP_NOPROPS);

			case PP_REVISION_DELETION:
				{
					// delete this fragment
					bDeleted = true;

					// since we need real delete, we need to step out
					// of rev. marking mode for a moment ...
					bool bMark = isMarkRevisions();
					_setMarkRevisions(false);
					bRet = deleteSpan(iStart,iEndDelete,NULL,iRealDeleteCount);
					_setMarkRevisions(bMark);

					if(!bRet)
						bDeleted = false;
					
					return bRet;
				}
					
			case PP_REVISION_ADDITION_AND_FMT:
				// overlay the formatting and remove the revision attribute
				if(pf->getType() == pf_Frag::PFT_Strux)
				{
					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);					
					// the changeStrux function tries to locate the strux which _contains_ the
					// position we pass into it; however, iStart is the doc position of the actual
					// strux, so we have to skip over the strux
					return changeStruxFmt(PTC_RemoveFmt, iStart + 1, iEnd, ppAttr, PP_NOPROPS, pfs->getStruxType());
				}
				else
					return changeSpanFmt(PTC_RemoveFmt, iStart, iEnd, ppAttr, PP_NOPROPS);

			case PP_REVISION_FMT_CHANGE:
				// overlay the formatting and remove this revision
				// from the revision attribute
				for(i = 0; i < pRev->getPropertyCount(); i++)
				{
					const char* props, *value;
					pRev->getNthProperty(i, props, value);

					ppProps.push_back(props);
					ppProps.push_back(value ? value : "");
				}

				for(i = 0; i < pRev->getAttributeCount(); i++)
				{
					const char* props, *value;
					pRev->getNthAttribute(i, props, value);

					ppAttr2.push_back(props);
					ppAttr2.push_back(value ? value : "");
				}

				if(pRev->getType() != PP_REVISION_ADDITION_AND_FMT)	{
					// need to set a new revision attribute
					// first remove current revision from pRevAttr
					RevAttr.removeAllHigherOrEqualIds(pRev->getId());
					pRev = NULL;

					std::string revision = RevAttr.getXMLstring();

					if(revision.empty())
					{
						// no revision attribute left, which means we
						// have to remove it by separate call to changeSpanFmt

						// now we use the ppAttr set to remove the
						// revision attribute
						if(pf->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);					
							// the changeStrux function tries to locate the strux which _contains_ the
							// position we pass into it; however, iStart is the doc position of the actual
							// strux, so we have to skip over the strux
							bRet &= changeStruxFmt(PTC_RemoveFmt, iStart + 1, iEnd, ppAttr, PP_NOPROPS, pfs->getStruxType());
						}
						else
							bRet &= changeSpanFmt(PTC_RemoveFmt, iStart, iEnd, ppAttr, PP_NOPROPS);
					} else {
						ppAttr2.push_back(rev);
						ppAttr2.push_back(revision);
					}
				}

				UT_ASSERT_HARMLESS( !ppAttr2.empty() || !ppProps.empty() );

				if(pf->getType() == pf_Frag::PFT_Strux)
				{
					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
					// the changeStrux function tries to locate the strux which _contains_ the
					// position we pass into it; however, iStart is the doc position of the actual
					// strux, so we have to skip over the strux
					bRet &= changeStruxFmt(PTC_AddFmt,iStart+1,iEnd,ppAttr2,ppProps, pfs->getStruxType());
				}
				else
					bRet &= changeSpanFmt(PTC_AddFmt,iStart,iEnd,ppAttr2,ppProps);

				return bRet;

			default:
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		}
	}

	return false;
}

bool PD_Document::acceptAllRevisions()
{
	PD_DocIterator t(*this);
	UT_return_val_if_fail(t.getStatus() == UTIter_OK, false);
	
	notifyPieceTableChangeStart();
	
	beginUserAtomicGlob();	
	while(t.getStatus() == UTIter_OK)
	{
		pf_Frag * pf = const_cast<pf_Frag *>(t.getFrag());

		if(!pf)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			endUserAtomicGlob();
			notifyPieceTableChangeEnd();
			return false;
		}
		
		PT_AttrPropIndex API = pf->getIndexAP();

		const PP_AttrProp * pAP = NULL;
		m_pPieceTable->getAttrProp(API,&pAP);
		if(!pAP)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			endUserAtomicGlob();
			notifyPieceTableChangeEnd();
			return false;
		}
		
		const gchar * pszRevision = NULL;
		pAP->getAttribute("revision", pszRevision);
		
		if(pszRevision == NULL)
		{
			// no revisions on this fragment
			t += pf->getLength();
			continue;
		}
			
		PP_RevisionAttr RevAttr(pszRevision);
		RevAttr.pruneForCumulativeResult(this);
		const PP_Revision * pRev = NULL;
		if(RevAttr.getRevisionsCount())
			pRev = RevAttr.getNthRevision(0);
		
		if(!pRev)
		{
			// no revisions after pruning ???
			t += pf->getLength();
			continue;
		}
		
		UT_uint32 iStart = t.getPosition();
		UT_uint32 iEnd   = iStart + pf->getLength();
		bool bDeleted = false;
		
		_acceptRejectRevision(false /*accept*/, iStart, iEnd, pRev, RevAttr, pf, bDeleted);
		
		// advance -- the call to _acceptRejectRevision could have
		// resulted in deletion and/or merging of fragments; we have
		// to reset the iterator
		if(bDeleted)
			t.reset(iStart, NULL);
		else
			t.reset(iEnd, NULL);
	}

	// _acceptRejectRevison() function unfortunately leaves some unwanted fmt marks in the
	// document; we will purge all fmt marks
	purgeFmtMarks();
	
	endUserAtomicGlob();
	notifyPieceTableChangeEnd();
	signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	return true;
}
	
bool PD_Document::rejectAllHigherRevisions(UT_uint32 iLevel)
{
	PD_DocIterator t(*this);
	UT_return_val_if_fail(t.getStatus() == UTIter_OK, false);
	
	const PP_Revision * pRev;

	notifyPieceTableChangeStart();
	
	beginUserAtomicGlob();	
	while(t.getStatus() == UTIter_OK)
	{
		pf_Frag * pf = const_cast<pf_Frag *>(t.getFrag());

		if(!pf)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			endUserAtomicGlob();
			notifyPieceTableChangeEnd();
			return false;
		}
		
		PT_AttrPropIndex API = pf->getIndexAP();

		const PP_AttrProp * pAP = NULL;
		m_pPieceTable->getAttrProp(API,&pAP);
		if(!pAP)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			endUserAtomicGlob();
			notifyPieceTableChangeEnd();
			return false;
		}
		
		const gchar * pszRevision = NULL;
		pAP->getAttribute("revision", pszRevision);
		
		if(pszRevision == NULL)
		{
			// no revisions on this fragment
			t += pf->getLength();
			continue;
		}
			
		PP_RevisionAttr RevAttr(pszRevision);
		pRev = RevAttr.getLowestGreaterOrEqualRevision(iLevel+1);
		if(!pRev)
		{
			// no higher revisions
			t += pf->getLength();
			continue;
		}
		
		UT_uint32 iStart = t.getPosition();
		UT_uint32 iEnd   = iStart + pf->getLength();
		bool bDeleted = false;
		
		_acceptRejectRevision(true /*reject*/, iStart, iEnd, pRev, RevAttr, pf, bDeleted);
		
		// advance -- the call to _acceptRejectRevision could have
		// resulted in deletion and/or merging of fragments; we have
		// to reset the iterator
		if(bDeleted)
			t.reset(iStart, NULL);
		else
			t.reset(iEnd, NULL);
	}

	// _acceptRejectRevison() function unfortunately leaves some unwanted fmt marks in the
	// document; we will purge all fmt marks
	purgeFmtMarks();
	
	endUserAtomicGlob();
	notifyPieceTableChangeEnd();
	signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	return true;
}

/*!
   accepts or reject top visible revision between document positions
   iStart and iEnd.
   
   \param bReject  true if revisions are to be rejected
   \param iPos1    document position to start at
   \param iPos2     document position to finish at
   \param iLevel   the highest revision level to accept

   \return         true on success
   
   NB: For each fragment this function removes the highest revision <=
       iLevel. For example, if iLevel is 3 and fragment contains
       revisions 1,2, 4, revision 2 will be removed.
*/
bool PD_Document::acceptRejectRevision(bool bReject, UT_uint32 iPos1,
									   UT_uint32 iPos2, UT_uint32 iLevel)
{
	UT_uint32 iPosStart = UT_MIN(iPos1, iPos2);
	UT_uint32 iPosEnd   = UT_MAX(iPos1, iPos2);
	
	PD_DocIterator t(*this, iPosStart);
	UT_return_val_if_fail(t.getStatus() == UTIter_OK, false);
	
	
	const PP_Revision * pSpecial;
	const PP_Revision * pRev;
	UT_uint32 iLenProcessed = 0;
	bool bFirst = true;
	
	notifyPieceTableChangeStart();

	beginUserAtomicGlob();	
	while(t.getStatus() == UTIter_OK && iPosStart + iLenProcessed < iPosEnd)
	{
		pf_Frag * pf = const_cast<pf_Frag *>(t.getFrag());
		if(!pf)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			endUserAtomicGlob();
			notifyPieceTableChangeEnd();
			return false;
		}

		UT_uint32 iFragLen = pf->getLength();

		if(bFirst)
		{
			// we might be working only with a part of the frag
			bFirst = false;
			iFragLen -= (iPosStart - pf->getPos());
		}
		
		iLenProcessed += iFragLen;

		PT_AttrPropIndex API = pf->getIndexAP();

		const PP_AttrProp * pAP = NULL;
		m_pPieceTable->getAttrProp(API,&pAP);
		if(!pAP)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			endUserAtomicGlob();
			notifyPieceTableChangeEnd();
			return false;
		}
		
		const gchar * pszRevision = NULL;
		pAP->getAttribute("revision", pszRevision);
		
		if(pszRevision == NULL)
		{
			// no revisions on this fragment
			t += iFragLen;
			continue;
		}
			
		PP_RevisionAttr RevAttr(pszRevision);
		pRev = RevAttr.getGreatestLesserOrEqualRevision(iLevel, &pSpecial);
		if(!pRev)
		{
			// no visible revisions
			t += iFragLen;
			continue;
		}
		
		UT_uint32 iStart = t.getPosition();
		UT_uint32 iEnd   = iStart + iFragLen;

		bool bDeleted = false;
		_acceptRejectRevision(bReject, iStart, iEnd, pRev, RevAttr, pf, bDeleted);
		
		// advance -- the call to _acceptRejectRevision could have
		// resulted in deletion and/or merging of fragments; we have
		// to reset the iterator
		if(bDeleted)
			t.reset(iStart, NULL);
		else
			t.reset(iEnd, NULL);
	}

	endUserAtomicGlob();
	notifyPieceTableChangeEnd();
	signalListeners(PD_SIGNAL_UPDATE_LAYOUT);
	return true;
}


/*!
  Clears out the revisions table if no revisions are left in the document
*/
void PD_Document::purgeRevisionTable(bool bUnconditional /* = false */)
{
	if(getRevisions().getItemCount() == 0)
		return;

	if(!bUnconditional)
	{
		UT_String sAPI;
		UT_StringPtrMap hAPI;
	
		PD_DocIterator t(*this);

		// work our way thought the document looking for frags with
		// revisions attributes ...
		while(t.getStatus() == UTIter_OK)
		{
			const pf_Frag * pf = t.getFrag();
			UT_return_if_fail(pf);

			PT_AttrPropIndex api = pf->getIndexAP();

			UT_String_sprintf(sAPI, "%08x", api);

			if(!hAPI.contains(sAPI, NULL))
			{
				const PP_AttrProp * pAP;
				UT_return_if_fail(getAttrProp(api, &pAP));
				UT_return_if_fail(pAP);

				const gchar * pVal;

				if(pAP->getAttribute(PT_REVISION_ATTRIBUTE_NAME, pVal))
					return;

				// cache this api so we do not need to do this again if we
				// come across it
				hAPI.insert(sAPI,NULL);
			}

			t += pf->getLength();
		}
	}
	

	// if we got this far, we have not found any revisions in the
	// whole doc, clear out the table
	UT_DEBUGMSG(("PD_Document::purgeRevisionTable(): clearing\n"));
	_purgeRevisionTable();
}


/*!
    Starting to search this document at position pos where the two
    documents are known to become different, attepts to find location
    at which similarities resume

    \param pos: when called, should contain offset in present document
                at which the difference starts; on successfult return
                it will contain offset in present document where
                similarities resume

    \param iOffset2: when called contains offset to be added to
                     position pos in order to correctly position start
                     of the search in document d; on return it
                     contains offset to be add to pos in order to
                     obtain correct location of the resumption of
                     similarites in document d

   \param iKnownLength: on return contains the minium guaranteed length of the similarity

   \param d the document to which this document is to be compared

   \return returns true if it succeeds; if no further similarities are
           found returns false
*/
bool PD_Document::findWhereSimilarityResumes(PT_DocPosition &pos, UT_sint32 &iOffset2,
											 UT_uint32 & iKnownLength,
											 const PD_Document &d) const
{
	UT_return_val_if_fail(m_pPieceTable || d.m_pPieceTable, true);
		
	//  scroll through the documents comparing contents
	PD_DocIterator t1(*this, pos);
	PD_DocIterator t2(d, pos + iOffset2);

	// first, let's assume that the difference is an insertion in doc
	// 2; we will take a few chars from doc 1 and try to locate them
	// in doc 2

	// this is a similarity threshold, very arbitrary ...  if we match
	// iTry chars we will be happy if we do not match at least
	// iMinOverlap we will give up. We will use variable step
	UT_sint32 iTry = 128; 
	UT_sint32 iMinOverlap = 3;
	UT_sint32 iStep = 128;
	UT_sint32 i = 0;

	UT_uint32 iFoundPos1 = 0;
	UT_uint32 iFoundPos2 = 0;
	UT_sint32 iFoundOffset1 = 0;
	UT_sint32 iFoundOffset2 = 0;

	for(i = iTry; i >= iMinOverlap; i -= iStep)
	{
		UT_uint32 pos1 = t1.getPosition();
		UT_uint32 pos2 = t2.getPosition();

		UT_uint32 iPos = t2.find(t1,i,true);

		if(t2.getStatus() == UTIter_OK)
		{
			// we found what we were looking for
			iFoundPos1 = pos1;
			iFoundOffset1 = iPos - iFoundPos1;
			break;
		}
		else
		{
			// we did not find our text, reset position ...
			t2.setPosition(pos2);
			t1.setPosition(pos1);
			
			if(iStep > 1)
				iStep /= 2;
		}
	}

	// remember the length we found ...
	UT_sint32 iLen1 = i >= iMinOverlap ? i : 0;

	if(i == iTry)
	{
		// we found the whole iTry chunk, we will stop here ...
		pos = iFoundPos1;
		iOffset2 = iFoundOffset1;
		iKnownLength = iTry;
		return true;
	}
	
	// now do the same, but assuming our text is deleted from doc 2
	t2.setPosition(pos);
	t1.setPosition(pos + iOffset2);
	iStep = 128;
	
	for(i = iTry; i >= iMinOverlap; i -= iStep)
	{
		UT_uint32 pos1 = t1.getPosition();
		UT_uint32 pos2 = t2.getPosition();

		UT_uint32 iPos = t1.find(t2,i,true);

		if(t1.getStatus() == UTIter_OK)
		{
			// we found what we were looking for
			iFoundPos2 = iPos;
			iFoundOffset2 = pos2 - iFoundPos2;
			break;
		}
		else
		{
			// we did not find our text, reset position ...
			t2.setPosition(pos2);
			t1.setPosition(pos1);

			if(iStep > 1)
				iStep /= 2;
		}
	}

	UT_sint32 iLen2 = i >= iMinOverlap ? i : 0;

	if( !iLen1 && !iLen2)
		return false;
	
	// now we will go with whatever is longer
	if(iLen1 >= iLen2)
	{
		pos = iFoundPos1;
		iOffset2 = iFoundOffset1;
		iKnownLength = iLen1;
	}
	else
	{
		pos = iFoundPos2;
		iOffset2 = iFoundOffset2;
		iKnownLength = iLen2;
	}
	
	return true;
}


/*!
    finds the position of the first difference in content between this
    document and document d, starting search at given position

    \param pos when called this variable should contian document
               offset at which to start searching; on success this
               variable will contain offset of the difference in
               present document

    \param iOffset2 when called contains offset to be added to pos to
                    locate identical position in document d

    \param d   the document to compare with

    \return    returns false if no difference was found, true otherwise
*/
bool PD_Document::findFirstDifferenceInContent(PT_DocPosition &pos, UT_sint32 &iOffset2,
											   const PD_Document &d) const
{
	UT_return_val_if_fail(m_pPieceTable || d.m_pPieceTable, true);
		
	//  scroll through the documents comparing contents
	PD_DocIterator t1(*this, pos);
	PD_DocIterator t2(d, pos + iOffset2);

	while(t1.getStatus() == UTIter_OK && t2.getStatus() == UTIter_OK)
	{
		const pf_Frag * pf1 = t1.getFrag();
		const pf_Frag * pf2 = t2.getFrag();

		if(!pf1 || !pf2)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			return true;
		}
		
		if(pf1->getType() != pf2->getType())
		{
			pos = pf1->getPos();
			return true;
		}
		
		UT_uint32 iFOffset1 = t1.getPosition() - pf1->getPos();
		UT_uint32 iFOffset2 = t2.getPosition() - pf2->getPos();
		
		UT_uint32 iLen1 = pf1->getLength() - iFOffset1;
		UT_uint32 iLen2 = pf2->getLength() - iFOffset2;
		UT_uint32 iLen  = UT_MIN(iLen1, iLen2);

		if(   iLen1 == iLen2 && iFOffset1 == 0 && iFOffset2 == 0
		   && pf1->getType() != pf_Frag::PFT_Text)
		{
			// completely overlapping non-text frags ..
			if(!(pf1->isContentEqual(*pf2)))
			{
				pos = pf1->getPos();
				return true;
			}
		}
		else if(pf1->getType() != pf_Frag::PFT_Text)
		{
			// partially overlapping frags and not text
			pos = pf1->getPos();
			return true;
		}
		else
		{
			// we have two textual frags that overlap
			// work our way along the overlap ...
			for(UT_uint32 i = 0; i < iLen; ++i)
			{
				if(t1.getChar() != t2.getChar())
				{
					pos = t1.getPosition();
					return true;
				}
				
				++t1;
				++t2;
			}

			// we are already past the end of the shorter frag
			continue;
		}

		// advance both iterators by the processed length
		t1 += iLen;
		t2 += iLen;
	}

	if(t1.getStatus() == UTIter_OK && t2.getStatus() != UTIter_OK)
	{
		// document two is shorter ...
		pos = t1.getPosition();
		return true;
	}

	if(t1.getStatus() != UTIter_OK && t2.getStatus() == UTIter_OK)
	{
		// document 1 is shorter
		pos = t2.getPosition() - iOffset2;
		return true;
	}

	// if we got this far, we found no differences at all ...
	return false;
}

void PD_Document::setAutoRevisioning(bool autorev)
{
	AD_Document::setAutoRevisioning(autorev);

	// TODO tell our listeners to redo layout ...
	signalListeners(PD_SIGNAL_REFORMAT_LAYOUT);	
}



/*!
    Returns true if the contents of the two documents are identical
    if the function returns false, pos contains the document position
    at which first difference was encountered
*/
bool PD_Document::areDocumentContentsEqual(const AD_Document &D, UT_uint32 &pos) const
{
	pos = 0;
	if(D.getType() != ADDOCUMENT_ABIWORD)
		return false;
	
	PD_Document &d = (PD_Document &)D;
	UT_return_val_if_fail(m_pPieceTable || d.m_pPieceTable, false);
		
	// test the docs for length
	UT_uint32 end1, end2;

	pf_Frag * pf = m_pPieceTable->getFragments().getLast();

	UT_return_val_if_fail(pf,false);
		
	end1 = pf->getPos() + pf->getLength();
	
	pf = d.m_pPieceTable->getFragments().getLast();

	UT_return_val_if_fail(pf,false);
		
	end2 = pf->getPos() + pf->getLength();

	if(end1 != end2)
	{
		pos = UT_MIN(end1, end2);
		return false;
	}
	
	
	//  scroll through the documents comparing contents
	PD_DocIterator t1(*this);
	PD_DocIterator t2(d);

	while(t1.getStatus() == UTIter_OK && t2.getStatus() == UTIter_OK)
	{
		const pf_Frag * pf1 = t1.getFrag();
		const pf_Frag * pf2 = t2.getFrag();

		if(!pf1)
		{
			if(pf2)
				pos = pf2->getPos();
			else
				pos = 0;
			
			return false;
		}

		if(!pf2)
		{
			pos = pf1->getPos();
			return false;
		}
		

		if(pf1->getType() != pf2->getType())
		{
			pos = pf1->getPos();
			return false;
		}
		

		UT_uint32 iFOffset1 = t1.getPosition() - pf1->getPos();
		UT_uint32 iFOffset2 = t2.getPosition() - pf2->getPos();
		
		UT_uint32 iLen1 = pf1->getLength() - iFOffset1;
		UT_uint32 iLen2 = pf2->getLength() - iFOffset2;
		UT_uint32 iLen  = UT_MIN(iLen1, iLen2);

		if(iLen1 == iLen2 && iFOffset1 == 0 && iFOffset2 == 0)
		{
			// these two frags overlap exactly, so we can just use the
			// pf_Frag::isContentEqual() on them
			if(!(pf1->isContentEqual(*pf2)))
			{
				// TODO -- this is not position of the difference, but
				// of the start of the fragment (there difference
				// could be inside)
				pos = pf1->getPos();
				return false;
			}
			
		}
		else if(pf1->getType() != pf_Frag::PFT_Text)
		{
			// partially overlapping frags and not text
			pos = pf1->getPos();
			return false;
		}
		else
		{
			// we have two textual frags that overlap
			// work our way along the overlap ...
			for(UT_uint32 i = 0; i < iLen; ++i)
			{
				if(t1.getChar() != t2.getChar())
				{
					pos = t1.getPosition() + i;
					return false;
				}
				

				++t1;
				++t2;
			}

			// we are already past the end of the shorter frag
			continue;
		}

		// advance both iterators by the processed length
		t1 += iLen;
		t2 += iLen;
	}

	if((t1.getStatus() == UTIter_OK && t2.getStatus() != UTIter_OK))
	{
		// documents are of different length ...
		pos = t1.getPosition();
		return false;
	}

	if((t1.getStatus() != UTIter_OK && t2.getStatus() == UTIter_OK))
	{
		// documents are of different length ...
		pos = t2.getPosition();
		return false;
	}

	return true;
}

/*!
    Compare the format of the this document to another document;
    returns true if document formats are identical
    
    If the function returns false, pos contains the document position
    at which first difference was encountered

    NB: If the document contents are known not to be equal, it makes no
    sense to call this function.
*/
bool PD_Document::areDocumentFormatsEqual(const AD_Document &D, UT_uint32 &pos) const
{
	pos = 0;
	if(D.getType() != ADDOCUMENT_ABIWORD)
		return false;
	
	PD_Document &d = (PD_Document &)D;
	UT_return_val_if_fail(m_pPieceTable || d.m_pPieceTable, false);
		
	//  scroll through the documents comparing fmt
	PD_DocIterator t1(*this);
	PD_DocIterator t2(d);
		
	// in order to avoid repeated comparions of AP, we will store
	// record of matching AP's
	UT_StringPtrMap hFmtMap;
	
	while(t1.getStatus() == UTIter_OK && t2.getStatus() == UTIter_OK)
	{
		// need to cmp contents
		const pf_Frag * pf1 = t1.getFrag();
		const pf_Frag * pf2 = t2.getFrag();

		UT_return_val_if_fail(pf1 && pf2, false);

		PT_AttrPropIndex ap1 = pf1->getIndexAP();
		PT_AttrPropIndex ap2 = pf2->getIndexAP();

		// because the indexes are into different piecetables, we
		// have to expand them
		const PP_AttrProp * pAP1;
		const PP_AttrProp * pAP2;

		m_pPieceTable->getAttrProp(ap1, &pAP1);
		d.m_pPieceTable->getAttrProp(ap2, &pAP2);

		UT_return_val_if_fail(pAP1 && pAP2, false);

		UT_String s;
		UT_String_sprintf(s,"%08x%08x", ap1, ap2);
		bool bAreSame = hFmtMap.contains(s,NULL);
		
		if(!bAreSame)
		{
			if(!pAP1->isEquivalent(pAP2))
			{
				pos = t1.getPosition();
				return false;
			}
			else
			{
				hFmtMap.insert(s,NULL);
			}
		}
		
		UT_uint32 iLen = UT_MIN(pf1->getLength(),pf2->getLength());
		t1 += iLen;
		t2 += iLen;
	}

	if((t1.getStatus() == UTIter_OK && t2.getStatus() != UTIter_OK))
	{
		// documents are of different length ...
		pos = t1.getPosition();
		return false;
	}

	if((t1.getStatus() != UTIter_OK && t2.getStatus() == UTIter_OK))
	{
		// documents are of different length ...
		pos = t2.getPosition();
		return false;
	}

	return true;
}

void PD_Document::setMarkRevisions(bool bMark)
{
	if(isMarkRevisions() != bMark)
	{
		AD_Document::setMarkRevisions(bMark);
	 	signalListeners(PD_SIGNAL_REVISION_MODE_CHANGED);
	}
}

/*!
    This function crawls over the entire document and removes all fmt marks. It is principally meant
    to do PT clean up on import (for example, the structure of MS Word documents is such that we end
    up with a myriad of superfluous fmt marks in the document), and should be called by an importer
    at the end of import process.
*/
bool PD_Document::purgeFmtMarks()
{
	return m_pPieceTable->purgeFmtMarks();
}


bool PD_Document::getAttrProp(PT_AttrPropIndex apIndx, const PP_AttrProp ** ppAP, PP_RevisionAttr ** pRevisions,
							  bool bShowRevisions, UT_uint32 iRevisionId, bool &bHiddenRevision) const
{
	bool bRevisionAttrNeeded = pRevisions ? true : false;
	PP_RevisionAttr * pRevAttr = NULL;
	bHiddenRevision = false;

	const PP_AttrProp * pAP = NULL;

	if(!getAttrProp(apIndx,&pAP))
		return false;

	if(   pAP->getRevisedIndex() != 0xffffffff
	   && pAP->getRevisionState().isEqual(iRevisionId, bShowRevisions, isMarkRevisions()))
	{
		// the revision has a valid index to an inflated AP, so we use it
		bHiddenRevision = pAP->getRevisionHidden();

		const gchar* pRevision = NULL;

		if(bRevisionAttrNeeded && pAP->getAttribute("revision", pRevision))
		{
			*pRevisions = new PP_RevisionAttr(pRevision);
			UT_return_val_if_fail(pRevisions, false);
		}

		PT_AttrPropIndex revAPI = pAP->getRevisedIndex();

		getAttrProp(revAPI, ppAP);
		return true;
	}
	
	const PP_AttrProp * pNewAP = explodeRevisions(pRevAttr, pAP, bShowRevisions, iRevisionId, bHiddenRevision);

	if(pNewAP)
	{
		*ppAP = pNewAP;
	}
	else
	{
		*ppAP = pAP;
	}
	
	if(bRevisionAttrNeeded)
	{
		*pRevisions = pRevAttr;
	}
	else
	{
		delete pRevAttr;
	}
	
	return true;
}


/*!
    retrieves span AP corresponding to revision settings

    pRevisions : [out] the representation of the rev. attribute associated with the AP; if
    the caller does not need this, the pointer can be set to null
*/
bool PD_Document::getSpanAttrProp(pf_Frag_Strux* sdh, UT_uint32 offset, bool bLeftSide,
								  const PP_AttrProp ** ppAP,
								  PP_RevisionAttr ** pRevisions,
								  bool bShowRevisions, UT_uint32 iRevisionId,
								  bool &bHiddenRevision) const
{
	const PP_AttrProp *pAP = NULL;
	bool bRevisionAttrNeeded = pRevisions ? true : false;
	PP_RevisionAttr * pRevAttr = NULL;
	
	if(!getSpanAttrProp(sdh,offset,bLeftSide,&pAP))
		return false;

	if(   pAP->getRevisedIndex() != 0xffffffff
	   && pAP->getRevisionState().isEqual(iRevisionId, bShowRevisions, isMarkRevisions()))
	{
		// the revision has a valid index to an inflated AP, so we use it
		bHiddenRevision = pAP->getRevisionHidden();

		const gchar* pRevision = NULL;

		// only do this if the pRevisions pointer is set to NULL
		if(bRevisionAttrNeeded && pAP->getAttribute("revision", pRevision))
		{
			*pRevisions = new PP_RevisionAttr(pRevision);
			UT_return_val_if_fail(pRevisions, false);
		}
	
		PT_AttrPropIndex revAPI = pAP->getRevisedIndex();

		getAttrProp(revAPI, ppAP);
		return true;
	}
	
	const PP_AttrProp * pNewAP = explodeRevisions(pRevAttr, pAP, bShowRevisions, iRevisionId, bHiddenRevision);

	if(pNewAP)
	{
		*ppAP = pNewAP;
	}
	else
	{
		*ppAP = pAP;
	}

	if(bRevisionAttrNeeded)
	{
		*pRevisions = pRevAttr;
	}
	else
	{
		delete pRevAttr;
	}
	
	
	return true;
}

void PD_Document::_clearUndo()
{
	UT_return_if_fail(m_pPieceTable);
	m_pPieceTable->clearUndo();
}
	
void PD_Document::tellPTDoNotTweakPosition(bool b)
{
	UT_return_if_fail( m_pPieceTable );
	m_pPieceTable->setDoNotTweakPosition(b);
}

UT_uint32 PD_Document::getXID() const
{
	return m_pPieceTable->getXID();
}

UT_uint32 PD_Document::getTopXID() const
{
	return m_pPieceTable->getTopXID();
}

void PD_Document::fixMissingXIDs()
{
	m_pPieceTable->fixMissingXIDs();
}

/*!
    This function evaluates the xid value for the given frament and version level.

    The XID is a document-unique identifier of the frag; when we compare documents, we are
    interested not in document uniqueness but global uniqueness. We convert the
    document-unique xid to a globaly unique id by combining the xid with the UUID of
    document version: identical xid's in two documents represent identical elements if,
    and only if, the version UUIDs for the version of the document in which the element
    was created are identical. Therefore, as a part of the version record, we store the
    highest xid used in the document. This way we can determine in which version of the
    document the frag was created, based on its xid.

    Frags that have xid aboved the version threshold need to be treated as frags without xid.
*/
UT_uint32 PD_Document::getFragXIDforVersion(const pf_Frag * pf, UT_uint32 iVersion) const
{
	UT_return_val_if_fail( pf, 0 );

	if(iVersion >= getDocVersion())
	{
		// all xid's valid
		return pf->getXID();
	}
	
	const AD_VersionData * v = findHistoryRecord(iVersion);

	if(!v)
	{
		// if there is no version record for this version, find the nearest lower version
		for(UT_sint32 i = (UT_sint32)iVersion - 1; i > 0; --i)
		{
			v = findHistoryRecord(i);
			if(v)
				break;
		}

		if(!v)
			return 0;
	}
	

	UT_uint32 iXid = pf->getXID();

	if(iXid <= v->getTopXID())
		return iXid;

	// this frag's xid is above the version limit, i.e., this frag was inserted in a later
	// version of the document, and its xid cannot be used in document matching for the
	// given version level
	return 0;
}

#include <sstream>

PD_DocumentRDFHandle PD_Document::getDocumentRDF(void) const
{
    return m_hDocumentRDF;
}

PD_XMLIDCreatorHandle
PD_Document::makeXMLIDCreator()
{
    PD_XMLIDCreatorHandle ret( new PD_XMLIDCreator( this ));
    return ret;
}


class PD_XMLIDCreatorPrivate
{
public:
    std::set< std::string > m_cache;
    bool m_cacheIsVirgin;
};



PD_XMLIDCreator::PD_XMLIDCreator( PD_Document* doc )
    : m_doc( doc )
    , m_impl( new PD_XMLIDCreatorPrivate() )
{
    //
    // delay calling rebuildCache() for the first time until
    // the caller actually needs to use our methods.
    // No use = No cost.
    //
    m_impl->m_cacheIsVirgin = true;
}

PD_XMLIDCreator::~PD_XMLIDCreator()
{
    delete m_impl;
}

    

void
PD_XMLIDCreator::rebuildCache()
{
    m_impl->m_cacheIsVirgin = false;
    std::set< std::string >& m_cache = m_impl->m_cache;
    
    m_cache.clear();

    //
    // walk the document and grab all the xmlid values
    //
    if( m_doc )
    {
        pt_PieceTable* m_pPieceTable = m_doc->getPieceTable();
        
        pf_Frag * pf = NULL;
        pf = m_pPieceTable->getFragments().getFirst();
        while(pf)
        {
            PT_AttrPropIndex api = pf->getIndexAP();
            const PP_AttrProp* pAP = 0;
            const gchar * v = 0;
            
            if( m_doc->getAttrProp( api, &pAP ))
            {
                if( pAP->getAttribute(PT_XMLID, v) && v)
                {
                    m_cache.insert( v );
                }
            }
            
            
            pf = pf->getNext();
        }
    }

	UT_DEBUGMSG(("PD_XMLIDCreator::rebuildCache() cache.sz:%lu \n", (long unsigned)m_cache.size() ));
    
}

// msvc doesn't like this
// template <class STREAM>
// STREAM& operator<<( STREAM& oss, const UT_UTF8String& s )
// {
//     oss << s.utf8_str();
//     return oss;
// }

std::string
PD_XMLIDCreator::createUniqueXMLID( const std::string& desiredID, bool deepCopyRDF )
{
   
    if( m_impl->m_cacheIsVirgin )
        rebuildCache();
    
    std::set< std::string >& m_cache = m_impl->m_cache;
    UT_DEBUGMSG(("createUniqueXMLID() desired:%s\n", desiredID.c_str() ));
    
    // It is not in use already, let them have their choice.
    if( !m_cache.count( desiredID ) )
    {
        UT_DEBUGMSG(("createUniqueXMLID() xmlid is not in use, returning desired:%s\n", desiredID.c_str() ));
        m_cache.insert( desiredID );
        return desiredID;
    }

    UT_DEBUGMSG(("createUniqueXMLID() xmlid is in use! desired:%s\n", desiredID.c_str() ));
	UT_UUID* uuido = XAP_App::getApp()->getUUIDGenerator()->createUUID();
	std::string uuid;
	uuido->toString(uuid);
    delete uuido;

    std::string trimmedID = desiredID;
    
    //
    // Check to see if desiredID is already an ID which has x-ID-uuid
    // and if so, remove the old uuid so that we do not end up making
    // hugely long xml:id strings as copy and paste is repeated.
    //
    if( starts_with( desiredID, "x-" )
        && std::count( desiredID.begin(), desiredID.end(), '-' ) > 2 )
    {
        const int preambleLength = 2;
        int epos  = desiredID.find( '-', preambleLength );
        trimmedID = desiredID.substr( preambleLength, epos-preambleLength );
        UT_DEBUGMSG(("createUniqueXMLID() epos:%d trimmedID:%s desired:%s\n",
                     epos, trimmedID.c_str(), desiredID.c_str() ));
    }
    
    std::stringstream ss;
    ss << "x-" << trimmedID << "-" << uuid;
    std::string xmlid = ss.str();
    m_cache.insert( xmlid );
    UT_DEBUGMSG(("createUniqueXMLID() xmlid is in use! updated:%s\n", xmlid.c_str() ));

    // link RDF from the desired xml:id to the new xml:id
    m_doc->getDocumentRDF()->relinkRDFToNewXMLID( desiredID, xmlid, deepCopyRDF );
    
    return xmlid;
}


