/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* AbiSource
 * 
 * Copyright (C) 2011 Volodymyr Rudyj <vladimir.rudoy@gmail.com>
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
#include "ie_exp_HTML_NavigationHelper.h"

IE_Exp_HTML_BookmarkListener::IE_Exp_HTML_BookmarkListener(PD_Document *pDoc,
IE_Exp_HTML_NavigationHelper *pNavigationHelper) :
m_pDoc(pDoc),
m_pNavigationHelper(pNavigationHelper)


{

}

bool IE_Exp_HTML_BookmarkListener::populate(fl_ContainerLayout* /*sfh*/, 
                                            const PX_ChangeRecord * pcr)
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
                    m_pNavigationHelper->getBookmarks()[escape] = 
                        m_pNavigationHelper->getFilenameByPosition(
                        pcr->getPosition());
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
    PD_Document* pDocument, const UT_UTF8String &baseName)
: IE_TOCHelper(pDocument),
  m_suffix(""),
  m_minTOCLevel(0),
  m_minTOCIndex(0),
  m_baseName(UT_go_basename_from_uri(baseName.utf8_str()))
{
    m_suffix = strchr(m_baseName, '.');
    m_minTOCLevel = 10;
    for (int i = 0; i < getNumTOCEntries(); i++)
    {
        int currentLevel = 10;
        getNthTOCEntry(i, &currentLevel);
        if (currentLevel < m_minTOCLevel)
        {
            m_minTOCLevel = currentLevel;
            m_minTOCIndex = i;
        }
    }
            
    IE_Exp_HTML_BookmarkListener * bookmarkListener =
        new IE_Exp_HTML_BookmarkListener(pDocument, this);
    pDocument->tellListener(bookmarkListener);
    DELETEP(bookmarkListener);
}

IE_Exp_HTML_NavigationHelper::~IE_Exp_HTML_NavigationHelper()
{
	g_free(m_baseName);
}

UT_UTF8String IE_Exp_HTML_NavigationHelper::getBookmarkFilename(
    const UT_UTF8String& id) const
{
    std::map<UT_UTF8String, UT_UTF8String>::const_iterator bookmarkIter = m_bookmarks.find(id);
	if (bookmarkIter != m_bookmarks.end())
	{
		UT_DEBUGMSG(("Found bookmark %s at file %s", id.utf8_str(), bookmarkIter->second.utf8_str()));
		return bookmarkIter->second;
	} else
	{
		return UT_UTF8String();
	}    
}

UT_UTF8String IE_Exp_HTML_NavigationHelper::getFilenameByPosition(
    PT_DocPosition position) const
{
    PT_DocPosition posCurrent;
	char * s = UT_go_basename_from_uri(m_baseName);
    UT_UTF8String chapterFile = s;
	g_free(s);

    if (hasTOC())
    {
		PT_DocPosition minTocPosition;
		getNthTOCEntryPos(0, minTocPosition);
		if (position >= minTocPosition){

			for (int i = getNumTOCEntries() - 1; i >= m_minTOCIndex; i--)
			{
				int currentLevel;
				getNthTOCEntry(i, &currentLevel);
				getNthTOCEntryPos(i, posCurrent);

				if (currentLevel == m_minTOCLevel)
				{
					if (posCurrent <= position)
					{
						chapterFile = ConvertToClean(getNthTOCEntry(i, NULL)) + m_suffix;
						break;
					}
				}
			}
		}
    }

    UT_DEBUGMSG(("Got filename by position: %s\n", chapterFile.utf8_str()));
    return chapterFile;
}
