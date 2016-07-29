/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiWord
* Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
* Copyright (C) 2007, 2009 Hubert Figuiere
* Copyright (C) 2003-2005 Mark Gilbert <mg_abimail@yahoo.com>
* Copyright (C) 2002, 2004 Francis James Franklin <fjf@alinameridon.com>
* Copyright (C) 2001-2002 AbiSource, Inc.
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
#include "ie_exp_HTML_StyleTree.h"



IE_Exp_HTML_StyleTree::IE_Exp_HTML_StyleTree(IE_Exp_HTML_StyleTree * parent, const gchar * _style_name, PD_Style * style) :
        m_pDocument(0),
        m_parent(parent),
        m_list(0),
        m_count(0),
        m_max(0),
        m_bInUse(false),
        m_style_name(_style_name),
        m_class_name(_style_name),
        m_class_list(_style_name)
{
    UT_LocaleTransactor t(LC_NUMERIC, "C");

    if ((m_style_name == "Heading 1") ||
        (m_style_name == "Heading 2") ||
        (m_style_name == "Heading 3") ||
        (m_style_name == "Normal"))
    {
        m_class_name = "";
        m_class_list = "";
    }
    else
    {
        s_removeWhiteSpace(_style_name, m_class_name, true);
        m_class_list = m_class_name;
    }

    if (parent->class_list() != "")
    {
        m_class_list += " ";
        m_class_list += parent->class_list();
    }

    UT_uint32 j = 0;

    const gchar * szName = 0;
    const gchar * szValue = 0;

    UT_UTF8String name;
    UT_UTF8String value;

    while (style->getNthProperty(j++, szName, szValue))
    {
        name = szName;
        value = szValue;

        /* map property names to CSS equivalents
         */
        if (name == "text-position")
        {
            name = "vertical-align";
            if (value == "superscript")
                value = "super";
            else if (value == "subscript")
                value = "sub";

        }
        else if (name == "bgcolor")
        {
            name = "background-color";
        }
        else if (!is_CSS(szName)) continue;

        /* map property values to CSS equivalents
         */
        if (name == "font-family")
        {
            if (!((value == "serif") || (value == "sans-serif") ||
                (value == "cursive") || (value == "fantasy") || (value == "monospace")))
            {
                value = "'";
                value += szValue;
                value += "'";
            }
        }
        else if ((name == "color") || (name == "background-color"))
        {
            if (!value.empty() && (value != "transparent"))
            {
                value = UT_colorToHex(szValue, true);
            }
        }
        else if (strstr(name.utf8_str(), "width"))
        {
            if (strstr(name.utf8_str(), "border"))
            {
                double dPT = UT_convertToDimension(value.utf8_str(), DIM_PT);
                value = UT_UTF8String_sprintf("%.2fpt", dPT);
            }
            else
            {
                double dMM = UT_convertToDimension(value.utf8_str(), DIM_MM);
                value = UT_UTF8String_sprintf("%.1fmm", dMM);
            }
        }

        const std::string & cascade_value = lookup(name.utf8_str());
        if (!cascade_value.empty())
            if (cascade_value == value)
                continue;

        m_map.insert(map_type::value_type(name.utf8_str(),
                                          value.utf8_str()));
    }
    if ((m_style_name == "Heading 1") ||
        (m_style_name == "Heading 2") ||
        (m_style_name == "Heading 3") ||
        (m_style_name == "Section Heading") ||
        (m_style_name == "Chapter Heading"))
    {
        m_map.insert(map_type::value_type("page-break-after", "avoid"));
    }
}

IE_Exp_HTML_StyleTree::IE_Exp_HTML_StyleTree(PD_Document * pDocument) :
        m_pDocument(pDocument),
        m_parent(0),
        m_list(0),
        m_count(0),
        m_max(0),
        m_bInUse(false),
        m_style_name("None"),
        m_class_name(""),
        m_class_list("")
{
    const gchar ** ptr = s_prop_list;
    while (*ptr)
    {
        m_map.insert(map_type::value_type(*ptr, *(ptr + 1)));
        ptr += 2;
    }
}

IE_Exp_HTML_StyleTree::~IE_Exp_HTML_StyleTree()
{
    for (UT_uint32 i = 0; i < m_count; i++)
    {
        DELETEP(m_list[i]);
    }
    FREEP(m_list);
}

bool IE_Exp_HTML_StyleTree::add(const gchar * _style_name, PD_Style * style)
{
    if (m_list == 0)
    {
        m_list = reinterpret_cast<IE_Exp_HTML_StyleTree **> (g_try_malloc(8 * sizeof (IE_Exp_HTML_StyleTree *)));
        if (m_list == 0) return false;
        m_max = 8;
    }
    if (m_count == m_max)
    {
        IE_Exp_HTML_StyleTree ** more = 0;
        more = reinterpret_cast<IE_Exp_HTML_StyleTree **> (g_try_realloc(m_list, (m_max + 8) * sizeof (IE_Exp_HTML_StyleTree *)));
        if (more == 0) return false;
        m_list = more;
        m_max += 8;
    }

    IE_Exp_HTML_StyleTree * tree = 0;

    try
    {
        tree = new IE_Exp_HTML_StyleTree(this, _style_name, style);
    }
    catch (...)
    {
        tree = 0;
    }

    if (tree == 0) return false;

    m_list[m_count++] = tree;

    return true;
}

bool IE_Exp_HTML_StyleTree::add(const gchar * _style_name, PD_Document * pDoc)
{
    if ((pDoc == 0) || (_style_name == 0) || (*_style_name == 0))
        return false;

    if (m_parent)
        return m_parent->add(_style_name, pDoc);

    if (find(_style_name))
        return true;

    PD_Style * style = 0;
    pDoc->getStyle(_style_name, &style);
    if (!style)
        return false;

    IE_Exp_HTML_StyleTree * parent = NULL;

    PD_Style * basis = style->getBasedOn();

    const gchar * parent_name = NULL;
    if (basis &&
        basis->getAttribute(PT_NAME_ATTRIBUTE_NAME, parent_name) &&
        strcmp(_style_name, parent_name) != 0)
    {
        parent = const_cast<IE_Exp_HTML_StyleTree*> (find(basis));
        if (parent == NULL)
        {
            const gchar * basis_name = 0;
            basis->getAttribute(PT_NAME_ATTRIBUTE_NAME, basis_name);
            if (!basis_name)
                return false;

            if (basis->getBasedOn() && basis->getBasedOn()->getName() &&
                !strcmp(_style_name, basis->getBasedOn()->getName()))
            {
                parent = this;
            }
            else
            {
                if (!add(basis_name, pDoc))
                    return false;

                parent = const_cast<IE_Exp_HTML_StyleTree*> (find(basis));
            }
        }
    }
    else
        parent = this;

    if (!parent)
    {
        UT_ASSERT_NOT_REACHED();
        return false;
    }
    return parent->add(_style_name, style);
}

void IE_Exp_HTML_StyleTree::inUse()
{
    m_bInUse = true;
    if (m_parent)
        m_parent->inUse();
}

const IE_Exp_HTML_StyleTree * IE_Exp_HTML_StyleTree::findAndUse(const gchar * _style_name)
{
    const IE_Exp_HTML_StyleTree * style_tree = find(_style_name);

    if (style_tree)
    {
        IE_Exp_HTML_StyleTree * tree = const_cast<IE_Exp_HTML_StyleTree *> (style_tree);
        tree->inUse();
    }
    return style_tree;
}

const IE_Exp_HTML_StyleTree * IE_Exp_HTML_StyleTree::find(const gchar * _style_name) const
{
    if (m_style_name == _style_name)
        return this;

    const IE_Exp_HTML_StyleTree * tree = 0;

    for (UT_uint32 i = 0; i < m_count; i++)
    {
        tree = m_list[i]->find(_style_name);
        if (tree)
            break;
    }
    return tree;
}

const IE_Exp_HTML_StyleTree * IE_Exp_HTML_StyleTree::find(PD_Style * style) const
{
    const gchar * _style_name = 0;
    style->getAttribute(PT_NAME_ATTRIBUTE_NAME, _style_name);
    if (!_style_name)
        return NULL;

    return find(_style_name);
}

bool IE_Exp_HTML_StyleTree::descends(const gchar * _style_name) const
{
    if (m_parent == 0)
        return false;

    // the name comparison has to be be case insensitive
    if (!g_ascii_strcasecmp(m_style_name.utf8_str(), _style_name))
        return true;

    return m_parent->descends(_style_name);
}



const std::string & IE_Exp_HTML_StyleTree::lookup(const std::string & prop_name) const
{
    static std::string empty;
    map_type::const_iterator prop_iter = m_map.find(prop_name);

    if (prop_iter == m_map.end())
    {
        if (m_parent)
        {
            return m_parent->lookup(prop_name);
        }
        else
        {
            return empty;
        }
    }
    return (*prop_iter).second;
}

IE_Exp_HTML_StyleListener::IE_Exp_HTML_StyleListener(IE_Exp_HTML_StyleTree* styleTree):
        m_pStyleTree(styleTree)
{
    
}

void IE_Exp_HTML_StyleListener::styleCheck(PT_AttrPropIndex api)
{
    const PP_AttrProp * pAP = 0;
    bool bHaveProp = (api ? (m_pStyleTree->getDocument()->getAttrProp(api, &pAP)) : false);

    if (bHaveProp && pAP)
    {
        const gchar * szStyle = NULL;
        bool have_style = pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME, szStyle);

        if (have_style && szStyle)
        {
            m_pStyleTree->findAndUse(szStyle);
        }
    }
}

bool IE_Exp_HTML_StyleListener::populate(fl_ContainerLayout* /*sfh*/, const PX_ChangeRecord * pcr)
{
    switch (pcr->getType())
    {
    case PX_ChangeRecord::PXT_InsertSpan:
        styleCheck(pcr->getIndexAP());
        break;
    case PX_ChangeRecord::PXT_InsertObject:
        styleCheck(pcr->getIndexAP());
        break;
    default:
        break;
    }
    return true;
}

bool IE_Exp_HTML_StyleListener::populateStrux(pf_Frag_Strux* /*sdh*/,
                                const PX_ChangeRecord * pcr,
                                fl_ContainerLayout* * psfh)
{
    UT_return_val_if_fail(pcr->getType() == PX_ChangeRecord::PXT_InsertStrux, false);

    *psfh = 0; // we don't need it.

    const PX_ChangeRecord_Strux * pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

    switch (pcrx->getStruxType())
    {
    case PTX_Block:
        styleCheck(pcr->getIndexAP());
        break;
    case PTX_SectionFootnote:
        styleCheck(pcr->getIndexAP());
        break;
    case PTX_SectionEndnote:
        styleCheck(pcr->getIndexAP());
        break;
    default:
        break;
    }
    return true;
}

bool IE_Exp_HTML_StyleListener::change(fl_ContainerLayout* /*sfh*/,
                         const PX_ChangeRecord * /*pcr*/)
{
    UT_ASSERT_HARMLESS(0); // this function is not used.
    return false;
}

bool IE_Exp_HTML_StyleListener::insertStrux(fl_ContainerLayout* /*sfh*/,
                              const PX_ChangeRecord * /*pcr*/,
                              pf_Frag_Strux* /*sdh*/,
                              PL_ListenerId /* lid */,
                              void (* /*pfnBindHandles*/)(pf_Frag_Strux* /* sdhNew */,
                              PL_ListenerId /* lid */,
                              fl_ContainerLayout* /* sfhNew */))
{
    UT_ASSERT_HARMLESS(0); // this function is not used.
    return false;
}

bool IE_Exp_HTML_StyleListener::signal(UT_uint32 /* iSignal */)
{
    UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    return false;
}
