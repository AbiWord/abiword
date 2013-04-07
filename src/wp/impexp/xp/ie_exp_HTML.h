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

#ifndef IE_EXP_HTML_H
#define IE_EXP_HTML_H

// External includes
#include <gsf/gsf-output.h>

class IE_Exp_HTML;

// HTML exporter includes
#include "ie_impexp_HTML.h"
#include "ie_exp_HTML_util.h"
#include "ie_exp_HTML_UtilListeners.h"
#include "ie_exp_HTML_NavigationHelper.h"
#include "ie_exp_HTML_StyleTree.h"
#include "ie_exp_HTML_DocumentWriter.h"
#include "ie_exp_HTML_Listener.h"
#include "xap_Dlg_HTMLOptions.h"

// Abiword includes
#include <ut_debugmsg.h>
#include <ut_assert.h>
#include <ut_string_class.h>
#include <ut_go_file.h>
#include <xap_App.h>
#include <ie_types.h>
#include <ie_TOC.h>
#include <ap_Strings.h>
#include <xap_Dialog_Id.h>
#include <xap_DialogFactory.h>
#include <xap_Frame.h>
#include <xav_View.h>
#include <ie_exp.h>
#include <pt_Types.h>
#include <ut_path.h>

/* Define if the base unicode char is UCS-4
 */
#define HTML_UCS4


// The exporter/writer for HTML
class ABI_EXPORT IE_Exp_HTML_Sniffer : public IE_ExpSniffer
{
	friend class IE_Exp;

public:
	IE_Exp_HTML_Sniffer ();
	virtual ~IE_Exp_HTML_Sniffer () {}

	virtual bool recognizeSuffix (const gchar * szSuffix);
	virtual bool getDlgLabels (const gchar ** szDesc,
							   const gchar ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructExporter (PD_Document * pDocument,
										IE_Exp ** ppie);

	virtual UT_Confidence_t supportsMIME (const gchar * szMimeType);
};

class IE_Exp_HTML_NavigationHelper;
class IE_Exp_HTML_WriterFactory;

class ABI_EXPORT IE_Exp_HTML : public IE_Exp
{
public:
	IE_Exp_HTML (PD_Document * pDocument);
	virtual ~IE_Exp_HTML ();

	virtual UT_Error copyToBuffer(PD_DocumentRange * pDocRange,UT_ByteBuf *  bufHTML);

	static bool			RecognizeSuffix (const gchar * szSuffix);
	static UT_Error		StaticConstructor (PD_Document * pDocument,
										   IE_Exp ** ppie);
	static bool			GetDlgLabels (const gchar ** pszDesc,
									  const gchar ** pszSuffixList,
									  IEFileType * ft);
	static bool 		SupportsFileType (IEFileType ft);

	inline void			suppressDialog (bool disable = true) { m_bSuppressDialog = disable; }
	inline void			set_HTML4 (bool enable = true) { m_exp_opt.bIs4 = enable; }
	inline void			set_PHTML (bool enable = true) { m_exp_opt.bIsAbiWebDoc = enable; }
	inline void			set_MHTML (bool enable = true) { m_exp_opt.bMultipart = enable; }
	inline void			set_AddIdentifiers(bool enable = true) { m_exp_opt.bAddIdentifiers = enable; }
	inline void			set_MathMLRenderPNG ( bool enable = true) { m_exp_opt.bMathMLRenderPNG = enable; }
	inline void			set_SplitDocument ( bool enable = true) { m_exp_opt.bSplitDocument = enable; }

	inline const UT_UTF8String & getSuffix() const { return m_suffix; }
	inline IE_Exp_HTML_NavigationHelper *getNavigationHelper() { return m_pNavigationHelper; }
	void setWriterFactory(IE_Exp_HTML_WriterFactory *pWriterFactory);

private:
	UT_Error            _doOptions ();
	void _buildStyleTree();

protected:
	virtual UT_Error	_writeDocument ();
	void				_createChapter(PD_DocumentRange *range, const UT_UTF8String &title, bool isIndex);
	void _createMultipart();
public:
	virtual UT_Error	_writeDocument (bool bClipBoard, bool bTemplateBody);
	bool hasMathML(const UT_UTF8String &file);
	bool hasMathML(const std::string &file);
	static void printStyleTree(PD_Document *pDocument, UT_ByteBuf& sink);
private:
    // Returns document writer depending on settings
	IE_Exp_HTML_StyleTree *		m_style_tree;
	IE_Exp_HTML_StyleListener *m_styleListener;
	bool			m_bSuppressDialog;
	bool m_bDefaultWriterFactory;
	XAP_Exp_HTMLOptions	m_exp_opt;
	UT_UTF8String       m_sLinkCSS;
	UT_UTF8String       m_sTitle;
	IE_Exp_HTML_WriterFactory *m_pWriterFactory;
	// We need to know file suffix to create chapters with the same suffix as the main file
	UT_UTF8String m_suffix;
	std::map<UT_UTF8String, bool> m_mathmlFlags;
	IE_Exp_HTML_NavigationHelper *m_pNavigationHelper;
};

#endif /* IE_EXP_HTML_H */
