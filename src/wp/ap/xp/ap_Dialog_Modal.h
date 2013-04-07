/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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

#ifndef AP_DIALOG_MODAL_H
#define AP_DIALOG_MODAL_H

#include <string>

#include "xap_Dialog.h"
#include "fv_View.h"
#include "pd_DocumentRDF.h"
#include "xap_Strings.h"
class XAP_Frame;


class ABI_EXPORT AP_Dialog_Modal : public XAP_Dialog_NonPersistent
{
  protected:

public:
	AP_Dialog_Modal(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~AP_Dialog_Modal();

    virtual void                maybeClosePopupPreviewBubbles();
    void                        closePopupPreviewBubbles();
    virtual void                maybeReallowPopupPreviewBubbles();

    FV_View*                    getView() const;

  protected:

    FV_View_BubbleBlocker       m_bubbleBlocker;
};

#endif
