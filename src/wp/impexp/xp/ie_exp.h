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

#ifndef IE_EXP_H
#define IE_EXP_H

#include <stdio.h>
#include <map>
#include <string>

#include "ie_types.h"

#include "ut_go_file.h"
#include <gsf/gsf-output.h>

#include "ut_string_class.h"

class PD_Document;
class PD_DocumentRange;
class UT_ByteBuf;

//////////////////////////////////////////////////////////////////
// IE_Exp defines the base class for file exporters.
//////////////////////////////////////////////////////////////////

class IE_Exp;
class IE_FieldUpdater;

class ABI_EXPORT IE_ExpSniffer
{
	friend class IE_Exp;

public:
	virtual ~IE_ExpSniffer();

	// these you get for free
	inline bool supportsFileType (IEFileType type) {return m_type == type;}
	inline IEFileType getFileType() const {return m_type;}

	// these you must override these
	virtual bool recognizeSuffix (const char * szSuffix) = 0;
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft) = 0;

	virtual UT_UTF8String getPreferredSuffix();

	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie) = 0;

	/*!
	 * Return a number in the range [0,255] as to your confidence
	 * that you can export this MIME type. 0 being the least, 127 being
	 * so-so, 255 being absolutely sure
	 */
	virtual UT_Confidence_t supportsMIME (const char * /*szMIME*/) { return UT_CONFIDENCE_ZILCH; }

	const UT_UTF8String & name () const { return m_name; }

	bool getCanCopy () const {
		return m_bCanCopy;
	}

protected:
	IE_ExpSniffer(const char * name, bool canCopy = false);

private:
	const UT_UTF8String	m_name;

	// only IE_Exp ever calls this
	inline void setFileType (IEFileType type) {m_type = type;}
	IEFileType m_type;

	bool m_bCanCopy;
};

class ABI_EXPORT IE_Exp
{
public:
	// constructs an exporter of the right type based upon
	// either the filename or the key given.  caller is
	// responsible for destroying the exporter when finished
	// with it.

	static IEFileType	fileTypeForSuffix(const char * szSuffix);
	static IEFileType	fileTypeForMimetype(const char * szMimetype);
	static IEFileType	fileTypeForDescription(const char * szSuffix);
	static IEFileType fileTypeForSuffixes(const char * suffixList);
	static IE_ExpSniffer * snifferForFileType(IEFileType ieft);
	static const char * suffixesForFileType(IEFileType ieft);
	static const char * descriptionForFileType(IEFileType ieft);
	static UT_UTF8String preferredSuffixForFileType(IEFileType ieft);

	static UT_Error		constructExporter(PD_Document * pDocument,
										  const char * szFilename,
										  IEFileType ieft,
										  IE_Exp ** ppie,
										  IEFileType * pieft = NULL);
	static UT_Error		constructExporter(PD_Document * pDocument,
										  GsfOutput * output,
										  IEFileType ieft,
										  IE_Exp ** ppie,
										  IEFileType * pieft = NULL);
	static bool		    enumerateDlgLabels(UT_uint32 ndx,
										   const char ** pszDesc,
										   const char ** pszSuffixList,
										   IEFileType * ft);
	static UT_uint32	getExporterCount(void);

	static void registerExporter (IE_ExpSniffer *);
	static void unregisterExporter (IE_ExpSniffer *);
	static void unregisterAllExporters ();

	virtual ~IE_Exp();

	UT_Error		writeFile(const char * szFilename);
	UT_Error        writeFile(GsfOutput * fp);
	virtual UT_Error	copyToBuffer(PD_DocumentRange * pDocRange, UT_ByteBuf * pBuf);
	virtual void		write(const char * sz);
	virtual void		write(const char * sz, UT_uint32 length);
	virtual char        rewindChar(void);
	void write (const UT_String & inStr)
	  {
	    write ( inStr.c_str(), inStr.size() ) ;
	  }

	virtual const char *          getFileName(void) const
		{ return m_szFileName.c_str(); }

	UT_Confidence_t getFidelity () const {
		return m_fidelity;
	}

	void setProps (const char * props);

	bool isCopying () const {
		return getDocRange () != NULL;
	}

	void populateFields();


    // UTILS
    static UT_Error writeBufferToFile(const UT_ByteBuf * pByteBuf,
                             const std::string & imagedir,
                             const std::string & filename);

protected:
	// todo: remove the = 0 for 2.2
	IE_Exp(PD_Document * pDocument, UT_Confidence_t fidelity = 0);
	virtual UT_Error	_writeDocument(void) = 0;

	GsfOutput*	openFile(const char * szFilename);

	virtual GsfOutput* _openFile(const char *szFilename);

	// derived classes should use these to open/close
	// and write data to the actual file.  this will
	// let us handle file backups, etc.
	virtual UT_uint32	_writeBytes(const UT_Byte * pBytes, UT_uint32 length);
	virtual bool		_writeBytes(const UT_Byte * sz);
	virtual void		_abortFile(void);

	void				_cancelExport () { m_bCancelled = true; }

	bool		_closeFile(void);

	PD_Document * getDoc() const;
	PD_DocumentRange * getDocRange() const;
	/** Return the file pointer, for convenience */
	GsfOutput*          getFp() const
		{
			return m_fp;
		}

	bool				m_error;
	void               _setByteBuf(UT_ByteBuf * pBuf)
	{ m_pByteBuf = pBuf;}
	UT_ByteBuf *       _getByteBuf(void) const
	{ return m_pByteBuf;}

public:
	std::string getProperty (const std::string & key) const;

private:
	PD_Document *		m_pDocument;
	PD_DocumentRange *	m_pDocRange;
	UT_ByteBuf *		m_pByteBuf;
	std::string         m_szFileName;
	GsfOutput *				m_fp;
	bool                m_bOwnsFp;

	bool				m_bCancelled;

	std::map<std::string, std::string>	m_props_map;

	UT_Confidence_t		m_fidelity;
	IE_FieldUpdater  *  m_fieldUpdater;
};


#endif /* IE_EXP_H */
