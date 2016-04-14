/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
 * Copyright (C) 2016 Hubert Figuiere <hub@figuiere.net>
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

#include <stdlib.h>
#include <cstring>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_hash.h"
#include "ut_vector.h"
#include "ut_uuid.h"
#include "ut_misc.h"
#include "ut_path.h"
#include "xap_DialogFactory.h"

#include "xad_Document.h"
#include "xav_View.h"
#include "xap_App.h"
#include "xap_Strings.h"
#include "xap_Dlg_History.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "xap_Dialog_Id.h"

#ifdef ENABLE_RESOURCE_MANAGER
#include "xap_ResourceManager.h"
#endif

AD_Document::AD_Document() :
#ifdef ENABLE_RESOURCE_MANAGER
	m_pResourceManager(new XAP_ResourceManager),
#else
	m_pResourceManager(0),
#endif
	m_iRefCount(1),
	m_szEncodingName(""), // Should this have a default? UTF-8, perhaps?
    m_bPieceTableChanging(false),
	m_lastSavedTime(0),
	m_lastOpenedTime(time(NULL)),
    m_iEditTime(0),
    m_iVersion(0),
	m_bHistoryWasSaved(false),
	m_bMarkRevisions(false),
	m_bShowRevisions(true),
	m_iRevisionID(1),
	m_iShowRevisionID(0), // show all
	m_bAutoRevisioning(false),
    m_bForcedDirty(false),
	m_pUUID(NULL),
	m_pOrigUUID(NULL),
	m_pMyUUID(NULL),
	m_bDoNotAdjustHistory(false),
	m_bAfterFirstSave(false)
{	// TODO: clear the ignore list
	

	// create UUID for this doc
	UT_return_if_fail(XAP_App::getApp() && XAP_App::getApp()->getUUIDGenerator());

	m_pUUID = XAP_App::getApp()->getUUIDGenerator()->createUUID();
	UT_return_if_fail(m_pUUID);
	UT_return_if_fail(m_pUUID->isValid());
	//
	// Make a copy with the same value so we know when we're importing
	// a remote CR
	//
	m_pMyUUID =  XAP_App::getApp()->getUUIDGenerator()->createUUID();
	UT_return_if_fail(m_pMyUUID);
	UT_return_if_fail(m_pMyUUID->isValid());

	m_pOrigUUID =  XAP_App::getApp()->getUUIDGenerator()->createUUID();
	UT_return_if_fail(m_pOrigUUID);
	UT_return_if_fail(m_pOrigUUID->isValid());
	UT_UTF8String s;
	m_pUUID->toString(s);
	m_pOrigUUID->setUUID(s);
	m_pMyUUID->setUUID(s);
	UT_UTF8String OrigS;
	m_pOrigUUID->toString(OrigS);
	m_pOrigUUID->toString(m_sOrigUUIDString);
	m_pMyUUID->toString(m_sMyUUIDString);
	UT_DEBUGMSG(("!!!!!!!!!!----------------- Created string %s \n",s.utf8_str()));
	UT_DEBUGMSG(("!!!!!!!!!!----------------- Orig string %s \n",OrigS.utf8_str()));
}

AD_Document::~AD_Document()
{
	UT_ASSERT(m_iRefCount == 0);

   	// NOTE: let subclass clean up m_szFilename, so it matches the alloc mechanism

	// & finally...
#ifdef ENABLE_RESOURCE_MANAGER
	DELETEP(m_pResourceManager);
#endif

	UT_VECTOR_PURGEALL(AD_VersionData*, m_vHistory);
	UT_VECTOR_PURGEALL(AD_Revision*, m_vRevisions);

	DELETEP(m_pUUID);
	DELETEP(m_pOrigUUID);
	DELETEP(m_pMyUUID);
}

const std::string & AD_Document::getPrintFilename(void) const
{
	return m_sPrintFilename;
}

void AD_Document::setPrintFilename(const std::string & sFilename)
{
	m_sPrintFilename = sFilename;
}

bool AD_Document::isOrigUUID(void) const
{
  UT_UTF8String sDoc;
  UT_UTF8String sOrig;
  if((m_pMyUUID== NULL) || (m_pOrigUUID == NULL))
	  return false;
  m_pMyUUID->toString(sDoc);
  m_pOrigUUID->toString(sOrig);
  bool b = (strcmp(sDoc.utf8_str(),sOrig.utf8_str()) == 0);
  return b;
}

bool AD_Document::isPieceTableChanging(void) const
{
        return m_bPieceTableChanging;
}

/*!
    Creates a new UUID; this uuid has the 'MAC' portion set to the
    same value as the main doc uuid -- this guarantees that all id's
    generated this way will be document unique even though withou
    using MAC in the uuid we cannot absolutely guarantee universal
    uniqueness.
*/
UT_UUID * AD_Document::getNewUUID()const
{
	// when new uuid is requested, we will generate it reusing the MAC
	// part of the doc uuid. This will ensure that all uuid's in the
	// present document are unique even though we no longer use MAC
	// addrress in them
	UT_return_val_if_fail(XAP_App::getApp() && XAP_App::getApp()->getUUIDGenerator(), NULL);
	UT_return_val_if_fail(m_pUUID, NULL);
	UT_UUID * pUUID = XAP_App::getApp()->getUUIDGenerator()->createUUID(*m_pUUID);

	UT_return_val_if_fail(pUUID, NULL);
	pUUID->resetTime();
	UT_ASSERT(pUUID->isValid());
	return pUUID;
}

/*!
    see notes on getNewUUID()
*/
UT_uint32 AD_Document::getNewUUID32() const
{
#if 0
	UT_return_val_if_fail(XAP_App::getApp() && XAP_App::getApp()->getUUIDGenerator(),0);
	return XAP_App::getApp()->getUUIDGenerator()->getNewUUID32();
#else
	UT_UUID *pUUID = getNewUUID();
	UT_return_val_if_fail(pUUID, 0);
	UT_uint32 iRet = pUUID->hash32();
	delete pUUID;
	return iRet;
#endif
}

/*!
    see notes on getNewUUID()
*/
UT_uint64 AD_Document::getNewUUID64() const
{
#if 0
	UT_return_val_if_fail(XAP_App::getApp() && XAP_App::getApp()->getUUIDGenerator(),0);
	return XAP_App::getApp()->getUUIDGenerator()->getNewUUID64();
#else
	UT_UUID *pUUID = getNewUUID();
	UT_return_val_if_fail(pUUID, 0);
	UT_uint32 iRet = pUUID->hash32();
	delete pUUID;
	return iRet;
#endif
}


void AD_Document::ref(void)
{
	UT_ASSERT(m_iRefCount > 0);

	m_iRefCount++;
}


void AD_Document::unref(void)
{
	UT_ASSERT(m_iRefCount > 0);

	if (--m_iRefCount == 0)
	{
		delete this;
	}
}

const std::string & AD_Document::getFilename(void) const
{
	return m_szFilename;
}

// Document-wide Encoding name used for some file formats (Text, RTF, HTML)

void AD_Document::setEncodingName(const char *szEncodingName)
{
	if (szEncodingName == NULL)
		szEncodingName = "";

	m_szEncodingName = szEncodingName;
}

const char * AD_Document::getEncodingName(void) const
{
	return m_szEncodingName.size() ? m_szEncodingName.c_str() : 0;
}

void AD_Document::purgeHistory()
{
	UT_VECTOR_PURGEALL(AD_VersionData*, m_vHistory);
	m_bHistoryWasSaved = false;
}


/*!
    Add given version data to the document history.
*/
void AD_Document::addRecordToHistory(const AD_VersionData &vd)
{
	AD_VersionData * v = new AD_VersionData(vd);
	UT_return_if_fail(v);
	m_vHistory.addItem((void*)v);
}

/*!
    Get the version number for n-th record in version history
*/
UT_uint32 AD_Document::getHistoryNthId(UT_sint32 i)const
{
	if(!m_vHistory.getItemCount())
		return 0;

	AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(i);

	if(!v)
		return 0;

	return v->getId();
}

UT_uint32 AD_Document::getHistoryNthTopXID(UT_sint32 i)const
{
	if(!m_vHistory.getItemCount())
		return 0;

	AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(i);

	if(!v)
		return 0;

	return v->getTopXID();
}

/*!
    Get time stamp for n-th record in version history
    NB: the time stamp represents the last save time
*/
time_t AD_Document::getHistoryNthTime(UT_sint32 i)const
{
	if(!m_vHistory.getItemCount())
		return 0;

	AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(i);

	if(!v)
		return 0;

	return v->getTime();
}

time_t AD_Document::getHistoryNthTimeStarted(UT_sint32 i)const
{
	if(!m_vHistory.getItemCount())
		return 0;

	AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(i);

	if(!v)
		return 0;

	return v->getStartTime();
}

bool AD_Document::getHistoryNthAutoRevisioned(UT_sint32 i)const
{
	if(!m_vHistory.getItemCount())
		return 0;

	AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(i);

	if(!v)
		return false;

	return v->isAutoRevisioned();
}


/*!
    Get get cumulative edit time for n-th record in version history
*/
time_t AD_Document::getHistoryNthEditTime(UT_sint32 i)const
{
	if(!m_vHistory.getItemCount() || !m_pUUID)
		return 0;

	AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(i);

	if(!v)
		return 0;

	time_t t0 = v->getStartTime();
	time_t t1 = v->getTime();

	UT_ASSERT( t1 >= t0 );
	return t1-t0;
}

/*!
    Get the UID for n-th record in version history
*/
const UT_UUID & AD_Document::getHistoryNthUID(UT_sint32 i) const
{
	if(!m_vHistory.getItemCount())
		return UT_UUID::getNull();

	AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(i);

	if(!v)
		return UT_UUID::getNull();

	return v->getUID();
}


/*!
    Returns true if both documents are based on the same root document
*/
bool AD_Document::areDocumentsRelated(const AD_Document & d) const
{
	if((!m_pUUID && d.getDocUUID()) || (m_pUUID && !d.getDocUUID()))
		return false;

	return (*m_pUUID == *(d.getDocUUID()));
}

/*!
    Returns true if both documents are based on the same root document
    and all version records have identical UID's
    on return, the last identical version id is found in iVersion
*/
bool AD_Document::areDocumentHistoriesEqual(const AD_Document & d, UT_uint32 &iVersion) const
{
	iVersion = 0;
	
	if((!m_pUUID && d.getDocUUID()) || (m_pUUID && !d.getDocUUID()))
		return false;

	if(!(*m_pUUID == *(d.getDocUUID())))
		return false;

	UT_uint32 iCount = UT_MIN(getHistoryCount(), d.getHistoryCount());
	UT_uint32 iMaxCount = UT_MAX(getHistoryCount(), d.getHistoryCount());
	
	for(UT_uint32 i = 0; i < iCount; ++i)
	{
		AD_VersionData * v1 = (AD_VersionData*)m_vHistory.getNthItem(i);
		AD_VersionData * v2 = (AD_VersionData*)d.m_vHistory.getNthItem(i);
	
		if(!(*v1 == *v2))
			return false;

		iVersion = v1->getId();
	}

	if(iMaxCount != iCount)
		return false;
	
	return true;		
}

/*!
    Verifies to what extent we are able to restore given version of
    the document

    \return: return value indicates whether full/partial/no restore is possible
    
    \param UT_uint32 & iVersion: on entry contains the version number
                                 user wants to revert to; if return
                                 value indicates partial restore
                                 iVersion indicates which nearest
                                 (greater) version can be restored
                                 fully, or 0 if no such version exists
*/
AD_HISTORY_STATE AD_Document::verifyHistoryState(UT_uint32 &iVersion) const
{
	if(!m_vHistory.getItemCount())
		return ADHIST_NO_RESTORE;
	
	AD_HISTORY_STATE eRet = ADHIST_FULL_RESTORE; // be optimistic

	const AD_VersionData * v = NULL;
	UT_sint32 i;
	bool bFullRestore = false;
	bool bFound = false;
	
	// find the lowest autorevisioned record greater than iVersion and
	// evaluate the state of history above iVersion
	for(i = 0; i < getHistoryCount(); ++i)
	{
		v = (const AD_VersionData*)m_vHistory.getNthItem(i);

		if(!v)
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			continue;
		}

		if(v->getId() < iVersion + 1)
			continue;

		if(!v->isAutoRevisioned())
			continue;

		// if we got so far, we have an autorevisioned record greater
		// than iVersion
		
		if(!bFound)
		{
			bFound = true;

			if(v->getId() == iVersion + 1)
				bFullRestore = true;
			
			continue;
		}

		bFullRestore &= v->isAutoRevisioned();
	}

	if(!bFound)
	{
		// there are no autorevisioned records above our version
		return ADHIST_NO_RESTORE;
	}

	if(!bFullRestore)
	{
		eRet = ADHIST_PARTIAL_RESTORE;

		// we want to find out from which version would full restore
		// be possible
		UT_uint32 iMinVersion = 0; // assume nothing
		for(i = getHistoryCount(); i > 0; --i)
		{
			v = (const AD_VersionData*)m_vHistory.getNthItem(i-1);

			if(!v)
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				continue;
			}

			if(v->getId() <= iVersion) // too far down the table
				break;

			if(!v->isAutoRevisioned()) // break in the history
				break;

			iMinVersion = v->getId();
		}

		// iMinVersion now contains the lowest version with a full
		// record above it, which we pass back to the caller
		iVersion = iMinVersion;
	}

	return eRet;
}

const AD_VersionData * AD_Document::findHistoryRecord(UT_uint32 iVersion) const
{
	for(UT_sint32 i = 0; i < getHistoryCount(); ++i)
	{
		const AD_VersionData * v = (const AD_VersionData*)m_vHistory.getNthItem(i);

		if(v->getId() == iVersion)
			return v;
	}

	return NULL;
}


/*!
    Set UID for the present document
*/
void AD_Document::setDocUUID(const char * s)
{
	if(!m_pUUID)
	{
		UT_return_if_fail(0);
	}
	
	if(!m_pUUID->setUUID(s))
	{
		// string we were passed did not contain valid uuid
		// if our original id was valid, we will keep it, otherwise
		// make a new one
		if(!m_pUUID->isValid())
			m_pUUID->makeUUID();
	}
}

/*!
    Set Orig UID for the present document
*/
void AD_Document::setOrigUUID(const char * s)
{
	if(!m_pOrigUUID)
	{
		UT_return_if_fail(0);
	}
	
	if(!m_pOrigUUID->setUUID(s))
	{
		// string we were passed did not contain valid uuid
		// if our original id was valid, we will keep it, otherwise
		// make a new one
		if(!m_pOrigUUID->isValid())
			m_pOrigUUID->makeUUID();
	}
	m_pOrigUUID->toString(m_sOrigUUIDString);
}


/*!
    Set Orig UID for the present document
*/
void AD_Document::setMyUUID(const char * s)
{
	if(!m_pMyUUID)
	{
		UT_return_if_fail(0);
	}
	
	if(!m_pMyUUID->setUUID(s))
	{
		// string we were passed did not contain valid uuid
		// if our original id was valid, we will keep it, otherwise
		// make a new one
		if(!m_pMyUUID->isValid())
			m_pMyUUID->makeUUID();
	}
	m_pMyUUID->toString(m_sMyUUIDString);
}

/*!
    Get the UID of this document represented as a string (this
    function is primarily for exporters)
*/
const char * AD_Document::getDocUUIDString() const
{
	UT_return_val_if_fail(m_pUUID, NULL);
	static UT_UTF8String s;
	m_pUUID->toString(s);
	return s.utf8_str();
}


/*!
    Get the Original UID of this document represented as a string (this
    function is primarily for exporters)
*/
const char * AD_Document::getOrigDocUUIDString() const
{
	UT_return_val_if_fail(m_pOrigUUID, NULL);
	return m_sOrigUUIDString.utf8_str();
}


/*!
    Get the UID of the users of this document represented as a string (this
    function is primarily for exporters)
	
	NOTE: don't make this a static variable, as this value might change over
	the life of the document
*/
UT_UTF8String AD_Document::getMyUUIDString() const
{
	UT_return_val_if_fail(m_pMyUUID, "");
	return m_sMyUUIDString;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

/*!
    find revision with id iId and return its index in revision table
    \param iId  -- id of revision we are interested in
    \return     -- if found index into revision vector if not found < 0
*/
UT_sint32 AD_Document::getRevisionIndxFromId(UT_uint32 iId) const
{
	for(UT_sint32 i = 0; i < m_vRevisions.getItemCount(); i++)
	{
		if(m_vRevisions.getNthItem(i)->getId() == iId)
			return i;
	}

	return -1;
}

/**
 * Should we pay attention to change tracking?
 */
bool AD_Document::usingChangeTracking() const
{
    bool ret = false;
    
    ret |= isMarkRevisions();
    ret |= ( getHighestRevisionId() > 1 );

    return ret;
}



UT_uint32 AD_Document::getHighestRevisionId() const
{
	UT_uint32 iId = 0;

	for(UT_sint32 i = 0; i < m_vRevisions.getItemCount(); i++)
	{
		iId = UT_MAX(iId, m_vRevisions.getNthItem(i)->getId());
	}

	return iId;
}

const AD_Revision * AD_Document::getHighestRevision() const
{
	UT_uint32 iId = 0;
	const AD_Revision * r = NULL;

	for(UT_sint32 i = 0; i < m_vRevisions.getItemCount(); i++)
	{
		const AD_Revision * t = m_vRevisions.getNthItem(i);
		UT_uint32 t_id = t->getId();

		if(t_id > iId)
		{
			iId = t_id;
			r = t;
		}
	}

	return r;
}

bool AD_Document::addRevision(UT_uint32 iId, UT_UCS4Char * pDesc, time_t tStart, UT_uint32 iVer, bool bGenCR)
{
	for(UT_sint32 i = 0; i < m_vRevisions.getItemCount(); i++)
	{
		const AD_Revision * r = m_vRevisions.getNthItem(i);
		if(r->getId() == iId)
			return false;
	}

	AD_Revision * pRev = new AD_Revision(iId, pDesc, tStart, iVer);
	addRevision(pRev, bGenCR);
	m_iRevisionID = iId;
	return true;
}

bool AD_Document::addRevision(UT_uint32 iId,
							  const UT_UCS4Char * pDesc, UT_uint32 iLen,
							  time_t tStart, UT_uint32 iVer,bool bGenCR)
{
	for(UT_sint32 i = 0; i < m_vRevisions.getItemCount(); i++)
	{
		const AD_Revision * r = m_vRevisions.getNthItem(i);
		if(r->getId() == iId)
			return false;
	}

	UT_UCS4Char * pD = NULL;
	
	if(pDesc)
	{
		pD = new UT_UCS4Char [iLen + 1];
		UT_UCS4_strncpy(pD,pDesc,iLen);
		pD[iLen] = 0;
	}
	
	AD_Revision * pRev = new AD_Revision(iId, pD, tStart, iVer);
	addRevision(pRev,bGenCR);
	m_iRevisionID = iId;
	return true;
}

bool AD_Document::addRevision(AD_Revision * pRev, bool bGenCR)
{
	m_vRevisions.addItem(pRev);
	if(bGenCR)
	{
		const gchar * szAtts[11]={"docprop","revision",
							 "revision",NULL,
							 "revision-desc",NULL,
							 "revision-time",NULL,
							 "revision-ver",NULL,NULL};
		UT_UTF8String sID,sTime,sVer;
		UT_UTF8String_sprintf(sID,"%d",pRev->getId());
		UT_UTF8String_sprintf(sTime,"%d",pRev->getStartTime());
		UT_UTF8String_sprintf(sVer,"%d",pRev->getVersion());
		UT_UTF8String sDesc(pRev->getDescription());
		szAtts[3]= sID.utf8_str();
		szAtts[5] = sDesc.utf8_str();
		szAtts[7] = sTime.utf8_str();
		szAtts[9] = sVer.utf8_str();
		createAndSendDocPropCR(szAtts,NULL);
	}
	forceDirty();
	return true;
}

void AD_Document::_purgeRevisionTable()
{
	UT_VECTOR_PURGEALL(AD_Revision*, m_vRevisions);
	m_vRevisions.clear();
}

void AD_Document::setMarkRevisions(bool bMark)
{
	if(m_bMarkRevisions != bMark)
	{
		m_bMarkRevisions = bMark;
		forceDirty();
	}
}

void AD_Document::toggleMarkRevisions()
{
	setMarkRevisions(!m_bMarkRevisions);
}

void AD_Document::setShowRevisions(bool bShow)
{
	if(m_bShowRevisions != bShow)
	{
		m_bShowRevisions = bShow;
		forceDirty();
	}
}

void AD_Document::toggleShowRevisions()
{
	setShowRevisions(!m_bShowRevisions);
}

void AD_Document::setShowRevisionId(UT_uint32 iId)
{
	if(iId != m_iShowRevisionID)
	{
		m_iShowRevisionID = iId;
		forceDirty();
	}
}

void AD_Document::setRevisionId(UT_uint32 iId)
{
	if(iId != m_iRevisionID)
	{
		m_iRevisionID  = iId;
		// not in this case; this value is not persistent between sessions
		//forceDirty();
	}
}

void AD_Document::setAutoRevisioning(bool b)
{
	if(b != m_bAutoRevisioning)
	{
		// First of all, we will increase the document version number;
		// this will allow us to match autorevision id to a document
		// version number. However, do not increase the version number,
		// etc., if == 0 (we have a new, unsaved document)
		time_t t = time(NULL);
		
		if(m_bAfterFirstSave)
		{
			m_iVersion++;
		
			AD_VersionData v(m_iVersion, t, b, getTopXID());
			addRecordToHistory(v);
		}
		
		m_bAutoRevisioning = b;

		if(b)
		{
			// now create new revision
			// if we did not adjust version because we have just been
			// loaded and not saved, we do not want to adjust the
			// revision number either
			if(m_bAfterFirstSave)
			{	
				const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
				UT_return_if_fail(pSS);
				UT_UCS4String ucs4(pSS->getValue(XAP_STRING_ID_MSG_AutoRevision));

				UT_uint32 iId = getRevisionId() + 1;

				setRevisionId(iId);
				addRevision(iId, ucs4.ucs4_str(),ucs4.length(),t, m_iVersion);
			}
			else if(getHighestRevisionId() != getRevisionId())
			{
				// we have not saved yet, but the revision we are
				// going to be using is not in the revision table, add it
				const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
				UT_return_if_fail(pSS);
				UT_UCS4String ucs4(pSS->getValue(XAP_STRING_ID_MSG_AutoRevision));

				UT_uint32 iId = getRevisionId();
				addRevision(iId, ucs4.ucs4_str(),ucs4.length(),t, m_iVersion);
			}
			
				
			// collapse all revisions ...
			setShowRevisionId(PD_MAX_REVISION);
			setShowRevisions(false);
		}
		else
		{
			// we have to wipe out all of the revision information
			// from the document; this is because non-revisioned items
			// are treated as revision level 0 and so any
			// revisions in the document would overshadow any
			// subsequent changes -- see bug 7183

			// step out of revision mode ...
			_setMarkRevisions(false);
			m_bAutoRevisioning = false;

			if(acceptAllRevisions())
			{
				// we succeeded in restoring the document, so now clear the
				// history record
				_purgeRevisionTable();
				
				m_bDoNotAdjustHistory = true;
				save();
				m_bDoNotAdjustHistory = false;
			}

			// go back to revisioning mode so that the
			// setMarkRevisions() below triggers rebuild ...
			_setMarkRevisions(true);

		}
		
		setMarkRevisions(b);
	}
}

/*!
   Find revision id that corresponds to given document version
   \return: id > 0 on success or 0 on failure
*/
UT_uint32 AD_Document::findAutoRevisionId(UT_uint32 iVersion) const
{
	for(UT_sint32 i = 0; i < m_vRevisions.getItemCount(); i++)
	{
		const AD_Revision *pRev= m_vRevisions.getNthItem(i);
		UT_return_val_if_fail(pRev, 0);
		
		if(pRev->getVersion() == iVersion)
			return pRev->getId();
	}

	UT_DEBUGMSG(("AD_Document::findAutoRevisionId: autorevision for version %d not found\n",
				 iVersion));

	return 0;
}

/*!
    Finds the nearest autorevision for the given document version
    \param UT_uint32 iVersion: the document version
    \param bool bLesser: indicates whether nearest lesser or nearest
                         greater autorevision is required
    \return: id > 0 on success or 0 on failure
*/
UT_uint32 AD_Document::findNearestAutoRevisionId(UT_uint32 iVersion, bool bLesser) const
{
	UT_uint32 iId = 0;
	
	for(UT_sint32 i = 0; i < m_vRevisions.getItemCount(); i++)
	{
		const AD_Revision *pRev= m_vRevisions.getNthItem(i);
		UT_return_val_if_fail(pRev, 0);

		if(bLesser)
		{
			if(pRev->getVersion() < iVersion)
				iId = pRev->getId();
			else
				break;
		}
		else
		{
			if(pRev->getVersion() > iVersion)
				return pRev->getId();
		}
	}

#ifdef DEBUG
	if(iId == 0)
	{
		UT_DEBUGMSG(("AD_Document::findNearestAutoRevisionId: not found [ver. %d, bLesser=%d]\n",
					 iVersion, bLesser));
	}
#endif
	return iId;
}


/*!
    Update document history and version information; should only be
    called inside save() and saveAs()
*/
void AD_Document::_adjustHistoryOnSave()
{
	if(m_bDoNotAdjustHistory)
		return;
	
	// record this as the last time the document was saved + adjust
	// the cumulative edit time
	m_iVersion++;
	
	if(!m_bHistoryWasSaved || m_bAutoRevisioning)
	{
		// if this is the first save, we will record the time the doc
		// was opened as the start time, otherwise, we will use the
		// current time
		time_t t = !m_bHistoryWasSaved ? m_lastOpenedTime : time(NULL);
		
		AD_VersionData v(m_iVersion,t,m_bAutoRevisioning,getTopXID());
		m_lastSavedTime = v.getTime(); // store the time of this save
		addRecordToHistory(v);

		m_bHistoryWasSaved = true;
	}
	else
	{
		UT_return_if_fail(m_vHistory.getItemCount() > 0);

		// change the edit time of the last entry and create a new UID
		// for the record
		AD_VersionData * v = (AD_VersionData*)m_vHistory.getNthItem(m_vHistory.getItemCount()-1);

		UT_return_if_fail(v);
		v->setId(m_iVersion);
		v->newUID();
		m_lastSavedTime = v->getTime();
	}

	if(m_bAutoRevisioning)
	{
		// create new revision
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
		UT_return_if_fail(pSS);
		UT_UCS4String ucs4(pSS->getValue(XAP_STRING_ID_MSG_AutoRevision));

		UT_uint32 iId = getRevisionId()+1;
		setRevisionId(iId);
		addRevision(iId, ucs4.ucs4_str(),ucs4.length(),time(NULL), m_iVersion);
	}
}

bool AD_Document::_restoreVersion(XAP_Frame * pFrame, UT_uint32 iVersion)
{
	UT_return_val_if_fail(pFrame, false);
	
	if(isDirty())
	{
		if(pFrame->showMessageBox(XAP_STRING_ID_MSG_HistoryConfirmSave,
								  XAP_Dialog_MessageBox::b_YN,
								  XAP_Dialog_MessageBox::a_YES,
								  getFilename().c_str())
		   == XAP_Dialog_MessageBox::a_NO)
		{
			return false;
		}

		save();
	}

	// save the document under a different name ...
	// create unique new name
	UT_uint32 i = 0;

	std::string path = getFilename();
	std::string extension;

	size_t ndot = path.find_last_of('.');
	if(ndot != std::string::npos) {
		extension = path.substr(ndot+1);
		path.resize(ndot);
	}

	std::string s1;

	do {
		i++;

		s1 = path + UT_std_string_sprintf("_version_%d-%d", iVersion, i);

		if(!extension.empty()) {
			s1 += "." + extension;
		}

	} while(UT_isRegularFile(s1.c_str()));

	m_bDoNotAdjustHistory = true;
	saveAs(s1.c_str(), getLastSavedAsType());
	m_bDoNotAdjustHistory = false;

	// step out of revision mode ...
	_setMarkRevisions(false);
	m_bAutoRevisioning = false;

	UT_uint32 iRevisionId = findAutoRevisionId(iVersion);

	// the revision id is the id of a revision that has been used to
	// modify version iVersion of the document. To restore iVersion,
	// we therefore have to reject all revisions >= iRevisionId
	UT_return_val_if_fail( iRevisionId > 0, false );
	iRevisionId--;
	
	if(rejectAllHigherRevisions(iRevisionId))
	{
		// we succeeded in restoring the document, so now clear the
		// history record
		UT_sint32 iCount = getHistoryCount();
		const AD_VersionData * pVLast = NULL;
		time_t iEditTime = 0;
		
		for(UT_sint32 j = 0; j < iCount; ++j)
		{
			AD_VersionData * v = (AD_VersionData *)m_vHistory.getNthItem(j);
			if(!v)
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				continue;
			}

			if(v->getId() == iVersion)
			{
				pVLast = v;
				continue;
			}
			
			if(v->getId() > iVersion)
			{
				// remember the lenth of the session
				iEditTime += (v->getTime() - v->getStartTime());
				
				delete v;
				m_vHistory.deleteNthItem(j);
				iCount--;
				j--;
			}
		}

		UT_return_val_if_fail(pVLast,false);
		
		// set the document version correctly
		setDocVersion(iVersion);
		setLastSavedTime(pVLast->getTime());
		setLastOpenedTime(time(NULL));

		UT_ASSERT(m_iEditTime >= iEditTime);
		m_iEditTime -= iEditTime;
		
		// now save me as I am
		m_bDoNotAdjustHistory = true;
		save();
		_clearUndo();
		m_bDoNotAdjustHistory = false;
	}

	return true;
}

bool AD_Document::showHistory(AV_View * pView)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());	
	UT_return_val_if_fail(pFrame, false);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_History * pDialog
	  = static_cast<XAP_Dialog_History *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_HISTORY));
	
	UT_return_val_if_fail(pDialog,false);

	pDialog->setDocument(this);
	pDialog->runModal(pFrame);
	
	bool bShow   = (pDialog->getAnswer() == XAP_Dialog_History::a_OK);
	bool bRet = false;

	if (bShow)
	{
		UT_uint32 iVersion = pDialog->getSelectionId();
		UT_uint32 iOrigVersion = iVersion;
		
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		if(iVersion)
		{
			switch(verifyHistoryState(iVersion))
			{
				case ADHIST_PARTIAL_RESTORE:
					{
						// display warning message allowing user to
						// cancel, etc.
						UT_return_val_if_fail(pSS,false);
						UT_String s1, s2;
						const char * msg1, * msg2, * msg3, * msg4;
						
						if(iVersion)
						{
							// full restore possible
							msg1 = pSS->getValue(XAP_STRING_ID_MSG_HistoryPartRestore1);
							msg2 = pSS->getValue(XAP_STRING_ID_MSG_HistoryPartRestore2);
							msg4 = pSS->getValue(XAP_STRING_ID_MSG_HistoryPartRestore4);
							UT_return_val_if_fail(msg1 && msg2 && msg4, false);

							s1 = msg1;
							s1 += " ";
							s1 += msg2;
							s1 += " ";
							s1 += msg4;

							UT_String_sprintf(s2,s1.c_str(),iOrigVersion,iVersion,iOrigVersion);
						
							switch(pFrame->showMessageBox(s2.c_str(), 
														  XAP_Dialog_MessageBox::b_YNC, 
														  XAP_Dialog_MessageBox::a_YES))
							{
								case XAP_Dialog_MessageBox::a_NO:
									bRet = _restoreVersion(pFrame, iOrigVersion);
									break;
									
								case XAP_Dialog_MessageBox::a_YES:
									bRet = _restoreVersion(pFrame, iVersion);
									break;

								case XAP_Dialog_MessageBox::a_CANCEL:
									break;

								default:
									UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
							}
							
						}
						else
						{
							// full restore not possible
							msg1 = pSS->getValue(XAP_STRING_ID_MSG_HistoryPartRestore1);
							msg3 = pSS->getValue(XAP_STRING_ID_MSG_HistoryPartRestore3);
							msg4 = pSS->getValue(XAP_STRING_ID_MSG_HistoryPartRestore4);
							UT_return_val_if_fail(msg1 && msg3 && msg4,false);

							s1 = msg1;
							s1 += " ";
							s1 += msg3;
							s1 += " ";
							s1 += msg4;
						
							UT_String_sprintf(s2, s1.c_str(), iOrigVersion);

							switch(pFrame->showMessageBox(s2.c_str(), 
														  XAP_Dialog_MessageBox::b_OC, 
														  XAP_Dialog_MessageBox::a_OK))
							{
								case XAP_Dialog_MessageBox::a_OK:
									bRet = _restoreVersion(pFrame, iOrigVersion);
									break;

								case XAP_Dialog_MessageBox::a_CANCEL:
									break;

								default:
									UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
							}
							
						}
					}
					break;
					
				case ADHIST_FULL_RESTORE:
					bRet = _restoreVersion(pFrame, iVersion);
					break;
					
				case ADHIST_NO_RESTORE:
					// issue a warning message and quit
					{
						
						UT_return_val_if_fail(pSS,false);
						UT_String s2;
						const char * msg1;
						
						msg1 = pSS->getValue(XAP_STRING_ID_MSG_HistoryNoRestore);
						UT_return_val_if_fail(msg1,false);
						
						UT_String_sprintf(s2, msg1, iOrigVersion);

						pFrame->showMessageBox(s2.c_str(), 
											   XAP_Dialog_MessageBox::b_O, 
											   XAP_Dialog_MessageBox::a_OK);
					}
					break;
					
				default:
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			}
		}
	}

	pDialogFactory->releaseDialog(pDialog);

	return bRet;
}

UT_Error AD_Document::saveAs(const char * szFilename, int ieft, const char * props)
{
	UT_Error e = _saveAs(szFilename, ieft, props);
	
	m_bAfterFirstSave |= (UT_OK == e);
	return e;
}


UT_Error AD_Document::saveAs(const char * szFilename, int ieft, bool cpy, const char * props)
{
	UT_Error e = _saveAs(szFilename, ieft, cpy, props);

	m_bAfterFirstSave |= (UT_OK == e);
	return e;
}


UT_Error AD_Document::save(void)
{
	UT_Error e = _save();

	m_bAfterFirstSave |= (UT_OK == e);
	return e;
}

bool AD_Document::purgeAllRevisions(AV_View * pView)
{
	UT_return_val_if_fail( pView, false );
	
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());	
	UT_return_val_if_fail( pFrame, false );
	
	if(pFrame->showMessageBox(XAP_STRING_ID_MSG_NoUndo,
							  XAP_Dialog_MessageBox::b_YN,
							  XAP_Dialog_MessageBox::a_YES,
							  getFilename().c_str())
	   == XAP_Dialog_MessageBox::a_NO)
	{
		return false;
	}
	
	setMarkRevisions(false);
	bool bRet = acceptAllRevisions();
	purgeRevisionTable(true);
	_clearUndo();
	return bRet;
}


///////////////////////////////////////////////////
// AD_VersionData
//
// constructor for new entries
AD_VersionData::AD_VersionData(UT_uint32 v, time_t start, bool autorev, UT_uint32 xid)
	:m_iId(v),m_pUUID(NULL),m_tStart(start),m_bAutoRevision(autorev),m_iTopXID(xid)
{
	// we do not create uuid's based on the main doc uuid as
	// AD_Document::getNewUUID() does; this is because we need to be
	// able to distinguish between versions of documents created at
	// different place at the same time.
	UT_UUIDGenerator * pGen = XAP_App::getApp()->getUUIDGenerator();
	UT_return_if_fail(pGen);
	
	m_pUUID = pGen->createUUID();
	UT_return_if_fail(m_pUUID);
	m_tStart = m_pUUID->getTime();
}


// constructors for importers
AD_VersionData::AD_VersionData(UT_uint32 v, UT_UTF8String &uuid, time_t start, bool autorev, UT_uint32 iTopXID):
	m_iId(v),m_pUUID(NULL),m_tStart(start),m_bAutoRevision(autorev),m_iTopXID(iTopXID)
{
	UT_UUIDGenerator * pGen = XAP_App::getApp()->getUUIDGenerator();
	UT_return_if_fail(pGen);
	
	m_pUUID = pGen->createUUID(uuid);
	UT_ASSERT_HARMLESS(m_pUUID);
}

AD_VersionData::AD_VersionData(UT_uint32 v, const char *uuid, time_t start, bool autorev, UT_uint32 iTopXID):
	m_iId(v),m_pUUID(NULL),m_tStart(start),m_bAutoRevision(autorev),m_iTopXID(iTopXID)
{
	UT_UUIDGenerator * pGen = XAP_App::getApp()->getUUIDGenerator();
	UT_return_if_fail(pGen);
	
	m_pUUID = pGen->createUUID(uuid);
	UT_ASSERT_HARMLESS(m_pUUID);
}

// copy constructor
AD_VersionData::AD_VersionData(const AD_VersionData & v):
	m_iId(v.m_iId), m_pUUID(NULL), m_bAutoRevision(v.m_bAutoRevision), m_iTopXID(v.m_iTopXID)
{
	UT_return_if_fail(v.m_pUUID);
	UT_UUIDGenerator * pGen = XAP_App::getApp()->getUUIDGenerator();
	UT_return_if_fail(pGen);
	
	m_pUUID = pGen->createUUID(*(v.m_pUUID));
	UT_ASSERT(m_pUUID);

	m_tStart = v.m_tStart;
}

AD_VersionData & AD_VersionData::operator = (const AD_VersionData &v)
{
	m_iId       = v.m_iId;
	*m_pUUID    = *(v.m_pUUID);
	m_tStart    = v.m_tStart;
	m_iTopXID   = v.m_iTopXID;
	m_bAutoRevision = v.m_bAutoRevision;
	return *this;
}

bool AD_VersionData::operator == (const AD_VersionData &v)
{
	return (m_iId == v.m_iId && m_tStart == v.m_tStart
			&& *m_pUUID == *(v.m_pUUID) && m_bAutoRevision == v.m_bAutoRevision && m_iTopXID == v.m_iTopXID);
}

AD_VersionData::~AD_VersionData()
{
	DELETEP(m_pUUID);
}

time_t AD_VersionData::getTime()const
{
	if(!m_pUUID)
		return 0;

	return m_pUUID->getTime();
}

bool AD_VersionData::newUID()
{
	if(!m_pUUID)
		return false;

	return m_pUUID->makeUUID();
}
