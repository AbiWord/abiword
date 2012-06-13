/* AbiSource
 * 
 * Copyright (C) 2005 INdT
 * Author: Daniel d'Andrada T. de Carvalho <daniel.carvalho@indt.org.br>
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

#ifndef _ODE_STYLE_MASTERPAGE_H_
#define _ODE_STYLE_MASTERPAGE_H_

// AbiWord includes
#include <ut_string_class.h>

// External includes
#include <gsf/gsf-output.h>
#include <stdio.h>

// AbiWord classes
class PP_AttrProp;

/**
 * A <style:master-page> element.
 */
class ODe_Style_MasterPage {
public:

    ODe_Style_MasterPage (const gchar* pName, const gchar* pPageLayoutName);
    
    virtual ~ODe_Style_MasterPage();

    void fetchAttributesFromAbiSection(const PP_AttrProp* pAP);

    void setName(const UT_UTF8String& rName) {
        m_name = rName;
    }

    void setPageLayoutName(const UT_UTF8String& rPageLayoutName) {
        m_pageLayoutName = rPageLayoutName;
    }

    bool hasProperties() const {
        return !m_abiHeaderId.empty() ||
               !m_abiFooterId.empty();
    }

    // Write the <style:master-page> element.
    bool write(GsfOutput* pODT) const;
    
    GsfOutput* getHeaderContentTempFile() const {return m_pHeaderContentTemp;}
    GsfOutput* getHeaderEvenContentTempFile() const {return m_pHeaderEvenContentTemp;}
    GsfOutput* getFooterContentTempFile() const {return m_pFooterContentTemp;}
    GsfOutput* getFooterEvenContentTempFile() const {return m_pFooterEvenContentTemp;}
    
    const UT_UTF8String& getAbiHeaderId() const {return m_abiHeaderId;}
    const UT_UTF8String& getAbiHeaderEvenId() const {return m_abiHeaderEvenId;}
    const UT_UTF8String& getAbiFooterId() const {return m_abiFooterId;}
    const UT_UTF8String& getAbiFooterEvenId() const {return m_abiFooterEvenId;}
    
private:
    UT_UTF8String m_name;           // style:name
    UT_UTF8String m_pageLayoutName; // style:page-layout-name
    
    // <section header="2" ... >
    UT_UTF8String m_abiHeaderId;
    // <section header-even="3" ... >
    UT_UTF8String m_abiHeaderEvenId;
    
    // <section footer="5" ... >
    UT_UTF8String m_abiFooterId;
    // <section footer-even="6" ... >
    UT_UTF8String m_abiFooterEvenId;
    
    // Temporary files that will hold header and footer content.
    GsfOutput* m_pHeaderContentTemp;
    GsfOutput* m_pHeaderEvenContentTemp;
    GsfOutput* m_pFooterContentTemp;
    GsfOutput* m_pFooterEvenContentTemp;
};

#endif //_ODE_STYLE_MASTERPAGE_H_
