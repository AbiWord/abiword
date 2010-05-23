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
 
// Class definition include
#include "ODe_Styles.h"

// Internal includes
#include "ODe_Common.h"
#include "ODe_Style_Style.h"

// AbiWord includes
#include <pd_Document.h>
#include <ut_vector.h>
#include <pd_Style.h>
#include <pp_AttrProp.h>

/**
 * Constructor
 */
ODe_Styles::ODe_Styles(PD_Document* pAbiDoc)
    : m_pAbiDoc(pAbiDoc)
{
}

/**
 * Destructor
 */
ODe_Styles::~ODe_Styles() {
    UT_GenericVector<ODe_Style_Style*>* pStyleVector;
    UT_uint32 i, count;
    
    pStyleVector = m_textStyles.enumerate();
    count = pStyleVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pStyleVector)[i];
    }
    
    pStyleVector = m_paragraphStyles.enumerate();
    count = pStyleVector->getItemCount();
    for (i=0; i<count; i++) {
        delete (*pStyleVector)[i];
    }
}


/**
 * Fetch all paragraph and text (character) styles defined in the Abi document.
 */
bool ODe_Styles::fetchRegularStyleStyles() {
    
    const PD_Style* pStyle = NULL;
    UT_GenericVector<PD_Style*> vecStyles;
    m_pAbiDoc->getAllUsedStyles(&vecStyles);
    const PP_AttrProp* pAP;
    PT_AttrPropIndex api;
    
    for (UT_sint32 k=0; k < vecStyles.getItemCount(); k++)
    {
        pStyle = vecStyles.getNthItem(k);

        api = pStyle->getIndexAP();
        if( !m_pAbiDoc->getAttrProp(api, &pAP) ) {
            return false;
        }
        
        if(!_addStyle(pAP)) {
            return false;
        }
    }




    UT_GenericVector<PD_Style*>* pStyles = NULL;
    m_pAbiDoc->enumStyles(pStyles);
    if (pStyles == NULL) {
        return false;
    }
    UT_uint32 iStyleCount = m_pAbiDoc->getStyleCount();
    bool ok = true;
    
    for (UT_uint32 k=0; k < iStyleCount && ok; k++)
    {
        pStyle = pStyles->getNthItem(k);
        if (pStyle == NULL) {
            return false;
        }
        
        if (!pStyle->isUserDefined() ||
            (vecStyles.findItem(const_cast<PD_Style*>(pStyle))) >= 0)
            continue;

        api = pStyle->getIndexAP();
        if( !m_pAbiDoc->getAttrProp(api, &pAP) ) {
            return false;
        }

        if(!_addStyle(pAP)) {
            ok = false;
        }
    }

    delete pStyles;
    
    return ok;
}

void ODe_Styles::addGraphicsStyle(ODe_Style_Style* pStyle)
{
    m_graphicStyles.insert(pStyle->getName().utf8_str(), pStyle);
}

/**
 * Add an OpenDocument style (paragraph or text family
 * given an AbiWord style.
 */
void ODe_Styles::addStyle(const UT_UTF8String& sStyle)
{
    UT_return_if_fail(sStyle != "");

    PD_Style* pStyle = NULL;
    m_pAbiDoc->getStyle(sStyle.utf8_str(), &pStyle);
    UT_return_if_fail(pStyle);

    PT_AttrPropIndex api = pStyle->getIndexAP();
    const PP_AttrProp* pAP = NULL;    
    if (!m_pAbiDoc->getAttrProp(api, &pAP))
        return;

    _addStyle(pAP);
}

/**
 * Adds an OpenDocumnet style (paragraph or text family)
 * given its attributes and properties from AbiWord.
 */
bool ODe_Styles::_addStyle(const PP_AttrProp* pAP) {
    const gchar* pName;
    const gchar* pType;
    ODe_Style_Style* pStyle;
    bool ok;

    UT_return_val_if_fail(pAP,false);
    
    ok = pAP->getAttribute(PT_NAME_ATTRIBUTE_NAME, pName);
    if (!ok) return false;
    
    ok = pAP->getAttribute(PT_TYPE_ATTRIBUTE_NAME, pType);
    if (!ok) {return false;}
    
    
    if ( !strcmp(pType, "P") ) {
        
        pStyle = new ODe_Style_Style();
        pStyle->setFamily("paragraph");
        m_paragraphStyles.insert(pName, pStyle);
        
    } else if( !strcmp(pType, "C") ) {
        
        pStyle = new ODe_Style_Style();
        pStyle->setFamily("text");
        m_textStyles.insert(pName, pStyle);
        
    } else {
        return false;
    }
    
    return pStyle->fetchAttributesFromAbiStyle(pAP);
}


/**
 * Writes the <office:styles> element.
 */
bool ODe_Styles::write(GsfOutput* pODT) const {
    UT_UTF8String output;
    
    output += " <office:styles>\n";
    ODe_writeUTF8String(pODT, output);
    output.clear();

    UT_return_val_if_fail(_writeStyles(pODT, m_defaultStyles.enumerate()), false);
	UT_return_val_if_fail(_writeStyles(pODT, m_textStyles.enumerate()), false);
	UT_return_val_if_fail(_writeStyles(pODT, m_paragraphStyles.enumerate()), false);
	UT_return_val_if_fail(_writeStyles(pODT, m_graphicStyles.enumerate()), false);
    
    output += " </office:styles>\n";
    ODe_writeUTF8String(pODT, output);
    output.clear();
    
    return true;
}

bool ODe_Styles::_writeStyles(GsfOutput* pODT, UT_GenericVector<ODe_Style_Style*>* pStyleVector) const
{
    for (UT_sint32 i = 0; i < pStyleVector->getItemCount(); i++) {
        ODe_Style_Style* pStyle = (*pStyleVector)[i];
        if (!pStyle->write(pODT, "  "))
            return false;
    }
	return true;
}
