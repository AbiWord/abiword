/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* Abiword
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

#ifndef IE_MAILMERGE_H
#define IE_MAILMERGE_H

#include <map>
#include <string>
#include <vector>

#include "ut_types.h"

typedef UT_sint32 IEMergeType;
#define IEMT_Unknown ((IEMergeType)-1)

class IE_MailMerge;
class PD_Document;

class ABI_EXPORT IE_MergeSniffer
{
  friend class IE_MailMerge;

public:

	virtual ~IE_MergeSniffer();

	// these you get for free
	inline bool supportsFileType (IEMergeType type) {return m_type == type;}
	inline IEMergeType getFileType() const {return m_type;}

	// these you must override these next methods!!!

	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you recognize the contents. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
											   UT_uint32 iNumbytes) = 0;
	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you recognize the suffix. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t recognizeSuffix (const char * szSuffix) = 0;

	virtual bool getDlgLabels (const char ** szDesc,
	                           const char ** szSuffixList,
	                           IEMergeType * ft) = 0;

	virtual UT_Error constructMerger (IE_MailMerge ** ppie) = 0;

	// test helper.
	friend class IE_MergeSniffer_TH;
protected:

	IE_MergeSniffer() {}

private:

	// only IE_MailMerge ever calls this
	inline void setFileType (IEMergeType type) {m_type = type;}
	IEMergeType m_type;
};

class ABI_EXPORT IE_MailMerge
{

public:
	// Test helper
	friend class IE_MailMerge_TH;

	virtual ~IE_MailMerge ();

	// constructs an importer of the right type based upon
	// either the filename or sniffing the file.  caller is
	// responsible for destroying the importer when finished
	// with it.

	virtual UT_Error	mergeFile(const char * szFilename) = 0;
	virtual UT_Error getHeaders (const char * szFilename, std::vector<std::string> & out_vec) = 0;

	static IEMergeType	fileTypeForContents(const char * szBuf,
	                                            UT_uint32 iNumbytes);

	static IEMergeType	fileTypeForSuffix(const char * szSuffix);
	static IEMergeType	fileTypeForDescription(const char * szSuffix);

	static IEMergeType fileTypeForSuffixes(const char * suffixList);

	static IE_MergeSniffer * snifferForFileType(IEMergeType ieft);
	static const char * suffixesForFileType(IEMergeType ieft);
	static const char * descriptionForFileType(IEMergeType ieft);

	static UT_Error	constructMerger(const char * szFilename,
	                                IEMergeType ieft,
	                                IE_MailMerge ** ppie,
	                                IEMergeType * pieft = NULL);

	static bool	    enumerateDlgLabels(UT_uint32 ndx,
	                                       const char ** pszDesc,
	                                       const char ** pszSuffixList,
	                                       IEMergeType * ft);

	static UT_uint32	getMergerCount(void);
	static void registerMerger (IE_MergeSniffer * sniffer);
	static void unregisterAllMergers ();

	class ABI_EXPORT IE_MailMerge_Listener
	{
	public:
		virtual ~IE_MailMerge_Listener ()
			{
			}

		virtual PD_Document* getMergeDocument () const = 0;
		virtual bool fireUpdate () = 0;

	protected:

		IE_MailMerge_Listener ()
			{
			}
	};

	void setListener (IE_MailMerge_Listener * listener);

	const std::map<std::string, std::string> & getCurrentMapping() const
	{ return m_map; }

protected:

	IE_MailMerge ();

	bool fireMergeSet ();
	void addMergePair (const std::string & key,
			   const std::string & value);

private:

	static std::vector<IE_MergeSniffer *> & getSniffers();

	IE_MailMerge_Listener * m_pListener;
	std::map<std::string, std::string> m_map;
};

void IE_MailMerge_RegisterXP ();
void IE_MailMerge_UnRegisterXP ();

#endif
