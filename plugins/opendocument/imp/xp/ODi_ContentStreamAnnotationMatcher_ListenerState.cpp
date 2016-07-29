/* AbiSource
 * 
 * Copyright (C) 2011 Ben Martin
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
#include "ODi_ContentStreamAnnotationMatcher_ListenerState.h"

// Internal includes
#include "ODi_FontFaceDecls.h"
#include "ODi_Office_Styles.h"
#include "ODi_StartTag.h"
#include "ODi_Style_Style.h"
#include "ODi_Style_List.h"
#include "ODi_ListenerStateAction.h"
#include "ODi_Abi_Data.h"
#include <ut_misc.h>


/**
 * Constructor
 */
ODi_ContentStreamAnnotationMatcher_ListenerState::ODi_ContentStreamAnnotationMatcher_ListenerState (
    PD_Document* pDocument,
    GsfInfile* pGsfInfile,
    ODi_Office_Styles* pStyles,
    ODi_ElementStack& rElementStack,
    ODi_Abi_Data& rAbiData  )
    : ODi_ListenerState("ContentStreamAnnotationMatcher", rElementStack),
      m_pAbiDocument (pDocument),
      m_pGsfInfile(pGsfInfile),
      m_pStyles(pStyles),
      m_rAbiData(rAbiData)
{
    UT_ASSERT_HARMLESS(m_pAbiDocument);
    UT_ASSERT_HARMLESS(m_pStyles);
    UT_ASSERT_HARMLESS(m_pGsfInfile);
}


ODi_ContentStreamAnnotationMatcher_ListenerState::~ODi_ContentStreamAnnotationMatcher_ListenerState()
{
}


void
ODi_ContentStreamAnnotationMatcher_ListenerState::startElement ( const gchar* pName,
                                                                 const gchar** ppAtts,
                                                                 ODi_ListenerStateAction& /*rAction*/ )
{
    if (!strcmp(pName, "office:annotation"))
    {
        if( const gchar* s = UT_getAttribute("office:name", ppAtts))
        {
            m_rAbiData.m_openAnnotationNames.insert(s);
        }
    }
    else if (!strcmp(pName, "office:annotation-end"))
    {
        const gchar* name = UT_getAttribute("office:name", ppAtts);
        if( name && m_rAbiData.m_openAnnotationNames.count(name) )
        {
            m_rAbiData.m_openAnnotationNames.erase(name);
            m_rAbiData.m_rangedAnnotationNames.insert(name);
        }
   }
}


void
ODi_ContentStreamAnnotationMatcher_ListenerState::endElement ( const gchar* pName,
                                                               ODi_ListenerStateAction& rAction)
{
    if (!strcmp(pName, "office:annotation"))
    {
    }
    else if (!strcmp(pName, "office:annotation-end"))
    {
    }
    else if (!strcmp(pName, "office:document-content"))
    {
        // We're done.
        rAction.popState();
    }
}


void
ODi_ContentStreamAnnotationMatcher_ListenerState::charData ( const gchar* /*pBuffer*/, 
                                                             int /*length*/)
{
}


const std::set< std::string >&
ODi_ContentStreamAnnotationMatcher_ListenerState::getRangedAnnotationNames() const
{
    return m_rAbiData.m_rangedAnnotationNames;
}

