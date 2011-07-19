#ifndef IE_EXP_HTML_STYLETREE_H
#define	IE_EXP_HTML_STYLETREE_H


#include "ie_exp_HTML_util.h"

#include <ut_types.h>
#include <ut_locale.h>
#include <pd_Document.h>
#include <pd_Style.h>
#include <pl_Listener.h>
#include <pp_AttrProp.h>
#include <pp_Property.h>
#include <pp_PropertyMap.h>
#include <px_ChangeRecord.h>
#include <px_CR_Object.h>
#include <px_CR_Span.h>
#include <px_CR_Strux.h>
#include <pt_Types.h>

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

    PD_Style * m_style;

    typedef std::map<std::string, std::string> map_type;
    map_type m_map;

    IE_Exp_HTML_StyleTree(IE_Exp_HTML_StyleTree * parent, const char * name, PD_Style * style);
public:
    IE_Exp_HTML_StyleTree(PD_Document * pDocument);
    ~IE_Exp_HTML_StyleTree();
    
    PD_Document *getDocument() const { return m_pDocument; }

private:
    bool add(const char * style_name, PD_Style * style);
public:
    bool add(const char * style_name, PD_Document * pDoc);

private:
    void inUse();
public:
    const IE_Exp_HTML_StyleTree * findAndUse(const char * style_name);

    const IE_Exp_HTML_StyleTree * find(const char * style_name) const;
    const IE_Exp_HTML_StyleTree * find(PD_Style * style) const;

    bool descends(const char * style_name) const;

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

    const std::string & lookup(const std::string & prop_name) const;
    
    
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

class ABI_EXPORT IE_Exp_HTML_StyleListener : public PL_Listener {
public:
    
    IE_Exp_HTML_StyleListener(IE_Exp_HTML_StyleTree *styleTree);

    bool populate(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr);

    bool populateStrux(PL_StruxDocHandle sdh,
            const PX_ChangeRecord * pcr,
            PL_StruxFmtHandle * psfh);

    bool change(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr);

    bool insertStrux(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr,
            PL_StruxDocHandle sdh,
            PL_ListenerId lid,
            void (*pfnBindHandles) (PL_StruxDocHandle sdhNew,
            PL_ListenerId lid,
            PL_StruxFmtHandle sfhNew));

    bool signal(UT_uint32 iSignal);
    
private:
    void styleCheck(PT_AttrPropIndex api);
    IE_Exp_HTML_StyleTree *m_styleTree;

};

#endif	/* IE_EXP_HTML_STYLETREE_H */

