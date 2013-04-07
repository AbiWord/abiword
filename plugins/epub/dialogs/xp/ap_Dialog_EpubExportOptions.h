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
#ifndef AP_DIALOG_EPUBEXPORTOPTIONS_H
#define	AP_DIALOG_EPUBEXPORTOPTIONS_H

#include "xap_App.h"
#include "xap_Dialog.h"
#include "xap_Prefs.h"

#define EPUB_EXPORT_SCHEME_NAME "EpubExporterOptions"

extern pt2Constructor ap_Dialog_EpubExportOptions_Constructor;
struct XAP_Exp_EpubExportOptions
{
    bool bSplitDocument;
    bool bRenderMathMLToPNG;
    bool bEpub2;
};

class AP_Dialog_EpubExportOptions : public XAP_Dialog_NonPersistent
{
public:
    AP_Dialog_EpubExportOptions(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

    virtual ~AP_Dialog_EpubExportOptions(void);

    virtual void runModal(XAP_Frame * pFrame) = 0;

    bool shouldSave() const;

    void setEpubExportOptions(XAP_Exp_EpubExportOptions * exp_opt, XAP_App * app);
    static void getEpubExportDefaults(XAP_Exp_EpubExportOptions * exp_opt, XAP_App * app);
protected:
    bool m_bShouldSave;

    inline bool get_Epub2 () const { return m_exp_opt->bEpub2; }
    inline bool get_SplitDocument () const { return m_exp_opt->bSplitDocument; }
    inline bool get_RenderMathMlToPng () const { return m_exp_opt->bRenderMathMLToPNG; }

    inline bool can_set_Epub2 () const { return true; }
    inline bool can_set_SplitDocument() const { return true; }
    inline bool can_set_RenderMathMlToPng() const { return !m_exp_opt->bEpub2; }


    void set_Epub2  (bool enable);
    void set_SplitDocument (bool enable);
    void set_RenderMathMlToPng (bool enable);

    void saveDefaults();
    void restoreDefaults();

private:
    XAP_Exp_EpubExportOptions * m_exp_opt;
    XAP_App * m_app;

};
#endif	/* AP_DIALOG_EPUBEXPORTOPTIONS_H */

