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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
#ifndef IE_EXP_EPUB_H_
#define IE_EXP_EPUB_H_

#include "ie_exp_EPUB_EPUB3Writer.h"
#include "ap_Dialog_EpubExportOptions.h"

#include <string>
#include <vector>
#include <algorithm>
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
#include <ut_path.h>
#include <ie_TOC.h>

#define EPUB_MIMETYPE "application/epub+zip"
#define OPF_MIMETYPE "application/oebps-package+xml"
#define OCF201_NAMESPACE "urn:oasis:names:tc:opendocument:xmlns:container"
#define OPF201_NAMESPACE "http://www.idpf.org/2007/opf"
#define OPS201_NAMESPACE "http://www.idpf.org/2007/ops"
#define EPUB3_PACKAGE_PROFILE "http://www.idpf.org/epub/30/profile/package/"
#define DC_NAMESPACE "http://purl.org/dc/elements/1.1/"
#define NCX_NAMESPACE "http://www.daisy.org/z3986/2005/ncx/"
class IE_Exp_EPUB: public IE_Exp
{

public:
    IE_Exp_EPUB(PD_Document * pDocument);
    virtual ~IE_Exp_EPUB();

protected:
    virtual UT_Error _writeDocument();

private:
    UT_Error writeStructure();
    UT_Error writeNavigation();
    UT_Error writeContainer();
    UT_Error package();
    UT_Error compress();

    // Methods for EPUB 2.0.1 document generation
    UT_Error EPUB2_writeStructure();
    UT_Error EPUB2_writeNavigation();

    // Methods for EPUB 3 document generation
    UT_Error EPUB3_writeStructure();
    UT_Error EPUB3_writeNavigation();


    std::string getAuthor() const;
    std::string getTitle() const;
    std::string getLanguage() const;

    UT_Error doOptions();
    void registerDialogs();

    static std::vector<std::string> getFileList(const std::string &directory);
    static void closeNTags(GsfXMLOut* xml, int n);
    static std::string escapeForId(const std::string & src);
    static std::string getMimeType(const std::string &uri);

    std::string m_baseTempDir;
    std::string m_oebpsDir;
    GsfOutfile* m_root;
    GsfOutput* m_oebps;
    IE_Exp_HTML *m_pHmtlExporter;
    // Array with file id`s in linear reading order
    std::vector<std::string> m_opsId;

    XAP_Dialog_Id m_iDialogExport;
    XAP_Exp_EpubExportOptions m_exp_opt;
};

#endif /* IE_EXP_EPUB_H_ */
