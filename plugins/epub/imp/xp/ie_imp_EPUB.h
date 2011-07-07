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

#ifndef IE_IMP_EPUB_H_
#define IE_IMP_EPUB_H_

#include <gsf/gsf-infile-zip.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-libxml.h>
#include <gsf/gsf-input-memory.h>
#include <ut_go_file.h>
#include <stdexcept>
#include <vector>
#include <map>

// AbiWord includes
#include <ie_imp.h>
#include <ie_imp_XHTML.h>
#include <ut_xml.h>
#include <ie_imp_PasteListener.h>

#define EPUB_MIMETYPE "application/epub+zip"

typedef std::pair<UT_UTF8String, UT_UTF8String> string_pair;
/**
 * Class used to import EPUB files
 */
class IE_Imp_EPUB: public IE_Imp
{
public:
    IE_Imp_EPUB(PD_Document * pDocument);
    virtual ~IE_Imp_EPUB();
    virtual bool pasteFromBuffer(PD_DocumentRange * pDocRange,
            const unsigned char * pData, UT_uint32 lenData,
            const char * szEncoding = 0);
protected:
    virtual UT_Error _loadFile(GsfInput * input);

private:
    GsfInfile* m_epub;
    UT_UTF8String m_rootfilePath;
    UT_UTF8String m_tmpDir;
    UT_UTF8String m_opsDir;
    std::vector<UT_UTF8String> m_spine;
    std::map<UT_UTF8String, UT_UTF8String> m_manifestItems;

    UT_Error readMetadata();
    UT_Error readPackage();
    UT_Error uncompress();
    UT_Error readStructure();
    static GsfOutput* createFileByPath(const char* path);
};

/*
 * Listener for parsing container.xml data
 */
class ContainerListener: public UT_XML::Listener
{
public:
    void startElement(const gchar * name, const gchar ** atts);
    void endElement(const gchar * name);
    void charData(const gchar * buffer, int length);

    UT_UTF8String getRootFilePath() const;

private:
    UT_UTF8String m_rootFilePath;
};

/*
 * Listener for parsing .opf
 */
class OpfListener: public UT_XML::Listener
{
public:
    void startElement(const gchar * name, const gchar ** atts);
    void endElement(const gchar * name);
    void charData(const gchar * buffer, int length);

    std::map<UT_UTF8String, UT_UTF8String> getManifestItems() const
    {
        return m_manifestItems;
    }
    std::vector<UT_UTF8String> getSpine() const
    {
        return m_spine;
    }

    OpfListener();

private:
    /* Vector with list of OPS files needed to be imported. Sorted in the linear
     * reading order
     */
    std::vector<UT_UTF8String> m_spine;
    /* Map with all files that will be used for import
     */
    std::map<UT_UTF8String, UT_UTF8String> m_manifestItems;

    bool m_inManifest;
    bool m_inSpine;
};

/*
 * Listener for parsing .ncx
 */
class NavigationListener: public UT_XML::Listener
{
public:
    void startElement(const gchar * name, const gchar ** atts);
    void endElement(const gchar * name);
    void charData(const gchar * buffer, int length);
};

#endif

