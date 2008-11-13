/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
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
 

#ifndef _IE_IMP_OPENDOCUMENT_H_
#define _IE_IMP_OPENDOCUMENT_H_


// Internal includes
#include "ODi_Office_Styles.h"
#include "ODi_Abi_Data.h"

// AbiWord inlcudes
#include <ie_imp.h>

// External includes
#include <gsf/gsf.h>

// Internal classes
class ODi_StreamListener;


/**
 * Class used to import OpenDocument files
 */
class IE_Imp_OpenDocument : public IE_Imp
{
public:

    IE_Imp_OpenDocument (PD_Document * pDocument);
    virtual ~IE_Imp_OpenDocument ();

 protected:
    virtual UT_Error _loadFile(GsfInput * input);
    
private:

    UT_Error _handleManifestStream ();
    UT_Error _handleMimetype ();
    UT_Error _handleMetaStream ();
    UT_Error _handleSettingsStream ();
    UT_Error _handleStylesStream ();
    UT_Error _handleContentStream ();
    void _setDocumentProperties();

    static UT_Error _handleStream (GsfInfile* pGsfInfile,
                   const char* pStream, UT_XML::Listener& rListener);

    static UT_Error _parseStream ( GsfInfile* pGsfInfile, 
                  const char* pStream,
                  UT_XML & parser );

    GsfInfile* m_pGsfInfile;
    
    ODi_StreamListener* m_pStreamListener;
    ODi_Office_Styles m_styles;
    ODi_Abi_Data* m_pAbiData;
    
};

#endif //_IE_IMP_OPENDOCUMENT_H_
