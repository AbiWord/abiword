/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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


#ifndef PD_DOCUMENT_H
#define PD_DOCUMENT_H

// TODO should the filename be UT_UCSChar rather than char ?

#include <stdio.h>
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
#include <fribidi.h>

class UT_ByteBuf;
class UT_GrowBuf;
class pt_PieceTable;
class PP_AttrProp;
class pf_Frag_Strux;
class PX_ChangeRecord;
class PD_Style;
class XAP_App;
class fd_Field;
class po_Bookmark;
class fl_AutoNum;
class fl_BlockLayout;
class fp_Run;

#ifdef PT_TEST
#include "ut_test.h"
#endif

enum
{
	PD_SIGNAL_UPDATE_LAYOUT,
	PD_SIGNAL_REFORMAT_LAYOUT,
	PD_SIGNAL_DOCPROPS_CHANGED_REBUILD,
	PD_SIGNAL_DOCPROPS_CHANGED_NO_REBUILD
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

// a helper class for keeping track of revisions in the document
class PD_Revision
{
  public:
	PD_Revision(UT_uint32 iId, UT_UCS4Char * pDesc, time_t start)
		:m_iId(iId),m_pDescription(pDesc), m_tStart(start){};
	
	~PD_Revision(){delete [] m_pDescription;}
	
	UT_uint32         getId()const{return m_iId;}
	UT_UCS4Char *     getDescription() const {return m_pDescription;}

	// NB: getStartTime() == 0 should be interpreted as 'unknown'
	time_t            getStartTime() const {return m_tStart;}
	void              setStartTime(time_t t) {m_tStart = t;}

  private:
	UT_uint32     m_iId;
	UT_UCS4Char * m_pDescription;
	time_t        m_tStart;
};

// a helper class for history tracking
class PD_VersionData
{
  public:

	// constructor for importers
	PD_VersionData(UT_uint32 v, time_t t, UT_uint32 e, UT_uint32 uid)
		:m_iId(v),m_tTime(t), m_iEditTime(e), m_iUID(uid){};
	
	// constructor for new entries
	PD_VersionData(UT_uint32 v, time_t t, UT_uint32 e);

	// copy constructor
	PD_VersionData(const PD_VersionData & v)
		:m_iId(v.m_iId),m_tTime(v.m_tTime),m_iEditTime(v.m_iEditTime), m_iUID(v.m_iUID){};
	
	PD_VersionData & operator = (const PD_VersionData &v)
	{
		m_iId       = v.m_iId;
		m_tTime     = v.m_tTime;
		m_iEditTime = v.m_iEditTime;
		m_iUID      = v.m_iUID;
		return *this;
	}

	bool operator == (const PD_VersionData &v)
	{
		return (m_iId == v.m_iId && m_tTime == v.m_tTime && m_iUID == v.m_iUID);
	}

	UT_uint32   getId()const{return m_iId;}
	time_t      getTime()const {return m_tTime;}
	UT_uint32   getEditTime()const {return m_iEditTime;}
	UT_uint32   getUID()const {return m_iUID;}
	
	// we only allow modification of edit time ...
	void        setEditTime(UT_uint32 t) {m_iEditTime = t;}
	
  private:
	UT_uint32   m_iId;
	time_t      m_tTime;
	UT_uint32   m_iEditTime;
	UT_uint32   m_iUID;
};

// class for managing Documents UID
#define PD_DOCUID_SIZE 4
class PD_DocumentUID
{
  public:
	// constructor for new documents
	PD_DocumentUID();

	// constructor for importers
	PD_DocumentUID(const char * uid);
	
	bool operator == (const PD_DocumentUID & u)
	{
		for(UT_uint32 i = 0; i < PD_DOCUID_SIZE; ++i)
			if (m_iUID[i] != u.m_iUID[i])
				return false;

		return true;
	}

	const char * getUIDString() const {return m_sUID.c_str();}
	
  private:
	UT_uint32 m_iUID[PD_DOCUID_SIZE];
	UT_String m_sUID;
};

class PD_DocumentDiff
{
  public:
	PD_DocumentDiff(bool bDel, PT_DocPosition p1, PT_DocPosition p2, UT_uint32 len)
		:m_bDeleted(bDel), m_pos1(p1), m_pos2(p2), m_len(len){};

#ifdef DEBUG
	void _dump() const;
#endif
	
	bool           m_bDeleted;
	PT_DocPosition m_pos1;
	PT_DocPosition m_pos2;
	UT_uint32      m_len;
};



/*!
 PD_Document is the representation for a document.
*/

class ABI_EXPORT PD_Document : public AD_Document
{
public:
	PD_Document(XAP_App *pApp);

	virtual UT_Error		readFromFile(const char * szFilename, int ieft, const char * impProps = NULL);
	virtual UT_Error		importFile(const char * szFilename, int ieft, bool markClean = false, bool bImportStylesFirst = true,
									   const char * impProps = NULL);
	virtual UT_Error		importStyles(const char * szFilename, int ieft, bool bDocProps = false);

	virtual UT_Error		newDocument(void);

	virtual bool			isDirty(void) const;
	virtual void            forceDirty();
	
	virtual bool			canDo(bool bUndo) const;
	virtual UT_uint32		undoCount(bool bUndo) const;
	virtual bool			undoCmd(UT_uint32 repeatCount);
	virtual bool			redoCmd(UT_uint32 repeatCount);

	UT_Error				saveAs(const char * szFilename, int ieft, const char * expProps = NULL);
	UT_Error   				saveAs(const char * szFilename, int ieft, bool cpy, const char * expProps = NULL);
	UT_Error				save(void);

	void					beginUserAtomicGlob(void);
	void					endUserAtomicGlob(void);

	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties);
	bool					insertObject(PT_DocPosition dpos,
										 PTObjectType pto,
										 const XML_Char ** attributes,
										 const XML_Char ** properties, fd_Field ** pField );

	bool					insertSpan(PT_DocPosition dpos,
									   const UT_UCSChar * p,
									   UT_uint32 length,
									   PP_AttrProp *p_AttrProp = NULL);
	bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   PP_AttrProp *p_AttrProp_Before,
									   UT_uint32 &iRealDeleteCount,
									   bool bDeleteTableStruxes = false);

	bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  PT_DocPosition dpos2,
										  const XML_Char ** attributes,
										  const XML_Char ** properties);

	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts, pf_Frag_Strux ** ppfs_ret = 0);


	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts,
										  const XML_Char ** attributes,
										  const XML_Char ** properties, pf_Frag_Strux ** ppfs_ret = 0);

	void                    deleteHdrFtrStrux(PL_StruxDocHandle sdh);

	bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const XML_Char ** attributes,
										   const XML_Char ** properties,
										   PTStruxType pts);

	bool					changeStruxForLists(PL_StruxDocHandle sdh,
												const char * pszParentID);

	bool					insertFmtMark(PTChangeFmt ptc,
										  PT_DocPosition dpos,
										  PP_AttrProp *p_AttrProp);

	// the append- and insertBeforeFrag methods are only available while importing
	// the document.

	bool					appendStrux(PTStruxType pts, const XML_Char ** attributes, pf_Frag_Strux ** ppfs_ret = 0);
	bool					appendStruxFmt(pf_Frag_Strux * pfs, const XML_Char ** attributes);
	bool					appendFmt(const XML_Char ** attributes);
	bool					appendFmt(const UT_Vector * pVecAttributes);
	bool					appendSpan(const UT_UCSChar * p, UT_uint32 length);
	bool					appendObject(PTObjectType pto, const XML_Char ** attributes);
	bool					appendFmtMark(void);
	bool					appendStyle(const XML_Char ** attributes);
	bool                    changeStruxFormatNoUpdate(PTChangeFmt ptc ,PL_StruxDocHandle sdh,const XML_Char ** attributes);	
	bool					insertStruxBeforeFrag(pf_Frag * pF, PTStruxType pts,
												  const XML_Char ** attributes, pf_Frag_Strux ** ppfs_ret = 0);
	bool					insertSpanBeforeFrag(pf_Frag * pF, const UT_UCSChar * p, UT_uint32 length);
	bool					insertObjectBeforeFrag(pf_Frag * pF, PTObjectType pto,
												   const XML_Char ** attributes);
	bool					insertFmtMarkBeforeFrag(pf_Frag * pF);
	bool					insertFmtMarkBeforeFrag(pf_Frag * pF, const XML_Char ** attributes);

	pf_Frag *               findFragOfType(pf_Frag::PFType iType, UT_sint32 iSubtype = -1,
										   const pf_Frag * pfStart = NULL);
	pf_Frag *               getLastFrag() const;
	
	bool                    removeStyle(const XML_Char * name);
	bool					tellListener(PL_Listener * pListener);
	bool					tellListenerSubset(PL_Listener * pListener,
											   PD_DocumentRange * pDocRange);
	bool					addListener(PL_Listener * pListener, PL_ListenerId * pListenerId);
	bool					removeListener(PL_ListenerId listenerId);
	bool					signalListeners(UT_uint32 iSignal) const;
	bool					notifyListeners(const pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const;
	bool					notifyListeners(const pf_Frag_Strux * pfs,
											pf_Frag_Strux * pfsNew,
											const PX_ChangeRecord * pcr) const;

	bool					getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const;
	bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset, bool bLeftSide,
											const PP_AttrProp ** ppAP) const;
	const UT_UCSChar *		getPointer(PT_BufIndex bi) const; /* see warning on this function */
	bool					getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const;

	bool					getBounds(bool bEnd, PT_DocPosition & docPos) const;
	PTStruxType             getStruxType(PL_StruxDocHandle sdh) const;
	PT_DocPosition			getStruxPosition(PL_StruxDocHandle sdh) const;
	bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 PL_StruxFmtHandle * psfh) const;
	bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   PL_StruxFmtHandle * psfh) const;
	bool					getStruxOfTypeFromPosition(PT_DocPosition, PTStruxType pts, PL_StruxDocHandle * sdh) const;

	pf_Frag *				getFragFromPosition(PT_DocPosition docPos) const;

	bool					getNextStruxOfType(PL_StruxDocHandle sdh,PTStruxType pts,
											   PL_StruxDocHandle * nextsdh);
	bool					getPrevStruxOfType(PL_StruxDocHandle sdh,PTStruxType pts,
											   PL_StruxDocHandle * prevsdh);
	bool                    getNextStrux(PL_StruxDocHandle sdh, PL_StruxDocHandle *nextSDH);

	// data items

	bool					createDataItem(const char * szName, bool bBase64, const UT_ByteBuf * pByteBuf,
										   const void* pToken, void ** ppHandle);
	bool					getDataItemDataByName(const char * szName,
												  const UT_ByteBuf ** ppByteBuf, const void** ppToken, void ** ppHandle) const;
	bool					setDataItemToken(void* pHandle, void* pToken);
	bool					getDataItemData(void * pHandle,
											const char ** pszName, const UT_ByteBuf ** ppByteBuf, const void** ppToken) const;
	bool					enumDataItems(UT_uint32 k,
										  void ** ppHandle, const char ** pszName, const UT_ByteBuf ** ppByteBuf, const void** ppToken) const;

    PL_StruxDocHandle       findHdrFtrStrux(const XML_Char * pszHdtFtr,
											const XML_Char * pszHdrFtrID);
	bool                    verifySectionID(const XML_Char * pszId);
	PL_StruxDocHandle       getLastSectionSDH(void);
	PL_StruxDocHandle       getLastStruxOfType(PTStruxType pts);

	bool                    changeStruxAttsNoUpdate(PL_StruxDocHandle sdh, const char * attr, const char * attvalue);
	bool                    deleteStruxNoUpdate(PL_StruxDocHandle sdh);
	bool                    insertStruxNoUpdateBefore(PL_StruxDocHandle sdh, PTStruxType pts,const XML_Char ** attributes );
	bool                    isStruxBeforeThis(PL_StruxDocHandle sdh,  PTStruxType pts);
	PT_AttrPropIndex        getAPIFromSDH(PL_StruxDocHandle sdh);
    bool                    getAttributeFromSDH(PL_StruxDocHandle sdh, const char * szAttribute, const char ** pszValue);
    bool                    getPropertyFromSDH(PL_StruxDocHandle sdh, const char * szProperty, const char ** pszValue);
	// styles
	void                    getAllUsedStyles(UT_Vector * pVecStyles);
	PL_StruxFmtHandle       getNthFmtHandle(PL_StruxDocHandle sdh, UT_uint32 n);
	bool					getStyle(const char * szName, PD_Style ** ppStyle) const;
	PD_Style *				getStyleFromSDH(PL_StruxDocHandle sdh);
	PL_StruxDocHandle       getPrevNumberedHeadingStyle(PL_StruxDocHandle sdh);
	size_t                  getStyleCount(void);
	bool					enumStyles(UT_uint32 k,
									   const char ** pszName, const PD_Style ** ppStyle) const;
	bool					addStyleProperty(const XML_Char * szStyleName, const XML_Char * szPropertyName, const XML_Char * szPropertyValue);
	bool					addStyleProperties(const XML_Char * szStyleName, const XML_Char ** pProperties);
	bool	                setAllStyleAttributes(const XML_Char * szStyleName, const XML_Char ** pAttribs);
	bool	                addStyleAttributes(const XML_Char * szStyleName, const XML_Char ** pAttribs);

    PL_StruxDocHandle       findPreviousStyleStrux(const XML_Char * szStyle, PT_DocPosition pos);
    PL_StruxDocHandle       findForwardStyleStrux(const XML_Char * szStyle, PT_DocPosition pos);
	bool					updateDocForStyleChange(const XML_Char * szStyleName,
													bool isParaStyle);
	void                    updateAllLayoutsInDoc( PL_StruxDocHandle sdh);
	void					clearIfAtFmtMark(PT_DocPosition dpos);

	UT_uint32				getLastSavedAsType() { return m_lastSavedAsType; }
	UT_uint32				getLastOpenedType() { return m_lastOpenedType; }
	XAP_App *				getApp() { return m_pApp; }
	bool					updateFields(void);
	bool					getField(PL_StruxDocHandle sdh,
									 UT_uint32 offset,
                                     fd_Field * &pField);
	po_Bookmark * 			getBookmark(PL_StruxDocHandle sdh, UT_uint32 offset);
	void					setDontChangeInsPoint(void);
	void					allowChangeInsPoint(void);
	bool					getAllowChangeInsPoint(void) const;
	// Footnote functions
	bool                    isFootnoteAtPos(PT_DocPosition pos);
	bool                    isEndFootnoteAtPos(PT_DocPosition pos);
	UT_sint32               getEmbeddedOffset(PL_StruxDocHandle sdh,PT_DocPosition posOff, PL_StruxDocHandle & sdhEmbedded);

// Table functions

	PL_StruxDocHandle       getEndTableStruxFromTableSDH(PL_StruxDocHandle tableSDH);
	PL_StruxDocHandle       getEndCellStruxFromCellSDH(PL_StruxDocHandle cellSDH);
	PL_StruxDocHandle       getEndTableStruxFromTablePos(PT_DocPosition posTable);
	bool                    getRowsColsFromTableSDH(PL_StruxDocHandle tableSDH, UT_sint32 * numRows, UT_sint32 * numCols);
	PL_StruxDocHandle       getCellSDHFromRowCol(PL_StruxDocHandle tableSDH, UT_sint32 row, UT_sint32 col);
	void                    miniDump(PL_StruxDocHandle sdh, UT_sint32 nstruxes);

	// List Functions
	fl_AutoNum *			getListByID(UT_uint32 id) const;
	fl_AutoNum *			getNthList(UT_uint32 i) const;
	bool					enumLists(UT_uint32 k, fl_AutoNum ** pAutoNum);
	UT_uint32				getListsCount(void) const;
	void					addList(const fl_AutoNum * pAutoNum);
	bool					appendList(const XML_Char ** attributes);
	bool					fixListHierarchy(void);
	void					removeList(fl_AutoNum * pAutoNum,PL_StruxDocHandle sdh );
	void					listUpdate(PL_StruxDocHandle sdh);
	void					StopList(PL_StruxDocHandle sdh);
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
	fp_PageSize				m_docPageSize;
	bool					setPageSizeFromFile(const XML_Char ** attributes);

	bool					isBookmarkUnique(const XML_Char * pName) const;
	bool					isBookmarkRelativeLink(const XML_Char * pName) const;
	UT_uint32				getBookmarkCount()const {return m_vBookmarkNames.getItemCount();}
	const XML_Char *		getNthBookmark(UT_uint32 n)const{return reinterpret_cast<const XML_Char *>(m_vBookmarkNames.getNthItem(n));}
	void					addBookmark(const XML_Char * pName);
	void					removeBookmark(const XML_Char * pName);

	/////////////////////////////////////////////////////////////////////////////
	// Functions for dealing with revisions
	//
	bool                    isMarkRevisions() const{ return m_bMarkRevisions;}
	void                    toggleMarkRevisions();
	void                    setMarkRevisions(bool bMark);
	bool                    isShowRevisions() const{ return m_bShowRevisions;}
	void                    setShowRevisions(bool bShow);
	void                    toggleShowRevisions();
	UT_uint32               getShowRevisionId() const {return m_iShowRevisionID;}
	void                    setShowRevisionId(UT_uint32 iId);
	UT_uint32               getRevisionId() const{ return m_iRevisionID;}
	void                    setRevisionId(UT_uint32 iId);

	bool                    addRevision(UT_uint32 iId, UT_UCS4Char * pDesc,
										time_t tStart);
	
	bool                    addRevision(UT_uint32 iId, const UT_UCS4Char * pDesc, UT_uint32 iLen,
										time_t tStart);
	
	UT_Vector &             getRevisions() {return m_vRevisions;}
	UT_uint32               getHighestRevisionId() const;
	const PD_Revision *     getHighestRevision() const;
	void                    purgeRevisionTable();
	
	void					notifyPieceTableChangeStart(void);
	void					notifyPieceTableChangeEnd(void);



	pt_PieceTable *			getPieceTable(void) const
		{ return m_pPieceTable; }
#ifdef PT_TEST
	void					__dump(FILE * fp) const;
	//! Pointer to last instatiated PD_Document. Used for debugging.
	static PD_Document*		m_pDoc;
#endif

	// If we're using styles to format a document, prevent accidental use of other formatting
        // tools.  Disable all explicit formatting tools (font, color, boldness, etc.)
	inline bool areStylesLocked () const { return m_bLockedStyles; }    // See also lockStyles
	void lockStyles(bool b);

	virtual void setMetaDataProp (const UT_String & key, const UT_UTF8String & value);
	virtual bool getMetaDataProp (const UT_String & key, UT_UTF8String & outProp) const;

	UT_StringPtrMap & getMetaData () { return m_metaDataMap ; }

	// document-level property handling functions
	const PP_AttrProp *     getAttrProp() const;
	PT_AttrPropIndex        getAttrPropIndex() const {return m_indexAP;}
	bool                    setAttrProp(const XML_Char ** ppAttr);
	bool                    setAttributes(const XML_Char ** ppAttr);
	bool                    setProperties(const XML_Char ** ppProps);
	void                     setDontImmediatelyLayout(bool b)
		{ m_bDontImmediatelyLayout = b;}
	bool                     isDontImmediateLayout(void)
		{ return m_bDontImmediatelyLayout;}

	// map UT_String=>UT_UTF8String*
	UT_UTF8String getMailMergeField(const UT_String & key) const;
	bool mailMergeFieldExists(const UT_String & key) const;
	void setMailMergeField(const UT_String & key,
						   const UT_UTF8String & value);

	void setMailMergeLink (const char * file) {
		m_mailMergeLink = file;
	}

	UT_UTF8String getMailMergeLink () const {
		// return a copy of me
		return m_mailMergeLink;
	}
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
	bool      isIdUnique(UT_UniqueId::idType t, UT_uint32 i) {return m_UID.isIdUnique(t,i);}

	UT_uint32 getEditTime()const;
	void      setEditTime(UT_uint32 t);
	
	void      setDocVersion(UT_uint32 i);
	UT_uint32 getDocVersion() const {return m_iVersion;}

	void      addRecordToHistory(const PD_VersionData & v);
	UT_uint32 getHistoryCount()const {return m_vHistory.getItemCount();}
	UT_uint32 getHistoryNthId(UT_uint32 i)const;
	time_t    getHistoryNthTime(UT_uint32 i)const;
	UT_uint32 getHistoryNthEditTime(UT_uint32 i)const;
	UT_uint32 getHistoryNthUID(UT_uint32 i)const;

	bool      areDocumentsRelated (const PD_Document &d) const;
	bool      areDocumentHistoriesEqual(const PD_Document &d) const;
	bool      areDocumentContentsEqual(const PD_Document &d) const;
	bool      areDocumentFormatsEqual(const PD_Document &d) const;
	bool      areDocumentStylesheetsEqual(const PD_Document &d) const;

	bool      findFirstDifferenceInContent(PT_DocPosition &pos, UT_sint32 &iOffset2,
										   const PD_Document &d) const;
	
	bool      findWhereSimilarityResumes(PT_DocPosition &pos, UT_sint32 &iOffset2,
										 UT_uint32 & iKnownLength,
										 const PD_Document &d) const;

	bool      diffDocuments(const PD_Document &d, UT_Vector & vDiff) const;
	void      diffIntoRevisions(const PD_Document &d);
	
	const PD_DocumentUID * getDocUID()const {return m_pDocUID;}
	void                   setDocUID(PD_DocumentUID * u);
	const char *           getDocUIDString()const;
	
protected:
	~PD_Document();

	void                    _adjustEditTimeOnSave();
	void                    _adjustHistoryOnSave();
	void					_setClean(void);
	void					_destroyDataItemData(void);
	bool					_syncFileTypes(bool bReadSaveWriteOpen);

public:
	// these functions allow us to retrieve visual direction at document
	// position pos from an associated layout. They are intended to be
	// used by exporters into (daft) formats where such information
	// might be required (e.g. RTF).
	bool                    exportGetVisDirectionAtPos(PT_DocPosition pos, FriBidiCharType &type);
private:
	bool                    _exportInitVisDirection(PT_DocPosition pos);
	bool                    _exportFindVisDirectionRunAtPos(PT_DocPosition pos);
	
private:
	bool					m_ballowListUpdates;
	pt_PieceTable *			m_pPieceTable;
	UT_Vector				m_vecListeners;
	UT_Vector				m_vecLists;
	bool                    m_bHasListStopped;

	UT_StringPtrMap		    m_hashDataItems;
public:
	IE_FileInfo				m_fileImpExpInfo;
private:
	IEFileType				m_lastOpenedType;
	IEFileType				m_lastSavedAsType;
	XAP_App *				m_pApp;
	bool					m_bDoingPaste;
	bool					m_bAllowInsertPointChange;
	bool                    m_bRedrawHappenning;
	bool                    m_bLoading;
	bool                    m_bForcedDirty;
	UT_Vector				m_vBookmarkNames;
	bool                    m_bLockedStyles;
	UT_StringPtrMap         m_metaDataMap;
	bool                    m_bMarkRevisions;
	bool                    m_bShowRevisions;
	UT_uint32               m_iRevisionID;
	UT_uint32               m_iShowRevisionID;
	UT_Vector               m_vRevisions;
	PT_AttrPropIndex        m_indexAP;
	bool                    m_bDontImmediatelyLayout;

	// mapping UT_String=>UT_UTF8String pointer
	UT_StringPtrMap         m_mailMergeMap;

	UT_UCS4Char             m_iLastDirMarker;

	UT_UTF8String           m_mailMergeLink;

	// these are for use with the export*VisDirection functions
	const fl_BlockLayout *  m_pVDBl;
	fp_Run *                m_pVDRun;
	PT_DocPosition          m_iVDLastPos;
	UT_UniqueId             m_UID;

	// these are for tracking versioning
	UT_uint32               m_iVersion;
	bool                    m_bWasSaved;
	UT_uint32               m_iEditTime;
	UT_Vector               m_vHistory;
	PD_DocumentUID *        m_pDocUID;
};

#endif /* PD_DOCUMENT_H */
