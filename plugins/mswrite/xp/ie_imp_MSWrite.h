/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2000 Hubert Figuiere
 * Copyright (C) 2010-2011 Ingo Brueckl
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef IE_IMP_MSWRITE_H
#define IE_IMP_MSWRITE_H

#include <string>

#include "ie_impexp_MSWrite.h"
#include "ie_imp.h"
#include "ut_bytebuf.h"
#include "ut_mbtowc.h"
#include "ut_string_class.h"

typedef struct
{
	short ffid;             // unused
	const char *name;
	const char *codepage;
} wri_font;

class IE_Imp_MSWrite_Sniffer : public IE_ImpSniffer
{
public:
	IE_Imp_MSWrite_Sniffer();
	virtual ~IE_Imp_MSWrite_Sniffer() {}
	virtual bool getDlgLabels(const char **szDesc, const char **szSuffixList, IEFileType *ft);
	virtual const IE_SuffixConfidence *getSuffixConfidence();
	virtual const IE_MimeConfidence *getMimeConfidence() { return 0; }
	virtual UT_Confidence_t recognizeContents(const char *szBuf, UT_uint32 iNumbytes);
	virtual UT_Error constructImporter(PD_Document *pDocument, IE_Imp **ppie);

private:
	friend class IE_Imp;
};

class IE_Imp_MSWrite : public IE_Imp
{
public:
	IE_Imp_MSWrite(PD_Document *pDocument);
	~IE_Imp_MSWrite();

protected:
	virtual UT_Error _loadFile(GsfInput *input);

private:
	enum pap_t {All, Header, Footer};
	enum hdrftr_t {headerfirst, header, footerfirst, footer};

	GsfInput *mFile;
	UT_ByteBufPtr mData;      // complete data buffer as extracted out of the file
	UT_UCS4String mText;   // text buffer

	wri_struct *wri_file_header;
	wri_struct *wri_picture_header;
	wri_struct *wri_ole_header;

	UT_UCS4_mbtowc charconv;   // Windows codepage to unicode conversion

	std::string mDefaultCodepage;
	int xaLeft, xaRight;
	bool hasHeader, hasFooter, page1Header, page1Footer;
	wri_font *wri_fonts;
	int wri_fonts_count;
	unsigned int pic_nr;
	bool lf;

	UT_Error parse_file();
	bool read_ffntb();
	bool read_sep();
	bool read_pap(pap_t process);
	bool read_txt(int from, int to);
	bool read_pic(int from, int size);
	void _append_hdrftr(hdrftr_t which);

	void free_ffntb();
	const char *get_codepage(const char *facename, int *facelen) const;
	void set_codepage(const char *charset);
	void translate_char(const UT_Byte ch, UT_UCS4String &buf);
};

#endif
