/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* Abiword
 * Copyright (C) 2001 Christian Biesinger <cbiesinger@web.de>
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

#ifndef IE_IMP_STAROFFICE_H
#define IE_IMP_STAROFFICE_H

#include <stdio.h>
#include <map>
#include <string>

#include "ut_string_class.h"
#include "ut_types.h"
#include "ut_iconv.h"
#include "ie_imp.h"

#include "sdw_cryptor.h"

class PD_Document;

// The following struct definition was taken from the OpenOffice file
// sot/inc/stg.hxx line 85ff, with changes because of the different
// cross-platform toolkits. That file is available under the LGPL.
struct ClsId {
	UT_sint32 n1;
	UT_sint16 n2, n3;
	UT_uint8 n4, n5, n6, n7, n8, n9, n10, n11;
};

/*! Reads a string from the file where the first uint16 contains the length. If it
 * is zero-terminated, length must include the byte for termination.
 * str must be delete[]'d, not free'd!
 * aLength, if non-null, contains the length of the string in bytes. */
void readByteString(GsfInput* stream, char*& str, UT_uint16* aLength = NULL)
    throw(UT_Error);

/*! Reads a bytestring from a stream and converts it to UCS-4. Optionally,
 * it can also decrypt it.
 * @param stream The stream to read from
 * @param str The string where the bytestring should be stored
 * @param converter Iconv handle for charset conversion
 * @param cryptor (Optional) The cryptor used for decrypting the string */
void readByteString(GsfInput* stream, UT_UCS4Char*& str, UT_iconv_t converter, SDWCryptor* cryptor = NULL) throw(UT_Error);

class DocHdr {
	public:
		DocHdr() : sBlockName(NULL), converter(reinterpret_cast<UT_iconv_t>(-1)) {}
		~DocHdr() { if (sBlockName) free(sBlockName); if (UT_iconv_isValid(converter)) UT_iconv_close(converter); }
		/*! Reads the document header
		 * \param stream The OLE Stream to load from - should be the one
		 *               with the name "StarWriterDocument"
		 * \throw UT_Error on failure */
		void load(GsfInput* stream) throw(UT_Error);

		UT_uint8 cLen; // ???
		UT_uint16 nVersion;
		UT_uint16 nFileFlags;
		UT_sint32 nDocFlags;
		UT_uint32 nRecSzPos;
		UT_sint32 nDummy;
		UT_uint16 nDummy16; // actually 2x dummy8
		UT_uint8 cRedlineMode; // should actually be an enum, see sw/inc/redlenum.hxx#L83
		UT_uint8 nCompatVer;

		UT_uint8 cPasswd[16]; // password verification data

		UT_uint8 cSet; // the encoding to use
		UT_uint8 cGui;

		UT_uint32 nDate;
		UT_uint32 nTime;

		UT_UCS4Char* sBlockName; // name of a text module

		UT_iconv_t converter; // Iconv handle for converting from the doc charset to UCS_2_INTERNAL

		SDWCryptor* cryptor; // used for decrypting the document or null if not encrypted
};

// A text attribute record
struct TextAttr {
	TextAttr() : data(NULL), isOff(false), isPara(false) {}
	~TextAttr() { if (data) delete[] data; }
	bool startSet, endSet; // true if the start/end attribute is valid
	UT_uint16 which;
	UT_uint16 ver;
	UT_uint16 start;
	UT_uint16 end;

	UT_uint8* data; // possible additional data. NULL if no data existant.
	gsf_off_t dataLen;

	std::string attrName;
	std::string attrVal;
	bool isOff; // if true, attrVal is undefined
	bool isPara; // should be applied to paragraph, not span
};

// File Flags: (from sw/source/core/sw3io/sw3ids.hxx lines 65ff)
#define SWGF_BLOCKNAME  0x0002 // Header has textmodule
#define SWGF_HAS_PASSWD 0x0008 // Stream is password protected
#define SWGF_HAS_PGNUMS 0x0100 // Stream has pagenumbers
#define SWGF_BAD_FILE   0x8000 // There was an error writing the file

// Document Flags: (from sw/source/core/sw3io/sw3doc.cxx 733ff)
#define SWDF_BROWSEMODE1 0x1   // show document in browse mode?
#define SWDF_BROWSEMODE2 0x2   // same as above, one of them need to be set
#define SWDF_HTMLMODE 0x4      // document is in HTML Mode
#define SWDF_HEADINBROWSE 0x8  // Show headers in Browse Mode
#define SWDF_FOOTINBROWSE 0x10 // Show footers in browse mode
#define SWDF_GLOBALDOC 0x20    // Is a global document (a global document can contain chapter documents... I think)
#define SWDF_GLOBALDOCSAVELINK 0x40 // Include sections that are linked to the global document when saving
#define SWDF_LABELDOC 0x80     // is a label ("etiketten") document

// File versions (sw/source/core/sw3io/sw3ids.hxx 77ff)
#define SWG_POOLIDS 0x3        // IDs for Stringpool-Strings
#define SWG_LAYFRAMES 0x5      // Layout Frames
#define SWG_RECSIZES '%'       // Record Sizes
#define SWG_LONGIDX 0x201
#define SWG_LONGRECS 0x209     // Record-Length > 8/16MB
#define SWG_MAJORVERSION SWG_LONGIDX


// Document Sections
#define SWG_ATTRIBUTE 'A' // attribute of a textnode
#define SWG_COMMENT  'C' // comments
#define SWG_JOBSETUP 'J' // Printer Job Setup
#define SWG_CONTENTS 'N' // Textpart
#define SWG_ATTRSET  'S' // set of attributes
#define SWG_STRINGPOOL '!'
#define SWG_TEXTNODE 'T'
#define SWG_EOF      'Z'

// File format constants, from OpenOffice's tools/inc/solar.h line 471ff
#define SOFFICE_FILEFORMAT_31	3450
#define SOFFICE_FILEFORMAT_40	3580
#define SOFFICE_FILEFORMAT_50	5050
#define SOFFICE_FILEFORMAT_60	6200

// Print Job constants
#define JOBSET_FILE364_SYSTEM 0xFFFF
#define JOBSET_FILE605_SYSTEM 0xFFFE

#define IDX_NOCONV_FF 0xFFFC

// Staroffice document sniffer.
class IE_Imp_StarOffice_Sniffer : public IE_ImpSniffer
{
	public:
		IE_Imp_StarOffice_Sniffer();
		virtual ~IE_Imp_StarOffice_Sniffer() {}

		virtual const IE_SuffixConfidence * getSuffixConfidence ();
		virtual const IE_MimeConfidence * getMimeConfidence ();
		virtual UT_Confidence_t recognizeContents(GsfInput * input);
		virtual bool getDlgLabels(const char** szDesc, const char** szSuffixList, IEFileType *ft);
		virtual UT_Error constructImporter(PD_Document* pDocument, IE_Imp **ppie);
};

// Actual Importer
class IE_Imp_StarOffice : public IE_Imp
{
	public:
		IE_Imp_StarOffice(PD_Document *pDocument);
		~IE_Imp_StarOffice();

	protected:
		virtual UT_Error _loadFile(GsfInput * input);

	private:
		FILE* mFile;
		GsfInfile* mOle;
		GsfInput *mDocStream;
		DocHdr mDocHdr;

		/// Map for the SWG_STRINGPOOL
	typedef std::map<UT_uint16, std::basic_string<UT_UCS4Char> > stringpool_map;
	stringpool_map mStringPool;

		/*! Reads the record size from the stream. That is, usually
		 * three bytes starting from the current position.
		 * \param aStream the stream to read from
		 * \param aSize Reference to the size of the record
		 * \param aEOR End of Record - file position where the record is finished*/
		void readRecSize(GsfInput* stream, UT_uint32& aSize, gsf_off_t* aEOR = NULL) throw(UT_Error);
		/*! Reads a string from the file where the first sint32 contains the length. If it
		 * is zero-terminated, length must include the byte for termination. The string will
		 * be converted to the charset given in mDocHdr. If the document is encrypted, the
		 * string will be decrypted before character set conversion.
		 * \param stream The stream to read from
		 * \param str Reference to pointer to UT_UCS4Char, where the string is stored.
		 * Must be free'd. Is NULL if the function fails. */
		void readByteString(GsfInput* stream, UT_UCS4Char*& str) throw(UT_Error) {
			::readByteString(stream, str, mDocHdr.converter, mDocHdr.cryptor);
		}

		/*! Finds the version number, given a version string.
		 * \return One of SOFFICE_FILEFORMAT_31, SOFFICE_FILEFORMAT_40 and
		 *         SOFFICE_FILEFORMAT_50 */
		static UT_uint32 getVersion(const char* szVerString);
};

/* Helper functions; all throw an UT_IE_BOGUSDOCUMENT on error */
/*! Reads the header of a flag record from the stream
 * \param flags Flags (also contain the length in the 4 least significant bytes)
 * \param newPos (optional) Pointer to a variable where the position after the
 * flags record is stored. */
void readFlagRec(GsfInput* stream, UT_uint8& flags, gsf_off_t* newPos = NULL) throw(UT_Error);

/*! Reads one character from the given GsfInput.
 * \param aStream The OLE Stream
 * \param aChar Reference to the character
 * \throw UT_Error on failure */
inline void readChar(GsfInput* aStream, char& aChar) throw(UT_Error) {
	if (!gsf_input_read(aStream, 1, reinterpret_cast<guint8*>(&aChar)))
		throw UT_IE_BOGUSDOCUMENT;
}

inline void streamRead(GsfInput* aStream, UT_uint8& aDest) throw(UT_Error) {
	if (!gsf_input_read(aStream, 1, static_cast<guint8*>(&aDest)))
		throw UT_IE_BOGUSDOCUMENT;
}

inline void streamRead(GsfInput* aStream, UT_sint8& aDest) throw(UT_Error) {
	streamRead(aStream, reinterpret_cast<UT_uint8 &>(aDest));
}

inline void streamRead(GsfInput* aStream, char& aDest) throw(UT_Error) {
	streamRead(aStream, reinterpret_cast<UT_uint8 &>(aDest));
}


inline void streamRead(GsfInput* aStream, UT_uint16& aDest, bool isLittleEndian = true) throw(UT_Error) {
	guint8 buf [2];
	if (!gsf_input_read(aStream, 2, buf))
		throw UT_IE_BOGUSDOCUMENT;
	if (isLittleEndian) {
		aDest = buf [0] | (buf [1] << 8);
	}
	else {
		aDest = buf [1] | (buf [0] << 8);
	}
}

inline void streamRead(GsfInput* aStream, UT_sint16& aDest, bool isLittleEndian = true) throw(UT_Error) {
	streamRead(aStream, reinterpret_cast<UT_uint16 &>(aDest), isLittleEndian);
}

inline void streamRead(GsfInput* aStream, UT_uint32& aDest, bool isLittleEndian = true) throw(UT_Error) {
	guint8 buf [4];
	if (!gsf_input_read(aStream, 4, buf))
		throw UT_IE_BOGUSDOCUMENT;
	if (isLittleEndian) {
		aDest = buf [0] | (buf [1] << 8) | (buf [2] << 16) | (buf [3] << 24);
	}
	else {
		aDest = buf [3] | (buf [2] << 8) | (buf [1] << 16) | (buf [0] << 24);
	}
}

inline void streamRead(GsfInput* aStream, UT_sint32& aDest, bool isLittleEndian = true) throw(UT_Error) {
	streamRead(aStream, reinterpret_cast<UT_uint32 &>(aDest), isLittleEndian);
}

// reads the value as uint8
inline void streamRead(GsfInput* aStream, bool& aDest) throw(UT_Error) {
	streamRead(aStream, reinterpret_cast<UT_uint8&>(aDest));
}

// Class ID
inline void streamRead(GsfInput* aStream, ClsId& aClsId) throw(UT_Error) {
	streamRead(aStream, aClsId.n1);
	streamRead(aStream, aClsId.n2);
	streamRead(aStream, aClsId.n3);
	streamRead(aStream, aClsId.n4);
	streamRead(aStream, aClsId.n5);
	streamRead(aStream, aClsId.n6);
	streamRead(aStream, aClsId.n7);
	streamRead(aStream, aClsId.n8);
	streamRead(aStream, aClsId.n9);
	streamRead(aStream, aClsId.n10);
	streamRead(aStream, aClsId.n11);
}
#include "ut_debugmsg.h"

// for completeness...
inline void streamRead(GsfInput* aStream, char* aBuffer, UT_uint32 length) throw(UT_Error) {
	if (!gsf_input_read(aStream, length, reinterpret_cast<guint8 *>(aBuffer)))
		throw UT_IE_BOGUSDOCUMENT;
}

inline void streamRead(GsfInput* aStream, UT_uint8* aBuffer, UT_uint32 length) throw(UT_Error) {
	if (!gsf_input_read(aStream, length, static_cast<guint8*>(aBuffer)))
		throw UT_IE_BOGUSDOCUMENT;
}

// readRecSize must have been called already. readFlagRec must not.
// aEoa = position of the end of the attr.
void streamRead(GsfInput* aStream, TextAttr& aAttr, gsf_off_t aEoa) throw(UT_Error);

#endif /* IE_IMP_STAROFFICE_H */
