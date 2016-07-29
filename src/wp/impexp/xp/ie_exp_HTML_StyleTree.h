/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#ifndef IE_EXP_HTML_STYLETREE_H
#define IE_EXP_HTML_STYLETREE_H

#include "ie_exp_HTML_util.h"
#include "ut_locale.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string_class.h"
#include "pt_Types.h"
#include "pl_Listener.h"
#include "pd_Document.h"
#include "pd_Style.h"
#include "pp_AttrProp.h"
#include "pp_Property.h"
#include "pp_PropertyMap.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "px_CR_Span.h"
#include "px_CR_Strux.h"

#define IS_TRANSPARENT_COLOR(c) (!strcmp(c, "transparent"))

#define BT_NORMAL		1
#define BT_HEADING1		2
#define BT_HEADING2		3
#define BT_HEADING3		4
#define BT_BLOCKTEXT	5
#define BT_PLAINTEXT	6
#define BT_NUMBEREDLIST	7
#define BT_BULLETLIST	8

class ABI_EXPORT IE_Exp_HTML_StyleTree{
private:
    PD_Document * m_pDocument; // root element of tree only (atm, anyway)

    IE_Exp_HTML_StyleTree * m_parent;
    IE_Exp_HTML_StyleTree ** m_list;

    UT_uint32 m_count;
    UT_uint32 m_max;

    bool m_bInUse;

    UT_UTF8String m_style_name;
    UT_UTF8String m_class_name;
    UT_UTF8String m_class_list;

    typedef std::map<std::string, std::string> map_type;
    map_type m_map;

    IE_Exp_HTML_StyleTree(IE_Exp_HTML_StyleTree * parent, const gchar * name, PD_Style * style);
public:
    IE_Exp_HTML_StyleTree(PD_Document * pDocument);
    ~IE_Exp_HTML_StyleTree();

private:
    bool add(const gchar * style_name, PD_Style * style);
public:
    bool add(const gchar * style_name, PD_Document * pDoc);

private:
    void inUse();
public:
    const IE_Exp_HTML_StyleTree * findAndUse(const gchar * style_name);

    const IE_Exp_HTML_StyleTree * find(const gchar * style_name) const;
    const IE_Exp_HTML_StyleTree * find(PD_Style * style) const;

    bool descends(const gchar * style_name) const;

    template<typename StyleListener>
    void print(StyleListener * listener) const;

    const IE_Exp_HTML_StyleTree * operator[] (UT_uint32 i) const {
        return (i < m_count) ? m_list[i] : 0;
    }

    UT_uint32 count() const {
        return m_count;
    }

    const UT_UTF8String & style_name() const {
        return m_style_name;
    }

    const UT_UTF8String & class_name() const {
        return m_class_name;
    }

    const UT_UTF8String & class_list() const {
        return m_class_list;
    }

    PD_Document* getDocument() const{
        return m_pDocument;
    }

    const std::string & lookup(const std::string & prop_name) const;

};

class ABI_EXPORT IE_Exp_HTML_StyleListener : public PL_Listener {
public:
    IE_Exp_HTML_StyleListener(IE_Exp_HTML_StyleTree *styleTree);

    bool populate(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr);

    bool populateStrux(pf_Frag_Strux* sdh,
            const PX_ChangeRecord * pcr,
            fl_ContainerLayout* * psfh);

    bool change(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr);

    bool insertStrux(fl_ContainerLayout* sfh,
            const PX_ChangeRecord * pcr,
            pf_Frag_Strux* sdh,
            PL_ListenerId lid,
            void (*pfnBindHandles) (pf_Frag_Strux* sdhNew,
            PL_ListenerId lid,
            fl_ContainerLayout* sfhNew));

    bool signal(UT_uint32 iSignal);
private:
    void styleCheck(PT_AttrPropIndex api);

    IE_Exp_HTML_StyleTree *m_pStyleTree;
};

template<typename StyleListener>
void IE_Exp_HTML_StyleTree::print(StyleListener * listener) const {
    if (!m_bInUse) return;

    if (strstr(m_style_name.utf8_str(), "List")) return;

    if (m_parent) {
        UT_UTF8String selector("*.");
        if (m_class_name.byteLength()) {
            UT_UTF8String tmp = m_class_name;
            tmp.escapeXML();
            selector += tmp.utf8_str();
        } else {
            if (m_style_name == "Normal")
                selector = "p, h1, h2, h3, li";
            else if (m_style_name == "Heading 1")
                selector = "h1";
            else if (m_style_name == "Heading 2")
                selector = "h2";
            else if (m_style_name == "Heading 3")
                selector = "h3";
        }
        listener->styleOpen(selector);

        for (map_type::const_iterator iter = m_map.begin();
                iter != m_map.end(); iter++) {
            listener->styleNameValue((*iter).first.c_str(),
                    (*iter).second.c_str());
        }
        listener->styleClose();
    }
    for (UT_uint32 i = 0; i < m_count; i++) {
        m_list[i]->print(listener);
    }
}

struct StyleListener {
    UT_ByteBuf& m_sink;
    UT_UTF8String m_utf8_0;
    UT_uint32 m_styleIndent;

    StyleListener(UT_ByteBuf & sink)
    : m_sink(sink), m_styleIndent(0) {
    }

    bool get_Compact() {
        return false;
    }

    void tagRaw(UT_UTF8String & content) {
        m_sink.append((const UT_Byte*) content.utf8_str(), content.byteLength());
    }

    void styleIndent() {
        m_utf8_0 = "";

        for (UT_uint32 i = 0; i < m_styleIndent; i++) m_utf8_0 += "\t";
    }

    void styleOpen(const UT_UTF8String & rule) {
        styleIndent();

        m_utf8_0 += rule;
        m_utf8_0 += " {";
        if (!get_Compact())
            m_utf8_0 += MYEOL;

        tagRaw(m_utf8_0);

        m_styleIndent++;
    }

    void styleClose() {
        if (m_styleIndent == 0) {
            UT_DEBUGMSG(("WARNING: CSS style group over-closing!\n"));
            return;
        }
        m_styleIndent--;

        styleIndent();

        m_utf8_0 += "}";
        if (!get_Compact())
            m_utf8_0 += MYEOL;

        tagRaw(m_utf8_0);
    }

    void styleNameValue(const gchar * name, const UT_UTF8String & value) {
        styleIndent();

        m_utf8_0 += name;
        m_utf8_0 += ":";
        m_utf8_0 += value;
        m_utf8_0 += ";";
        if (!get_Compact())
            m_utf8_0 += MYEOL;

        tagRaw(m_utf8_0);
    }

    void styleText(const UT_UTF8String & content) {
        m_utf8_0 = content;
        tagRaw(m_utf8_0);
    }
};

#endif
