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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef IE_EXP_HTML_MAINLISTENER_H
#define IE_EXP_HTML_MAINLISTENER_H

// HTML exporter includes
#include "ie_exp_HTML.h"
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_StyleTree.h"
#include "ie_exp_HTML_Writer.h"
#include "xap_Dlg_HTMLOptions.h"

#include <ut_string_class.h>
#include <ut_png.h>
#include <xap_App.h>
#include <xap_EncodingManager.h>
#include <xap_Dialog_Id.h>
#include <xap_DialogFactory.h>
#include <xap_Frame.h>
#include <xav_View.h>
#include <pt_Types.h>
#include <pl_Listener.h>
#include <pd_Document.h>
#include <pd_Style.h>
#include <pp_AttrProp.h>
#include <pp_Property.h>
#include <pp_PropertyMap.h>
#include <px_ChangeRecord.h>
#include <px_CR_Object.h>
#include <px_CR_Span.h>
#include <px_CR_Strux.h>



class IE_Exp_HTML_Writer;

class ABI_EXPORT IE_Exp_HTML_MainListener : public PL_Listener {
public:
    IE_Exp_HTML_MainListener(IE_Exp_HTML_Writer *pWriter);

    ~IE_Exp_HTML_MainListener();

    bool populate(PL_StruxFmtHandle sfh,
            const PX_ChangeRecord * pcr);

    bool populateStrux(PL_StruxDocHandle sdh,
            const PX_ChangeRecord * pcr,
            PL_StruxFmtHandle * psfh);

    bool endOfDocument();

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
    PT_DocPosition m_iEmbedStartPos;
    bool m_bIgnoreTillEnd;
    bool m_bIgnoreTillNextSection;
    IE_Exp_HTML_Writer* m_pWriter;
};

#endif