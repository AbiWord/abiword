/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
#ifndef IE_EXP_HTML_NAVIGATIONHELPER_H
#define	IE_EXP_HTML_NAVIGATIONHELPER_H

#include "ie_exp_HTML_util.h"

// External includes
#include <vector>
#include <string>
#include <gsf/gsf-output.h>

// Abiword includes
#include <pd_Document.h>
#include <pl_Listener.h>
#include <px_ChangeRecord.h>
#include <px_CR_Span.h>
#include <px_CR_Strux.h>
#include <px_CR_Object.h>
#include <fd_Field.h>
#include <fl_TOCLayout.h>
#include <ie_Table.h>
#include <ie_TOC.h>
#include <ut_go_file.h>


class IE_Exp_HTML_NavigationHelper : public IE_TOCHelper {
public:
    IE_Exp_HTML_NavigationHelper(PD_Document *pDocument, 
            const UT_UTF8String &baseName);
    
    UT_UTF8String getBookmarkFilename(const UT_UTF8String &id);
    UT_UTF8String getFilenameByPosition(PT_DocPosition position);
    inline int getMinTOCLevel() const { return m_minTOCLevel; }
    inline int getMinTOCIndex() const { return m_minTOCIndex; }
    inline std::map<UT_UTF8String, UT_UTF8String> & getBookmarks() 
        { return m_bookmarks; }
private:
    UT_UTF8String m_suffix;
    int m_minTOCLevel;
    int m_minTOCIndex;
    std::map<UT_UTF8String, UT_UTF8String> m_bookmarks;
    UT_UTF8String m_baseName;
};

class ABI_EXPORT IE_Exp_HTML_BookmarkListener : public PL_Listener
{
public:
IE_Exp_HTML_BookmarkListener(PD_Document* pDoc,
        IE_Exp_HTML_NavigationHelper *pNavigationHelper);
bool populate(PL_StruxFmtHandle sfh,
              const PX_ChangeRecord * pcr);
// Not used

bool populateStrux(PL_StruxDocHandle /*sdh*/,
                   const PX_ChangeRecord * /*pcr*/,
                   PL_StruxFmtHandle * /*psfh*/)
{
    return true;
}
// Not used

bool change(PL_StruxFmtHandle /*sfh*/,
            const PX_ChangeRecord * /*pcr*/)
{
    return true;
}
// Not used

bool insertStrux(PL_StruxFmtHandle /*sfh*/,
                 const PX_ChangeRecord * /*pcr*/,
                 PL_StruxDocHandle /*sdh*/,
                 PL_ListenerId /*lid*/,
                 void (* /*pfnBindHandles*/) (PL_StruxDocHandle sdhNew,
                 PL_ListenerId lid,
                 PL_StruxFmtHandle sfhNew))
{
    return true;
}
// Not used

bool signal(UT_uint32 /*iSignal*/)
{
    return true;
}

private:
PD_Document * m_pDoc;
IE_Exp_HTML_NavigationHelper *m_pNavigationHelper;

};

#endif	/* IE_EXP_HTML_NAVIGATIONHELPER_H */

