/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Application Framework
 * Copyright (C) 2012 Hubert Figuiere
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

#ifndef __XAP_QT_APP_H__
#define __XAP_QT_APP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xap_App.h"
#include "xap_Qt_TB_CFactory.h"
#include "xap_QtDialogFactory.h"

class QApplication;

class ABI_EXPORT XAP_QtApp
	: public XAP_App
{
public:
	XAP_QtApp(const char*);
	virtual ~XAP_QtApp();

	virtual const char * getDefaultEncoding () const
	{ return "UTF-8"; }

	virtual void							reallyExit();
	virtual XAP_DialogFactory *				getDialogFactory()
	{ return &m_dialogFactory; }
	virtual XAP_Toolbar_ControlFactory *	getControlFactory()
	{ return &m_controlFactory; }
	virtual const char *					getUserPrivateDirectory() const;

protected:
	int exec(); // QApplication Exec.

	void							_setAbiSuiteLibDir();

	XAP_QtDialogFactory			m_dialogFactory;
	AP_QtToolbar_ControlFactory	m_controlFactory;

	QApplication* m_app;

private:
	int m_qtArgc;
	const char* m_qtArgv;
};

#endif

