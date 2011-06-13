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
#include "ut_path.h"

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

	// We need to create temporary directory to which
	// HTML plugin will export our document
	m_baseTempDir = "file://";
	m_baseTempDir += g_get_tmp_dir();
	m_baseTempDir += "/";

	// To generate unique directory name we`ll use document UUID
	m_baseTempDir += getDoc()->getDocUUIDString();
	UT_go_directory_create (m_baseTempDir.utf8_str (), 750, NULL);
	
	writeContainer();
	writeStructure();
	package();
	return UT_OK;
}

void IE_Exp_EPUB::writeContainer()
{
	UT_UTF8String metaInfPath = m_baseTempDir + "/META-INF";
	UT_go_directory_create(metaInfPath.utf8_str (), 750, NULL);
	UT_UTF8String containerPath = metaInfPath + "/container.xml"; 
 
	GsfOutput* containerFile = UT_go_file_create (containerPath.utf8_str (), NULL);
	GsfXMLOut * containerXml = gsf_xml_out_new (containerFile);
	gsf_xml_out_start_element (containerXml, "container");
	gsf_xml_out_add_cstr (containerXml, "version", "1.0");
	gsf_xml_out_add_cstr(containerXml, "xmlns", OCF201_NAMESPACE);
	gsf_xml_out_start_element (containerXml, "rootfiles");
	gsf_xml_out_start_element (containerXml, "rootfile");
	gsf_xml_out_add_cstr (containerXml, "full-path", "OEBPS/book.opf");
	gsf_xml_out_add_cstr(containerXml, "media-type", OPF_MIMETYPE);
	gsf_xml_out_end_element(containerXml);
	gsf_xml_out_end_element (containerXml);
	gsf_xml_out_end_element (containerXml);
	gsf_output_close (containerFile);
}

void IE_Exp_EPUB::writeStructure()
{
	m_oebpsDir = m_baseTempDir + "/OEBPS";
	UT_go_directory_create (m_oebpsDir.utf8_str (), 750, NULL);
	UT_UTF8String indexPath = m_oebpsDir + "/index.xhtml";

	// Exporting document to XHTML using HTML export plugin 
	char *szIndexPath = (char*)g_malloc(strlen(indexPath.utf8_str()) + 1);
	strcpy(szIndexPath, indexPath.utf8_str());
	IE_Exp_HTML *pExpHtml = new IE_Exp_HTML(getDoc());
	pExpHtml->suppressDialog(true);     
	pExpHtml->setProps("embed-css:no;html4:no;");
	pExpHtml->writeFile(szIndexPath);
	g_free(szIndexPath);
}

void IE_Exp_EPUB::package()
{
	UT_UTF8String mimetypeFile = m_baseTempDir + "/mimetype";
	GsfOutput *mimetype = UT_go_file_create (mimetypeFile.utf8_str (), NULL);
	gsf_output_write(mimetype, strlen(EPUB_MIMETYPE), (const guint8*)EPUB_MIMETYPE);
	gsf_output_close (mimetype);

	
	std::vector<UT_UTF8String> listing = getFileList(m_oebpsDir.substr(7, m_oebpsDir.length() -7));
	
	UT_UTF8String opfPath = m_oebpsDir + "/book.opf";
	GsfOutput* opf = UT_go_file_create(opfPath.utf8_str(), NULL);
	GsfXMLOut* opfXml = gsf_xml_out_new(opf);
	gsf_xml_out_start_element (opfXml, "package");
	gsf_xml_out_add_cstr(opfXml, "version", "2.0");
	gsf_xml_out_add_cstr(opfXml, "xmlns", OPF201_NAMESPACE);
	gsf_xml_out_add_cstr(opfXml, "unique-identifier", getDoc()->getDocUUIDString());
	
	gsf_xml_out_start_element(opfXml, "metadata");
	gsf_xml_out_add_cstr(opfXml, "xmlns:dc", DC_NAMESPACE);
	gsf_xml_out_add_cstr(opfXml,"xmlns:opf", OPF201_NAMESPACE);
	
	gsf_xml_out_start_element(opfXml, "dc:title");
	// gsf_xml_out_add_cstr(opfXml, NULL, getDoc()->to
	gsf_xml_out_end_element(opfXml);
	gsf_xml_out_end_element(opfXml);

	gsf_xml_out_start_element(opfXml, "manifest");
	for(std::vector<UT_UTF8String>::iterator i = listing.begin(); i != listing.end(); i++)
	{
		UT_UTF8String fullItemPath = m_oebpsDir + "/" + *i;
		gsf_xml_out_start_element(opfXml, "item");
		gsf_xml_out_add_cstr(opfXml, "id", "");
		gsf_xml_out_add_cstr(opfXml, "href", (*i).utf8_str());
		gsf_xml_out_add_cstr(opfXml, "media-type", UT_go_get_mime_type(fullItemPath.utf8_str()));
		gsf_xml_out_end_element(opfXml);
	}
	gsf_xml_out_end_element(opfXml);

	gsf_xml_out_start_element(opfXml, "spine");
	gsf_xml_out_end_element(opfXml);
	



	gsf_xml_out_end_element(opfXml);
	gsf_output_close(opf);
	
	for(std::vector<UT_UTF8String>::iterator i = listing.begin(); i != listing.end(); i++)
	{
		UT_DEBUGMSG(((*i).utf8_str()));
	}

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
			UT_UTF8String entryFullPath = currentDir + "/";
			entryFullPath += entryName;

			if (g_file_test(entryFullPath.utf8_str(), G_FILE_TEST_IS_DIR))
			{
				dirs.push_back(entryFullPath);
			} else
			{
				result.push_back(entryFullPath.substr(directory.length() + 1, entryFullPath.length() - directory.length()));
			}
		}

		g_dir_close(baseDir);

	}

	return result;
}

