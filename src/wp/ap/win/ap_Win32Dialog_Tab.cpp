/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Tab.h"
#include "ap_Win32Dialog_Tab.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Tab * p = new AP_Win32Dialog_Tab(pFactory,id);
	return p;
}

AP_Win32Dialog_Tab::AP_Win32Dialog_Tab(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Tab(pDlgFactory,id)
{
}

AP_Win32Dialog_Tab::~AP_Win32Dialog_Tab(void)
{
}

void AP_Win32Dialog_Tab::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

/*
	NOTE: This template can be used to create a working stub for a 
	new dialog on this platform.  To do so:
	
	1.  Copy this file (and its associated header file) and rename 
		them accordingly. 

	2.  Do a case sensitive global replace on the words Stub and STUB
		in both files. 

	3.  Add stubs for any required methods expected by the XP class. 
		If the build fails because you didn't do this step properly,
		you've just broken the donut rule.  

	4.	Replace this useless comment with specific instructions to 
		whoever's porting your dialog so they know what to do.
		Skipping this step may not cost you any donuts, but it's 
		rude.  

	This file should *only* be used for stubbing out platforms which 
	you don't know how to implement.  When implementing a new dialog 
	for your platform, you're probably better off starting with code
	from another working dialog.  
*/	

	UT_ASSERT(UT_NOT_IMPLEMENTED);
}




void AP_Win32Dialog_Tab::_controlEnable( tControl id, UT_Bool value )
{
}


eTabType AP_Win32Dialog_Tab::_gatherAlignment()
{
	// for ( UT_uint32 i = (UT_uint32)id_ALIGN_LEFT; 
	// 	  i <= (UT_uint32)id_ALIGN_BAR;
	// 	  i++ )

//	return m_current_alignment;
return FL_TAB_NONE;

}

void AP_Win32Dialog_Tab::_setAlignment( eTabType a )
{
/*
	// NOTE - tControl id_ALIGN_LEFT .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl id = (tControl)((UT_uint32)id_ALIGN_LEFT + (UT_uint32)a);	
	UT_ASSERT( id >= id_ALIGN_LEFT && id <= id_ALIGN_BAR );

	// time to set the alignment radiobutton widget
	GtkWidget *w = _lookupWidget( id );
	UT_ASSERT(w && GTK_IS_RADIO_BUTTON(w));

	// tell the change routines to ignore this message
	m_bInSetCall = UT_TRUE;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w), TRUE );
	m_bInSetCall = UT_FALSE;
*/

}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

eTabLeader AP_Win32Dialog_Tab::_gatherLeader()
{
	return FL_LEADER_NONE;
}

void AP_Win32Dialog_Tab::_setLeader( eTabLeader a )
{
/*
	// NOTE - tControl id_LEADER_NONE .. id_ALIGN_BAR must be in the same order
	// as the tAlignment enums.

	// magic noted above
	tControl id = (tControl)((UT_uint32)id_LEADER_NONE + (UT_uint32)a);	
	UT_ASSERT( id >= id_LEADER_NONE && id <= id_LEADER_UNDERLINE );

	// time to set the alignment radiobutton widget
	GtkWidget *w = _lookupWidget( id );
	UT_ASSERT(w && GTK_IS_RADIO_BUTTON(w));

	// tell the change routines to ignore this message

	m_bInSetCall = UT_TRUE;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(w), TRUE );
	m_bInSetCall = UT_FALSE;
*/
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_Win32Dialog_Tab::_gatherDefaultTabStop()
{
	return 5;
}

void AP_Win32Dialog_Tab::_setDefaultTabStop( UT_sint32 a )
{
	UT_UNUSED(a);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const UT_Vector& AP_Win32Dialog_Tab::_gatherTabList()
{
	return m_tabInfo;
}

void AP_Win32Dialog_Tab::_setTabList( const UT_Vector &v )
{
/*
	GList *gList = NULL;
	GtkList *wList = GTK_LIST(_lookupWidget( id_LIST_TAB ));
	UT_uint32 i;
	fl_TabStop *pTabInfo;

	// clear all the items from the list
	gtk_list_clear_items( wList, 0, -1 );

	for ( i = 0; i < v.getItemCount(); i++ )
	{
		pTabInfo = (fl_TabStop *)v.getNthItem(i);

		// this will do for the time being, but if we want 
		//GtkWidget *li = gtk_list_item_new_with_label( pTabInfo->pszTab );
		UT_DEBUGMSG(("%s:%d need to fix\n", __FILE__,__LINE__));

		GtkWidget *li = gtk_list_item_new_with_label( 
							UT_convertToDimensionlessString( pTabInfo->iPositionLayoutUnits,  pTabInfo->iPosition ));
		gtk_object_set_user_data( GTK_OBJECT(li), (gpointer) pTabInfo );

		// we want to DO stuff
		gtk_signal_connect(GTK_OBJECT(li),
						   "select",
						   GTK_SIGNAL_FUNC(s_list_select),
						   (gpointer) this);


		// show this baby
		gtk_widget_show(li);

		gList = g_list_append( gList, li );
	}
	
	gtk_list_insert_items( wList, gList, 0 );
*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

UT_sint32 AP_Win32Dialog_Tab::_gatherSelectTab()
{
	return 0;
//	return m_iGtkListIndex;
}

void AP_Win32Dialog_Tab::_setSelectTab( UT_sint32 v )
{
/*
	m_iGtkListIndex = v;

	if ( v == -1 )	// we don't want to select anything
	{
		gtk_list_unselect_all(GTK_LIST(_lookupWidget(id_LIST_TAB)));
	}
	else
	{
		UT_ASSERT(UT_NOT_IMPLEMENTED);
	}
*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

const char * AP_Win32Dialog_Tab::_gatherTabEdit()
{
	return NULL;
//	return gtk_entry_get_text( GTK_ENTRY( _lookupWidget( id_EDIT_TAB ) ) );
}

void AP_Win32Dialog_Tab::_setTabEdit( const char *pszStr )
{
/*
	GtkWidget *w = _lookupWidget( id_EDIT_TAB );

	// first, we stop the entry from sending the changed signal to our handler
	gtk_signal_handler_block_by_data(  GTK_OBJECT(w), (gpointer) this );

	// then set the text
	gtk_entry_set_text( GTK_ENTRY(w), pszStr );

	// turn signals back on
	gtk_signal_handler_unblock_by_data(  GTK_OBJECT(w), (gpointer) this );
*/

}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void AP_Win32Dialog_Tab::_clearList()
{
/*
	GtkList *wList = GTK_LIST(_lookupWidget( id_LIST_TAB ));

	// clear all the items from the list
	gtk_list_clear_items( wList, 0, -1 );
*/
}
