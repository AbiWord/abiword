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

#ifndef _ODE_STYLES_H_
#define _ODE_STYLES_H_

// AbiWord includes
#include <ut_hash.h>

// Internal includes
#include "ODe_DefaultStyles.h"

// External includes
#include <gsf/gsf-output.h>

// Internal classes
class ODe_Style_Style;
class ODe_Style_MasterPage;
class ODe_Style_PageLayout;

// AbiWord classes
class PD_Document;
class PP_AttrProp;
class PD_Style;

/**
 * This class stores all normal and automatic styles.
 */
class ODe_Styles {
public:

    ODe_Styles(PD_Document* pAbiDoc);

    ~ODe_Styles();

    // Fetch all regular <style:style> elements (the ones that will be defined
    // inside <office:styles>).
    bool fetchRegularStyleStyles();

    // Writes the <office:styles> element.
    bool write(GsfOutput* pODT) const;

    ODe_DefaultStyles& getDefaultStyles() {
        return m_defaultStyles;
    }
    
    UT_GenericVector<ODe_Style_Style*>* getParagraphStylesEnumeration() {
        return m_paragraphStyles.enumerate();
    }
    
    UT_GenericVector<ODe_Style_Style*>* getTextStylesEnumeration() {
        return m_textStyles.enumerate();
    }

    UT_GenericVector<ODe_Style_Style*>* getGraphicStylesEnumeration() {
        return m_graphicStyles.enumerate();
    }

	ODe_Style_Style* getGraphicsStyle(const gchar* name) {
        return m_graphicStyles.pick(name);
	}

	void addGraphicsStyle(ODe_Style_Style* pStyle);

    void addStyle(const UT_UTF8String& sStyle);

private:
    bool _addStyle(const PP_AttrProp* pAP);
    bool _writeStyles(GsfOutput* pODT, UT_GenericVector<ODe_Style_Style*>* pStyleVector) const;

	PD_Document* m_pAbiDoc;
    ODe_DefaultStyles m_defaultStyles;
    UT_GenericStringMap<ODe_Style_Style*> m_textStyles;
    UT_GenericStringMap<ODe_Style_Style*> m_paragraphStyles;
	UT_GenericStringMap<ODe_Style_Style*> m_graphicStyles;
};

#endif //_ODE_STYLES_H_
