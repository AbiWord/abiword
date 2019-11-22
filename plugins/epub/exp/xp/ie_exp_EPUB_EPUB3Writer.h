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
#ifndef IE_EXP_EPUB_EPUB3WRITER_H
#define	IE_EXP_EPUB_EPUB3WRITER_H

// Abiword includes
#include <ie_exp_HTML_DocumentWriter.h>
#define EPUB3_CONTENT_PROFILE "http://www.idpf.org/epub/30/profile/content/"

class IE_Exp_EPUB_EPUB3Writer : public IE_Exp_HTML_DocumentWriter {
public:
    IE_Exp_EPUB_EPUB3Writer(IE_Exp_HTML_OutputWriter* pOutputWriter);
    virtual void openAnnotation() override;
    virtual void closeAnnotation() override;

    virtual void openDocument() override;
    virtual void insertDTD() override;
    virtual void insertTOC(const gchar *title, const std::vector<UT_UTF8String> &items,
            const std::vector<UT_UTF8String> &itemUriList) override;
    virtual void insertEndnotes(const std::vector<UT_UTF8String> &endnotes) override;
    virtual void insertFootnotes(const std::vector<UT_UTF8String> &footnotes) override;
    virtual void insertAnnotations(const std::vector<UT_UTF8String> &titles,
            const std::vector<UT_UTF8String> &authors,
            const std::vector<UT_UTF8String> &annotations) override;
};

class IE_Exp_EPUB_EPUB3WriterFactory : public IE_Exp_HTML_WriterFactory
{
public:
    virtual IE_Exp_HTML_DocumentWriter *constructDocumentWriter(
        IE_Exp_HTML_OutputWriter* pOutputWriter) override;
};

#endif	/* IE_EXP_EPUB_EPUB3WRITER_H */

