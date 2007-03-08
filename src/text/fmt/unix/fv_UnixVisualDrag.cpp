/* AbiWord
 * Copyright (c) 2003 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "fv_UnixVisualDrag.h"
#include "fv_View.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

static const GtkTargetEntry targets[] = {
  {"text/rtf", 0, 0}};

FV_UnixVisualDrag::FV_UnixVisualDrag (FV_View * pView)
  :  FV_VisualDragText (pView),m_bDragOut(false)
{

}

FV_UnixVisualDrag::~FV_UnixVisualDrag()
{

}

void FV_UnixVisualDrag::mouseDrag(UT_sint32 x, UT_sint32 y)
{
     UT_DEBUGMSG(("Mouse drag x=%d y=%d \n",x,y));
     if(x> 0 && x < m_pView->getWindowWidth())
     {
         m_bDragOut = false;
	 _mouseDrag(x,y);
	 return;
     }
     if(!m_bDragOut)
     {
         XAP_Frame * pFrame = static_cast<XAP_Frame*>(m_pView->getParentData());
	 XAP_UnixFrameImpl * pFrameImpl =static_cast<XAP_UnixFrameImpl *>( pFrame->getFrameImpl());
	 GtkWidget * pWindow = pFrameImpl->getTopLevelWindow();
	 GtkTargetList *target_list = gtk_target_list_new(targets, G_N_ELEMENTS(targets));
	 GdkDragContext *context = gtk_drag_begin(pWindow, target_list,
						(GdkDragAction)(GDK_ACTION_COPY ), 1, NULL);
	 gdk_drag_status(context, GDK_ACTION_COPY, 0);
	 gtk_target_list_unref(target_list);
	 m_bDragOut = true;
	 getGraphics()->setClipRect(getCurFrame());
	 m_pView->updateScreen(false);
	 getGraphics()->setClipRect(NULL);
	 setMode(FV_VisualDrag_NOT_ACTIVE);
	 m_pView->setPrevMouseContext(EV_EMC_VISUALTEXTDRAG);
	 return;
     }
     m_bDragOut = true;

}

