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
#include "ie_types.h"
#include "fp_PageSize.h"
#include "ut_string_class.h"

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

#ifdef PT_TEST
#include "ut_test.h"
#endif

enum
{
	PD_SIGNAL_UPDATE_LAYOUT,
	PD_SIGNAL_REFORMAT_LAYOUT
};

/*!
 PD_Document is the representation for a document.
*/

class ABI_EXPORT PD_Document : public AD_Document
{
public:
	PD_Document(XAP_App *pApp);

	virtual UT_Error	       	readFromFile(const char * szFilename, int ieft);
	virtual UT_Error            importFile(const char * szFilename, int ieft, bool markClean = false);
	virtual UT_Error	       	newDocument(void);
	virtual bool			isDirty(void) const;

	virtual bool			canDo(bool bUndo) const;
	virtual UT_uint32 undoCount(bool bUndo) const;
	virtual bool			undoCmd(UT_uint32 repeatCount);
	virtual bool			redoCmd(UT_uint32 repeatCount);

	UT_Error				saveAs(const char * szFilename, int ieft);
	UT_Error   				saveAs(const char * szFilename, int ieft, bool cpy);
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
									   PP_AttrProp *p_AttrProp_Before = NULL);

	bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  PT_DocPosition dpos2,
										  const XML_Char ** attributes,
										  const XML_Char ** properties);

	bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts);
	void                    deleteHdrFtrStrux(PL_StruxDocHandle sdh);

	bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const XML_Char ** attributes,
										   const XML_Char ** properties,
										   PTStruxType pts);

	bool					changeStruxForLists(PL_StruxDocHandle sdh,
												const char * pszParentID);

	// the append- methods are only available while importing
	// the document.

	bool					appendStrux(PTStruxType pts, const XML_Char ** attributes);
	bool					appendFmt(const XML_Char ** attributes);
	bool					appendFmt(const UT_Vector * pVecAttributes);
	bool					appendSpan(const UT_UCSChar * p, UT_uint32 length);
	bool					appendObject(PTObjectType pto, const XML_Char ** attributes);
	bool					appendFmtMark(void);
	bool					appendStyle(const XML_Char ** attributes);
	bool                    removeStyle(const XML_Char * name);
	bool					tellListener(PL_Listener * pListener);
	bool					tellListenerSubset(PL_Listener * pListener,
											   PD_DocumentRange * pDocRange);
	bool					addListener(PL_Listener * pListener, PL_ListenerId * pListenerId);
	bool					removeListener(PL_ListenerId listenerId);
	bool					signalListeners(UT_uint32 iSignal) const;
	bool					notifyListeners(pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const;
	bool					notifyListeners(pf_Frag_Strux * pfs,
											pf_Frag_Strux * pfsNew,
											const PX_ChangeRecord * pcr) const;

	bool					getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const;
	bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset, bool bLeftSide,
											const PP_AttrProp ** ppAP) const;
	const UT_UCSChar *		getPointer(PT_BufIndex bi) const; /* see warning on this function */
	bool					getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
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

	bool					getNextStruxOfType(PL_StruxDocHandle sdh,PTStruxType pts,
											   PL_StruxDocHandle * nextsdh);
	bool					getPrevStruxOfType(PL_StruxDocHandle sdh,PTStruxType pts,
											   PL_StruxDocHandle * prevsdh);

	// data items

	bool					createDataItem(const char * szName, bool bBase64, const UT_ByteBuf * pByteBuf,
										   void* pToken, void ** ppHandle);
	bool					getDataItemDataByName(const char * szName,
												  const UT_ByteBuf ** ppByteBuf, void** ppToken, void ** ppHandle) const;
	bool					setDataItemToken(void* pHandle, void* pToken);
	bool					getDataItemData(void * pHandle,
											const char ** pszName, const UT_ByteBuf ** ppByteBuf, void** ppToken) const;
	bool					enumDataItems(UT_uint32 k,
										  void ** ppHandle, const char ** pszName, const UT_ByteBuf ** ppByteBuf, void** ppToken) const;

    PL_StruxDocHandle       findHdrFtrStrux(const XML_Char * pszHdtFtr, 
											const XML_Char * pszHdrFtrID);
	bool                    verifySectionID(const XML_Char * pszId);
	PL_StruxDocHandle       getLastSectionSDH(void);
	bool                    changeSectionAttsNoUpdate(PL_StruxDocHandle sdh, const char * attr, const char * attvalue);

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

	const char *			getFileName() { return m_szFilename; }
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
	bool					isPieceTableChanging(void);
	void					notifyPieceTableChangeStart(void);
	void					notifyPieceTableChangeEnd(void);
	
	// List Functions
	fl_AutoNum *			getListByID(UT_uint32 id) const;
	fl_AutoNum *			getNthList(UT_uint32 i) const; 
	bool					enumLists(UT_uint32 k, fl_AutoNum ** pAutoNum);
	UT_uint32				getListsCount(void) const; 
	void					addList(fl_AutoNum * pAutoNum);
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
	void					setDefaultPageSize(void);
	const char *			getDefaultPageSize(void);
	bool					setPageSizeFromFile(const XML_Char ** attributes);
	
	bool					isBookmarkUnique(const XML_Char * pName) const;
	bool					isBookmarkRelativeLink(const XML_Char * pName) const;
	UT_uint32				getBookmarkCount()const {return m_vBookmarkNames.getItemCount();}
	const XML_Char *		getNthBookmark(UT_uint32 n)const{return (const XML_Char *)m_vBookmarkNames.getNthItem(n);}
	void					addBookmark(const XML_Char * pName);
	void					removeBookmark(const XML_Char * pName);


#ifdef PT_TEST
	void					__dump(FILE * fp) const;
	pt_PieceTable *			getPieceTable(void) const
		{ return m_pPieceTable; }
	//! Pointer to last instatiated PD_Document. Used for debugging.
	static PD_Document*		m_pDoc;
#endif

	// If we're using styles to format a document, prevent accidental use of other formatting
        // tools.  Disable all explicit formatting tools (font, color, boldness, etc.) 
	inline bool areStylesLocked () const { return m_bLockedStyles; }    // See also lockStyles
	inline void lockStyles(bool b) { m_bLockedStyles = b; }             // See also areStylesLocked

	void setMetaDataProp ( const UT_String & key,
			       const UT_String & value ) ;

	bool getMetaDataProp ( const UT_String & key,
			       UT_String & outProp ) const ;

	UT_StringPtrMap & getMetaData () { return m_metaDataMap ; }

	inline void setAuthor ( const UT_String & value )
	  {
	    setMetaDataProp ( "Author", value ) ;
	  }

	inline UT_String getAuthor () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Author", ret ) ;
	    return ret ;
	  }

	inline void setSubject ( const UT_String & value )
	  {
	    setMetaDataProp ( "Subject", value ) ;
	  }

	inline UT_String getSubject () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Subject", ret ) ;
	    return ret ;
	  }

	inline void setTitle ( const UT_String & value )
	  {
	    setMetaDataProp ( "Title", value ) ;
	  }

	inline UT_String getTitle () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Title", ret ) ;
	    return ret ;
	  }

	inline void setSummary ( const UT_String & value )
	  {
	    setMetaDataProp ( "Summary", value ) ;
	  }

	inline UT_String getSummary () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Summary", ret ) ;
	    return ret ;
	  }

	inline void setOrginization ( const UT_String & value )
	  {
	    setMetaDataProp ( "Orginization", value ) ;
	  }

	inline UT_String getOrginization () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Orginization", ret ) ;
	    return ret ;
	  }

	inline void setRole ( const UT_String & value )
	  {
	    setMetaDataProp ( "Role", value ) ;
	  }

	inline UT_String getRole () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Role", ret ) ;
	    return ret ;
	  }

	inline void setKeywords ( const UT_String & value )
	  {
	    setMetaDataProp ( "Keywords", value ) ;
	  }

	inline UT_String getKeywords () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Keywords", ret ) ;
	    return ret ;
	  }

	inline void setComments ( const UT_String & value )
	  {
	    setMetaDataProp ( "Comments", value ) ;
	  }

	inline UT_String getComments () const
	  {
	    UT_String ret;
	    getMetaDataProp ( "Comments", ret ) ;
	    return ret ;
	  }

protected:
	~PD_Document();

	void					_setClean(void);
	void					_destroyDataItemData(void);
	bool					_syncFileTypes(bool bReadSaveWriteOpen);

private:
	bool					m_ballowListUpdates;
	pt_PieceTable *			m_pPieceTable;
	UT_Vector				m_vecListeners;
	UT_Vector				m_vecLists;
	bool                    m_bHasListStopped;
	
	UT_StringPtrMap		    m_hashDataItems;

	IEFileType				m_lastOpenedType;
	IEFileType				m_lastSavedAsType;
	XAP_App *				m_pApp;
	bool					m_bPieceTableChanging;
	bool					m_bDoingPaste;
	bool					m_bAllowInsertPointChange;
	bool                    m_bRedrawHappenning;
	bool                    m_bLoading;
	bool m_bForcedDirty;
	UT_Vector				m_vBookmarkNames;
	bool m_bLockedStyles;
	UT_StringPtrMap m_metaDataMap;
};

#endif /* PD_DOCUMENT_H */
