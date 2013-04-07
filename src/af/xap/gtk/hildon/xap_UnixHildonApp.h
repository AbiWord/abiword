/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2005 INdT
 * Author: Renato Araujo <renato.filho@indt.org.br>
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

#ifndef XAP_UNIXHILDONAPP_H
#define XAP_UNIXHILDONAPP_H

#include <unistd.h>
#include <sys/stat.h>
#include "xap_UnixApp.h"

#include <gtk/gtk.h>
#include <gtk/gtkwidget.h>
#include <libosso.h>

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class ABI_EXPORT XAP_UnixHildonApp : public XAP_UnixApp
{
public:
	XAP_UnixHildonApp(const char* szAppName);
	virtual ~XAP_UnixHildonApp();

	virtual bool initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue);

	GObject * getHildonProgram() const;

	void         processStartupQueue();
	virtual void clearStateInfo();

	void setHibernate(bool b){m_bHibernate = b;}
	bool getHibernate(void)const {return m_bHibernate;}
	bool isTopmost(void)const;

protected:
	virtual bool _saveState(XAP_StateData & sd);
	virtual bool _retrieveState(XAP_StateData & sd);

private:
	osso_context_t *       m_pOsso;
	mutable GObject *      m_pHildonProgram;
	bool                   m_bHibernate;


public:
	static bool s_bInitDone;
	static bool s_bRestoreNeeded;

};

#endif /* XAP_UNIXHILDONAPP_H */
