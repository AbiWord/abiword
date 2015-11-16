/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ie_mailmerge.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_go_file.h"
#include "ut_std_vector.h"
#include "pd_Document.h"

#include <gsf/gsf-input.h>

static std::vector<IE_MergeSniffer *> s_sniffers;

std::vector<IE_MergeSniffer *> & IE_MailMerge::getSniffers()
{
    return s_sniffers;
}

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
		std::map<std::string, std::string>::const_iterator iter;
		for(iter = m_map.begin(); iter != m_map.end(); ++iter) {
			pDoc->setMailMergeField (iter->first, iter->second);
		}
	}
	
	bool bret = m_pListener->fireUpdate ();
	m_map.clear();
	
	return bret;
}

void IE_MailMerge::addMergePair (const std::string & key,
								 const std::string & value)
{
	m_map[key] = value;
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

	for (std::vector<IE_MergeSniffer *>::const_iterator iter = s_sniffers.begin();
         iter != s_sniffers.end(); ++iter)
	{
		IE_MergeSniffer * s = *iter;
		UT_Confidence_t confidence = s->recognizeContents(szBuf, iNumbytes);
		if ((confidence > 0) && ((IEMT_Unknown == best) || (confidence >= best_confidence)))
		{
			best_confidence = confidence;
			for (UT_uint32 a = 0; a < nrElements; a++)
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
	if (!szSuffix || !*szSuffix)
		return IEMT_Unknown;
	
	IEMergeType best = IEMT_Unknown;
	UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
	
	// we have to construct the loop this way because a
	// given filter could support more than one file type,
	// so we must query a suffix match for all file types
	UT_uint32 nrElements = getMergerCount();
	
	for (UT_uint32 k=0; k < nrElements; k++)
	{
		IE_MergeSniffer * s = s_sniffers.at(k);
		
		UT_Confidence_t confidence = s->recognizeSuffix(szSuffix);
		if ((confidence > 0) && ((IEMT_Unknown == best) || (confidence >= best_confidence)))
		{
			best_confidence = confidence;
			for (UT_uint32 a = 0; a < nrElements; a++)
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
		IE_MergeSniffer * pSniffer = s_sniffers.at(k);
		
		const char * szDummy;
		const char * szDescription2 = 0;
		
		if (pSniffer->getDlgLabels(&szDescription2,&szDummy,&ieft))
		{
			if (!strcmp(szDescription,szDescription2))
				return ieft;
		}
		else
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}
	
	return ieft;
}

IEMergeType IE_MailMerge::fileTypeForSuffixes(const char * suffixList)
{
	IEMergeType ieft = IEMT_Unknown;
	if (!suffixList)
		return ieft;
	
	std::string utSuffix (suffixList);
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
			std::string suffix (utSuffix.substr(start, i-start));
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
		IE_MergeSniffer * s = s_sniffers.at(k);
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
	{
		return szSuffixes;
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

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
	{
		return szDescription;
	}
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

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
		char szBuf[4097] = "";  // 4096 ought to be enough
		UT_uint32 iNumbytes = 0;
		GsfInput *f = NULL;
		
		// we must open in binary mode for UCS-2 compatibility
		if ( ( f = UT_go_file_open( szFilename, NULL ) ) != NULL )
		{
		  gsf_off_t stream_size = gsf_input_size(f);
		  if (stream_size == -1)
				return UT_ERROR;
		  iNumbytes = UT_MIN(sizeof(szBuf) - 1, static_cast<UT_uint64>(stream_size));
		  gsf_input_read(f, iNumbytes, (guint8*)szBuf);
		  g_object_unref(G_OBJECT(f));
		  szBuf[iNumbytes] = '\0';
		}
		
		UT_Confidence_t   best_confidence = UT_CONFIDENCE_ZILCH;
		IE_MergeSniffer * best_sniffer = 0;
		
		for (UT_uint32 k=0; k < nrElements; k++)
		{
		    IE_MergeSniffer * s = s_sniffers.at(k);

		    UT_Confidence_t content_confidence = UT_CONFIDENCE_ZILCH;
		    UT_Confidence_t suffix_confidence = UT_CONFIDENCE_ZILCH;
			
		    if ( iNumbytes > 0 )
				content_confidence = s->recognizeContents(szBuf, iNumbytes);
		    
		    std::string suffix = UT_pathSuffix(szFilename) ;
		    if (!suffix.empty())
			{
				suffix_confidence = s->recognizeSuffix(suffix.c_str());
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
		IE_MergeSniffer * s = s_sniffers.at(k);
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
		IE_MergeSniffer * s = s_sniffers.at(ndx);
		return s->getDlgLabels(pszDesc,pszSuffixList,ft);
    }
	
	return false;
}

UT_uint32 IE_MailMerge::getMergerCount(void)
{
	return s_sniffers.size();
}

void IE_MailMerge::registerMerger (IE_MergeSniffer * s)
{
	s_sniffers.push_back(s);

	s->setFileType(s_sniffers.size());
}

void IE_MailMerge::unregisterAllMergers ()
{
    UT_std_vector_purgeall(s_sniffers);
    s_sniffers.clear();
}

/************************************************************************/
/************************************************************************/

class ABI_EXPORT IE_MailMerge_XML_Listener : public IE_MailMerge, 
											 public UT_XML::Listener
{
	
public:
	
	IE_MailMerge_XML_Listener ()
		: IE_MailMerge(), UT_XML::Listener(), mAcceptingText(false), mLooping(true), m_vecHeaders(0)
		{
		}
	
	virtual ~IE_MailMerge_XML_Listener()
		{
		}
	
	virtual void startElement (const gchar * name, const gchar ** atts) 
		{
			mCharData.clear ();
			mKey.clear ();
			
			if (!strcmp (name, "awmm:field"))
			{
				const gchar * key = UT_getAttribute("name", atts);
				if (key) {
					mKey = key;
					mAcceptingText = true;
				}
			}
		}
	
	virtual void endElement (const gchar * name)
		{
			if (!strcmp(name, "awmm:field") && mLooping) {      
				if (m_vecHeaders)
					addOrReplaceVecProp (mKey);
				else
					addMergePair (mKey, mCharData);
			} 
			else if (!strcmp(name, "awmm:record") && mLooping) {
				if (m_vecHeaders)
					mLooping = false;
				else
					mLooping = fireMergeSet ();
				// todo: UT_XML::stop()
			}
			mCharData.clear ();
			mKey.clear ();
		}
	
	virtual void charData (const gchar * buffer, int length)
		{
			if (buffer && length && mAcceptingText && mLooping) {
				std::string buf(buffer, length);
				mCharData += buf;
			}
		}

	virtual UT_Error mergeFile(const char* szFilename)
		{
			UT_XML default_xml;

			default_xml.setListener (this);

			std::string sFile;
			convertURI(sFile, szFilename);
			return default_xml.parse (sFile.c_str());
		}
	
	virtual UT_Error getHeaders (const char * szFilename, std::vector<std::string> & out_vec) {
		UT_XML default_xml;
		
		m_vecHeaders = &out_vec;
		
		default_xml.setListener (this);

		std::string sFile;
		convertURI(sFile, szFilename);
		return default_xml.parse (sFile.c_str());
	}

private:
	
	void addOrReplaceVecProp (const std::string & str) {
		UT_sint32 iCount = m_vecHeaders->size();
		
		UT_sint32 i = 0;
		for(i=0; i < iCount ; i ++)
		{
			const std::string & prop = m_vecHeaders->at(i);
			if (prop == str)
				return;
		}

		m_vecHeaders->push_back (str);
	}

	void convertURI(std::string &sFile, const char *szURI)
	{
		bool bURI = UT_go_path_is_uri(szURI);
		if(bURI)
		{
			const gchar *filename = UT_go_filename_from_uri(szURI);
			sFile = filename;
			FREEP(filename);
		}
		else
		{
			sFile = szURI;
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		}
	}

	std::string mKey;
	std::string mCharData;
	bool mAcceptingText;
	bool mLooping;

	std::vector<std::string> * m_vecHeaders;
};

class ABI_EXPORT IE_XMLMerge_Sniffer : public IE_MergeSniffer
{
	
public:
	
	IE_XMLMerge_Sniffer(){}
	virtual ~IE_XMLMerge_Sniffer (){}
	
	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
											   UT_uint32 /*iNumbytes*/){
		if (strstr(szBuf, "http://www.abisource.com/mailmerge/1.0") != 0 &&
			strstr(szBuf, "merge-set") != 0)
			return UT_CONFIDENCE_PERFECT;
		return UT_CONFIDENCE_ZILCH;
	}
	
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix) {
		return (!g_ascii_strcasecmp (szSuffix, ".xml") ? UT_CONFIDENCE_SOSO : UT_CONFIDENCE_POOR);
	}
	
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEMergeType * ft){
		*szDesc = "XML Mail Merge (*.xml)";
		*szSuffixList = "*.xml";
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
		}

	UT_Error mergeFile(const char * szFilename, bool justHeaders) 
		{
		  UT_ByteBuf item;

		  UT_Byte ch;
		  
		  UT_uint32 lineno = 0;
		  bool cont = true;
		  bool in_quotes = false;
		  
		  GsfInput * fp = UT_go_file_open(szFilename, NULL);
		  if (!fp)
		    return UT_ERROR;
		  
		  m_headers.clear();
		  m_items.clear();
		  
		  // line 1 == Headings/titles
		  // line 2..n == Data
		  
		  while (cont && (NULL != gsf_input_read (fp, 1, &ch))){
		    if (ch == '\r' && !in_quotes) // swallow carriage return unless in quoted block
		      continue;
		    else if (ch == '\n' && !in_quotes) { // newline. fire changeset
		      defineItem (item, lineno == 0);
		      item.truncate (0);

		      if(justHeaders)
			break;

		      if (lineno != 0)
			cont = fire ();

		      lineno++;
		    }
		    else if (ch == m_delim && !in_quotes) {
		      defineItem (item, lineno == 0);
		      item.truncate (0);
		    }
		    else if (ch == '"' && in_quotes) {
		      if (NULL != gsf_input_read (fp, 1, &ch)) {
			if (ch == '"') // 2 double quotes == escaped quote
			  item.append (&ch, 1);
			else { // assume that it's the end of the quoted sequence and ch is the delimiter char or a newline
			  in_quotes = false;
			  defineItem (item, lineno == 0);
			  item.truncate (0);
			  
			  if (ch == '\n') {
			    if(justHeaders)
			      break;

			    if (lineno != 0)
			      cont = fire ();
			    lineno++;
			  }
			}
		      } else {
			// eof??
			defineItem (item, lineno == 0);
			item.truncate (0);
			in_quotes = false;
		      }
		    }
		    else if ((ch == '"') && !in_quotes && (item.getLength () == 0)) // beginning of a quoted sequence
		      in_quotes = true;
		    else // append whatever we're given
		      item.append(&ch, 1);
		  }
		  
		  g_object_unref (G_OBJECT(fp));
		  
		  // if there's a non-empty line that wasn't terminated by a newline, fire it off
		  if (m_items.size())
		    fire ();
		  
		  return UT_OK;
		}

	virtual UT_Error mergeFile(const char * szFilename) {
	  return mergeFile(szFilename, false);
	}

	virtual UT_Error getHeaders (const char * szFilename, std::vector<std::string> & out_vec) {
	  out_vec.clear();
	  UT_Error err = mergeFile(szFilename, true);

	  if (err == UT_OK) {
		  out_vec.insert(out_vec.begin(), m_headers.begin(), m_headers.end());
	  }

	  return err;
	}
	
private:
	
	void defineItem (const UT_ByteBuf & item, bool isHeader)
		{
			std::string dup((const char *)item.getPointer(0), item.getLength());
			if (isHeader)
				m_headers.push_back (dup);
			else
				m_items.push_back(dup);
		}

	bool fire ()
		{
			if (m_headers.size () != m_items.size ()) {
				UT_ASSERT_HARMLESS(m_headers.size () == m_items.size ());
				return false;
			}

			for (std::size_t i = 0; i < m_headers.size (); i++) {
				std::string key, val;

				key = m_headers.at(i);
				val = m_items.at(i);

				addMergePair (key, val);
			}

			m_items.clear ();

			return fireMergeSet ();
		}

	std::vector<std::string> m_headers;
	std::vector<std::string> m_items;

	char m_delim;
	bool mLooping;
};

class ABI_EXPORT IE_Delimiter_Sniffer : public IE_MergeSniffer
{
	
public:
	
	IE_Delimiter_Sniffer(const std::string & desc,
						 const std::string & suffix,
						 char delim)
		: m_desc(desc), m_suffix (suffix), m_delim(delim)
		{}

	virtual ~IE_Delimiter_Sniffer (){}
	
	virtual UT_Confidence_t recognizeContents (const char * szBuf, 
											   UT_uint32 /*iNumbytes*/){
		char delim[2];
		delim[0] = m_delim;
		delim[1] = '\0';

		if (strstr(szBuf, delim) != 0)
			return UT_CONFIDENCE_SOSO;
		return UT_CONFIDENCE_ZILCH;
	}
	
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix) {
	  // skip over "*"
		return (!g_ascii_strcasecmp (szSuffix, (m_suffix.c_str() + 1)) ? UT_CONFIDENCE_PERFECT : UT_CONFIDENCE_POOR);
	}

	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEMergeType * ft){
		*szDesc = m_desc.c_str();
		*szSuffixList = m_suffix.c_str();
		*ft = getFileType();

		return true;
	}
	
	virtual UT_Error constructMerger (IE_MailMerge ** ppie) {
		*ppie = new IE_MailMerge_Delimiter_Listener(m_delim);
		return UT_OK;
	}
	
private:
	std::string m_desc;
	std::string m_suffix;
	char m_delim;
};

/************************************************************************/
/************************************************************************/

void IE_MailMerge_RegisterXP ()
{
	IE_MailMerge::registerMerger (new IE_XMLMerge_Sniffer ());
	IE_MailMerge::registerMerger (new IE_Delimiter_Sniffer ("Comma Separated Values (*.csv)", "*.csv", ','));
	IE_MailMerge::registerMerger (new IE_Delimiter_Sniffer ("Tab Separated Values (*.tsv)", "*.tsv", '\t'));
}

void IE_MailMerge_UnRegisterXP ()
{
	IE_MailMerge::unregisterAllMergers ();
}
