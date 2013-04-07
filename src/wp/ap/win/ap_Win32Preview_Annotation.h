/* AbiSource Application Framework
 * Copyright (C) 2009 J.M. Maurer <uwog@uwog.net>
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

#ifndef AP_WIN32PREVIEW_ANNOTATION_H
#define AP_WIN32PREVIEW_ANNOTATION_H

#include "xap_App.h"
#include "ap_Preview_Annotation.h"

class XAP_Frame;

class AP_Win32Preview_Annotation : public AP_Preview_Annotation
{
public:
	AP_Win32Preview_Annotation(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Preview_Annotation(void);

	virtual void			runModeless(XAP_Frame * pFrame);
	virtual void			destroy(void);
	virtual void			activate(void);
	virtual void			draw(const UT_Rect *clip);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

private:
	void					_createToolTip(HWND hwndParent);

	HWND					m_hToolTip;
};

#endif /* AP_WIN32PREVIEW_ANNOTATION_H */
