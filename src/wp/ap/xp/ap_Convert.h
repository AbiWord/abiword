/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_CONVERT_H
#define AP_CONVERT_H

#include "ie_types.h"
#include "pd_Document.h"
#include "ut_string_class.h"

//////////////////////////////////////////////////////////////////

class GR_Graphics;
class PD_Document;
class ABI_EXPORT AP_Convert
{
 public:
	AP_Convert(int verbose=1);
	~AP_Convert(void);

	bool convertTo(const char * szSourceFilename,
		       IEFileType sourceFormat,
		       const char * szTargetFilename,
		       IEFileType targetFormat);

	bool convertTo(const char * szFilename,
		       const char * szSourceSuffixOrMime,
		       const char * szTargetFilename,
		       const char * szTargetSuffixOrMime);

	bool convertTo(const char * szSourceFilename,
		       const char * szSourceSuffixOrMime,
		       const char * szTargetSuffixOrMime);

	void setVerbose(int level);
	void setMergeSource (const char * source);

	bool print(const char * file, GR_Graphics * pGraphics, const char * szFileExtension = NULL);
	bool printFirstPage(GR_Graphics * pGraphics, PD_Document * pDoc);

	void setImpProps (const char * props) {
		m_impProps = props;
	}

	void setExpProps (const char * props) {
		m_expProps = props;
	}

 private:
	int m_iVerbose;
	UT_UTF8String m_mergeSource;

	UT_UTF8String m_impProps;
	UT_UTF8String m_expProps;
};

#endif /* AP_CONVERT_H */
