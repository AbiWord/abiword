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

#include <gsf/gsf.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-infile.h>

#include <time.h> // for struct tm and asctime

#include "docinfo.h"
#include "pd_Document.h"
#include "ie_imp_StarOffice.h"
#include "ie_imp_StarOffice_encodings.h"
#include "ut_debugmsg.h"
#include "ut_iconv.h"
#include "ut_string_class.h"
#include "ut_assert.h"

SDWDocInfo::SDWDocInfo() {}
SDWDocInfo::~SDWDocInfo() {}

/** Reads a bytestring, followed by a padding. aMaxlen is the max. number of bytes to read. */
static void readPaddedByteString(GsfInput* aStream, UT_UCS4String& aString,
                                 UT_iconv_t aConverter, UT_uint32 aMaxlen)
    throw(UT_Error) 
{
	UT_UCS4Char* str;
	readByteString(aStream, str, aConverter);
	aString = str;
	free(str);

	// XXX need original length, not ucs4 length
	UT_uint32 len = aString.size();
	if (len > aMaxlen)
		throw UT_IE_BOGUSDOCUMENT;
	if (gsf_input_seek(aStream, aMaxlen-len, G_SEEK_CUR))
		throw UT_IE_BOGUSDOCUMENT;
}

class AutoGsfInput {
	public:
		AutoGsfInput(GsfInput* aStream = NULL) : mStream(aStream) {}
		~AutoGsfInput() { close(); }
		void close() { if (mStream) g_object_unref(G_OBJECT(mStream)); }

		GsfInput*& operator=(GsfInput* aStream) { close(); mStream = aStream; return mStream; }

		operator GsfInput*() { return mStream; }
		operator GsfInput**() { close(); return &mStream; }
	private:
		GsfInput* mStream;
};

/** StarOffice's Timestamp - first, a bytestring, then date, then time */
class TimeStamp {
	public:
		TimeStamp(UT_iconv_t aConverter) : mDate(0), mTime(0), mConverter(aConverter) {}
		void load(GsfInput* aStream);

		UT_uint32 mDate;
		UT_uint32 mTime;
		UT_UCS4String mString;

		/** Converts this timestamp to a human-readable string */
		std::string ToString() const;
	private:
		UT_iconv_t mConverter;
		enum {
			TimeStampLen = 31
		};
};

void TimeStamp::load(GsfInput* aStream) {
	readPaddedByteString(aStream, mString, mConverter, TimeStampLen);
	streamRead(aStream, mDate);
	streamRead(aStream, mTime);
}

std::string TimeStamp::ToString() const {
	struct tm theDate;
	theDate.tm_sec = (mTime / 100) % 100;
	theDate.tm_min = (mTime / 10000) % 100;
	theDate.tm_hour = (mTime / 1000000) % 100;
	theDate.tm_mday = mDate % 100;
	theDate.tm_mon = ((mDate / 100) % 100) - 1;
	theDate.tm_year = (mDate / 10000) - 1900;
	theDate.tm_isdst = -1;

	mktime(&theDate);
	char buf[64];
	strftime(buf, sizeof(buf), "%x %X", &theDate);
	UT_DEBUGMSG(("SDW: (TimeStamp is %s)\n", buf));
	return buf;
}

// set an UCS-4 Metadata by converting it to UTF-8 and treating it as ASCII
static inline void do_SetMetadata(PD_Document* aDoc, const std::string & aKey, UT_UCS4String aData) {
	std::string str(aData.utf8_str());
	aDoc->setMetaDataProp(aKey, str);
}

void SDWDocInfo::load(GsfInfile* aDoc, PD_Document* aPDDoc) 
	throw(UT_Error)
{
	char* headStr = NULL;

	try {
		UT_DEBUGMSG(("SDW: Loading Docinfo...\n"));
		// firstly, set StarOffice as generator
		aPDDoc->setMetaDataProp(PD_META_KEY_GENERATOR, "StarOffice");
		AutoGsfInput docInfo =  gsf_infile_child_by_name(aDoc, "SfxDocumentInfo");
		if (!(GsfInput*)docInfo)
			throw UT_IE_BOGUSDOCUMENT;

		readByteString(docInfo, headStr);
		if (strcmp(headStr, "SfxDocumentInfo") != 0)
			throw UT_IE_BOGUSDOCUMENT;

		UT_uint16 version;
		streamRead(docInfo, version);
		bool pwProtect;
		streamRead(docInfo, pwProtect);
		UT_uint16 charset;
		streamRead(docInfo, charset);
		auto_iconv converter(findConverter((UT_uint8)charset));
		if (!UT_iconv_isValid(converter))
			throw UT_IE_BOGUSDOCUMENT;

		bool graphPortable, queryTemplateReload;
		streamRead(docInfo, graphPortable);
		streamRead(docInfo, queryTemplateReload);

		TimeStamp stamp(converter);
		stamp.load(docInfo); // creator
		do_SetMetadata(aPDDoc, PD_META_KEY_CREATOR, stamp.mString);
		aPDDoc->setMetaDataProp(PD_META_KEY_DATE, stamp.ToString());
		stamp.load(docInfo); // modifier
		// (ab-)using Contributor as the person who last modified the doc...
		do_SetMetadata(aPDDoc, PD_META_KEY_CONTRIBUTOR, stamp.mString);
		aPDDoc->setMetaDataProp(PD_META_KEY_DATE_LAST_CHANGED, stamp.ToString());
		stamp.load(docInfo); // printer (person, not device)

		UT_UCS4String data;
		readPaddedByteString(docInfo, data, converter, 63);
		do_SetMetadata(aPDDoc, PD_META_KEY_TITLE, data);
		readPaddedByteString(docInfo, data, converter, 63);
		do_SetMetadata(aPDDoc, PD_META_KEY_SUBJECT, data);
		readPaddedByteString(docInfo, data, converter, 255);
		do_SetMetadata(aPDDoc, PD_META_KEY_DESCRIPTION, data);
		readPaddedByteString(docInfo, data, converter, 127);
		do_SetMetadata(aPDDoc, PD_META_KEY_KEYWORDS, data);

		// Read user-defined data
		for (int i = 0; i < 4; i++) {
			UT_UCS4String key, value;
			readPaddedByteString(docInfo, key, converter, 19);
			readPaddedByteString(docInfo, value, converter, 19);
			std::string prefixedKey = std::string(CUSTOM_META_PREFIX) + key.utf8_str();
			do_SetMetadata(aPDDoc, prefixedKey, value);
		}

		// rest of the stream contains no useful information

		delete [] headStr;
		UT_DEBUGMSG(("SDW: Docinfo done loading\n"));
	}
	catch(UT_Error & e) {
		if(headStr) {
			delete [] headStr;
		}
		throw e;
	}
}

