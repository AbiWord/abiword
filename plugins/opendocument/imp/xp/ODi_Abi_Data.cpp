/* AbiSource
 * 
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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
 
// Class definition include
#include "ODi_Abi_Data.h"

// AbiWord includes
#include <pd_Document.h>
#include <pt_Types.h>
#include <ie_impGraphic.h>
#include <fg_GraphicRaster.h>

// External includes
#include <glib-object.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>


/**
 * Constructor
 */
ODi_Abi_Data::ODi_Abi_Data(PD_Document* pDocument, GsfInfile* pGsfInfile) :
    m_pAbiDocument (pDocument), m_pGsfInfile (pGsfInfile) {
}


/**
 * Adds an data item (<d> tag) in the AbiWord document for the specified image.
 * 
 * Code mainly from Dom Lachowicz and/or Robert Staudinger.
 * 
 * @param rDataId Receives the id that has been given to the added data item.
 * @param ppAtts The attributes of a <draw:image> element.
 */
bool ODi_Abi_Data::addImageDataItem(UT_String& rDataId, const gchar** ppAtts) {
    
    const gchar* pHRef = UT_getAttribute ("xlink:href", ppAtts);
    UT_return_val_if_fail(pHRef,false);

    // If we have a string smaller then this we are in trouble. File corrupted?
    UT_return_val_if_fail((strlen(pHRef) >= 10 /*10 == strlen("Pictures/a")*/), false);

    UT_Error error = UT_OK;
    UT_ByteBuf img_buf;
    GsfInfile* pPictures_dir;
    FG_Graphic* pFG = NULL;
    const UT_ByteBuf* pPictData = NULL;
    UT_uint32 imageID;
    
    // The subdirectory that holds the picture. e.g: "ObjectReplacements" or "Pictures"
    UT_String dirName;
    
    // The file name of the picture. e.g.: "Object 1" or "10000201000000D100000108FF0E3707.png" 
    UT_String fileName;
    
    const std::string id = m_href_to_id[pHRef];
    if (!id.empty()) {
        // This image was already added.
        // Use the existing data item id.
        rDataId = id;
        return true;
    }
    
    
    // Get a new, unique, ID.
    imageID = m_pAbiDocument->getUID(UT_UniqueId::Image);
    UT_String_sprintf(rDataId, "%d", imageID);
    
    // Add this id to the list
	href_id_map_t::iterator iter = m_href_to_id
		.insert(m_href_to_id.begin(),
				href_id_map_t::value_type(pHRef, 
										  rDataId.c_str()));
    UT_ASSERT(iter != m_href_to_id.end());

    _splitDirectoryAndFileName(pHRef, dirName, fileName);

    pPictures_dir =
        GSF_INFILE(gsf_infile_child_by_name(m_pGsfInfile, dirName.c_str()));

    UT_return_val_if_fail(pPictures_dir, false);

    // Loads img_buf
    error = _loadStream(pPictures_dir, fileName.c_str(), img_buf);
    g_object_unref (G_OBJECT (pPictures_dir));

    if (error != UT_OK) {
        return false;
    }


    // Builds pImporter from img_buf
    error = IE_ImpGraphic::loadGraphic (img_buf, IEGFT_Unknown, &pFG);
    if ((error != UT_OK) || !pFG) {
        // pictData is already freed in ~FG_Graphic
      return false;
    }

    // Builds pPictData from pFG
    // TODO: can we get back a vector graphic?
    pPictData = pFG->getBuffer();

    if (!pPictData) {
        // i don't think that this could ever happen, but...
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        return false;
    }

    //
    // Create the data item.
    //

    if (!m_pAbiDocument->createDataItem(rDataId.c_str(),
                                        false,
                                        pPictData,
                                        pFG->getMimeType(),
                                        NULL)) {
            
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        return false;
    }    

    return true;
}

/**
 * Adds an data item (<d> tag) in the AbiWord document for the specified image.
 * 
 * Code mainly from Dom Lachowicz and/or Robert Staudinger.
 * 
 * @param rDataId Receives the id that has been given to the added data item.
 * @param ppAtts The attributes of a <draw:image> element.
 */
bool ODi_Abi_Data::addObjectDataItem(UT_String& rDataId, const gchar** ppAtts, int& pto_Type) {
    
    const gchar* pHRef = UT_getAttribute ("xlink:href", ppAtts);
    UT_return_val_if_fail(pHRef,false);

    // If we have a string smaller then this we are in trouble. File corrupted?
    UT_return_val_if_fail((strlen(pHRef) >= 10 /*10 == strlen("Pictures/a")*/), false);

    UT_Error error = UT_OK;
    UT_ByteBuf *object_buf;
    GsfInfile* pObjects_dir;
    UT_uint32 objectID;

    // The subdirectory that holds the picture. e.g: "ObjectReplacements" or "Pictures"
    UT_String dirName;
    
    // The file name of the picture. e.g.: "Object 1" or "10000201000000D100000108FF0E3707.png" 
    UT_String fileName;
    
    const std::string id = m_href_to_id[pHRef];
    if (!id.empty()) {
        // This object was already added.
        // Use the existing data item id.
        rDataId = id;
        return true;
    }
    
    
    // Get a new, unique, ID.
    objectID = m_pAbiDocument->getUID(UT_UniqueId::Math);
    UT_String_sprintf(rDataId, "MathLatex%d", objectID);
    
    // Add this id to the list
	href_id_map_t::iterator iter = m_href_to_id
		.insert(m_href_to_id.begin(),
				href_id_map_t::value_type(pHRef, 
										  rDataId.c_str()));
    UT_ASSERT(iter != m_href_to_id.end());

    _splitDirectoryAndFileName(pHRef, dirName, fileName);

    if (fileName.empty ())
      fileName = "content.xml";

    pObjects_dir =
        GSF_INFILE(gsf_infile_child_by_name(m_pGsfInfile, dirName.c_str()));

    UT_return_val_if_fail(pObjects_dir, false);

    // Loads object_buf
    object_buf = new UT_ByteBuf ();
    error = _loadStream(pObjects_dir, fileName.c_str(), *object_buf);
    g_object_unref (G_OBJECT (pObjects_dir));

    if (error != UT_OK) {
	delete object_buf;
        return false;
    }

    // check to ensure that we're seeing math. this can probably be made smarter.
    static const char math_header[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE math:math";

    if ((object_buf->getLength () > strlen (math_header)) &&
	(strncmp ((const char*)object_buf->getPointer (0), math_header, strlen (math_header)) != 0)) {
	delete object_buf;
        return false;
    }

    //
    // Create the data item.
    //

    if (!m_pAbiDocument->createDataItem(rDataId.c_str(), false, object_buf, 
                                        "application/mathml+xml", NULL)) {            
        UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        return false;
    }  

    pto_Type = PTO_Math;
    return true;
}


/**
 * Code from Dom Lachowicz and/or Robert Staudinger.
 */
UT_Error ODi_Abi_Data::_loadStream (GsfInfile* oo,
                                   const char* stream,
                                   UT_ByteBuf& buf ) {
    guint8 const *data = NULL;
    size_t len = 0;
    static const size_t BUF_SZ = 4096;
  
    buf.truncate (0);
    GsfInput * input = gsf_infile_child_by_name(oo, stream);

    if (!input)
        return UT_ERROR;
  
    if (gsf_input_size (input) > 0) {
        while ((len = gsf_input_remaining (input)) > 0) {
            len = UT_MIN (len, BUF_SZ);
            if (NULL == (data = gsf_input_read (input, len, NULL))) {
                g_object_unref (G_OBJECT (input));
                return UT_ERROR;
            }
            buf.append ((const UT_Byte *)data, len);
        }
    }
  
    g_object_unref (G_OBJECT (input));
    return UT_OK;
}

/**
 * Takes a string like "./ObjectReplacements/Object 1" and split it into
 * subdirectory name ("ObjectReplacements") and file name ("Object 1").
 */
void ODi_Abi_Data::_splitDirectoryAndFileName(const gchar* pHRef, UT_String& dirName, UT_String& fileName) const {
    UT_String href;
    UT_String str;
    int iStart, nChars, i, len;
    
    href = pHRef;
    
    ////
    // Get the directory name
    
    str = href.substr(0, 2);
    if (str == "./") {
        iStart = 2;
    } else {
        iStart = 0;
    }

    len = href.length();
    for (i=iStart, nChars=0; i<len; i++) {
        if (href[i] == '/') {
            i=len; // exit loop
        } else {
            nChars++;
        }
    }
    
    UT_ASSERT (nChars > 0 && nChars < len);
    
    dirName = href.substr(iStart, nChars);
    
    ////
    // Get the file name
    
    iStart = iStart + nChars + 1;
    
    nChars = len - iStart;
    
    UT_ASSERT (nChars); // The file name must have at least one char.
    
    fileName = href.substr(iStart, nChars);
}
