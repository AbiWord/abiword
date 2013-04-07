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
#ifndef AP_UNIXDIALOG_EPUBEXPORTOPTIONS_H
#define	AP_UNIXDIALOG_EPUBEXPORTOPTIONS_H

#include "ap_Dialog_EpubExportOptions.h"
#include "xap_Frame.h"
#include "xap_UnixApp.h"
#include "xap_UnixDialogHelper.h"

class ABI_EXPORT AP_UnixDialog_EpubExportOptions: public AP_Dialog_EpubExportOptions {
public:

    AP_UnixDialog_EpubExportOptions(XAP_DialogFactory * pDlgFactory,
            XAP_Dialog_Id id);
    virtual ~AP_UnixDialog_EpubExportOptions(void);
    virtual void runModal(XAP_Frame * pFrame);
    static XAP_Dialog * static_constructor(XAP_DialogFactory *pDF,
            XAP_Dialog_Id id);

    void toggle_Epub2();
    void toggle_SplitDocument();
    void toggle_RenderMathMlToPng();
    void refreshStates();
private:
    void event_OK(void);
    void event_SaveSettings(void);
    void event_RestoreSettings(void);
    void event_Cancel(void);

    GtkWidget * _constructWindow(void);

    GtkWidget * m_windowMain;

    GtkWidget * m_wEpub2;
    GtkWidget * m_wSplitDocument;
    GtkWidget * m_wRenderMathMlToPng;

};

#endif	/* AP_UNIXDIALOG_EPUBEXPORTOPTIONS_H */

