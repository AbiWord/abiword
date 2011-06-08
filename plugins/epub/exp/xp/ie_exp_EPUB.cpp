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
    
        m_outputFile = GSF_OUTFILE(gsf_outfile_zip_new (getFp(), NULL));
        GsfOutput *mimetype = gsf_outfile_new_child (m_outputFile, "mimetype", FALSE);
        gsf_output_write(mimetype, strlen(EPUB_MIMETYPE), (const guint8*)EPUB_MIMETYPE);
        gsf_output_close(mimetype);
        GsfOutput *oebpsDir = gsf_outfile_new_child(m_outputFile, "oebps", TRUE);
               
        // Now we need to create temporary file to which
        // HTML plugin will export our document
        UT_UTF8String tmpPath;
        tmpPath = "file://";
        tmpPath += g_get_tmp_dir();
        tmpPath += "/";
        // To generate unique filename we`ll use document UUID
        tmpPath += getDoc()->getDocUUIDString();
        tmpPath += ".html";


        char *tmpFilename = (char*)g_malloc(strlen(tmpPath.utf8_str()) + 1);
        strcpy(tmpFilename, tmpPath.utf8_str());
        

        IE_Exp_HTML *pExpHtml = new IE_Exp_HTML(getDoc());
        pExpHtml->suppressDialog(true);     
        pExpHtml->setProps("embed-css: no;");
        pExpHtml->writeFile(tmpFilename);

        
  
        g_free(tmpFilename); 
      //  gsf_output_close(tempFile);
        gsf_output_close(oebpsDir);
        gsf_output_close(GSF_OUTPUT(m_outputFile));
        return UT_OK;
}


