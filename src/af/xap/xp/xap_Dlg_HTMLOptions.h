/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2002 AbiSource, Inc.
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

#ifndef XAP_DIALOG_HTMLOPTIONS_H
#define XAP_DIALOG_HTMLOPTIONS_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "xap_Dialog.h"

class XAP_App;

struct XAP_Exp_HTMLOptions
{
	bool	bIs4;
	bool	bIsAbiWebDoc;
	bool	bDeclareXML;
	bool	bAllowAWML;
	bool	bEmbedCSS;
	bool	bEmbedImages;

	/* other options, not set/saved/restore by options dialog
	 */
	bool	bMultipart;
};

class XAP_Dialog_HTMLOptions : public XAP_Dialog_NonPersistent
{
public:
	XAP_Dialog_HTMLOptions (XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	virtual ~XAP_Dialog_HTMLOptions (void);

	virtual void	runModal (XAP_Frame * pFrame) = 0;

	bool			shouldSave () const { return m_bShouldSave; }

	void			setHTMLOptions (XAP_Exp_HTMLOptions * exp_opt, XAP_App * app);
	static void		getHTMLDefaults (XAP_Exp_HTMLOptions * exp_opt, XAP_App * app);
	
protected:
	inline bool		get_HTML4 ()        const { return m_exp_opt->bIs4; }
	inline bool		get_PHTML ()        const { return m_exp_opt->bIsAbiWebDoc; }
	inline bool		get_Declare_XML ()  const { return m_exp_opt->bDeclareXML; }
	inline bool		get_Allow_AWML ()   const { return m_exp_opt->bAllowAWML; }
	inline bool		get_Embed_CSS ()    const { return m_exp_opt->bEmbedCSS; }
	inline bool		get_Embed_Images () const { return m_exp_opt->bEmbedImages; }
	inline bool		get_Multipart ()    const { return m_exp_opt->bMultipart; }

	inline bool		can_set_Declare_XML ()  const { return m_exp_opt->bIs4 ? false : true; }
	inline bool		can_set_Allow_AWML ()   const { return m_exp_opt->bIs4 ? false : true; }
	inline bool		can_set_Embed_CSS ()    const { return m_exp_opt->bIsAbiWebDoc ? false : true; }
	inline bool		can_set_Embed_Images () const { return m_exp_opt->bMultipart ? false : true; }

	void			set_HTML4 (bool enable);
	void			set_PHTML (bool enable);
	void			set_Declare_XML (bool enable);
	void			set_Allow_AWML (bool enable);
	void			set_Embed_CSS (bool enable);
	void			set_Embed_Images (bool enable);

	void			saveDefaults ();
	void			restoreDefaults ();

	bool					m_bShouldSave;

private:
	XAP_Exp_HTMLOptions *	m_exp_opt;
	XAP_App *				m_app;
};

#endif /* XAP_DIALOG_HTMLOPTIONS_H */
