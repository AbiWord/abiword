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

#include "ie_imp_EPUB.h"

IE_Imp_EPUB::IE_Imp_EPUB(PD_Document* pDocument) :
    IE_Imp(pDocument)
{

}

IE_Imp_EPUB::~IE_Imp_EPUB()
{

}

bool IE_Imp_EPUB::pasteFromBuffer(PD_DocumentRange* pDocRange,
				  const unsigned char* pData, UT_uint32 lenData, const char* /*szEncoding*/)
{
    UT_return_val_if_fail(getDoc() == pDocRange->m_pDoc,false);
    UT_return_val_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2,false);

    PD_Document * newDoc = new PD_Document();
    newDoc->createRawDocument();
    IE_Imp_EPUB * pEPUBImp = new IE_Imp_EPUB(newDoc);
    //
    // Turn pData into something that can be imported by the open documenb
    // importer.
    //
    GsfInput * pInStream = gsf_input_memory_new((const guint8 *) pData,
            (gsf_off_t) lenData, FALSE);
    pEPUBImp->loadFile(newDoc, pInStream);

    newDoc->finishRawCreation();

    IE_Imp_PasteListener * pPasteListen = new IE_Imp_PasteListener(getDoc(),
            pDocRange->m_pos1, newDoc);
    newDoc->tellListener(static_cast<PL_Listener *> (pPasteListen));
    delete pPasteListen;
    delete pEPUBImp;
    UNREFP( newDoc);
    return true;
}

UT_Error IE_Imp_EPUB::_loadFile(GsfInput* input)
{
    m_epub = gsf_infile_zip_new(input, NULL);

    if (m_epub == NULL)
    {
        UT_DEBUGMSG(("Can`t create gsf input zip object\n"));
        return UT_ERROR;
    }

    UT_DEBUGMSG(("Reading metadata\n"));
    if (readMetadata() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to read metadata\n"));
        return UT_ERROR;
    }

    UT_DEBUGMSG(("Reading package information\n"));
    if (readPackage() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to read package information\n"));
        return UT_ERROR;
    }

    UT_DEBUGMSG(("Uncompressing OPS data\n"));
    if (uncompress() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to uncompress data\n"));
        return UT_ERROR;
    }

    UT_DEBUGMSG(("Reading OPS data\n"));
    if (readStructure() != UT_OK)
    {
        UT_DEBUGMSG(("Failed to read OPS data\n"));
        return UT_ERROR;
    }

    return UT_OK;

}

UT_Error IE_Imp_EPUB::readMetadata()
{
    GsfInput* metaInf = gsf_infile_child_by_name(m_epub, "META-INF");

    if (metaInf == NULL)
    {
        UT_DEBUGMSG(("Can`t open container META-INF dir\n"));
        return UT_ERROR;
    }

    GsfInput* meta = gsf_infile_child_by_name(GSF_INFILE(metaInf),
            "container.xml");

    if (meta == NULL)
    {
        UT_DEBUGMSG(("Can`t open container metadata\n"));
        return UT_ERROR;
    }

    size_t metaSize = gsf_input_size(meta);

    if (metaSize == 0)
    {
        UT_DEBUGMSG(("Container metadata file is empty\n"));
        return UT_ERROR;
    }

    gchar* metaXml = (gchar*) gsf_input_read(meta, metaSize, NULL);

    std::string rootfilePath;
    UT_XML metaParser;
    ContainerListener containerListener;
    metaParser.setListener(&containerListener);

    if (metaParser.sniff(metaXml, metaSize, "container"))
    {
        UT_DEBUGMSG(("Parsing container.xml file\n"));
        metaParser.parse(metaXml, metaSize);
    }
    else
    {
        UT_DEBUGMSG(("Incorrect container.xml file\n"));
        return UT_ERROR;
    }

    m_rootfilePath = containerListener.getRootFilePath();

    g_object_unref(G_OBJECT(meta));
    g_object_unref(G_OBJECT(metaInf));

    return UT_OK;
}

UT_Error IE_Imp_EPUB::readPackage()
{
    gchar **aname = g_strsplit(m_rootfilePath.c_str(), G_DIR_SEPARATOR_S, 0);
    GsfInput* opf = gsf_infile_child_by_aname(m_epub, (const char**) aname);

    UT_DEBUGMSG(("Getting parent\n"));
    GsfInfile* opfParent = gsf_input_container(opf);
    m_opsDir = std::string(gsf_input_name(GSF_INPUT(opfParent)));

    UT_DEBUGMSG(("OPS dir: %s\n", m_opsDir.c_str()));

    if (opf == NULL)
    {
        UT_DEBUGMSG(("Can`t open .opf file\n"));
        return UT_ERROR;
    }

    size_t opfSize = gsf_input_size(opf);
    gchar* opfXml = (gchar*) gsf_input_read(opf, opfSize, NULL);

    UT_XML opfParser;
    OpfListener opfListener;
    opfParser.setListener(&opfListener);
    if (opfParser.sniff(opfXml, opfSize, "package"))
    {
        UT_DEBUGMSG(("Parsing opf file\n"));
        opfParser.parse(opfXml, opfSize);
    }
    else
    {
        UT_DEBUGMSG(("Incorrect opf file found \n"));
        return UT_ERROR;
    }

    g_strfreev(aname);
    g_object_unref(G_OBJECT(opf));
    //g_object_unref(G_OBJECT(opfParent));

    m_spine = opfListener.getSpine();
    m_manifestItems = opfListener.getManifestItems();

    return UT_OK;
}

UT_Error IE_Imp_EPUB::uncompress()
{
    m_tmpDir = UT_go_filename_to_uri(g_get_tmp_dir());
    m_tmpDir += G_DIR_SEPARATOR_S;
    m_tmpDir += getDoc()->getDocUUIDString();

    if (!UT_go_directory_create(m_tmpDir.c_str(), NULL))
    {
        UT_DEBUGMSG(("Can`t create temporary directory\n"));
        return UT_ERROR;
    }
    GsfInput *opsDirInput = gsf_infile_child_by_name(m_epub,
            m_opsDir.c_str());
    UT_DEBUGMSG(("Child count : %d", gsf_infile_num_children(m_epub)));
    if (opsDirInput == NULL)
    {
        UT_DEBUGMSG(("Failed to open OPS dir\n"));
        return UT_ERROR;
    }

    for (std::map<std::string, std::string>::iterator i =
            m_manifestItems.begin(); i != m_manifestItems.end(); i++)
    {
        gchar *itemFileName = UT_go_filename_from_uri(
                (m_tmpDir + G_DIR_SEPARATOR_S + (*i).second).c_str());
        gchar** aname =
                g_strsplit((*i).second.c_str(), G_DIR_SEPARATOR_S, 0);

        GsfInput* itemInput = gsf_infile_child_by_aname(
                GSF_INFILE(opsDirInput), (const char**) aname);
        GsfOutput* itemOutput = createFileByPath(itemFileName);
        gsf_input_seek(itemInput, 0, G_SEEK_SET);
        gsf_input_copy(itemInput, itemOutput);
        g_strfreev(aname);
        g_free(itemFileName);
        g_object_unref(G_OBJECT(itemInput));
        gsf_output_close(itemOutput);
    }

    g_object_unref(G_OBJECT(opsDirInput));

    return UT_OK;
}

UT_Error IE_Imp_EPUB::readStructure()
{
    getDoc()->createRawDocument();
    getDoc()->finishRawCreation();

    for (std::vector<std::string>::iterator i = m_spine.begin(); i
            != m_spine.end(); i++)
    {
        std::map<std::string, std::string>::iterator iter =
                m_manifestItems.find(*i);

        if (iter == m_manifestItems.end())
        {
            UT_DEBUGMSG(("Manifest item with id %s not found\n", (*i).c_str()));
            return UT_ERROR;
        }
	std::string itemPath = m_tmpDir + G_DIR_SEPARATOR_S + (iter->second);
        PT_DocPosition posEnd = 0;
        getDoc()->getBounds(true, posEnd);

        if (i != m_spine.begin())
        {
            getDoc()->insertStrux(posEnd, PTX_Section, PP_NOPROPS, PP_NOPROPS);
            getDoc()->insertStrux(posEnd+1, PTX_Block, PP_NOPROPS, PP_NOPROPS);
            posEnd+=2;
        }

        GsfInput* itemInput = UT_go_file_open(itemPath.c_str(), NULL);
        if (itemInput == NULL)
        {
            UT_DEBUGMSG(("Can`t open item for reading\n"));
            return UT_ERROR;
        }

        PD_Document *currentDoc = new PD_Document();
        currentDoc->createRawDocument();
        const char *suffix = strchr(itemPath.c_str(), '.');
        XAP_App::getApp()->getPrefs()->setIgnoreNextRecent();
        if (currentDoc->importFile(itemPath.c_str(),
                IE_Imp::fileTypeForSuffix(suffix), true, false, NULL) != UT_OK)
        {
            UT_DEBUGMSG(("Failed to import file %s\n", itemPath.c_str()));
            return UT_ERROR;
        }

        currentDoc->finishRawCreation();
        // const gchar * attributes[3] = {
        //     "listid",
        //     "0",
        //     0
        // };

        // PT_DocPosition pos;
        // currentDoc->getBounds(true, pos);
        // currentDoc->insertStrux(pos, PTX_Block, attributes, PP_NOPROPS, PP_NOPROPS);

        IE_Imp_PasteListener * pPasteListener = new IE_Imp_PasteListener(
                getDoc(), posEnd, currentDoc);
        currentDoc->tellListener(static_cast<PL_Listener *> (pPasteListener));


        DELETEP(pPasteListener);
        UNREFP(currentDoc);
        g_object_unref(G_OBJECT(itemInput));
    }

    return UT_OK;
}

GsfOutput* IE_Imp_EPUB::createFileByPath(const char* path)
{
    gchar** components = g_strsplit(path, G_DIR_SEPARATOR_S, 0);
    std::string curPath = "";

    int current = 0;
    GsfOutput* output = NULL;
    while (components[current] != NULL)
    {
        curPath += components[current];
        current++;

        char *uri = UT_go_filename_to_uri(curPath.c_str());
        bool fileExists = UT_go_file_exists(uri);
        if (!fileExists && (components[current] != NULL))
        {
            UT_go_directory_create(uri, NULL);
        }
        else
        {
            if (!fileExists)
            {
                output = UT_go_file_create(uri, NULL);
                break;
            }
        }

        g_free(uri);

        if (components[current] != NULL)
        {
            curPath += G_DIR_SEPARATOR_S;
        }
    }

    g_strfreev(components);
    return output;
}

void ContainerListener::startElement(const gchar* name, const gchar** atts)
{
    if (!UT_go_utf8_collate_casefold(name, "rootfile"))
    {
        m_rootFilePath = std::string(UT_getAttribute("full-path", atts));
        UT_DEBUGMSG(("Found rootfile%s\n", m_rootFilePath.c_str()));
    }
}

void ContainerListener::endElement(const gchar* /*name*/)
{
}

void ContainerListener::charData(const gchar* /*buffer*/, int /*length*/)
{

}

const std::string & ContainerListener::getRootFilePath() const
{
    return m_rootFilePath;
}

/*

 */

OpfListener::OpfListener() :
    m_inManifest(false)
{

}

void OpfListener::startElement(const gchar* name, const gchar** atts)
{
    if (!UT_go_utf8_collate_casefold(name, "manifest"))
    {
        m_inManifest = true;
    }

    if (!UT_go_utf8_collate_casefold(name, "spine"))
    {
        m_inSpine = true;
    }

    if (m_inManifest)
    {
        if (!UT_go_utf8_collate_casefold(name, "item"))
        {
            m_manifestItems.insert(
				   make_pair(std::string(UT_getAttribute("id", atts)),
					     std::string(UT_getAttribute("href", atts))));
            UT_DEBUGMSG(("Found manifest item: %s\n", UT_getAttribute("href", atts)));
        }
    }

    if (m_inSpine)
    {
        if (!UT_go_utf8_collate_casefold(name, "itemref"))
        {
            // We can ignore "linear" attribute as it said in specification
	    m_spine.push_back(std::string(UT_getAttribute("idref", atts)));
            UT_DEBUGMSG(("Found spine itemref: %s\n", UT_getAttribute("idref", atts)));
        }
    }

}

void OpfListener::endElement(const gchar* /*name*/)
{

}

void OpfListener::charData(const gchar* /*buffer*/, int /*length*/)
{

}

/*

 */

void NavigationListener::startElement(const gchar* /*name*/, const gchar** /*atts*/)
{

}

void NavigationListener::endElement(const gchar* /*name*/)
{

}

void NavigationListener::charData(const gchar* /*buffer*/, int /*length*/)
{

}
