/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
 * Copyright (c) 2016 Hubert Figuière
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


#ifndef PD_DOCUMENT_H
#define PD_DOCUMENT_H

#include <stdio.h>
#include <string>
#include <vector>
#include <map>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_hash.h"
#include "xad_Document.h"
#include "ut_xml.h"
#include "pt_Types.h"
#include "pl_Listener.h"
#include "pf_Frag.h"
#include "ie_FileInfo.h"
#include "fp_PageSize.h"
#include "ut_string_class.h"
#include "ut_misc.h"
#include "px_ChangeRecord.h"
#include "pp_Property.h"

#include <gsf/gsf-input.h>
#include <gsf/gsf-output.h>

#include <list>
#include <memory>

class UT_ByteBuf;
class UT_GrowBuf;
class pt_PieceTable;
class PP_AttrProp;
class PP_Revision;
class PP_RevisionAttr;
class pf_Frag_Strux;
class pf_Frag_Object;
class PX_ChangeRecord;
class PD_Style;
class PD_DocIterator;
class fd_Field;
class po_Bookmark;
class fl_AutoNum;
class fl_BlockLayout;
class fp_Run;
class UT_UTF8String;
class pp_Author;
class PD_DocumentRDF;
class AP_StatusBar;
class PL_ListenerCoupleCloser;

typedef std::shared_ptr<PD_DocumentRDF> PD_DocumentRDFHandle;

#ifdef PT_TEST
#include "ut_test.h"
#endif

enum
{
	PD_SIGNAL_UPDATE_LAYOUT = 0,
	PD_SIGNAL_REFORMAT_LAYOUT,
	PD_SIGNAL_DOCPROPS_CHANGED_REBUILD,
	PD_SIGNAL_DOCPROPS_CHANGED_NO_REBUILD,
	PD_SIGNAL_REVISION_MODE_CHANGED,
	PD_SIGNAL_DOCNAME_CHANGED,
	PD_SIGNAL_DOCDIRTY_CHANGED,
	PD_SIGNAL_SAVEDOC,
	PD_SIGNAL_DOCCLOSED // TODO: remove this
};

/////////////////////////////////////////////////////////
// proper prefixes for the namespaces that we support
/////////////////////////////////////////////////////////

// Dublin Core Namespace (http://dublincore.org/documents/dces/)
#define DC_META_PREFIX      "dc."

// AbiWord Namespace
#define ABIWORD_META_PREFIX "abiword."

// User-defined custom namespace
#define CUSTOM_META_PREFIX  "custom."

/////////////////////////////////////////////////////////
// The key names for our in-house metadata
/////////////////////////////////////////////////////////

// A formal name given to the resource
#define PD_META_KEY_TITLE        "dc.title"

// An entity primarily responsible for making the content of the resource
// typically a person, organization, or service
#define PD_META_KEY_CREATOR      "dc.creator"

// The topic of the content of the resource, *typically* including keywords
#define PD_META_KEY_SUBJECT      "dc.subject"

// An account of the content of the resource
#define PD_META_KEY_DESCRIPTION  "dc.description"

// An entity responsible for making the resource available
#define PD_META_KEY_PUBLISHER    "dc.publisher"

// An entity responsible for making contributions to the content of the resource
#define PD_META_KEY_CONTRIBUTOR  "dc.contributor"

// A date associated with an event in the life cycle of the resource
#define PD_META_KEY_DATE         "dc.date"

// The nature or genre of the content of the resource
// See http://dublincore.org/documents/dcmi-type-vocabulary/
#define PD_META_KEY_TYPE         "dc.type"

// The physical or digital manifestation of the resource. mime-type-ish
// always application/x-abiword
#define PD_META_KEY_FORMAT       "dc.format"

// A Reference to a resource from which the present resource is derived
#define PD_META_KEY_SOURCE       "dc.source"

// A language of the intellectual content of the resource
#define PD_META_KEY_LANGUAGE     "dc.language"

// A reference to a related resource (see-also)
#define PD_META_KEY_RELATION     "dc.relation"

// The extent or scope of the content of the resource
// spatial location, temporal period, or jurisdiction
#define PD_META_KEY_COVERAGE     "dc.coverage"

// Information about rights held in and over the resource
#define PD_META_KEY_RIGHTS       "dc.rights"

/////////////////////////////////////////////////////////
// abiword extensions to the dublin core element set
/////////////////////////////////////////////////////////

// searchable, indexable keywords
#define PD_META_KEY_KEYWORDS          "abiword.keywords"

// the last time this document was saved
#define PD_META_KEY_DATE_LAST_CHANGED "abiword.date_last_changed"

// the creator (product) of this document. AbiWord, KWord, etc...
#define PD_META_KEY_GENERATOR         "abiword.generator"

class ABI_EXPORT  ImagePage
{
public:
	ImagePage(UT_UTF8String & sImageId, UT_sint32 iPage, double xInch, double yInch, const char * pzProps);
	virtual ~ImagePage(void);
	const UT_UTF8String * getImageId(void) const;
	UT_sint32 getPageNo(void) const;
	double getXInch(void) const;
	double getYInch(void) const;
	const	UT_UTF8String * getProps(void) const;

private:
	UT_UTF8String m_sImageId;
	UT_sint32 m_iPage;
	double m_xInch;
	double m_yInch;
	UT_UTF8String m_sProps;
};

/*!
 * Helpder class to import Page Referenced TextBoxes
 */
class ABI_EXPORT TextboxPage
{
public:
	TextboxPage(UT_sint32 iPage, double xInch, double yInch,const char * pzProps, UT_ByteBuf & sContent);
	virtual ~TextboxPage(void);
	const UT_ByteBuf * getContent(void) const;
	UT_sint32 getPageNo(void) const;
	double getXInch(void) const;
	double getYInch(void) const;
	const UT_UTF8String * getProps(void) const;
private:
	UT_sint32 m_iPage;
	double m_xInch;
	double m_yInch;
	UT_UTF8String m_sProps;
	UT_ByteBuf m_sContent;
};

class PD_XMLIDCreatorPrivate;

class ABI_EXPORT PD_XMLIDCreator
{
    friend class PD_Document;
    PD_Document* m_doc;
    PD_XMLIDCreatorPrivate* m_impl;

  protected:

    PD_XMLIDCreator( PD_Document* doc );
    void rebuildCache();

  public:

    ~PD_XMLIDCreator();
    std::string createUniqueXMLID( const std::string& desiredID, bool deepCopyRDF = false );
};
typedef std::shared_ptr<PD_XMLIDCreator> PD_XMLIDCreatorHandle;


/*!
 PD_Document is the representation for a document.
*/

class ABI_EXPORT PD_Document : public AD_Document
{
public:
	PD_Document();

	virtual AD_DOCUMENT_TYPE getType() const {return ADDOCUMENT_ABIWORD;}

	virtual UT_Error		readFromFile(const char * szFilename, int ieft, const char * impProps = NULL);
	virtual UT_Error		importFile(const char * szFilename, int ieft, bool markClean = false, bool bImportStylesFirst = true,
									   const char * impProps = NULL);
	UT_Error		readFromFile(GsfInput *input, int ieft, const char * impProps = NULL);
	UT_Error		importFile(GsfInput *input, int ieft, bool markClean = false, bool bImportStylesFirst = true,
							   const char * impProps = NULL);
	virtual UT_Error		importStyles(const char * szFilename, int ieft, bool bDocProps = false);
	AP_StatusBar *          getStatusBar(void);
	void                    updateStatus(void);
	virtual UT_Error		newDocument(void);

	UT_Error  		saveAs(GsfOutput * output, int ieft, bool cpy = false, const char * expProps = NULL);


	UT_Error                createRawDocument(void);
	void                    finishRawCreation(void);

	virtual bool			isDirty(void) const;
	virtual void            forceDirty();
	bool                    isConnected(void);
	virtual bool			canDo(bool bUndo) const;
	virtual UT_uint32		undoCount(bool bUndo) const;
	virtual bool			undoCmd(UT_uint32 repeatCount);
	virtual bool			redoCmd(UT_uint32 repeatCount);
	bool                    isDoingTheDo(void) const;

	// Caret Methods

	void                    removeCaret(const std::string& sCaretID);

	// Page Referenced methods

	void                    addPageReferencedImage(UT_UTF8String & sImageId, UT_sint32 iPage, double xInch, double yInch, const char * pzProps);
	void                    addPageReferencedTextbox(UT_ByteBuf & sContent,UT_sint32 iPage, double xInch, double yInch,const char * pzProps);
	ImagePage *             getNthImagePage(UT_sint32 iImagePage);
	TextboxPage *           getNthTextboxPage(UT_sint32 iTextboxPage);
	void                    clearAllPendingObjects(void);

	// Author methods

	void                    setShowAuthors(bool bAuthors);
	bool                    isShowAuthors(void) const
	{ return m_bShowAuthors;}
	bool                    isExportAuthorAtts(void) const;
	void                    setExportAuthorAtts(bool bExport);
	UT_GenericVector<pp_Author*>& getAuthors()
	{ return m_vecAuthors; }
	UT_sint32               getMyAuthorInt(void) const;
	void                    setMyAuthorInt(UT_sint32 iAuthor);
	UT_sint32               getLastAuthorInt(void) const;
	UT_sint32               getNumAuthors(void) const;
	pp_Author *             getNthAuthor(UT_sint32 i) const;
	pp_Author *             getAuthorByInt(UT_sint32 i) const;
	pp_Author *             addAuthor(UT_sint32 iAuthor);
	bool                    sendAddAuthorCR(pp_Author * pAuthor);
	bool                    sendChangeAuthorCR(pp_Author * pAuthor);
	UT_sint32               findFirstFreeAuthorInt(void) const;
 private:
	bool                    addAuthorAttributeIfBlank(PP_PropertyVector & atts);
	bool                    addAuthorAttributeIfBlank( PP_AttrProp *&p_AttrProp);
	bool                    _buildAuthorProps(pp_Author * pAuthor, PP_PropertyVector & props);
 public:
	//
	void					beginUserAtomicGlob(void);
	void					endUserAtomicGlob(void);
	void                    setMarginChangeOnly(bool b);
	bool                    isMarginChangeOnly(void) const;
	bool                    changeObjectFormatNoUpdate(PTChangeFmt ptc, pf_Frag_Object* odh, const PP_PropertyVector & attributes,const PP_PropertyVector & properties );
PT_AttrPropIndex            getAPIFromSOH(pf_Frag_Object* odh);
	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const PP_PropertyVector & attributes,
										 const PP_PropertyVector & properties);
	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const PP_PropertyVector & attributes,
										 const PP_PropertyVector & properties, fd_Field ** pField );

	bool					insertSpan(PT_DocPosition dpos,
									   const UT_UCSChar * p,
									   UT_uint32 length,
									   PP_AttrProp *p_AttrProp = NULL,
									   UT_uint32 * insertedSpanLength = NULL);
	bool					insertSpan(PT_DocPosition dpos,
									   const std::string& s,
									   PP_AttrProp *p_AttrProp = NULL);

	bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   PP_AttrProp *p_AttrProp_Before,
									   UT_uint32 &iRealDeleteCount,
									   bool bDeleteTableStruxes = false);

	bool                    deleteFmtMark( PT_DocPosition dpos);

	bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  PT_DocPosition dpos2,
										  const PP_PropertyVector & attributes,
										  const PP_PropertyVector & properties);

	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts, pf_Frag_Strux ** ppfs_ret = 0);
	bool					deleteStrux(PT_DocPosition dpos,
										PTStruxType pts,
										bool bRecordChange);

	bool                    createAndSendCR(PT_DocPosition dpos,UT_sint32 iType,bool bsave,UT_Byte iGlob);
	// XXX PP_PropertyVector is not visible in xad_Document
	virtual bool            createAndSendDocPropCR( const gchar ** pAtts,const gchar ** pProps );
	bool                    createAndSendDocPropCR(const PP_PropertyVector & pAtts,const PP_PropertyVector & pProps);
	bool                    changeDocPropeties(const PP_PropertyVector & szAtts, const PP_PropertyVector & pProps);

	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts,
										const PP_PropertyVector & attributes,
										const PP_PropertyVector & properties,
										pf_Frag_Strux ** ppfs_ret = NULL);

	void                    deleteHdrFtrStrux(pf_Frag_Strux* sdh);

	bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const PP_PropertyVector & attributes,
										   const PP_PropertyVector & properties,
										   PTStruxType pts);

	bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const PP_PropertyVector & attributes,
										   const PP_PropertyVector & properties);


	bool					changeStruxFmtNoUndo(PTChangeFmt ptc,
										   pf_Frag_Strux* sdh,
										   const PP_PropertyVector & attributes,
										   const PP_PropertyVector & properties);

	bool					changeStruxForLists(pf_Frag_Strux* sdh,
												const char * pszParentID);

	bool					insertFmtMark(PTChangeFmt ptc,
										  PT_DocPosition dpos,
										  PP_AttrProp *p_AttrProp);

	bool                    changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pts,
													 const PP_PropertyVector & attrs, const PP_PropertyVector & props,
													 bool bSkipEmbededSections);

	bool                    changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pts,
													 const PP_PropertyVector & attrs, const std::string & props,
													 bool bSkipEmbededSections);

	// the append- and insertBeforeFrag methods are only available while importing
	// the document.

	bool					appendStrux(PTStruxType pts, const PP_PropertyVector & attributes, pf_Frag_Strux ** ppfs_ret = 0);
	bool					appendStruxFmt(pf_Frag_Strux * pfs, const PP_PropertyVector & attributes);
	bool                    appendLastStruxFmt(PTStruxType pts, const PP_PropertyVector & attrs, const PP_PropertyVector & props,
											   bool bSkipEmbededSections);
	bool                    appendLastStruxFmt(PTStruxType pts, const PP_PropertyVector & attrs, const std::string & props,
											   bool bSkipEmbededSections);
	bool					appendFmt(const PP_PropertyVector & vecAttributes);
	bool					appendSpan(const UT_UCSChar * p, UT_uint32 length);
	bool					appendObject(PTObjectType pto, const PP_PropertyVector & attributes);
	bool					appendFmtMark(void);
	bool					appendStyle(const PP_PropertyVector & attributes);
	bool                    changeStruxFormatNoUpdate(PTChangeFmt ptc, pf_Frag_Strux* sdh, const PP_PropertyVector & attributes);
	bool					insertStruxBeforeFrag(pf_Frag * pF, PTStruxType pts,
												  const PP_PropertyVector & attributes, pf_Frag_Strux ** ppfs_ret = 0);
	bool					insertSpanBeforeFrag(pf_Frag * pF, const UT_UCSChar * p, UT_uint32 length);
	bool					insertObjectBeforeFrag(pf_Frag * pF, PTObjectType pto,
												   const PP_PropertyVector & attributes);
	bool					insertFmtMarkBeforeFrag(pf_Frag * pF);
	bool					insertFmtMarkBeforeFrag(pf_Frag * pF, const PP_PropertyVector & attributes);

	pf_Frag *               findFragOfType(pf_Frag::PFType iType, UT_sint32 iSubtype = -1,
										   pf_Frag * pfStart = NULL) const;
	pf_Frag *               getLastFrag() const;
	bool                    checkForSuspect(void);
	bool                    repairDoc(void);
	bool                    removeStyle(const gchar * name);
	bool					tellListener(PL_Listener * pListener);
	bool					tellListenerSubset( PL_Listener * pListener,
                                                PD_DocumentRange * pDocRange,
                                                PL_ListenerCoupleCloser* closer = 0 );
	bool					addListener(PL_Listener * pListener, PL_ListenerId * pListenerId);
	bool					removeListener(PL_ListenerId listenerId);
	bool					signalListeners(UT_uint32 iSignal) const;
	bool					notifyListeners(const pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const;
	bool					notifyListeners(const pf_Frag_Strux * pfs,
											pf_Frag_Strux * pfsNew,
											const PX_ChangeRecord * pcr) const;
	void					deferNotifications(void);
	void					processDeferredNotifications(void);
	UT_sint32               getAdjustmentForCR(const PX_ChangeRecord * pcr) const;

	// the first two of these functions just retrieve the AP with the given index; the latter two
	// return AP that represents state of things with current revision settings
	bool					getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const;
	bool					getSpanAttrProp(pf_Frag_Strux* sdh, UT_uint32 offset, bool bLeftSide,
											const PP_AttrProp ** ppAP) const;

	bool                    getAttrProp(PT_AttrPropIndex apIndx, const PP_AttrProp ** ppAP, PP_RevisionAttr ** pRevisions,
										bool bShowRevisions, UT_uint32 iRevisionId, bool &bHiddenRevision) const;

	bool                    getSpanAttrProp(pf_Frag_Strux* sdh, UT_uint32 offset, bool bLeftSide,
											const PP_AttrProp ** ppAP,
											PP_RevisionAttr ** pRevisions,
											bool bShowRevisions, UT_uint32 iRevisionId,
											bool &bHiddenRevision) const;

	const UT_UCSChar *		getPointer(PT_BufIndex bi) const; /* see warning on this function */
	bool					getBlockBuf(pf_Frag_Strux* sdh, UT_GrowBuf * pgb) const;

	bool					getBounds(bool bEnd, PT_DocPosition & docPos) const;
	PTStruxType             getStruxType(pf_Frag_Strux* sdh) const;
	PT_DocPosition			getStruxPosition(pf_Frag_Strux* sdh) const;
	bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 fl_ContainerLayout* * psfh) const;
	bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   fl_ContainerLayout* * psfh) const;
	bool					getStruxOfTypeFromPosition(PT_DocPosition, PTStruxType pts, pf_Frag_Strux* * sdh) const;
    pf_Frag_Strux*       getBlockFromPosition( PT_DocPosition pos ) const;

	pf_Frag *				getFragFromPosition(PT_DocPosition docPos) const;

	bool					getNextStruxOfType(pf_Frag_Strux* sdh,PTStruxType pts,
											   pf_Frag_Strux* * nextsdh);
	bool					getPrevStruxOfType(pf_Frag_Strux* sdh,PTStruxType pts,
											   pf_Frag_Strux* * prevsdh);
	bool                    getNextStrux(pf_Frag_Strux* sdh, pf_Frag_Strux* *nextSDH);

	// data items

	virtual bool			createDataItem(const char * szName, bool bBase64,
                                           const UT_ByteBuf * pByteBuf,
										   const std::string & mime_type,
                                           PD_DataItemHandle* ppHandle);
	virtual bool            replaceDataItem(const char * szName, const UT_ByteBuf * pByteBuf);
	virtual bool			getDataItemDataByName(const char * szName,
												  const UT_ByteBuf ** ppByteBuf,
                                                  std::string* pMimeType,
                                                  PD_DataItemHandle* ppHandle) const;
	bool					setDataItemToken(PD_DataItemHandle pHandle, void* pToken) const;
	bool					getDataItemData(PD_DataItemHandle pHandle,
											const char ** pszName, const UT_ByteBuf ** ppByteBuf,
											const void** ppToken) const;
	bool					getDataItemFileExtension(const char *szDataID, std::string &sExt, bool bDot = true) const;
	bool					enumDataItems(UT_uint32 k,
										  PD_DataItemHandle* ppHandle, const char ** pszName,
										  const UT_ByteBuf ** ppByteBuf, std::string * pMimeType) const;

    pf_Frag_Strux*       findHdrFtrStrux(const gchar * pszHdtFtr,
											const gchar * pszHdrFtrID);
	bool                    verifySectionID(const gchar * pszId);
	const pf_Frag_Strux*       getLastSectionSDH(void) const;
	pf_Frag_Strux*       getLastSectionMutableSDH(void);
	pf_Frag_Strux*       getLastStruxOfType(PTStruxType pts);

	bool                    changeStruxAttsNoUpdate(pf_Frag_Strux* sdh, const char * attr, const char * attvalue);
	bool                    deleteStruxNoUpdate(pf_Frag_Strux* sdh);
	bool                    deleteFragNoUpdate(pf_Frag * pf);
	bool                    insertStruxNoUpdateBefore(pf_Frag_Strux* sdh, PTStruxType pts, const PP_PropertyVector & attributes );
	bool                    isStruxBeforeThis(pf_Frag_Strux* sdh,  PTStruxType pts);

	// the function below does exactly what the name says -- returns the AP index; in revisions mode
	// you would need the AP at that index to have revision attribute exploded -- use one of the
	// functions below to retrieve props and attrs correctly reflecting revisions settings
	PT_AttrPropIndex        getAPIFromSDH(pf_Frag_Strux* sdh);
    bool                    getAttributeFromSDH(pf_Frag_Strux* sdh, bool bShowRevisions, UT_uint32 iRevisionLevel,
												const char * szAttribute, const char ** pszValue);

    bool                    getPropertyFromSDH(const pf_Frag_Strux* sdh, bool bShowRevisions, UT_uint32 iRevisionLevel,
											   const char * szProperty, const char ** pszValue) const;
	// styles
	void                    getAllUsedStyles(UT_GenericVector<PD_Style*> * pVecStyles);
	fl_ContainerLayout*       getNthFmtHandle(pf_Frag_Strux* sdh, UT_uint32 n);
    const char *            getDefaultStyle() const;
	bool					getStyle(const char * szName, PD_Style ** ppStyle) const;
	PD_Style *				getStyleFromSDH(pf_Frag_Strux* sdh);
	pf_Frag_Strux*       getPrevNumberedHeadingStyle(pf_Frag_Strux* sdh);
	size_t                  getStyleCount(void);
	bool					enumStyles(UT_uint32 k,
									   const char ** pszName, const PD_Style ** ppStyle) const;
	bool                    enumStyles(UT_GenericVector<PD_Style*> * & pStyles) const;
	bool					getStyleProperty(const gchar * szStyleName, const gchar * szPropertyName, const gchar *& szPropertyValue);
	bool					addStyleProperty(const gchar * szStyleName, const gchar * szPropertyName, const gchar * szPropertyValue);
	bool					addStyleProperties(const gchar * szStyleName, const PP_PropertyVector & pProperties);
	bool	                setAllStyleAttributes(const gchar * szStyleName, const PP_PropertyVector & pAttribs);
	bool	                addStyleAttributes(const gchar * szStyleName, const PP_PropertyVector & pAttribs);

    pf_Frag_Strux*       findPreviousStyleStrux(const gchar * szStyle, PT_DocPosition pos);
    pf_Frag_Strux*       findForwardStyleStrux(const gchar * szStyle, PT_DocPosition pos);
	bool					updateDocForStyleChange(const gchar * szStyleName,
													bool isParaStyle);
	void                    updateAllLayoutsInDoc( pf_Frag_Strux* sdh);
	void					clearIfAtFmtMark(PT_DocPosition dpos);

	virtual UT_uint32		getLastSavedAsType() const { return m_lastSavedAsType; }
	UT_uint32				getLastOpenedType() const { return m_lastOpenedType; }
	bool					updateFields(void);
	bool					getField(pf_Frag_Strux* sdh,
									 UT_uint32 offset,
                                     fd_Field * &pField);
	po_Bookmark * 			getBookmark(pf_Frag_Strux* sdh, UT_uint32 offset);
	pf_Frag *               findBookmark(const char * pName, bool bEnd = false, pf_Frag * pfStart = NULL);
	bool                    hasMath(void);

	void					setDontChangeInsPoint(void);
	void					allowChangeInsPoint(void);
	bool					getAllowChangeInsPoint(void) const;
	// Footnote functions
	bool                    isFootnoteAtPos(PT_DocPosition pos);
	bool                    isEndFootnoteAtPos(PT_DocPosition pos);
	UT_sint32               getEmbeddedOffset(pf_Frag_Strux* sdh,PT_DocPosition posOff, pf_Frag_Strux* & sdhEmbedded);
	bool                    hasEmbedStruxOfTypeInRange(PT_DocPosition posStart, PT_DocPosition posEnd, 
													   PTStruxType iType) const;

	// TOC functions
	bool                    isTOCAtPos(PT_DocPosition pos);

	// FRAME function
	bool                    isFrameAtPos(PT_DocPosition pos);
	bool                    isEndFrameAtPos(PT_DocPosition pos);
	bool                    isHdrFtrAtPos(PT_DocPosition pos);
	bool                    isSectionAtPos(PT_DocPosition pos);
	bool                    isBlockAtPos(PT_DocPosition pos);

// Table functions

	bool                    isTableAtPos(PT_DocPosition pos);
	bool                    isEndTableAtPos(PT_DocPosition pos);
	bool                    isCellAtPos(PT_DocPosition pos);
	pf_Frag_Strux*       getEndTableStruxFromTableSDH(pf_Frag_Strux* tableSDH);
	pf_Frag_Strux*       getEndCellStruxFromCellSDH(pf_Frag_Strux* cellSDH);
	pf_Frag_Strux*       getEndTableStruxFromTablePos(PT_DocPosition posTable);
	bool                    getRowsColsFromTableSDH(pf_Frag_Strux* tableSDH,
													bool bShowRevisions, UT_uint32 iRevisionLevel,
													UT_sint32 * numRows, UT_sint32 * numCols);
	pf_Frag_Strux*       getCellSDHFromRowCol(pf_Frag_Strux* tableSDH,
												 bool bShowRevisions, UT_uint32 iRevisionLevel,
												 UT_sint32 row,
												 UT_sint32 col);
	void                    miniDump(pf_Frag_Strux* sdh, UT_sint32 nstruxes);
    bool                    dumpDoc( const char* msg, PT_DocPosition currentpos, PT_DocPosition endpos );

	bool                    isInsertHyperLinkValid(PT_DocPosition pos) const;


	// List Functions
	fl_AutoNum *			getListByID(UT_uint32 id) const;
	fl_AutoNum *			getNthList(UT_uint32 i) const;
	bool					enumLists(UT_uint32 k, fl_AutoNum ** pAutoNum);
	UT_uint32				getListsCount(void) const;
	void					addList(fl_AutoNum * pAutoNum);
	bool					appendList(const PP_PropertyVector & attributes);
	bool					fixListHierarchy(void);
	void					removeList(fl_AutoNum * pAutoNum,pf_Frag_Strux* sdh );
	void					listUpdate(pf_Frag_Strux* sdh);
	void					StopList(pf_Frag_Strux* sdh);
	void					disableListUpdates(void);
	void					enableListUpdates(void);
	void					updateDirtyLists(void);
	bool					areListUpdatesAllowed(void);
	void                    setHasListStopped(bool bStop) {m_bHasListStopped = bStop;}
	bool                    hasListStopped(void) const {return m_bHasListStopped;}

	void					setDoingPaste(void);
	void					clearDoingPaste(void);
	bool					isDoingPaste(void);

	void                    setRedrawHappenning(bool bIsHappening) {m_bRedrawHappenning  = bIsHappening;}
	bool                    isRedrawHappenning(void) const {return m_bRedrawHappenning;}

	// PageSize functions
	bool                    convertPercentToInches(const char * szPercent, UT_UTF8String & sInches);
	bool					setPageSizeFromFile(const PP_PropertyVector & attributes);
	const	fp_PageSize	*	getPageSize(void) const
	{ return & m_docPageSize;}
	fp_PageSize             m_docPageSize; // Move this to private later
	bool					isBookmarkUnique(const gchar * pName) const;
	bool					isBookmarkRelativeLink(const gchar * pName) const;
	UT_sint32				getBookmarkCount() const {return m_vBookmarkNames.size();}
	const std::string & 	getNthBookmark(UT_sint32 n) const {return m_vBookmarkNames.at(n);}
	void					addBookmark(const gchar * pName);
	void					removeBookmark(const gchar * pName);

	/////////////////////////////////////////////////////////////////////////////
	// Functions for dealing with revisions
	//
	virtual void            setMarkRevisions(bool bMark);
	// primarly for use by the PieceTable
	void                    setMarkRevisionsNoNotify(bool bMark) {AD_Document::setMarkRevisions(bMark);}

	virtual bool            acceptRejectRevision(bool bReject,
												 UT_uint32 iStart,
												 UT_uint32 iEnd,
												 UT_uint32 iLevel);

	virtual bool            rejectAllHigherRevisions(UT_uint32 iLevel);
	virtual bool            acceptAllRevisions();

	const PP_AttrProp *     explodeRevisions(PP_RevisionAttr *& pRevisions, const PP_AttrProp * pAP,
											 bool bShow, UT_uint32 iId, bool &bHiddenRevision) const;

	virtual void            purgeRevisionTable(bool bUnconditional = false);

	void					notifyPieceTableChangeStart(void);
	void					notifyPieceTableChangeEnd(void);



	pt_PieceTable *			getPieceTable(void) const
		{ return m_pPieceTable; }
    PD_DocumentRDFHandle    getDocumentRDF(void) const;
    PD_XMLIDCreatorHandle   makeXMLIDCreator();

#ifdef PT_TEST
	void					__dump(FILE * fp) const;
	//! Pointer to last instatiated PD_Document. Used for debugging.
	static PD_Document*		m_pDoc;
#endif

	// If we're using styles to format a document, prevent accidental use of other formatting
        // tools.  Disable all explicit formatting tools (font, color, boldness, etc.)
	inline bool areStylesLocked () const { return m_bLockedStyles; }    // See also lockStyles
	void lockStyles(bool b);

	virtual void setMetaDataProp (const std::string & key, const std::string & value);
	virtual bool getMetaDataProp (const std::string & key, std::string & outProp) const;

	// RIVERA TODO not working and may not be needed
	virtual void setAnnotationProp (const std::string & key, const std::string & value);
	virtual bool getAnnotationProp (const std::string & key, std::string & outProp) const;

	const std::map<std::string,std::string> & getMetaData () const
	{
		return m_metaDataMap ;
	}

	// document-level property handling functions
	const PP_AttrProp *     getAttrProp() const;
	PT_AttrPropIndex        getAttrPropIndex() const {return m_indexAP;}
	bool                    setAttrProp(const PP_PropertyVector & ppAttr);
	bool                    setAttributes(const PP_PropertyVector & ppAttr);
	bool                    setProperties(const PP_PropertyVector & ppProps);
	void                     setDontImmediatelyLayout(bool b)
		{ m_bDontImmediatelyLayout = b;}
	bool                     isDontImmediateLayout(void) const
		{ return m_bDontImmediatelyLayout;}

	/* Okay, as far as I can tell this is a non-persistent document property since it is not
	 * written to the AbiWord file when the document is saved. In fact, it is only set if a
	 * mail-merge source/link is given on the command line.
	 *
	 * Mail merge fields are, naturally, saved and loaded, but the Insert->Mail Merge Field...
	 * dialog doesn't reflect the current document's fields but rather some internal set of
	 * fields, which is confusing if you are trying to work with muliple mail merge sources.
	 */

	std::string 	getMailMergeField(const std::string & key) const;
	bool			mailMergeFieldExists(const std::string & key) const;
	void			setMailMergeField(const std::string & key, const std::string & value);

	void			clearMailMergeMap();

	void setMailMergeLink (const char * file) {
		m_mailMergeLink = file;
	}

	const std::string & 							getMailMergeLink() const { return m_mailMergeLink; }
	const std::map<std::string, std::string> &	getMailMergeMap() const  { return m_mailMergeMap; }

	void invalidateCache(void);

	/*
	   The purpose of the following methods is to generate and manage
	   document-wide unique identifiers; the indetifiers are type
	   specific, with the types defined in UT_UniqueId class (ut_mics.h).

       UT_uint32 getUID(type):    Generates an id of a given type or
                                  UT_UID_INVALID if unique id cannot
                                  be generated (0 <= uid < UT_UID_INVALID).

       bool isIdUnique(type, id): Returns true if a given id can be
                                  used as a unique identifier of a
                                  given type; before the identifier is
                                  used, the caller should call
                                  setMinUID(id+1) to ensure integrity of
                                  the UID generator.

       bool setMinUID(type, uid): Allows to set a minimum value for all
                                  future identifiers; it returns true on
                                  success false on failure. The purpose is
                                  to allow an easy insertion of
                                  document-stored id's into the UID space
                                  by document importers: if the importer
                                  encounters a stored value ID it should
                                  call setMinUID(type, ID+1).
	*/

	UT_uint32 getUID(UT_UniqueId::idType t) {return m_UID.getUID(t);}
	bool      setMinUID(UT_UniqueId::idType t, UT_uint32 i) {return m_UID.setMinId(t,i);}
	bool      isIdUnique(UT_UniqueId::idType t, UT_uint32 i) const {return m_UID.isIdUnique(t,i);}

	virtual bool  areDocumentContentsEqual(const AD_Document &d, UT_uint32 &pos) const;
	virtual bool  areDocumentFormatsEqual(const AD_Document &d, UT_uint32 &pos) const;
	virtual bool  areDocumentStylesheetsEqual(const AD_Document &d) const;

	bool      findFirstDifferenceInContent(PT_DocPosition &pos, UT_sint32 &iOffset2,
										   const PD_Document &d) const;

	bool      findWhereSimilarityResumes(PT_DocPosition &pos, UT_sint32 &iOffset2,
										 UT_uint32 & iKnownLength,
										 const PD_Document &d) const;

	virtual void   setAutoRevisioning(bool autorev);

	virtual UT_uint32 getXID() const;
	virtual UT_uint32 getTopXID() const;
	void              fixMissingXIDs();
	UT_uint32         getFragXIDforVersion(const pf_Frag * pf, UT_uint32 iVersion) const;
	void              removeConnections(void);
	void              changeConnectedDocument(PD_Document * pDoc);
	UT_sint32     getNewHdrHeight(void) const
	{ return m_iNewHdrHeight;}
	UT_sint32     getNewFtrHeight(void) const
	{ return m_iNewFtrHeight;}

	void       setNewHdrHeight(UT_sint32 newHeight)
	{ m_iNewHdrHeight = newHeight;}
	void       setNewFtrHeight(UT_sint32 newHeight)
	{ m_iNewFtrHeight = newHeight;}

	bool                    purgeFmtMarks();

	void                    tellPTDoNotTweakPosition(bool b);

	void                    setVDNDinProgress(bool b) {m_bVDND = b;}
	bool                    isVDNDinProgress() const {return m_bVDND;}
	UT_sint32               getCRNumber() const { return m_iCRCounter; }
	void					setCRNumber(UT_sint32 iCRCounter) { m_iCRCounter = iCRCounter; }
	UT_sint32               getNextCRNumber(void);
    std::list<AV_View*>     getAllViews() const;
    void                    getAllViews(UT_GenericVector<AV_View *> * vecViews) const;
	void                    ignoreSignals(void)
	{ m_bIgnoreSignals = true;}
	void                    dontIgnoreSignals(void)
	{ m_bIgnoreSignals = false;}
	void                    setClean(void)
	{ _setClean();}

	void					setCoalescingMask(bool mask)
	{ m_bCoalescingMask = mask; }
	bool					isCoalescingMasked() const
	{ return m_bCoalescingMask; }
protected:
	virtual ~PD_Document();

	virtual UT_Error		_saveAs(const char * szFilename, int ieft, const char * expProps = NULL);
	virtual UT_Error   		_saveAs(const char * szFilename, int ieft, bool cpy, const char * expProps = NULL);
	virtual UT_Error        _saveAs(GsfOutput *output, int ieft, bool cpy, const char * expProps);
	virtual UT_Error		_save(void);


	void					_setClean(void);
	void					_destroyDataItemData(void);
	bool					_syncFileTypes(bool bReadSaveWriteOpen);

	bool                    _acceptRejectRevision(bool bReject, UT_uint32 iStart, UT_uint32 iEnd,
												  const PP_Revision * pRev,
												  PP_RevisionAttr &RevAttr, pf_Frag * pf,
												  bool & bDeleted);

	virtual void            _clearUndo();

	UT_Error _importFile(const char * szFilename, int ieft,
						 bool markClean, bool bImportStylesFirst,
						 bool isImportFile, const char* impProps);
	UT_Error _importFile(GsfInput * input, int ieft,
						 bool markClean, bool bImportStylesFirst,
						 bool bIsImportFile, const char* impProps);

	bool     _removeRepeatedHdrFtr(pf_Frag_Strux * pfs ,UT_GenericVector<pf_Frag_Strux *> * vecHdrFtrs, UT_sint32 i);
	bool     _pruneSectionAPI(pf_Frag_Strux * pfs,const char * szHType, UT_GenericVector<pf_Frag_Strux *> *vecHdrFtrs);
	bool     _matchSection(pf_Frag_Strux * pfs, UT_GenericVector<pf_Frag_Strux *> *vecSections);
	bool     _removeHdrFtr(pf_Frag_Strux * pfs);
	bool     _checkAndFixTable(pf_Frag_Strux * pfs);
public:
	// these functions allow us to retrieve visual direction at document
	// position pos from an associated layout. They are intended to be
	// used by exporters into (daft) formats where such information
	// might be required (e.g. RTF).
	bool                    exportGetVisDirectionAtPos(PT_DocPosition pos, UT_BidiCharType &type);
private:
	bool                    _exportInitVisDirection(PT_DocPosition pos);
	bool                    _exportFindVisDirectionRunAtPos(PT_DocPosition pos);

	bool                    _sendAuthorCR(const char *attrName,
                                          pp_Author *pAuthor);
private:
	bool					m_ballowListUpdates;
	pt_PieceTable *			m_pPieceTable;
    PD_DocumentRDFHandle    m_hDocumentRDF;
	UT_GenericVector<PL_Listener *> m_vecListeners;
	UT_GenericVector<fl_AutoNum *> m_vecLists;
	bool                    m_bHasListStopped;

	typedef std::map<std::string, PD_DataItemHandle> hash_data_items_t;
	hash_data_items_t m_hashDataItems;
public:
	IE_FileInfo				m_fileImpExpInfo;
private:
	IEFileType				m_lastOpenedType;
	IEFileType				m_lastSavedAsType;
	bool					m_bDoingPaste;
	bool					m_bAllowInsertPointChange;
	bool                    m_bRedrawHappenning;
	bool                    m_bLoading;
	std::vector<std::string> m_vBookmarkNames;
	bool                    m_bLockedStyles;
	std::map<std::string, std::string> m_metaDataMap;
	PT_AttrPropIndex        m_indexAP;
	bool                    m_bDontImmediatelyLayout;

	std::map<std::string, std::string>  m_mailMergeMap;

	UT_UCS4Char             m_iLastDirMarker;

	std::string             m_mailMergeLink;

	// these are for use with the export*VisDirection functions
	const fl_BlockLayout *  m_pVDBl;
	fp_Run *                m_pVDRun;
	PT_DocPosition          m_iVDLastPos;
	UT_UniqueId             m_UID;
	UT_sint32               m_iNewHdrHeight;
	UT_sint32               m_iNewFtrHeight;
	bool                    m_bMarginChangeOnly;
	UT_GenericVector<pf_Frag *> m_vecSuspectFrags;

	bool                    m_bVDND;
    UT_sint32               m_iCRCounter;
	mutable UT_sint32       m_iUpdateCount;
	bool                    m_bIgnoreSignals;

	bool					m_bCoalescingMask;
	bool                    m_bShowAuthors;
	UT_GenericVector<pp_Author *>  m_vecAuthors;
	bool                    m_bExportAuthorAtts;
	UT_sint32               m_iMyAuthorInt;
	UT_sint32               m_iLastAuthorInt;
	UT_GenericVector<ImagePage *> m_pPendingImagePage;
	UT_GenericVector<TextboxPage *> m_pPendingTextboxPage;
	UT_sint32               m_iStruxCount;
	std::string             m_sUserName;
public:
	const std::string &getUserName() const { return m_sUserName; }
};

#endif /* PD_DOCUMENT_H */
