/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#include "ie_mailmerge.h"
#include "ut_vector.h"
#include "ut_string_class.h"
#include "ut_misc.h"

#include "ut_assert.h"
#include "ut_debugmsg.h"

static const UT_uint32 merge_size_guess = 3;
static UT_Vector m_sniffers (merge_size_guess);

/************************************************************************/
/************************************************************************/

IE_MailMerge::~IE_MailMerge ()
{
}

IE_MailMerge::IE_MailMerge ()
	: m_pListener (0)
{
}

IE_MergeSniffer::~IE_MergeSniffer()
{
}

/************************************************************************/
/************************************************************************/

bool IE_MailMerge::fireMergeSet ()
{
	PD_Document * pDoc = 0;
	
	pDoc = m_pListener->getMergeDocument ();
	if (pDoc) {
		UT_StringPtrMap::UT_Cursor _hc1(&m_map);
		for (const UT_UTF8String * _hval1 = static_cast<const UT_UTF8String *>(_hc1.first());
			 _hc1.is_valid(); 
			 _hval1 = static_cast<const UT_UTF8String *>(_hc1.next()) )
		{ 
			if (_hval1)
				pDoc->setMailMergeField (_hc1.key(), *_hval1);
			else
				pDoc->setMailMergeField (_hc1.key(), "");
		}
	}
	
	bool bret = m_pListener->fireUpdate ();
	UT_HASH_PURGEDATA(UT_UTF8String*, &m_map, delete) ;
	
	return bret;
}

void IE_MailMerge::addMergePair (const UT_UTF8String & key,
								 const UT_UTF8String & value)
{
	UT_UTF8String * ptrvalue = new UT_UTF8String ( value ) ;
	m_map.set ( key.utf8_str(), ptrvalue ) ;
}

void IE_MailMerge::setListener (IE_MailMerge_Listener * listener)
{
	m_pListener = listener;
}

/************************************************************************/
/************************************************************************/

IEMergeType IE_MailMerge::fileTypeForContents(const char * szBuf,
											  UT_uint32 iNumbytes){
  	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a match for all file types
	UT_uint32 nrElements = getMergerCount();
	
	IEMergeType best = IEMT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
	
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_MergeSniffer * s = static_cast<IE_MergeSniffer *>(m_sniffers.getNthItem (k));
		UT_Confidence_t confidence = s->recognizeContents(szBuf, iNumbytes);
		if ((confidence > 0) && ((IEMT_Unknown == best) || (confidence >= best_confidence)))
		{
			best_confidence = confidence;
			for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
		    {
				if (s->supportsFileType((IEMergeType) (a+1)))
				{
					best = (IEMergeType) (a+1);
					
					// short-circuit if we're 100% sure
					if ( UT_CONFIDENCE_PERFECT == best_confidence )
						return best;
					
					break;
				}
		    }
		}
	}
	
	return best;
}

IEMergeType IE_MailMerge::fileTypeForSuffix(const char * szSuffix)
{
	if (!szSuffix)
		return IEMT_Unknown;
	
	IEMergeType best = IEMT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getMergerCount();
	
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_MergeSniffer * s = static_cast<IE_MergeSniffer *>(m_sniffers.getNthItem(k));
		
		UT_Confidence_t confidence = s->recognizeSuffix(szSuffix);
		if ((confidence > 0) && ((IEMT_Unknown == best) || (confidence >= best_confidence)))
		{
			best_confidence = confidence;
			for (UT_sint32 a = 0; a < static_cast<int>(nrElements); a++)
			{
				if (s->supportsFileType(static_cast<IEMergeType>(a+1)))
				{
					best = static_cast<IEMergeType>(a+1);
					
					// short-circuit if we're 100% sure
					if ( UT_CONFIDENCE_PERFECT == best_confidence )
						return best;
					break;
				}
			}
		}
	}
	
	return best;	
}

IEMergeType IE_MailMerge::fileTypeForDescription(const char * szDescription)
{
	IEMergeType ieft = IEMT_Unknown;
	
	if (!szDescription)
		return ieft;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getMergerCount();
	
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_MergeSniffer * pSniffer = static_cast<IE_MergeSniffer *>(m_sniffers.getNthItem(k));
		
		const char * szDummy;
		const char * szDescription2 = 0;
		
		if (pSniffer->getDlgLabels(&szDescription2,&szDummy,&ieft))
		{
			if (!UT_strcmp(szDescription,szDescription2))
				return ieft;
		}
		else
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	
	return ieft;
}

IEMergeType IE_MailMerge::fileTypeForSuffixes(const char * suffixList)
{
	IEMergeType ieft = IEMT_Unknown;
	if (!suffixList)
		return ieft;
	
	UT_String utSuffix (suffixList);
	const size_t len = strlen(suffixList);
	size_t i = 0;
	
	while (true)
	{
		while (i < len && suffixList[i] != '.')
			i++;
		
		// will never have all-space extension
		
		const size_t start = i;
		while (i < len && suffixList[i] != ';')
			i++;
		
		if (i <= len) {
			UT_String suffix (utSuffix.substr(start, i-start).c_str());
			UT_DEBUGMSG(("DOM: suffix: %s\n", suffix.c_str()));
			
			ieft = fileTypeForSuffix (suffix.c_str());
			if (ieft != IEMT_Unknown || i == len)
				return ieft;
			
			i++;
		}
	}
	return ieft;
}

IE_MergeSniffer * IE_MailMerge::snifferForFileType(IEMergeType ieft)
{
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getMergerCount();
	
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_MergeSniffer * s = static_cast<IE_MergeSniffer*>(m_sniffers.getNthItem(k));
		if (s->supportsFileType(ieft))
			return s;
	}
	
	// The passed in filetype is invalid.
	return 0;
}

const char * IE_MailMerge::suffixesForFileType(IEMergeType ieft)
{
	const char * szDummy;
	const char * szSuffixes = 0;
	IEMergeType ieftDummy;

	IE_MergeSniffer * pSniffer = snifferForFileType(ieft);
	
	if (pSniffer->getDlgLabels(&szDummy,&szSuffixes,&ieftDummy))
		return szSuffixes;
	else
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	// The passed in filetype is invalid.
	return 0;
}

const char * IE_MailMerge::descriptionForFileType(IEMergeType ieft)
{
	const char * szDummy;
	const char * szDescription = 0;
	IEMergeType ieftDummy;
	
	IE_MergeSniffer * pSniffer = snifferForFileType(ieft);
	
	if (pSniffer->getDlgLabels(&szDescription,&szDummy,&ieftDummy))
		return szDescription;
	else
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	// The passed in filetype is invalid.
	return 0;
}

static UT_Confidence_t s_confidence_heuristic ( UT_Confidence_t content_confidence, 
												UT_Confidence_t suffix_confidence )
{
	return (UT_Confidence_t) ( (static_cast<double>(content_confidence) * 0.85) + (static_cast<double>(suffix_confidence) * 0.15) ) ;
}

UT_Error IE_MailMerge::constructMerger(const char * szFilename,
									   IEMergeType ieft,
									   IE_MailMerge ** ppie, 
									   IEMergeType * pieft)
{
	UT_return_val_if_fail(ieft != IEMT_Unknown || (szFilename && *szFilename), UT_ERROR);
	UT_return_val_if_fail(ppie, UT_ERROR);
	
	UT_uint32 nrElements = getMergerCount();
	
	// no filter will support IEMT_Unknown, so we try to detect
	// from the contents of the file or the filename suffix
	// the importer to use and assign that back to ieft.
	// Give precedence to the file contents
	if (ieft == IEMT_Unknown && szFilename && *szFilename)
	{
		char szBuf[4096] = "";  // 4096 ought to be enough
		UT_uint32 iNumbytes = 0;
		FILE *f = NULL;
		
		// we must open in binary mode for UCS-2 compatibility
		if ( ( f = fopen( szFilename, "rb" ) ) != static_cast<FILE *>(0) )
		{
			iNumbytes = fread(szBuf, 1, sizeof(szBuf), f);
			fclose(f);
		}
		
		UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
		IE_MergeSniffer * best_sniffer = 0;
		
		for (UT_uint32 k=0; k < nrElements; k++)
		{
		    IE_MergeSniffer * s = static_cast<IE_MergeSniffer *>(m_sniffers.getNthItem (k));

		    UT_Confidence_t content_confidence = UT_CONFIDENCE_ZILCH;
		    UT_Confidence_t suffix_confidence = UT_CONFIDENCE_ZILCH;
			
		    if ( iNumbytes > 0 )
				content_confidence = s->recognizeContents(szBuf, iNumbytes);
		    
		    const char * suffix = UT_pathSuffix(szFilename) ;
		    if ( suffix != NULL )
			{
				suffix_confidence = s->recognizeSuffix(UT_pathSuffix(szFilename));
			}
		    
		    UT_Confidence_t confidence = s_confidence_heuristic ( content_confidence, 
																  suffix_confidence ) ;
		    
		    if ( confidence != 0 && confidence >= best_confidence )
			{
				best_sniffer = s;
				best_confidence = confidence;
				ieft = (IEMergeType) (k+1);
			}
		}
		if (best_sniffer)
		{
			if (pieft != NULL) *pieft = ieft;
			return best_sniffer->constructMerger (ppie);
		}
	}
	
	UT_ASSERT_HARMLESS(ieft != IEMT_Unknown);
	
	// tell the caller the type of importer they got
	if (pieft != NULL) 
		*pieft = ieft;
	
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_MergeSniffer * s = static_cast<IE_MergeSniffer *>(m_sniffers.getNthItem (k));
		if (s->supportsFileType(ieft))
			return s->constructMerger(ppie);
	}
	
	return UT_ERROR;
}
  
bool IE_MailMerge::enumerateDlgLabels(UT_uint32 ndx,
									  const char ** pszDesc,
									  const char ** pszSuffixList,
									  IEMergeType * ft)
{
	UT_uint32 nrElements = getMergerCount();
	if (ndx < nrElements)
    {
		IE_MergeSniffer * s = static_cast<IE_MergeSniffer *>(m_sniffers.getNthItem (ndx));
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
    }
	
	return false;
}

UT_uint32 IE_MailMerge::getMergerCount(void)
{
	return m_sniffers.size();
}

void IE_MailMerge::registerMerger (IE_MergeSniffer * s)
{
	UT_uint32 ndx = 0;
	UT_Error err = m_sniffers.addItem (s, &ndx);
	
	UT_return_if_fail(err == UT_OK);
	UT_return_if_fail(ndx >= 0);
	
	s->setFileType(ndx+1);
}

void IE_MailMerge::unregisterMerger (IE_MergeSniffer * s)
{
	UT_uint32 ndx = s->getFileType(); // 1:1 mapping
	
	UT_return_if_fail(ndx >= 0);
	
	m_sniffers.deleteNthItem (ndx-1);
	
	// Refactor the indexes
	IE_MergeSniffer * pSniffer = 0;
	UT_uint32 size  = m_sniffers.size();
	UT_uint32 i     = 0;
	for(i = ndx-1; i < size; i++)
    {
		pSniffer = static_cast <IE_MergeSniffer *>(m_sniffers.getNthItem(i));
		if (pSniffer)
			pSniffer->setFileType(i+1);
    }
}

void IE_MailMerge::unregisterAllMergers ()
{
	IE_MergeSniffer * pSniffer = 0;
	UT_uint32 size = m_sniffers.size();
	
	for (UT_uint32 i = 0; i < size; i++)
    {
		pSniffer = static_cast <IE_MergeSniffer *>(m_sniffers.getNthItem(i));
		if (pSniffer)
			pSniffer->unref();
    }
}

/************************************************************************/
/************************************************************************/

class ABI_EXPORT IE_MailMerge_XML_Listener : public IE_MailMerge, 
											 public UT_XML::Listener
{
	
public:
	
	IE_MailMerge_XML_Listener ()
		: IE_MailMerge(), UT_XML::Listener(), mAcceptingText(false), mLooping(true)
		{
		}
	
	virtual ~IE_MailMerge_XML_Listener()
		{
		}
	
	virtual void startElement (const XML_Char * name, const XML_Char ** atts) 
		{
			mCharData.clear ();
			mKey.clear ();
			
			if (!UT_strcmp (name, "awmm:field"))
			{
				const XML_Char * key = UT_getAttribute("name", atts);
				if (key) {
					mKey = key;
					mAcceptingText = true;
				}
			}
		}
	
	virtual void endElement (const XML_Char * name)
		{
			if (!UT_strcmp(name, "awmm:field") && mCharData.size() && mLooping) {      
				addMergePair (mKey, mCharData);
			} 
			else if (!UT_strcmp(name, "awmm:record") && mLooping) {
				mLooping = fireMergeSet ();
				// todo: UT_XML::stop()
			}
			mCharData.clear ();
			mKey.clear ();
		}
	
	virtual void charData (const XML_Char * buffer, int length)
		{
			if (buffer && length && mAcceptingText && mLooping) {
				UT_String buf(buffer, length);
				mCharData += buf.c_str();
			}
		}

	virtual UT_Error mergeFile(const char* szFilename)
		{
			UT_XML default_xml;

			default_xml.setListener (this);
			return default_xml.parse (szFilename);
		}
	
private:
	
	UT_UTF8String mKey;
	UT_UTF8String mCharData;
	bool mAcceptingText;
	bool mLooping;
};

class ABI_EXPORT IE_XMLMerge_Sniffer : public IE_MergeSniffer
{
	
public:
	
	IE_XMLMerge_Sniffer(){}
	virtual ~IE_XMLMerge_Sniffer (){}
	
	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
											   UT_uint32 iNumbytes){
		if (strstr(szBuf, "http://www.abisource.com/mailmerge/1.0") != 0 &&
			strstr(szBuf, "merge-set") != 0)
			return UT_CONFIDENCE_PERFECT;
		return UT_CONFIDENCE_ZILCH;
	}
	
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix) {
		return (!UT_stricmp (szSuffix, ".xml") ? UT_CONFIDENCE_SOSO : UT_CONFIDENCE_POOR);
	}
	
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEMergeType * ft){
		*szDesc = "XML Mail Merge";
		*szSuffixList = ".xml";
		*ft = getFileType();
		return true;
	}
	
	virtual UT_Error constructMerger (IE_MailMerge ** ppie) {
		*ppie = new IE_MailMerge_XML_Listener ();
		return UT_OK;
	}
	
};

/************************************************************************/
/************************************************************************/

class ABI_EXPORT IE_MailMerge_Delimiter_Listener : public IE_MailMerge
{
	
public:
	
	explicit IE_MailMerge_Delimiter_Listener (char delim)
		: IE_MailMerge(), m_delim(delim), mLooping(true)
		{
		}
	
	virtual ~IE_MailMerge_Delimiter_Listener()
		{
			UT_VECTOR_PURGEALL(UT_UTF8String*, m_headers);
			UT_VECTOR_PURGEALL(UT_UTF8String*, m_items);
		}
	
	virtual UT_Error mergeFile(const char * szFilename) {
		// TODO: this isn't utf-8 correct by a long shot

		UT_UTF8String item;

		char ch[2];
		ch[1] = '\0';

		UT_uint32 lineno = 0;
		bool cont = true;

		FILE * fp = fopen(szFilename, "rb");
		if (!fp)
			return UT_ERROR;

		// line 1 == Headings/titles
		// line 2..n == Data

		while (cont && (1 == fread (ch, 1, 1, fp))){
			if (ch[0] == '\r')
				continue;
			else if (ch[0] == '\n') {
				defineItem (item, lineno == 0);
				if (lineno != 0)
					cont = fire ();
				lineno++;
				item.clear ();
				continue;
			}
			else if (ch[0] == m_delim) {
				defineItem (item, lineno == 0);
				item.clear ();
			}
			else
				item += ch;
		}

		fclose (fp);

		return UT_ERROR;
	}
	
private:
	
	void defineItem (const UT_UTF8String & item, bool isHeader)
		{
			if (!item.size())
				return;

			UT_UTF8String * dup = new UT_UTF8String (item);
			if (isHeader)
				m_headers.addItem (static_cast<void *>(dup));
			else
				m_items.addItem(static_cast<void *>(dup));
		}

	bool fire ()
		{
			if (m_headers.size () != m_items.size ()) {
				UT_ASSERT(m_headers.size () == m_items.size ());
				return false;
			}

			for (UT_uint32 i = 0; i < m_headers.size (); i++) {
				UT_UTF8String * key, * val;

				key = static_cast<UT_UTF8String *>(m_headers.getNthItem(i));
				val = static_cast<UT_UTF8String *>(m_items.getNthItem(i));

				addMergePair (*key, *val);
			}

			UT_VECTOR_PURGEALL(UT_UTF8String*, m_items);
			m_items.clear ();

			return fireMergeSet ();
		}

	UT_Vector m_headers;
	UT_Vector m_items;

	char m_delim;
	bool mLooping;
};

class ABI_EXPORT IE_Delimiter_Sniffer : public IE_MergeSniffer
{
	
public:
	
	IE_Delimiter_Sniffer(const UT_UTF8String & desc,
						 const UT_UTF8String & suffix,
						 char delim): 
		m_desc(desc), m_suffix (suffix), m_delim(delim)
		{}

	virtual ~IE_Delimiter_Sniffer (){}
	
	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
											   UT_uint32 iNumbytes){
		char delim[2];
		delim[0] = m_delim;
		delim[1] = '\0';

		if (strstr(szBuf, delim) != 0)
			return UT_CONFIDENCE_SOSO;
		return UT_CONFIDENCE_ZILCH;
	}
	
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix) {
		return (!UT_stricmp (szSuffix, m_suffix.utf8_str()) ? UT_CONFIDENCE_PERFECT : UT_CONFIDENCE_POOR);
	}

	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEMergeType * ft){
		*szDesc = m_desc.utf8_str();
		*szSuffixList = m_suffix.utf8_str();
		*ft = getFileType();

		return true;
	}
	
	virtual UT_Error constructMerger (IE_MailMerge ** ppie) {
		*ppie = new IE_MailMerge_Delimiter_Listener(m_delim);
		return UT_OK;
	}
	
private:
	UT_UTF8String m_desc;
	UT_UTF8String m_suffix;
	char m_delim;
};

/************************************************************************/
/************************************************************************/

void IE_MailMerge_RegisterXP ()
{
	IE_MailMerge::registerMerger (new IE_XMLMerge_Sniffer ());
	IE_MailMerge::registerMerger (new IE_Delimiter_Sniffer ("Comma Separated Values", ".csv", ','));
	IE_MailMerge::registerMerger (new IE_Delimiter_Sniffer ("Tabbed Text", ".tdt", '\t'));
}

void IE_MailMerge_UnRegisterXP ()
{
	IE_MailMerge::unregisterAllMergers ();
}
