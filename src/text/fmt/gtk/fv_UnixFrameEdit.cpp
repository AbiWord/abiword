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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "fv_UnixFrameEdit.h"
#include "ut_units.h"
#include "fv_View.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xap_UnixApp.h"

static const GtkTargetEntry targets[] = {
  { (gchar*)"text/uri-list",0,0}};


FV_UnixFrameEdit::FV_UnixFrameEdit (FV_View * pView)
	: FV_FrameEdit(pView) 
{
	UT_ASSERT_HARMLESS(pView);
}

FV_UnixFrameEdit::~FV_UnixFrameEdit()
{
}

void FV_UnixFrameEdit::mouseDrag(UT_sint32 x, UT_sint32 y)
{
    UT_DEBUGMSG(("Mouse drag x=%d y=%d \n",x,y));
    bool bYOK = ( (y > 0) && (y < getView()->getWindowHeight()));
    if(!bYOK || (x> 0 && x < getView()->getWindowWidth()))
     {
         m_bDragOut = false;
	 _mouseDrag(x,y);
	 return;
     }
     if(FV_DragWhole != getDragWhat())
     {
         m_bDragOut = false;
	 _mouseDrag(x,y);
	 return;
     }
     if(FV_FrameEdit_DRAG_EXISTING != getFrameEditMode())
     {
         m_bDragOut = false;
	 _mouseDrag(x,y);
	 return;
     }
     if(!isImageWrapper())
     {
         m_bDragOut = false;
	 _mouseDrag(x,y);
	 return;
     }
     if(!m_bDragOut)
     {
	 const UT_ByteBuf * pBuf = NULL;
	 const char * pszData = getPNGImage(&pBuf);
	 UT_DEBUGMSG(("Got image buffer %p pszData %p \n",pBuf,pszData));
	 if(pBuf)
	 {
       //
       // write the image to a temperary file
       //
	     XAP_UnixApp * pXApp = static_cast<XAP_UnixApp *>(XAP_App::getApp());
	     pXApp->removeTmpFile();
	     char ** pszTmpName = pXApp->getTmpFile();
	     UT_UTF8String sTmpF = g_get_tmp_dir();
	     sTmpF += "/";
	     sTmpF += pszData;
	     sTmpF += ".png";
	     //
	     // Now write the contents of the buffer to a temp file.
	     //
	     FILE * fd = fopen(sTmpF.utf8_str(),"w");
	     fwrite(pBuf->getPointer(0),sizeof(UT_Byte),pBuf->getLength(),fd);
	     fclose(fd);

       //
       // OK set up the gtk drag and drop code to andle this
       //
	     XAP_Frame * pFrame = static_cast<XAP_Frame*>(getView()->getParentData());
	     XAP_UnixFrameImpl * pFrameImpl =static_cast<XAP_UnixFrameImpl *>( pFrame->getFrameImpl());
	     GtkWidget * pWindow = pFrameImpl->getTopLevelWindow();
	     GtkTargetList *target_list = gtk_target_list_new(targets, G_N_ELEMENTS(targets));
	     GdkDragContext *context = gtk_drag_begin_with_coordinates(
               pWindow, target_list,
               (GdkDragAction)(GDK_ACTION_COPY ), 1, NULL, x, y);

	     gdk_drag_status(context, GDK_ACTION_COPY, 0);
	     gtk_target_list_unref(target_list);
	     *pszTmpName = g_strdup(sTmpF.utf8_str());  
	     UT_DEBUGMSG(("Created Tmp File %s XApp %s \n",sTmpF.utf8_str(),*pXApp->getTmpFile()));

	 }
	 //
	 // OK quit dragging the image and return to the previous state
	 //
	 m_bDragOut = true;
	 abortDrag();
     }
     m_bDragOut = true;
}
