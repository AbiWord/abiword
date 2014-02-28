/* AbiSource Program Utilities
 *
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
 * Copyright (C) 2009 Hubert Figuiere
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

#ifndef _ODI_STYLE_LIST_H_
#define _ODI_STYLE_LIST_H_

#include <string>
#include <vector>

// Internal includes
#include "ODi_ListenerState.h"

// Internal classes
class ODi_ListLevelStyle;

// AbiWord classes
class PD_Document;


/**
 * Represents a <text:list-style> element.
 */
class ODi_Style_List
    : public ODi_ListenerState {

public:

    ODi_Style_List(ODi_ElementStack& rElementStack) :
	    ODi_ListenerState("StyleList", rElementStack),
        m_bListStyle(false)
    {
    }

    virtual ~ODi_Style_List();

    void startElement (const gchar* pName, const gchar** ppAtts,
                               ODi_ListenerStateAction& rAction);

    void endElement (const gchar* pName,
                             ODi_ListenerStateAction& rAction);

    void charData (const gchar* /*pBuffer*/, int /*length*/) { };

    void defineAbiList(PD_Document* pDocument);
    void redefine(PD_Document* pDocument, UT_uint32 iLevel);

    ODi_ListLevelStyle* getLevelStyle(UT_uint32 level) const
        {
            UT_return_val_if_fail(level > 0, NULL);
            UT_return_val_if_fail(!m_levelStyles.empty(), NULL);
			UT_return_val_if_fail(level <= m_levelStyles.size(), NULL);

            // Levels starts from 1, but our vector starts from 0 (zero).
            return m_levelStyles[level-1];
        }

    UT_sint32 getLevelCount() const
        {
            return m_levelStyles.size();
        }

    void buildAbiPropertiesString() const;

private:
    // style:name attribute
    std::string m_name;

    // style:display-name attribute
    std::string m_displayName;

    // text:consecutive-numbering attribute
    bool m_bConsecutiveNumbering;

    std::vector<ODi_ListLevelStyle*> m_levelStyles;
    bool m_bListStyle;
};

#endif //_ODI_STYLE_LIST_H_
