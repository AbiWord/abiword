/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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


#ifndef AD_DOCUMENT_H
#define AD_DOCUMENT_H

#include <string>

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string_class.h"
#include "ut_vector.h"
#include "time.h"

// fwd. decl.
class XAP_ResourceManager;
class UT_UUID;
class XAP_Frame;
class AV_View;

// a helper class for history tracking
class ABI_EXPORT AD_VersionData
{
  public:

	// constructor for importers
	AD_VersionData(UT_uint32 v, UT_UTF8String &uuid, time_t start, bool autorev, UT_uint32 xid);
	AD_VersionData(UT_uint32 v, const char * uuid, time_t start, bool autorev, UT_uint32 xid);

	// constructor for new entries
	AD_VersionData(UT_uint32 v, time_t start, bool autorev, UT_uint32 xid);

	// copy constructor
	AD_VersionData(const AD_VersionData & v);

	virtual ~AD_VersionData();

	AD_VersionData & operator = (const AD_VersionData &v);

	bool operator == (const AD_VersionData &v);

	UT_uint32      getId()const{return m_iId;}
	time_t         getTime()const;
	time_t         getStartTime()const {return m_tStart;}
	const UT_UUID& getUID()const {return (const UT_UUID&)*m_pUUID;}
	bool           newUID(); // true on success

	void           setId(UT_uint32 iid) {m_iId = iid;}

	bool           isAutoRevisioned()const {return m_bAutoRevision;}
	void           setAutoRevisioned(bool autorev);

	UT_uint32      getTopXID() const {return m_iTopXID;}

  private:
	UT_uint32   m_iId;
	UT_UUID *   m_pUUID;
	time_t      m_tStart;
	bool        m_bAutoRevision;
	UT_uint32   m_iTopXID;
};

enum AD_HISTORY_STATE
{
	ADHIST_FULL_RESTORE,
	ADHIST_PARTIAL_RESTORE,
	ADHIST_NO_RESTORE
};


// a helper class for keeping track of revisions in the document
class ABI_EXPORT AD_Revision
{
  public:
	AD_Revision(UT_uint32 iId, UT_UCS4Char * pDesc, time_t start, UT_uint32 iVer = 0)
		:m_iId(iId),m_pDescription(pDesc), m_tStart(start), m_iVersion(iVer){};

	~AD_Revision(){delete [] m_pDescription;}

	UT_uint32         getId()const{return m_iId;}
	UT_UCS4Char *     getDescription() const {return m_pDescription;}

	// NB: getStartTime() == 0 should be interpreted as 'unknown'
	time_t            getStartTime() const {return m_tStart;}
	void              setStartTime(time_t t) {m_tStart = t;}

	UT_uint32         getVersion()const {return m_iVersion;}
	void              setVersion(UT_uint32 iVer) {m_iVersion = iVer;}

  private:
	UT_uint32     m_iId;
	UT_UCS4Char * m_pDescription;
	time_t        m_tStart;
	UT_uint32     m_iVersion;
};

enum AD_DOCUMENT_TYPE
{
	ADDOCUMENT_ABIWORD
};

struct _dataItemPair;
typedef _dataItemPair* PD_DataItemHandle;

class ABI_EXPORT AD_Document
{
public:
	AD_Document();
	void				ref(void);
	void				unref(void);

	virtual AD_DOCUMENT_TYPE getType() const = 0;

	XAP_ResourceManager &	resourceManager () const { return *m_pResourceManager; }

	const std::string     & getFilename(void) const;
	const std::string     & getPrintFilename(void) const;
	void                    setPrintFilename(const std::string & sFile);
	void                    clearFilename(void) {_setFilename(""); forceDirty();}
	// TODO - this should be returning IEFileType,
	// but that's AP stuff, so it's not here

	virtual UT_Error		readFromFile(const char * szFilename, int ieft, const char * props = NULL) = 0;
	virtual UT_Error		importFile(const char * szFilename, int ieft, bool markClean = false, bool bImportStylesFirst = true, const char * props = NULL) = 0;
	virtual UT_Error		newDocument() = 0;
	virtual bool			isDirty(void) const = 0;
	virtual void            forceDirty() {m_bForcedDirty = true;};
	bool                    isForcedDirty() const {return m_bForcedDirty;}

	virtual bool			canDo(bool bUndo) const = 0;
	virtual bool			undoCmd(UT_uint32 repeatCount) = 0;
	virtual bool			redoCmd(UT_uint32 repeatCount) = 0;

	UT_Error		        saveAs(const char * szFilename, int ieft, const char * props = NULL);
	UT_Error		        saveAs(const char * szFilename, int ieft, bool cpy, const char * props = NULL);
	UT_Error		        save(void);
	virtual bool			createDataItem(const char * szName,
										   bool bBase64,
										   const UT_ByteBuf * pByteBuf,
										   const std::string & mime_type,
										   PD_DataItemHandle* ppHandle) = 0;
	virtual bool            replaceDataItem(const char * szName,
											const UT_ByteBuf * pByteBuf) = 0;
	virtual bool			getDataItemDataByName(const char * szName,
												  const UT_ByteBuf ** ppByteBuf,
												  std::string * mime_type,
												  PD_DataItemHandle* ppHandle) const = 0;
public:

	/**
	 * Returns the # of seconds since the last save of this file
	 */
	time_t          getTimeSinceSave () const { return (time(NULL) - m_lastSavedTime); }
	time_t          getLastSavedTime() const {return m_lastSavedTime;}
	void            setLastSavedTime(time_t t) {m_lastSavedTime = t;}
	virtual UT_uint32 getLastSavedAsType() const = 0;

	time_t          getTimeSinceOpen () const { return (time(NULL) - m_lastOpenedTime); }
	time_t          getLastOpenedTime() const {return m_lastOpenedTime;}
	void            setLastOpenedTime(time_t t) {m_lastOpenedTime = t;}

	time_t          getEditTime()const {return (m_iEditTime + (time(NULL) - m_lastOpenedTime));}
	void            setEditTime(UT_uint32 t) {m_iEditTime = t;}

	void            setDocVersion(UT_uint32 i){m_iVersion = i;}
	UT_uint32       getDocVersion() const {return m_iVersion;}

	void			setEncodingName(const char * szEncodingName);
	const char *	getEncodingName() const;

	bool			isPieceTableChanging(void) const;

	virtual void setMetaDataProp (const std::string & key, const std::string & value) = 0;
	virtual bool getMetaDataProp (const std::string & key, std::string & outProp) const = 0;

	// RIVERA TODO not working and may not be needed
	virtual void setAnnotationProp (const std::string & key, const std::string & value) = 0;
	virtual bool getAnnotationProp (const std::string & key, std::string & outProp) const = 0;

	// history tracking
	void            addRecordToHistory(const AD_VersionData & v);
	void            purgeHistory();
	UT_sint32       getHistoryCount()const {return m_vHistory.getItemCount();}
	UT_uint32       getHistoryNthId(UT_sint32 i)const;
	time_t          getHistoryNthTime(UT_sint32 i)const;
	time_t          getHistoryNthTimeStarted(UT_sint32 i)const;
	time_t          getHistoryNthEditTime(UT_sint32 i)const;
	const UT_UUID&  getHistoryNthUID(UT_sint32 i)const;
	bool            getHistoryNthAutoRevisioned(UT_sint32 i)const;
	UT_uint32       getHistoryNthTopXID(UT_sint32 i)const;

	AD_HISTORY_STATE       verifyHistoryState(UT_uint32 &iVersion) const;
	const AD_VersionData * findHistoryRecord(UT_uint32 iVersion) const;
    bool                   showHistory(AV_View * pView);

	bool            areDocumentsRelated (const AD_Document &d) const;
	bool            areDocumentHistoriesEqual(const AD_Document &d, UT_uint32 &iVer) const;

	virtual bool    areDocumentContentsEqual(const AD_Document &d, UT_uint32 &pos) const = 0;
	virtual bool    areDocumentFormatsEqual(const AD_Document &d, UT_uint32 &pos) const = 0;
	virtual bool    areDocumentStylesheetsEqual(const AD_Document &d) const = 0;

	void            setDocUUID(const char * u);
	const char *    getDocUUIDString()const;
	const UT_UUID * getDocUUID()const {return m_pUUID;};
	void            setOrigUUID(const char * u);
	const char *    getOrigDocUUIDString()const;
	const UT_UUID * getOrigDocUUID()const {return m_pOrigUUID;};
	void            setMyUUID(const char * u);
	UT_UTF8String   getMyUUIDString()const;
	const UT_UUID * getMyUUID()const {return m_pMyUUID;};

	UT_UUID *       getNewUUID()   const;
	UT_uint32       getNewUUID32() const;
	UT_uint64       getNewUUID64() const;

	bool            addRevision(UT_uint32 iId, UT_UCS4Char * pDesc,
								time_t tStart, UT_uint32 iVersion, bool bGenCR=true);

	bool            addRevision(UT_uint32 iId, const UT_UCS4Char * pDesc, UT_uint32 iLen,
								time_t tStart, UT_uint32 iVersion, bool bGenCR=true);
	bool            addRevision(AD_Revision * pRev, bool bGenCR=true);
	virtual bool    createAndSendDocPropCR( const gchar ** pAtts, const gchar ** pProps) = 0;

	const UT_GenericVector<AD_Revision*> &         getRevisions() {return m_vRevisions;}
	UT_uint32           getHighestRevisionId() const;
	const AD_Revision*  getHighestRevision() const;
	UT_sint32           getRevisionIndxFromId(UT_uint32 iId) const;
    bool                usingChangeTracking() const;

	bool                isMarkRevisions() const{ return m_bMarkRevisions;}
	bool                isShowRevisions() const{ return m_bShowRevisions;}

	UT_uint32           getShowRevisionId() const {return m_iShowRevisionID;}
	UT_uint32           getRevisionId() const{ return m_iRevisionID;}

	UT_uint32           findAutoRevisionId(UT_uint32 iVersion) const;
	UT_uint32           findNearestAutoRevisionId(UT_uint32 iVersion, bool bLesser = true) const;

	void                toggleMarkRevisions();
	void                toggleShowRevisions();

	virtual void        setMarkRevisions(bool bMark);
	void                setShowRevisions(bool bShow);
	void                setShowRevisionId(UT_uint32 iId);
	void                setRevisionId(UT_uint32 iId);

	bool                isAutoRevisioning()const {return m_bAutoRevisioning;}
	virtual void        setAutoRevisioning(bool autorev);

	virtual void        purgeRevisionTable(bool bUnconditional = false) = 0;

	virtual bool        acceptRejectRevision(bool bReject,
											 UT_uint32 iStart,
											 UT_uint32 iEnd,
											 UT_uint32 iLevel) = 0;

	virtual bool        rejectAllHigherRevisions(UT_uint32 iLevel) = 0;

	virtual bool        acceptAllRevisions() = 0;

	bool                purgeAllRevisions(AV_View * pView);
	bool                isOrigUUID(void) const;
	void                setFilename(const char * name)
	{ _setFilename(name); }
	virtual UT_uint32   getXID() const = 0;
	virtual UT_uint32   getTopXID() const = 0;

 protected:
	virtual UT_Error	_saveAs(const char * szFilename, int ieft, const char * props = NULL) = 0;
	virtual UT_Error	_saveAs(const char * szFilename, int ieft, bool cpy, const char * props = NULL) = 0;
	virtual UT_Error	_save(void) = 0;

	void            _purgeRevisionTable();
	void            _adjustHistoryOnSave();
	void			_setFilename(const char * name)
		{ if (name) m_szFilename = name; else m_szFilename.clear(); }
	void            _setForceDirty(bool b) {m_bForcedDirty = b;}
	void            _setPieceTableChanging(bool b) {m_bPieceTableChanging = b;}
	void            _setMarkRevisions(bool bMark) {m_bMarkRevisions = bMark;}

    bool            _restoreVersion(XAP_Frame * pFrame, UT_uint32 iVersion);

	virtual void    _clearUndo() = 0;


	virtual ~AD_Document();		//  Use unref() instead.

private:
	XAP_ResourceManager *	m_pResourceManager;

	int				m_iRefCount;
	std::string		m_szFilename;
	UT_String		m_szEncodingName;

	bool			m_bPieceTableChanging;
	time_t          m_lastSavedTime;
	time_t          m_lastOpenedTime;
	time_t          m_iEditTime;     // previous edit time (up till open)
	UT_uint32       m_iVersion;

	// these are for tracking versioning
	bool            m_bHistoryWasSaved;
	UT_Vector       m_vHistory;
	UT_GenericVector<AD_Revision*> m_vRevisions;

	bool            m_bMarkRevisions;
	bool            m_bShowRevisions;
	UT_uint32       m_iRevisionID;
	UT_uint32       m_iShowRevisionID;
	bool            m_bAutoRevisioning;

	bool            m_bForcedDirty;

	UT_UUID *       m_pUUID;
	UT_UUID *       m_pOrigUUID;
	UT_UUID *       m_pMyUUID;
	bool            m_bDoNotAdjustHistory;
	bool            m_bAfterFirstSave;
 	UT_UTF8String   m_sMyUUIDString;
 	UT_UTF8String   m_sOrigUUIDString;
	std::string     m_sPrintFilename;
};


#endif /* AD_DOCUMENT_H */
