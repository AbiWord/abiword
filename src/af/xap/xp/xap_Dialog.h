/* AbiSource Application Framework
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

#ifndef XAP_DIALOG_H
#define XAP_DIALOG_H

#include "ut_types.h"
#include "xap_Types.h"


class XAP_DialogFactory;
class XAP_App;
class XAP_Frame;
class AV_View;

/*****************************************************************
******************************************************************
** This file defines the base classes for cross-platform dialogs.
******************************************************************
*****************************************************************/

typedef enum _XAP_Dialog_Type
{
	XAP_DLGT_NON_PERSISTENT		= 1,
	XAP_DLGT_FRAME_PERSISTENT	= 2,
	XAP_DLGT_APP_PERSISTENT		= 3,
	XAP_DLGT_MODELESS		= 4

} XAP_Dialog_Type;


class XAP_Dialog
{
public:
	XAP_Dialog(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog(void);

	virtual void				runModal(XAP_Frame * pFrame) = 0;

	XAP_Dialog_Id				getDialogId(void) const;
	
protected:
	XAP_App *					m_pApp;
	XAP_DialogFactory *			m_pDlgFactory;
	XAP_Dialog_Id				m_id;
};


class XAP_Dialog_NonPersistent : public XAP_Dialog
{
public:
	XAP_Dialog_NonPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_NonPersistent(void);

	virtual void				runModal(XAP_Frame * pFrame) = 0;

	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_NON_PERSISTENT; };
	
protected:
};

class XAP_Dialog_Persistent : public XAP_Dialog
{
public:
	XAP_Dialog_Persistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Persistent(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

protected:
	UT_Bool						m_bInUse;
};

class XAP_Dialog_FramePersistent : public XAP_Dialog_Persistent
{
public:
	XAP_Dialog_FramePersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_FramePersistent(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_FRAME_PERSISTENT; };
	
protected:
};

class XAP_Dialog_AppPersistent : public XAP_Dialog_Persistent
{
public:
	XAP_Dialog_AppPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_AppPersistent(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_APP_PERSISTENT; };
	
protected:
};


class XAP_Dialog_Modeless : public XAP_Dialog_AppPersistent
{
public:
	XAP_Dialog_Modeless(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_Dialog_Modeless(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);
	virtual void				runModeless(XAP_Frame * pFrame) = 0;
	virtual void				setActiveFrame(XAP_Frame *pFrame) = 0;
	virtual void				notifyCloseFrame(XAP_Frame *pFrame) = 0;
	virtual void				destroy(void)=0;
	virtual void				activate(void)=0;
        XAP_Frame *                             getActiveFrame();
	void						modeless_cleanup(void);
	UT_Bool						isRunning(void);

	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_APP_PERSISTENT; };
	
protected:
        XAP_Dialog_Modeless *                    m_pDialog;
};

#endif /* XAP_DIALOG_H */







