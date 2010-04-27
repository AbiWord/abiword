/* AbiSource Application Framework
 * Copyright (C) 2005 Hubert Figuiere
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




#include "tf_test.h"
#include "xap_UnixWidget.h"


TFTEST_MAIN("xap_UnixWidget-toggle_button")
{
	GtkWidget *gtkw = gtk_toggle_button_new();
	XAP_Widget *w = new XAP_UnixWidget(gtkw);


	w->setState(true);
	TFPASS(w->getState());
	w->setState(false);
	TFPASS(w->getState() == false);
	
	w->setValueInt(1);
	TFPASS(w->getValueInt() == 1);

	delete w;
	gtk_object_sink(GTK_OBJECT(gtkw));
}
