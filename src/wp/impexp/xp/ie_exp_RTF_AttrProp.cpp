/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include "pd_Style.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "ie_exp_RTF_AttrProp.h"




std::string eraseAP( const std::string& retvalue, const std::string& name )
{
    std::string ret = retvalue;
    UT_DEBUGMSG(("eraseAP() TOP:%s\n", ret.c_str() ));
    
    std::string::size_type start = ret.find( name );
    if( start == std::string::npos )
    {
        UT_DEBUGMSG(("eraseAP() NOT FOUND:%s\n", ret.c_str() ));
        return ret;
    }
    

    std::string::iterator first = ret.begin() + start;
    std::string::iterator last = first;
    std::string::iterator end  = ret.end();

    while( last != end && *last != ';' && *last != '}' )
    {
        ++last;
    }
    ret.erase( first, last );
    UT_DEBUGMSG(("eraseAP() END:%s\n", ret.c_str() ));
    return ret;
}


struct APFilterDropParaDeleteMarkers
{
    std::string operator()( const gchar * szName, const std::string& value ) const
    {
        if( !strcmp( szName, PT_REVISION_ATTRIBUTE_NAME ))
        {
            UT_DEBUGMSG(("APFilterDropParaDeleteMarkers::op() have rev:%s\n", value.c_str() ));
            if( std::string::npos != value.find( ABIATTR_PARA_START_DELETED_REVISION )
                || std::string::npos != value.find( ABIATTR_PARA_END_DELETED_REVISION ) )
            {
                UT_DEBUGMSG(("APFilterDropParaDeleteMarkers::op() MUTATE rev:%s\n", value.c_str() ));
                std::string ret = value;
                ret = eraseAP( ret, ABIATTR_PARA_START_DELETED_REVISION );
                ret = eraseAP( ret, ABIATTR_PARA_END_DELETED_REVISION );
                return ret;
            }
        }
        return value;
    }
};


///////////////

s_RTF_AttrPropAdapter_AP::s_RTF_AttrPropAdapter_AP( const PP_AttrProp * pSpanAP,
                                                    const PP_AttrProp * pBlockAP,
                                                    const PP_AttrProp * pSectionAP,
                                                    PD_Document * pDoc )
    : m_pSpanAP(pSpanAP)
    , m_pBlockAP(pBlockAP)
    , m_pSectionAP(pSectionAP)
    , m_pDoc(pDoc)
{
    m_attrAPFilterList.push_back( APFilterDropParaDeleteMarkers() );
}

s_RTF_AttrPropAdapter_AP::~s_RTF_AttrPropAdapter_AP()
{
}


const gchar * s_RTF_AttrPropAdapter_Style::getAttribute(const gchar * szName) const 
{
    const gchar * szValue = 0;
    m_pStyle->getAttribute(szName, szValue);
    return szValue;
}

const gchar * s_RTF_AttrPropAdapter_Style::getProperty(const gchar * szName) const 
{
    const gchar * szValue = 0;
    m_pStyle->getProperty(szName, szValue);
    return szValue;
}

const gchar * s_RTF_AttrPropAdapter_AP::getAttribute(const gchar * szName) const
{
    // UT_DEBUGMSG(("s_RTF_AttrPropAdapter_AP::getAttribute() szName:%s\n", szName ));

    // we should probably have something similar for attrs as PP_evalProperty() ...
	const gchar * pValue = NULL;

	if(m_pSpanAP && m_pSpanAP->getAttribute(szName, pValue))
		return m_attrAPFilterList( szName, pValue );

	if(m_pBlockAP && m_pBlockAP->getAttribute(szName, pValue))
		return m_attrAPFilterList( szName, pValue );

	if(m_pSectionAP && m_pSectionAP->getAttribute(szName, pValue))
		return m_attrAPFilterList( szName, pValue );

	return NULL;
}

const gchar * s_RTF_AttrPropAdapter_AP::getProperty(const gchar * szName) const 
{
    // UT_DEBUGMSG(("s_RTF_AttrPropAdapter_AP::getProperty() szName:%s\n", szName ));
    
    return PP_evalProperty(szName, m_pSpanAP, m_pBlockAP, m_pSectionAP, 
			   m_pDoc, true);
}

