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

#include "ie_exp_EPUB.h"

/*****************************************************************************/
/*****************************************************************************/
IE_Exp_EPUB::IE_Exp_EPUB(PD_Document * pDocument)
		: IE_Exp(pDocument)
{


}
IE_Exp_EPUB::~IE_Exp_EPUB()
{
	
}

UT_Error IE_Exp_EPUB::_writeDocument()
{
    
    m_root = gsf_outfile_zip_new(getFp(), NULL);

    if (m_root == NULL) 
    {
        UT_DEBUGMSG(("ZIP output is null\n"));
        return UT_ERROR;
    }
    
    m_oebps = gsf_outfile_new_child(m_root, "OEBPS", TRUE);
    if (m_oebps == NULL)
    {
        UT_DEBUGMSG(("Can`t create oebps output object\n"));
        return UT_ERROR;
    }
    
    // mimetype must a first file in archive
    GsfOutput *mimetype = gsf_outfile_new_child_full(m_root, "mimetype", FALSE,
        "compression-level", 0, NULL);
    gsf_output_write(mimetype, strlen(EPUB_MIMETYPE), (const guint8*) EPUB_MIMETYPE);
    gsf_output_close(mimetype);

    // We need to create temporary directory to which
    // HTML plugin will export our document
    m_baseTempDir = UT_go_filename_to_uri(g_get_tmp_dir());
    m_baseTempDir += G_DIR_SEPARATOR_S;

    // To generate unique directory name we`ll use document UUID
    m_baseTempDir += getDoc()->getDocUUIDString();
    UT_go_directory_create(m_baseTempDir.utf8_str(), 0644, NULL);

    if (writeContainer() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to write container\n"));
        return UT_ERROR;
    }
    if (writeStructure() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to write document structure\n"));
        return UT_ERROR;
    }
    if (writeNavigation() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to write navigation\n"));
        return UT_ERROR;
    }
    if (package() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to package document\n"));
        return UT_ERROR;
    }
    
    gsf_output_close(m_oebps);
    gsf_output_close(GSF_OUTPUT(m_root));
    return UT_OK;
}

 UT_Error IE_Exp_EPUB::writeContainer()
{
    GsfOutput* metaInf = gsf_outfile_new_child(m_root, "META-INF", TRUE);
    
    if (metaInf == NULL)
    {
        UT_DEBUGMSG(("Can`t create META-INF dir\n"));
        return UT_ERROR;
    }
    GsfOutput* container = gsf_outfile_new_child(GSF_OUTFILE(metaInf), "container.xml", FALSE);
    if (container == NULL)
    {
        UT_DEBUGMSG(("Can`t create container.xml\n"));
        gsf_output_close(metaInf);
        return UT_ERROR;
    }
    GsfXMLOut * containerXml = gsf_xml_out_new(container);
   
    // <container>
    gsf_xml_out_start_element(containerXml, "container");
    gsf_xml_out_add_cstr(containerXml, "version", "1.0");
    gsf_xml_out_add_cstr(containerXml, "xmlns", OCF201_NAMESPACE);
    // <rootfiles>
    gsf_xml_out_start_element(containerXml, "rootfiles");
    // <rootfile>
    gsf_xml_out_start_element(containerXml, "rootfile");
    gsf_xml_out_add_cstr(containerXml, "full-path", "OEBPS/book.opf");
    gsf_xml_out_add_cstr(containerXml, "media-type", OPF_MIMETYPE);
    // </rootfile>
    gsf_xml_out_end_element(containerXml);
    // </rootfiles>
    gsf_xml_out_end_element(containerXml);
    // </container>
    gsf_xml_out_end_element(containerXml);
    
    gsf_output_close(container);
    gsf_output_close(metaInf);
    return UT_OK;
}

UT_Error IE_Exp_EPUB::writeStructure()
{
    m_oebpsDir = m_baseTempDir + G_DIR_SEPARATOR_S;
    m_oebpsDir += "OEBPS";
    
    UT_go_directory_create(m_oebpsDir.utf8_str(), 0644, NULL);

    UT_UTF8String indexPath = m_oebpsDir + G_DIR_SEPARATOR_S;
    indexPath += "index.xhtml";

    // Exporting document to XHTML using HTML export plugin 
    char *szIndexPath = (char*) g_malloc(strlen(indexPath.utf8_str()) + 1);
    strcpy(szIndexPath, indexPath.utf8_str());
    IE_Exp_HTML *pExpHtml = new IE_Exp_HTML(getDoc());
    pExpHtml->suppressDialog(true);
    pExpHtml->setProps("embed-css:no;html4:no;use-awml:no;declare-xml:yes;mathml-render-png:yes;");
    // Though there is no table of contents at begining of the document, 
    // we still need to generate id`s for headings because of NCX
    pExpHtml->set_AddIdentifiers(true);
    pExpHtml->writeFile(szIndexPath);
    g_free(szIndexPath);
    delete pExpHtml;
    
    return UT_OK;
}

UT_Error IE_Exp_EPUB::writeNavigation()
{
    GsfOutput* ncx = gsf_outfile_new_child(GSF_OUTFILE(m_oebps), "toc.ncx", FALSE);
    if (ncx == NULL)
    {
        UT_DEBUGMSG(("Can`t create toc.ncx file\n"));
        return UT_ERROR;
    }
    GsfXMLOut* ncxXml = gsf_xml_out_new(ncx);
   
    // <ncx>
    gsf_xml_out_start_element(ncxXml, "ncx");
    gsf_xml_out_add_cstr(ncxXml, "xmlns", NCX_NAMESPACE);
    gsf_xml_out_add_cstr(ncxXml, "version", "2005-1");
    gsf_xml_out_add_cstr(ncxXml, "xml:lang", NULL); // TODO: add language
    // <head>
    gsf_xml_out_start_element(ncxXml, "head");
    // <meta name="dtb:uid" content=... >
    gsf_xml_out_start_element(ncxXml, "meta");
    gsf_xml_out_add_cstr(ncxXml, "name", "dtb:uid");
    gsf_xml_out_add_cstr(ncxXml, "content", getDoc()->getDocUUIDString());
    // </meta>
    gsf_xml_out_end_element(ncxXml);
    // <meta name="epub-creator" content=... >
    gsf_xml_out_start_element(ncxXml, "meta");
    gsf_xml_out_add_cstr(ncxXml, "name", "epub-creator");
    gsf_xml_out_add_cstr(ncxXml, "content", "AbiWord (http://www.abisource.com/)");
    // </meta>
    gsf_xml_out_end_element(ncxXml);
    // <meta name="dtb:depth" content=... >
    gsf_xml_out_start_element(ncxXml, "meta");
    gsf_xml_out_add_cstr(ncxXml, "name", "dtb:depth");
    gsf_xml_out_add_cstr(ncxXml, "content", "1");
    // </meta>
    gsf_xml_out_end_element(ncxXml);
    // <meta name="dtb:totalPageCount" content=... >
    gsf_xml_out_start_element(ncxXml, "meta");
    gsf_xml_out_add_cstr(ncxXml, "name", "dtb:totalPageCount");
    gsf_xml_out_add_cstr(ncxXml, "content", "0");
    // </meta>
    gsf_xml_out_end_element(ncxXml);
    // <meta name="dtb:totalPageCount" content=... >
    gsf_xml_out_start_element(ncxXml, "meta");
    gsf_xml_out_add_cstr(ncxXml, "name", "dtb:maxPageCount");
    gsf_xml_out_add_cstr(ncxXml, "content", "0");
    // </meta>
    gsf_xml_out_end_element(ncxXml);
    // </head>
    gsf_xml_out_end_element(ncxXml);
    
    // <docTitle>
    gsf_xml_out_start_element(ncxXml, "docTitle");
    gsf_xml_out_start_element(ncxXml, "text");
    gsf_xml_out_add_cstr(ncxXml, NULL, getTitle().utf8_str());
    gsf_xml_out_end_element(ncxXml);
    // </docTitle>
    gsf_xml_out_end_element(ncxXml);
    
    
    // <docAuthor>
    gsf_xml_out_start_element(ncxXml, "docAuthor");
    gsf_xml_out_start_element(ncxXml, "text");
    gsf_xml_out_add_cstr(ncxXml, NULL, getAuthor().utf8_str()); // TODO: Add title here
    gsf_xml_out_end_element(ncxXml);
    // </docAuthor>
    gsf_xml_out_end_element(ncxXml);
    
    IE_TOCHelper* toc = new IE_TOCHelper(getDoc());
    // <navMap>
    gsf_xml_out_start_element(ncxXml, "navMap");
    if (toc->hasTOC()){
        int lastLevel = 0;
        int curLevel = 0;
        for (int currentItem = 0; currentItem < toc->getNumTOCEntries(); currentItem++)
        {
            lastLevel = curLevel;
            UT_UTF8String itemStr = toc->getNthTOCEntry(currentItem, &curLevel);

            
            if ((lastLevel >= curLevel) && (currentItem != 0))
            {
                closeNTags(ncxXml, lastLevel - curLevel + 1);
                UT_DEBUGMSG(("RUDYJ: Closing  %d tags\n", lastLevel - curLevel + 1));
            }
            UT_DEBUGMSG(("RUDYJ: Current level is %d\n", curLevel));

            UT_UTF8String navClass = UT_UTF8String_sprintf("h%d", curLevel);
            UT_UTF8String navId = UT_UTF8String_sprintf("AbiTOC%d__", currentItem);
            UT_UTF8String navSrc = "index.xhtml#" + navId;
            gsf_xml_out_start_element(ncxXml, "navPoint");
            gsf_xml_out_add_cstr(ncxXml, "playOrder", UT_UTF8String_sprintf ("%d", currentItem + 1).utf8_str ());
            gsf_xml_out_add_cstr(ncxXml, "class", navClass.utf8_str());
            gsf_xml_out_add_cstr(ncxXml, "id", navId.utf8_str());
            
            gsf_xml_out_start_element(ncxXml, "navLabel");
            gsf_xml_out_start_element(ncxXml, "text");
            gsf_xml_out_add_cstr(ncxXml, NULL, itemStr.utf8_str());
            gsf_xml_out_end_element(ncxXml);
            gsf_xml_out_end_element(ncxXml);
            
            gsf_xml_out_start_element(ncxXml, "content");
            gsf_xml_out_add_cstr(ncxXml, "src", navSrc.utf8_str());
            gsf_xml_out_end_element(ncxXml);
            
        }
        
        closeNTags(ncxXml, curLevel);
    }
    else
    {
        gsf_xml_out_start_element(ncxXml, "navPoint");
        gsf_xml_out_add_cstr(ncxXml, "playOrder", "1");
        gsf_xml_out_add_cstr(ncxXml, "class", "h1");
        gsf_xml_out_add_cstr(ncxXml, "id", "index");

        gsf_xml_out_start_element(ncxXml, "navLabel");
        gsf_xml_out_start_element(ncxXml, "text");
        gsf_xml_out_add_cstr(ncxXml, NULL, getTitle().utf8_str());
        gsf_xml_out_end_element(ncxXml);
        gsf_xml_out_end_element(ncxXml);

        gsf_xml_out_start_element(ncxXml, "content");
        gsf_xml_out_add_cstr(ncxXml, "src", "index.xhtml");
        gsf_xml_out_end_element(ncxXml);
        gsf_xml_out_end_element(ncxXml);
    }
    // </navMap>
    gsf_xml_out_end_element(ncxXml);
    
    // </ncx>
    gsf_xml_out_end_element(ncxXml);
    gsf_output_close(ncx);
    delete toc;
    
    return UT_OK;
}

UT_Error IE_Exp_EPUB::package()
{

    GsfOutput* opf = gsf_outfile_new_child(GSF_OUTFILE(m_oebps), "book.opf", FALSE);
    if (opf == NULL)
    {
        UT_DEBUGMSG(("Can`t create book.opf\n"));
        return UT_ERROR;
    }
    GsfXMLOut* opfXml = gsf_xml_out_new(opf);
    // <package>
    gsf_xml_out_start_element(opfXml, "package");
    gsf_xml_out_add_cstr(opfXml, "version", "2.0");
    gsf_xml_out_add_cstr(opfXml, "xmlns", OPF201_NAMESPACE);
    gsf_xml_out_add_cstr(opfXml, "unique-identifier", "BookId");

    // <metadata>
    gsf_xml_out_start_element(opfXml, "metadata");
    gsf_xml_out_add_cstr(opfXml, "xmlns:dc", DC_NAMESPACE);
    gsf_xml_out_add_cstr(opfXml, "xmlns:opf", OPF201_NAMESPACE);
    // Generation of required Dublin Core metadata
    gsf_xml_out_start_element(opfXml, "dc:title");
    gsf_xml_out_add_cstr(opfXml, NULL, getTitle().utf8_str());
    gsf_xml_out_end_element(opfXml);
    gsf_xml_out_start_element(opfXml, "dc:identifier");
    gsf_xml_out_add_cstr(opfXml, "id", "BookId");
    gsf_xml_out_add_cstr(opfXml, NULL, getDoc()->getDocUUIDString());
    gsf_xml_out_end_element(opfXml);
    gsf_xml_out_start_element(opfXml, "dc:language");
    gsf_xml_out_add_cstr(opfXml, NULL, getLanguage().utf8_str());
    gsf_xml_out_end_element(opfXml);
    gsf_xml_out_start_element(opfXml, "dc:creator");
    gsf_xml_out_add_cstr(opfXml, "opf:role", "aut");
    gsf_xml_out_add_cstr(opfXml, NULL, getAuthor().utf8_str());
    gsf_xml_out_end_element(opfXml);
    // </metadata> 
    gsf_xml_out_end_element(opfXml);

    // <manifest>
    gsf_xml_out_start_element(opfXml, "manifest");
    std::vector<UT_UTF8String> listing = getFileList(m_oebpsDir.substr(7, m_oebpsDir.length() - 7));

    for (std::vector<UT_UTF8String>::iterator i = listing.begin(); i != listing.end(); i++) 
    {
        UT_UTF8String fullItemPath = m_oebpsDir + G_DIR_SEPARATOR_S + *i;
        gsf_xml_out_start_element(opfXml, "item");
        gsf_xml_out_add_cstr(opfXml, "id", escapeForId(*i).utf8_str());
        gsf_xml_out_add_cstr(opfXml, "href", (*i).utf8_str());
        gsf_xml_out_add_cstr(opfXml, "media-type", getMimeType(fullItemPath).utf8_str());
        gsf_xml_out_end_element(opfXml);       
    }

    // We`ll add .ncx file manually
    gsf_xml_out_start_element(opfXml, "item");
    gsf_xml_out_add_cstr(opfXml, "id", "ncx");
    gsf_xml_out_add_cstr(opfXml, "href", "toc.ncx");
    gsf_xml_out_add_cstr(opfXml, "media-type", "application/x-dtbncx+xml");
    gsf_xml_out_end_element(opfXml);
    // </manifest>
    gsf_xml_out_end_element(opfXml);
    
    // <spine>
    gsf_xml_out_start_element(opfXml, "spine");
    gsf_xml_out_add_cstr(opfXml, "toc", "ncx");
    gsf_xml_out_start_element(opfXml, "itemref");
    gsf_xml_out_add_cstr(opfXml, "idref", "indexxhtml");
    gsf_xml_out_end_element(opfXml);
    // </spine>
    gsf_xml_out_end_element(opfXml);

    // </package>
    gsf_xml_out_end_element(opfXml);
    gsf_output_close(opf);

    return compress();
}

std::vector<UT_UTF8String> IE_Exp_EPUB::getFileList(const UT_UTF8String &directory)
{
	std::vector<UT_UTF8String> result;
    std::vector<UT_UTF8String> dirs;

    dirs.push_back(directory);

    while (dirs.size() > 0) 
    {
        UT_UTF8String currentDir = dirs.back();
        dirs.pop_back();
        GDir* baseDir = g_dir_open(currentDir.utf8_str(), 0, NULL);

        gchar const *entryName = NULL;
        while ((entryName = g_dir_read_name(baseDir)) != NULL) 
        {
            if (entryName[0] == '.') 
            {
                // Files starting with dot should be skipped - it can be temporary files 
                // created by gsf
                continue;
            }
            UT_UTF8String entryFullPath = currentDir + G_DIR_SEPARATOR_S;
            entryFullPath += entryName;

            if (g_file_test(entryFullPath.utf8_str(), G_FILE_TEST_IS_DIR)) 
            {
                dirs.push_back(entryFullPath);
            } 
            else 
            {
                result.push_back(entryFullPath.substr(directory.length() + 1, entryFullPath.length() - directory.length()));
            }
        }

        g_dir_close(baseDir);

    }

    return result;
}

UT_Error IE_Exp_EPUB::compress() 
{


    GsfInfile* oebpsDir = gsf_infile_stdio_new(UT_go_filename_from_uri(m_oebpsDir.utf8_str()), NULL);


    if (oebpsDir == NULL) {
        UT_DEBUGMSG(("RUDYJ: Can`t open temporary OEBPS directory\n"));
        return UT_ERROR;
    }

    int childCount = gsf_infile_num_children(oebpsDir);  
    std::vector<UT_UTF8String> listing = getFileList(UT_go_filename_from_uri(m_oebpsDir.utf8_str()));
    for (std::vector<UT_UTF8String>::iterator i = listing.begin(); i != listing.end(); i++) {
        GsfOutput* item = gsf_outfile_new_child(GSF_OUTFILE(m_oebps), (*i).utf8_str(), FALSE);
        UT_UTF8String fullPath = m_oebpsDir + G_DIR_SEPARATOR_S + *i;
        GsfInput* file = UT_go_file_open(fullPath.utf8_str(), NULL);

        if (file == NULL) {
            UT_DEBUGMSG(("RUDYJ: Can`t open file\n"));
            return UT_ERROR;
        }

        gsf_output_seek(item, 0, G_SEEK_SET);
        gsf_input_seek(file, 0, G_SEEK_SET);
        gsf_input_copy(file, item);
        gsf_output_close(item);
    }
        
	return UT_OK;
}

void IE_Exp_EPUB::closeNTags(GsfXMLOut* xml, int n) 
{
    for (int i = 0; i < n; i++) 
    {
        gsf_xml_out_end_element(xml);
    }

}

UT_UTF8String IE_Exp_EPUB::escapeForId(const UT_UTF8String& src)
{
    
    UT_UTF8String result = "";
    
    UT_UTF8Stringbuf::UTF8Iterator i = src.getIterator();
    i = i.start();
    

    if (i.current())
    {
        while (true)
        {
            const char *pCurrent = i.current();
            
            if (*pCurrent == 0)
            {
                break;
            }

            if (isalnum(*pCurrent)){
                result += *pCurrent;
            }

            i.advance();
        }
    }
    return result;
}

UT_UTF8String IE_Exp_EPUB::getMimeType (const UT_UTF8String &uri)
{
    const gchar *extension = strchr(uri.utf8_str(), '.');
  
    if (extension == NULL)
    {
        return UT_go_get_mime_type (uri.utf8_str());
    } else  
    {
        if (!UT_go_utf8_collate_casefold(extension + 1, "xhtml"))
        {
            return "application/xhtml+xml";
        } else
        {
            return UT_go_get_mime_type(uri.utf8_str());
        }
    }
}

UT_UTF8String IE_Exp_EPUB::getAuthor() const
{
    UT_UTF8String property("");
    
    if (getDoc()->getMetaDataProp (PD_META_KEY_CREATOR, property) && property.size())
    {
        return property;
    } else
    {
        return "Converted by AbiWord(http://www.abisource.com/)";
    }
}

UT_UTF8String IE_Exp_EPUB::getTitle() const
{
    UT_UTF8String property("");
    
    if (getDoc()->getMetaDataProp (PD_META_KEY_TITLE, property) && property.size())
    {
        return property;
    } else
    {
        return "Untitled";
    }    
}

UT_UTF8String IE_Exp_EPUB::getLanguage() const
{
    UT_UTF8String property("");
    
    if (getDoc()->getMetaDataProp (PD_META_KEY_LANGUAGE, property) && property.size())
    {
        return property;
    } else
    {
        return "en_US";
    }    
}