/* AbiSource
 * 
 * Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
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
#ifndef IE_EXP_EPUB_H_
#define IE_EXP_EPUB_H_

#include <string>
#include <vector>
// External includes
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-zip.h>
#include <gsf/gsf-output.h>
#include <gsf/gsf-libxml.h>
#include <gsf/gsf-infile-stdio.h>
#include <gsf/gsf-infile.h>
// Abiword includes
#include <ie_exp.h>
#include <ie_exp_HTML.h>
#include <pd_Document.h>
#include <ut_go_file.h>
#include <ut_vector.h>

#define EPUB_MIMETYPE "application/epub+zip"
#define OPF_MIMETYPE "application/oebps-package+xml"
#define OCF201_NAMESPACE "urn:oasis:names:tc:opendocument:xmlns:container"
#define OPF201_NAMESPACE "http://www.idpf.org/2007/opf"
#define DC_NAMESPACE "http://purl.org/dc/elements/1.1/"
#define NCX_NAMESPACE "http://www.daisy.org/z3986/2005/ncx/"


class IE_Exp_EPUB : public IE_Exp {

public:
	IE_Exp_EPUB(PD_Document * pDocument);
	virtual ~IE_Exp_EPUB();

protected:
	virtual UT_Error _writeDocument();

private:
	UT_UTF8String m_baseTempDir;
	UT_UTF8String m_oebpsDir;
        GsfOutfile* m_root;
        GsfOutput* m_oebps;
	void writeStructure();
        void writeNavigation();
	void writeContainer();
	void package();
        UT_Error compress();
	static std::vector<UT_UTF8String> getFileList(const UT_UTF8String &directory);
        static void closeNTags(GsfXMLOut* xml, int n);
        static UT_UTF8String escapeForId(const UT_UTF8String & src);
};



#endif /* IE_EXP_EPUB_H_ */
