/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2001 Dom Lachowicz
 * Copyright (C) 2001, 2003, 2009 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_IMAGE_H
#define XAP_COCOADIALOG_IMAGE_H

#include "xap_Dlg_Image.h"

class XAP_CocoaFrame;
@class XAP_CocoaDialog_ImageController;

/*****************************************************************/

class XAP_CocoaDialog_Image: public XAP_Dialog_Image
{
public:
	XAP_CocoaDialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);

	virtual ~XAP_CocoaDialog_Image(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	void	event_Ok ();
	void	event_Cancel ();
	void	aspectCheckbox();
	void	doWidthSpin(bool bIncrement);
	void	doHeightSpin(bool bIncrement);
	void	doHeightEntry(void);
	void	setHeightEntry(void);
	void	setWidthEntry(void);
	void	doWidthEntry(void);
	void	adjustHeightForAspect(void);
	void	adjustWidthForAspect(void);

	void wrappingChanged(void);
private:
	void setWrappingGUI();
	void setPositionToGUI();

	double		m_dHeightWidth;

	XAP_CocoaDialog_ImageController *m_dlg;
};

#endif /* XAP_COCOADIALOG_IMAGE_H */
