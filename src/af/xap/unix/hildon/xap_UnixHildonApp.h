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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef XAP_UNIXHILDONAPP_H
#define XAP_UNIXHILDONAPP_H

#include <unistd.h>
#include <sys/stat.h>
#include "xap_UnixApp.h"

#include <gtk/gtkwidget.h>
#include <libosso.h>

class XAP_Args;

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

class ABI_EXPORT XAP_UnixHildonApp : public XAP_UnixApp
{
public:
	XAP_UnixHildonApp(XAP_Args* pArgs, const char* szAppName);
	virtual ~XAP_UnixHildonApp();

	virtual bool initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue);

	GtkWidget *  getHildonAppWidget() const;
	
protected:
	virtual void _saveState(XAP_StateData & sd);
	virtual void _retrieveState(XAP_StateData & sd);

private:
	osso_context_t *    m_pOsso;
	mutable GtkWidget * m_pHildonAppWidget;
	
};

#endif /* XAP_UNIXHILDONAPP_H */
