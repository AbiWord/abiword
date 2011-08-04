#include "ie_exp_HTML_NavigationHelper.h"

IE_Exp_HTML_BookmarkListener::IE_Exp_HTML_BookmarkListener(PD_Document *pDoc) :
m_pDoc(pDoc)

{

}

bool IE_Exp_HTML_BookmarkListener::populate(PL_StruxFmtHandle /*sfh*/, const PX_ChangeRecord * pcr)
{
    switch (pcr->getType())
    {
    case PX_ChangeRecord::PXT_InsertObject:
    {
        const PX_ChangeRecord_Object * pcro = 0;
        pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
        PT_AttrPropIndex api = pcr->getIndexAP();

        switch (pcro->getObjectType())
        {
        case PTO_Bookmark:
        {
            const PP_AttrProp * pAP = 0;
            bool bHaveProp = (api ? (m_pDoc->getAttrProp(api, &pAP)) : false);

            if (!bHaveProp || (pAP == 0))
                return true;

            const gchar * szType = 0;
            pAP->getAttribute("type", szType);

            if (szType == 0)
                return true; // ??

            if (g_ascii_strcasecmp(szType, "start") == 0)
            {
                const gchar * szName = 0;
                pAP->getAttribute("name", szName);

                if (szName)
                {
                    UT_UTF8String escape = szName;
                    escape.escapeURL();
                    // m_bookmarks[escape] = m_pie->getFilenameByPosition(pcr->getPosition());
                    UT_DEBUGMSG(("Added bookmark\n File: %s Id: %s\n", m_bookmarks[escape].utf8_str(), escape.utf8_str()));
                }
            }
            return true;
        }

        default:
            return true;
        }
    }
    default:
        return true;
    }
}

IE_Exp_HTML_NavigationHelper::IE_Exp_HTML_NavigationHelper(
    PD_Document* pDocument, const UT_UTF8String &baseName):
        IE_TOCHelper(pDocument),
                m_pTocHelper(new IE_TOCHelper(pDocument)),
    m_baseName(UT_go_basename_from_uri(baseName.utf8_str()))
{
    IE_Exp_HTML_BookmarkListener * bookmarkListener = new IE_Exp_HTML_BookmarkListener(pDocument);
    pDocument->tellListener(bookmarkListener);
    m_bookmarks = bookmarkListener->getBookmarks();
    DELETEP(bookmarkListener);
}

UT_UTF8String IE_Exp_HTML_NavigationHelper::getBookmarkFilename(
    const UT_UTF8String& id)
{
    std::map<UT_UTF8String, UT_UTF8String>::iterator bookmarkIter = m_bookmarks.find(id);
	if (bookmarkIter != m_bookmarks.end())
	{
		UT_DEBUGMSG(("Found bookmark %s at file %s", id.utf8_str(), m_bookmarks[id].utf8_str()));
		return m_bookmarks[id];
	} else
	{
		return UT_UTF8String();
	}    
}

UT_UTF8String IE_Exp_HTML_NavigationHelper::getFilenameByPosition(
    PT_DocPosition position)
{
    PT_DocPosition posCurrent;
    UT_UTF8String chapterFile = UT_go_basename_from_uri(m_baseName.utf8_str());

    if (m_pTocHelper->hasTOC())
    {
        for (int i = m_pTocHelper->getNumTOCEntries() - 1; i >= m_minTOCIndex; i--)
        {
            int currentLevel;
            m_pTocHelper->getNthTOCEntry(i, &currentLevel);
            m_pTocHelper->getNthTOCEntryPos(i, posCurrent);

            if (currentLevel == m_minTOCLevel)
            {
                if ((i != m_minTOCIndex) && (posCurrent <= position))
                {
                    chapterFile = ConvertToClean(m_pTocHelper->getNthTOCEntry(i, NULL)) /*+ m_suffix*/;
                    break;
                }
                else if ((i == m_minTOCIndex) && (posCurrent >= position))
                {
                    break;
                }
            }
        }
    }

    return (chapterFile);
}
