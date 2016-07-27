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

#include "fv_UnixVisualDrag.h"
#include "fv_View.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"
#include "xap_UnixApp.h"
#include "xap_App.h"
#include "ut_string.h"
#include <stdio.h>
#include "ut_uuid.h"
#include "pd_Document.h"
#include "ie_exp.h"
#include "ie_imp.h"
#include "ie_imp_RTF.h"

#include <gsf/gsf-input.h>
#include <gsf/gsf-output.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-utils.h>

static const GtkTargetEntry targets[] = {
  { (gchar*)"text/rtf", 0, 0},
  { (gchar*)"text/uri-list",0,0}
};

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
     bool bYOK = ( (y > 0) && (y < m_pView->getWindowHeight()));
     if(!bYOK || (x> 0 && x < m_pView->getWindowWidth()))
     {
         m_bDragOut = false;
	 _mouseDrag(x,y);
	 return;
     }
     if(!m_bDragOut)
     {
	 //
	 // OK write text to a local file
	 //
	 XAP_UnixApp * pXApp = static_cast<XAP_UnixApp *>(XAP_App::getApp());
	 pXApp->removeTmpFile();
	 char ** pszTmpName = pXApp->getTmpFile();
	 const UT_ByteBuf * pBuf = m_pView->getLocalBuf();
	 UT_return_if_fail(pBuf);
	 //
	 // Create filename from contents
	 //
	 PD_Document * newDoc = new PD_Document();
	 newDoc->createRawDocument();
	 //
	 // This code is to generate a file name from the contents of the
	 // dragged text. The idea is to give user some clue as to the
	 // contents of the file dropped onto Nautilus
	 //
	 GsfInput * source = static_cast<GsfInput *>(gsf_input_memory_new(pBuf->getPointer(0),
									  pBuf->getLength(),
									  FALSE));
	 IE_Imp_RTF * imp = new IE_Imp_RTF(newDoc);
	 imp->importFile(source);
	 delete(imp);
	 newDoc->finishRawCreation();
	 g_object_unref(G_OBJECT(source));
	 //
	 // OK we've created a document with the contents of the drag
	 // buffer, now extract the first 20 characters and use it
	 // as the file name.
	 //
	 IEFileType file_type = IE_Exp::fileTypeForSuffix(".txt");
	 GsfOutputMemory* sink = GSF_OUTPUT_MEMORY(gsf_output_memory_new());
	 newDoc->saveAs(GSF_OUTPUT(sink),file_type, true);
	 gsf_output_close(GSF_OUTPUT(sink));
	 UT_UTF8String sRaw = reinterpret_cast<const char *>(gsf_output_memory_get_bytes (sink));
	 UT_UCS4String sUCS4 = sRaw.ucs4_str ();
	 UT_UCS4String sProc;
	 sProc.clear();
	 UT_uint32 size = sRaw.size();
	 if(size > 20 )
	     size = 20;
	 UT_uint32 i = 0;
	 //
	 // remove illegal filename characters
	 //
	 for(i=0; i<size; i++)
	 {
	     UT_UCS4Char u = sUCS4[i];
	     gchar c = static_cast<gchar>(sUCS4[i]);
	     bool b = true;
	     if((u < 128) && (c == ':' || c == ';' || c=='\'' || c==',' || c=='"'
		|| c == '@' || c=='!' || c=='~' || c=='`' || c=='$' || c=='#'
		|| c == '%' || c=='*' || c=='(' || c==')' || c=='+'
		|| c == '{' || c=='[' || c=='}' || c==']' || c=='|'
		|| c == '\\' || c== '<' || c== '>' || c== '.' || c=='?'
			      || c=='/' || (c<32)))
	     {
		 b = false;
	     }
	     if(b)
	     {
		 sProc += u;
	     }
	 }
	 sRaw = sProc.utf8_str();
	 g_object_unref(G_OBJECT(sink));
	 UNREFP(newDoc);
	 UT_UTF8String sTmpF = g_get_tmp_dir();
	 sTmpF += "/";
	 sTmpF += sRaw;
	 sTmpF += ".rtf";
	 //
	 // Now write the contents of the buffer to a temp file.
	 //
	 FILE * fd = fopen(sTmpF.utf8_str(),"w");
	 fwrite(pBuf->getPointer(0),sizeof(UT_Byte),pBuf->getLength(),fd);
	 fclose(fd);
	 //
	 // Now setup the gtk drag and drop code to handle this.
	 //
         XAP_Frame * pFrame = static_cast<XAP_Frame*>(m_pView->getParentData());
	 XAP_UnixFrameImpl * pFrameImpl =static_cast<XAP_UnixFrameImpl *>( pFrame->getFrameImpl());
	 GtkWidget * pWindow = pFrameImpl->getTopLevelWindow();
	 GtkTargetList *target_list = gtk_target_list_new(targets, G_N_ELEMENTS(targets));
	 GdkDragContext *context = gtk_drag_begin_with_coordinates(
           pWindow, target_list,
           (GdkDragAction)(GDK_ACTION_COPY ), 1, NULL, x, y);

	 gdk_drag_status(context, GDK_ACTION_COPY, 0);
	 gtk_target_list_unref(target_list);
	 m_bDragOut = true;
	 getGraphics()->setClipRect(getCurFrame());
	 m_pView->updateScreen(false);
	 getGraphics()->setClipRect(NULL);
	 setMode(FV_VisualDrag_NOT_ACTIVE);
	 m_pView->setPrevMouseContext(EV_EMC_VISUALTEXTDRAG);
	 *pszTmpName = g_strdup(sTmpF.utf8_str());  
	 UT_DEBUGMSG(("Created Tmp File %s XApp %s \n",sTmpF.utf8_str(),*pXApp->getTmpFile()));
	 m_bDragOut = true;
	 return;
     }
     m_bDragOut = true;

}

