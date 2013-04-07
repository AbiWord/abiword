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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
// Class definition include
#include "ODi_ContentStream_ListenerState.h"

// Internal includes
#include "ODi_FontFaceDecls.h"
#include "ODi_Office_Styles.h"
#include "ODi_StartTag.h"
#include "ODi_Style_Style.h"
#include "ODi_Style_List.h"
#include "ODi_ListenerStateAction.h"
#include "ODi_Abi_Data.h"


/**
 * Constructor
 */
ODi_ContentStream_ListenerState::ODi_ContentStream_ListenerState (
                PD_Document* pDocument,
                GsfInfile* pGsfInfile,
                ODi_Office_Styles* pStyles,
                ODi_FontFaceDecls& rFontFaceDecls,
                ODi_ElementStack& rElementStack,
		ODi_Abi_Data& rAbiData  )
                : ODi_ListenerState("ContentStream", rElementStack),
                  m_pAbiDocument (pDocument),
                  m_pGsfInfile(pGsfInfile),
                  m_pStyles(pStyles),
                  m_rFontFaceDecls(rFontFaceDecls),
		  m_rAbiData(rAbiData)
{
    UT_ASSERT_HARMLESS(m_pAbiDocument);
    UT_ASSERT_HARMLESS(m_pStyles);
    UT_ASSERT_HARMLESS(m_pGsfInfile);
}


/**
 * Destructor
 */
ODi_ContentStream_ListenerState::~ODi_ContentStream_ListenerState()
{
}


/**
 * Called when the XML parser finds a start element tag.
 * 
 * @param pName The name of the element.
 * @param ppAtts The attributes of the parsed start tag.
 */
void ODi_ContentStream_ListenerState::startElement (const gchar* pName,
                                               const gchar** ppAtts,
                                               ODi_ListenerStateAction& rAction)
{
    if (!strcmp(pName, "office:font-face-decls")) {

        rAction.pushState("FontFaceDecls");

    } else if (!strcmp(pName, "office:body")) {

        // Here comes the document itself. So, at this point, all styles have
        // already been defined.
        m_pStyles->addedAllStyles(m_pAbiDocument,  m_rFontFaceDecls);

    } else if (!strcmp(pName, "style:style")) {
        ODi_ListenerState* pStyle = NULL;
        
        pStyle = m_pStyles->addStyle(ppAtts, 
				     m_rElementStack,
				     m_rAbiData);
        
        // pStyle can be null for unsupported (ignored) styles.
        if (pStyle) {
            rAction.pushState(pStyle, false);
        }
        
    } else if (!strcmp (pName, "text:list-style")) {
        
        ODi_ListenerState* pStyle;
            
        pStyle = m_pStyles->addList(ppAtts, m_rElementStack);
        rAction.pushState(pStyle, false);
        
    } else if (!strcmp(pName, "office:text")) {
        rAction.pushState("TextContent");
    }
}


/**
 *
 */
void ODi_ContentStream_ListenerState::endElement (const gchar* pName,
                                                ODi_ListenerStateAction& rAction)
{
    if (!strcmp(pName, "office:document-content")) {
        // We're done.
        rAction.popState();
    }
}


/**
 * 
 */
void ODi_ContentStream_ListenerState::charData (const gchar* /*pBuffer*/, 
												int /*length*/)
{
}
