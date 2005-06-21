/***************************************************************************
 *            xap_UnixHildonApp.h
 *
 *  Thu May  5 10:39:52 2005
 *  Author   INDT - Renato Araujo <renato.filho@indt.org.br>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
 
 
#ifndef _XAP_UNIXHILDONAPP_H
#define _XAP_UNIXHILDONAPP_H

//#include <libosso.h>
#include <hildon-widgets/hildon-app.h>


class XAP_UnixHildonApp
{
public:
	XAP_UnixHildonApp();
	~XAP_UnixHildonApp();

	bool initialize();
	void terminate();
	static HildonApp* getApp() 	{ return m_pHildonApp; } 


protected:
	//osso_context_t   *		m_osso_context;	
 	static HildonApp *		m_pHildonApp;

};

#endif /* _XAP_UNIXHILDONAPP_H */
