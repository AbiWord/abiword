/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef IE_IMP_H
#define IE_IMP_H

#include "ut_types.h"
#include "ie_types.h"

#include <map>

#include "ut_vector.h"
#include "ut_string_class.h"
#include "pd_Document.h"

#include "ut_go_file.h"
#include <gsf/gsf-input.h>

#if defined(__MINGW32__)
#undef snprintf
#define _GLIBCXX_USE_C99_DYNAMIC 1
#endif

#include <string>
#include <vector>

class PD_Document;
class IE_Imp;

/*!
 * IE_Imp defines the abstract base class for file importers.
 */
class ABI_EXPORT IE_ImpSniffer
{
	friend class IE_Imp;

public:

	virtual ~IE_ImpSniffer();

	// these you get for free
	inline bool supportsFileType (IEFileType type) {return m_type == type;}
	inline IEFileType getFileType() const {return m_type;}

	// these you must override these next 4 methods!!!

	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you recognize the contents. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
											   UT_uint32 iNumbytes);

	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you recognize the contents. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t recognizeContents (GsfInput * input);

	/*!
	 * Return a zero terminated array of IE_SuffixConfidence.
	 * This deprecates recognizeSuffix().
	 */
	virtual const IE_SuffixConfidence * getSuffixConfidence () = 0;

	/*!
	 * Return a zero terminated array of IE_MimeConfidence.
	 * This deprecates supportsMIME().
	 */
	virtual const IE_MimeConfidence * getMimeConfidence () = 0;

	virtual bool getDlgLabels (const char ** szDesc,
				   const char ** szSuffixList,
				   IEFileType * ft) = 0;
	virtual UT_Error constructImporter (PD_Document * pDocument,
					    IE_Imp ** ppie) = 0;

	const UT_UTF8String & name () const { return m_name; }

	bool getCanPaste () const {
		return m_bCanPaste;
	}
protected:
	IE_ImpSniffer(const char * name, bool canPaste = false);

private:
	const UT_UTF8String	m_name;

	// only IE_Imp ever calls this
	inline void setFileType (IEFileType type) {m_type = type;}
	IEFileType m_type;

	bool m_bCanPaste;
};

class ABI_EXPORT IE_Imp
{
public:

	// constructs an importer of the right type based upon
	// either the filename or sniffing the file.  caller is
	// responsible for destroying the importer when finished
	// with it.

	static IEFileType	fileTypeForContents(const char * szBuf,
						    UT_uint32 iNumbytes);

	static IEFileType	fileTypeForSuffix(const char * szSuffix);
	static IEFileType	fileTypeForDescription(const char * szSuffix);
	static IEFileType	fileTypeForMimetype(const char * szMimetype);

	static IEFileType fileTypeForSuffixes(const char * suffixList);

	static IE_ImpSniffer * snifferForFileType(IEFileType ieft);
	static const char * suffixesForFileType(IEFileType ieft);
	static const char * descriptionForFileType(IEFileType ieft);

	static UT_Error	constructImporter(PD_Document * pDocument,
									  const char * szFilename,
									  IEFileType ieft,
									  IE_Imp ** ppie,
									  IEFileType * pieft = NULL);

	static UT_Error	constructImporter(PD_Document * pDocument,
									  GsfInput * input,
									  IEFileType ieft,
									  IE_Imp ** ppie,
									  IEFileType * pieft = NULL);

	static UT_Error	constructImporter(PD_Document * pDocument,
									  IEFileType ieft,
									  IE_Imp ** ppie,
									  IEFileType * pieft = NULL);

	static bool	    enumerateDlgLabels(UT_uint32 ndx,
					       const char ** pszDesc,
					       const char ** pszSuffixList,
					       IEFileType * ft);

	static UT_uint32	getImporterCount(void);
	static void registerImporter (IE_ImpSniffer * sniffer);
	static void unregisterImporter (IE_ImpSniffer * sniffer);
	static void unregisterAllImporters ();
	static std::vector<std::string> & getSupportedMimeTypes ();
	static std::vector<std::string> & getSupportedMimeClasses ();
	static std::vector<std::string> & getSupportedSuffixes ();
	static const char * getMimeTypeForSuffix (const char * suffix);

	virtual ~IE_Imp();

	// kept around for backwards compat
	UT_Error importFile(const char * szFilename);
	UT_Error importFile(GsfInput * input);

	static UT_Error loadFile(PD_Document * doc, const char * szFilename, IEFileType ieft = IEFT_Unknown, const char * props = NULL, IEFileType * savedAsType = NULL);
	static UT_Error loadFile(PD_Document * doc, GsfInput * input, IEFileType ieft = IEFT_Unknown, const char * props = NULL, IEFileType * savedAsType = NULL);

	// default impl
	virtual  bool		pasteFromBuffer(PD_DocumentRange * pDocRange,
						const unsigned char * pData,
						UT_uint32 lenData,
						const char * szEncoding = 0);

	PD_Document *           getDoc() const;

	void setLoadStylesOnly(bool b) {m_bStylesOnly = b;}
	bool getLoadStylesOnly() const {return m_bStylesOnly;}
	virtual bool supportsLoadStylesOnly() {return false;}

	void setLoadDocProps(bool b) {m_bDocProps = b;}
	bool getLoadDocProps() const {return m_bDocProps;}

	UT_Confidence_t getFidelity () const {
		return m_fidelity;
	}

	void setProps (const char * props);
	bool isPasting(void) const { return m_isPaste;}

 protected:
	IE_Imp(PD_Document * pDocument, UT_Confidence_t fidelity = 0);

	PT_DocPosition getDocPos() const;
	void setClipboard (PT_DocPosition dpos);
	bool isClipboard () const;

	virtual bool appendStrux (PTStruxType pts, const PP_PropertyVector & attributes);
	virtual bool appendStruxFmt(pf_Frag_Strux * pfs, const PP_PropertyVector & attributes);
	virtual bool appendSpan (const UT_UCSChar * p, UT_uint32 length);
	virtual bool appendSpan ( const std::string& s );
	virtual bool appendObject (PTObjectType pto,
                               const PP_PropertyVector & attribs,
                               const PP_PropertyVector & props = PP_NOPROPS);
    // XXX remove
	bool appendFmt(const gchar ** attributes)
		{ return appendFmt(PP_std_copyProps(attributes)); }
	virtual bool appendFmt(const PP_PropertyVector & pVecAttributes);

	virtual UT_Error _loadFile (GsfInput * input) = 0;

public:
	const std::string & getProperty (const char * key) {
		return m_props_map[key];
	}

 private:
	PD_Document * m_pDocument;
	bool m_isPaste;
	PT_DocPosition m_dpos;
	bool m_bStylesOnly;
	bool m_bDocProps;

	typedef std::map<std::string, std::string> map_type;
	map_type m_props_map;

	UT_Confidence_t m_fidelity;
};

class ABI_EXPORT GsfInputMarker
{
	GsfInput *m_input;
	gsf_off_t m_position;
	bool m_reset;

	GsfInputMarker();
	GsfInputMarker(const GsfInputMarker & rhs);
	GsfInputMarker& operator=(const GsfInputMarker &rhs);

public:
	GsfInputMarker(GsfInput * input)
		: m_input(input), m_position(gsf_input_tell(input)), m_reset(false)
	{
		g_object_ref(G_OBJECT(m_input));
	}

	~GsfInputMarker()
	{
		reset();
		g_object_unref(G_OBJECT(m_input));
	}

	void reset()
	{
		if(!m_reset)
			{
				gsf_input_seek(m_input, m_position, G_SEEK_SET);
				m_reset = true;
			}
	}
};


#endif /* IE_IMP_H */
