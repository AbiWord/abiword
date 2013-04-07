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
#include "ap_Dialog_EpubExportOptions.h"
    
AP_Dialog_EpubExportOptions::AP_Dialog_EpubExportOptions(
    XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
    : XAP_Dialog_NonPersistent(pDlgFactory,id),
	  m_bShouldSave(true),
	  m_exp_opt(NULL),
	  m_app(NULL)
{
    
}

AP_Dialog_EpubExportOptions::~AP_Dialog_EpubExportOptions()
{
    
}

void AP_Dialog_EpubExportOptions::setEpubExportOptions(
    XAP_Exp_EpubExportOptions* exp_opt, XAP_App* app)
{
    UT_ASSERT(exp_opt);

    m_exp_opt = exp_opt;
    m_app = app;
}

void AP_Dialog_EpubExportOptions::getEpubExportDefaults(
    XAP_Exp_EpubExportOptions* exp_opt, XAP_App* app)
{
    UT_ASSERT(exp_opt);

    if (exp_opt == NULL) return;

    exp_opt->bEpub2 = true;
    exp_opt->bRenderMathMLToPNG = true;
    exp_opt->bSplitDocument = true;
    if (app == NULL) return;

    const XAP_Prefs * pPrefs = app->getPrefs();

    if (pPrefs == NULL) return;

    const gchar * szValue = NULL;
    bool haveValue = pPrefs->getPrefsValue(EPUB_EXPORT_SCHEME_NAME, &szValue);

    if (haveValue && szValue) {
        const char * pref = (const char *) szValue;

        exp_opt->bEpub2 = strstr(pref, "EPUB2") != NULL;
        exp_opt->bSplitDocument = strstr(pref, "split-document") != NULL;
        exp_opt->bRenderMathMLToPNG = strstr(pref, "mathml-to-png") != NULL;
      
    }
}

void AP_Dialog_EpubExportOptions::saveDefaults()
{
    UT_ASSERT(m_app);
    if (m_app == NULL) return;

    XAP_Prefs * pPrefs = m_app->getPrefs();
    if (pPrefs == NULL) return;

    XAP_PrefsScheme * pPScheme = pPrefs->getCurrentScheme();
    if (pPScheme == NULL) return;

    UT_UTF8String pref;

    if (m_exp_opt->bEpub2) {
        if (pref.byteLength()) pref += ",";
        pref += "EPUB2";
    }
    if (m_exp_opt->bSplitDocument) {
        if (pref.byteLength()) pref += ",";
        pref += "split-document";
    }
    if (m_exp_opt->bRenderMathMLToPNG) {
        if (pref.byteLength()) pref += ",";
        pref += "mathml-to-png";
    }
   
    const gchar * szValue = (const gchar *) pref.utf8_str();

    pPScheme->setValue(EPUB_EXPORT_SCHEME_NAME, szValue);
}

void AP_Dialog_EpubExportOptions::restoreDefaults()
{
    if (m_exp_opt == NULL) return;
    AP_Dialog_EpubExportOptions::getEpubExportDefaults(m_exp_opt, m_app);
}

void AP_Dialog_EpubExportOptions::set_Epub2(bool enable)
{
    m_exp_opt->bEpub2 = enable;
    // For EPUB2 we should always use rendering mathml to png
    // becuase EPUB 2.0.1 doesn`t declare MathML as core format. While
    // In EPUB3 we can use MathML.
    m_exp_opt->bRenderMathMLToPNG = enable;
}

void AP_Dialog_EpubExportOptions::set_SplitDocument(bool enable)
{
    m_exp_opt->bSplitDocument = enable;
}

void AP_Dialog_EpubExportOptions::set_RenderMathMlToPng(bool enable)
{
    m_exp_opt->bRenderMathMLToPNG = enable;
}

bool AP_Dialog_EpubExportOptions::shouldSave() const
{
	return m_bShouldSave;
}