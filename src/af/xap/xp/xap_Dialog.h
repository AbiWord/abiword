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

#ifndef AP_DIALOG_H
#define AP_DIALOG_H

#include "ut_types.h"
#include "ap_Types.h"
class AP_DialogFactory;
class AP_Frame;

/*****************************************************************
******************************************************************
** This file defines the base classes for cross-platform dialogs.
******************************************************************
*****************************************************************/

typedef enum _AP_Dialog_Type
{
	AP_DLGT_NON_PERSISTENT		= 1,
	AP_DLGT_FRAME_PERSISTENT	= 2,
	AP_DLGT_APP_PERSISTENT		= 3

} AP_Dialog_Type;


class AP_Dialog
{
public:
	AP_Dialog(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog(void);

	virtual void				runModal(AP_Frame * pFrame) = 0;
	AP_Dialog_Id				getDialogId(void) const;
	
protected:
	AP_DialogFactory *			m_pDlgFactory;
	AP_Dialog_Id				m_id;
};


class AP_Dialog_NonPersistent : public AP_Dialog
{
public:
	AP_Dialog_NonPersistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_NonPersistent(void);

	virtual void				runModal(AP_Frame * pFrame) = 0;

	static AP_Dialog_Type		s_getPersistence(void) { return AP_DLGT_NON_PERSISTENT; };
	
protected:
};

class AP_Dialog_Persistent : public AP_Dialog
{
public:
	AP_Dialog_Persistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_Persistent(void);

	virtual void				useStart(void);
	virtual void				runModal(AP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

protected:
	UT_Bool						m_bInUse;
};

class AP_Dialog_FramePersistent : public AP_Dialog_Persistent
{
public:
	AP_Dialog_FramePersistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_FramePersistent(void);

	virtual void				useStart(void);
	virtual void				runModal(AP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	static AP_Dialog_Type		s_getPersistence(void) { return AP_DLGT_FRAME_PERSISTENT; };
	
protected:
};

class AP_Dialog_AppPersistent : public AP_Dialog_Persistent
{
public:
	AP_Dialog_AppPersistent(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id);
	virtual ~AP_Dialog_AppPersistent(void);

	virtual void				useStart(void);
	virtual void				runModal(AP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	static AP_Dialog_Type		s_getPersistence(void) { return AP_DLGT_APP_PERSISTENT; };
	
protected:
};

#endif /* AP_DIALOG_H */
