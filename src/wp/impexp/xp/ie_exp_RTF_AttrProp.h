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


#ifndef IE_EXP_RTF_ATTRPROP_H
#define IE_EXP_RTF_ATTRPROP_H

#include "pt_Types.h"
class PD_Document;
class PD_Style;
class PP_AttrProp;


#include <list>
#include <boost/function.hpp>

/**
 * Collect a sequence of APFilter objects and apply them in turn when
 * a value is found for an attribute/property. Useful when you wish to
 * mutate some of the values for either the attribute or properties
 * but you do not wish to have to track the memory of your mutation.
 * This class contains a single std::string cache so the caller can
 * rely on the return value being sane until the next call to
 * operator(); Many APFilter functors can be called on the same attr/prop
 * and the result is:
 * push_back(f1); push_back(f2);
 * this->operator()( name, value ) == f2( name, f1(name, value ))
 *
 * If there are no filter objects then the degenerate case is to just
 * return the szValue given directly. Thus a APFilterList object
 * without any filtering should not present a significant performance
 * overhead. Since this is a template, the compiler has the option to
 * inline the code and perpahs the empty() test on the filterlist also
 * so that the cost becomes very very minimal.
 *
 * Added by monkeyiq in June 2011 in order to delete part of the
 * markup inside the revision attribute when a copy and paste is
 * happening. Specifically, the markers as to if a paragraph is
 * deleted need to be removed for pasted content as that content is
 * considered fresh and content will have been coalesed.
 *
 * Note that you can apply this filterlist to Attributes or Properties
 * depending on where you source the pValue from.
 *
 * Usage:
 * APFilterList al;
 * al.push_back( f2 );
 * ... somehow get szName and pValue from an AP
 * return al( szName, pValue );
 *
 * Where f2 is a filter like this:
 * struct APFilterDropParaDeleteMarkers
 * {
 *   std::string operator()( const gchar * szName, const std::string& value ) const
 *   {
 *     if( !strcmp( szName, "foo" ))
 *       return "bar";
 *     return value;
 *   }
 * };
 */
class APFilterList
    :
    public std::binary_function< const gchar *, const gchar *, const gchar * >
{
protected:
    mutable std::string m_cache;
    typedef boost::function2< std::string, const gchar *, const std::string& > m_filter_t;
    typedef std::list< m_filter_t > m_filterlist_t;
    m_filterlist_t m_filterlist;

public:
    const gchar* operator()( const gchar * szName, const gchar * szValue ) const
    {
        if( m_filterlist.empty() )
            return szValue;

        m_cache = szValue ? szValue : "";
        for( m_filterlist_t::const_iterator fi = m_filterlist.begin();
             fi != m_filterlist.end(); ++fi )
        {
            const m_filter_t& f = *fi;
            m_cache = f( szName, m_cache );
        }

        return m_cache.c_str();
    }
    void append( m_filter_t f )
    {
        m_filterlist.push_back(f);
    }
    void push_back( m_filter_t f )
    {
        append( f );
    }
};

/**
 * Template for new APFilter classes. Check the name or value and return something
 * else if desired.
 */
struct APFilterNULL
{
    std::string operator()( const gchar * szName, const std::string& value ) const
    {
        UT_UNUSED( szName );
        return value;
    }
};




/*!
 * This is an adapter interface. Some of the RTF export functions
 * are used for exporting both style sheets and document contents.
 * Styles and document elements use different ways of looking
 * up property values, but we don't care about that.
 *
 * s_RTF_AttrPropAdapter is an abstract class for accessing properties.
 * The function getProperty takes a property name as argument
 * and returns the value, or NULL if that property isn't defined.
 */

class ABI_EXPORT s_RTF_AttrPropAdapter
{
public:
	virtual ~s_RTF_AttrPropAdapter() {}
    virtual const gchar * getAttribute(const gchar * szName) const = 0;

    virtual const gchar * getProperty(const gchar * szName) const = 0;
};

class ABI_EXPORT s_RTF_AttrPropAdapter_Style : public s_RTF_AttrPropAdapter
{
private:
    const PD_Style * m_pStyle;

public:
	virtual ~s_RTF_AttrPropAdapter_Style() {}
    s_RTF_AttrPropAdapter_Style(const PD_Style * pStyle) : m_pStyle(pStyle) {}

    virtual const gchar * getAttribute(const gchar * szName) const;
    virtual const gchar * getProperty(const gchar * szName) const;
};

class ABI_EXPORT s_RTF_AttrPropAdapter_AP : public s_RTF_AttrPropAdapter
{
private:
    const PP_AttrProp * m_pSpanAP;
    const PP_AttrProp * m_pBlockAP;
    const PP_AttrProp * m_pSectionAP;
    PD_Document *m_pDoc;
    APFilterList m_attrAPFilterList;

public:
    s_RTF_AttrPropAdapter_AP(const PP_AttrProp * pSpanAP,
                             const PP_AttrProp * pBlockAP,
                             const PP_AttrProp * pSectionAP,
                             PD_Document * pDoc);
	virtual ~s_RTF_AttrPropAdapter_AP();

    virtual const gchar * getAttribute(const gchar * szName) const;
    virtual const gchar * getProperty(const gchar * szName) const;
};

#endif
