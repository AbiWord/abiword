/* The AbiWord Widget 
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001,2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include <string.h>

#include "abiwidget.h"
#include "gr_UnixGraphics.h"
#include "ev_EditMethod.h"
#include "ut_assert.h"
#include "fv_View.h"
#include "ap_UnixFrame.h"
#include "ap_FrameData.h"
#include "xap_Args.h"
#include "pd_Document.h"
#include "ie_imp.h"
#include "ie_exp.h"
#include "xap_UnixDialogHelper.h"
#include <glib-object.h>
#include "ap_UnixApp.h"
#include "ut_sleep.h"
#include "fv_View.h"
#include "fl_DocLayout.h"


static UT_Vector vecAbi;

/**************************************************************************/
/**************************************************************************/

/*
 * The AbiWord Widget
 *    by Martin Sevior and Dom Lachowicz
 *
 * Four score and 3 months ago, we decided that it would be cool to have
 * AbiWord exposed as a GTK+ widget. After inventing the computer and GTK+,
 * Dom and Martin undertook the task of exposing AbiWord's functionality
 * in freaky-fun GTKWidget form. And to this day, it has been a horrible
 * success.
 *
 * Not stymied by the hoopla and fanfare surrounding their earlier 
 * achievements, the two decided to actually make AbiWord's main frame
 * an instance of this AbiWidget. Following that, Martin undertook the
 * ordeal of exposing this Widget via a Bonobo interface (after first
 * inventing Bonobo, of course) so that other applications such as
 * Ximian Evolution could seamlessly embed instances of AbiWord inside
 * of themselves.
 *
 */

/**************************************************************************/
/**************************************************************************/

// Our widget's private storage data
// UnixApp and UnixFrame already properly inherit from either
// Their GTK+ or GNOME XAP equivalents, so we don't need to worry
// About that here

struct _AbiPrivData {
	AP_UnixApp           * m_pApp;
	AP_UnixFrame         * m_pFrame;
	char           * m_szFilename;
	bool                 externalApp;
	bool                 m_bMappedToScreen;
	bool                 m_bPendingFile;
	bool                 m_bMappedEventProcessed;
    bool                 m_bUnlinkFileAfterLoad;
	gint                 m_iNumFileLoads;
#ifdef HAVE_GNOME
	BonoboUIComponent    * m_uic;
#endif
};

/**************************************************************************/
/**************************************************************************/

//
// Our widget's arguments. 
//
enum {
  ARG_0,
  CURSOR_ON,
  INVOKE_NOARGS,
  MAP_TO_SCREEN,
  IS_ABI_WIDGET,
  UNLINK_AFTER_LOAD,
  DRAW,
  LOAD_FILE,
  ALIGNCENTER,
  ALIGNLEFT,
  ALIGNRIGHT,
  ALIGNJUSTIFY,
  COPY,
  CUT,
  PASTE,
  PASTESPECIAL,
  SELECTBLOCK,
  SELECTLINE,
  SELECTWORD,
  SELECTALL,
  INSERTDATA,
  INSERTSPACE,
  DELBOB,
  DELBOD,
  DELBOL,
  DELBOW,
  DELEOB,
  DELEOD,
  DELEOL,
  DELEOW,
  DELLEFT,
  DELRIGHT,
  EDITHEADER,
  EDITFOOTER,
  REMOVEHEADER,
  REMOVEFOOTER,
  EXTSELBOB,
  EXTSELBOD,
  EXTSELBOL,
  EXTSELBOW,
  EXTSELEOB,
  EXTSELEOD,
  EXTSELEOL,
  EXTSELEOW,
  EXTSELLEFT,
  EXTSELRIGHT,
  EXTSELNEXTLINE,
  EXTSELPAGEDOWN,
  EXTSELPAGEUP,
  EXTSELPREVLINE,
  EXTSELSCREENDOWN,
  EXTSELSCREENUP,
  TOGGLEBOLD,
  TOGGLEBOTTOMLINE,
  TOGGLEINSERTMODE,
  TOGGLEITALIC,
  TOGGLEOLINE,
  TOGGLEPLAIN,
  TOGGLESTRIKE,
  TOGGLESUB,
  TOGGLESUPER,
  TOGGLETOPLINE,
  TOGGLEULINE,
  TOGGLEUNINDENT,
  VIEWPARA,
  VIEWPRINTLAYOUT,
  VIEWNORMALLAYOUT,
  VIEWWEBLAYOUT,
  UNDO,
  REDO,
  WARPINSPTBOB,
  WARPINSPTBOD,
  WARPINSPTBOL,
  WARPINSPTBOP,
  WARPINSPTBOW,
  WARPINSPTEOB,
  WARPINSPTEOD,
  WARPINSPTEOL,
  WARPINSPTEOP,
  WARPINSPTEOW,
  WARPINSPTLEFT,
  WARPINSPTNEXTLINE,
  WARPINSPTNEXTPAGE,
  WARPINSPTNEXTSCREEN,
  WARPINSPTPREVLINE,
  WARPINSPTPREVPAGE,
  WARPINSPTPREVSCREEN,
  WARPINSPTPREVRIGHT, 
  ZOOM100,
  ZOOM200,
  ZOOM50,
  ZOOM75,
  ZOOMWHOLE,
  ZOOMWIDTH,
  ARG_LAST
};

// our parent class
static GtkBinClass * parent_class = 0;

static void s_abi_widget_map_cb(GObject * w,  GdkEvent *event,gpointer p);

static void s_abi_widget_destroy(GObject * w, gpointer abi);

static void s_abi_widget_delete(GObject * w, gpointer abi);

static void abi_widget_destroy (GObject *object);

/**************************************************************************/
/**************************************************************************/

// Here, we have some macros that:
// 1) Define the desired name for an EditMethod (EM_NAME)
// 2) Actually define a static function which maps directly 
//    onto an abi EditMethod (the rest)
//
// All that these functions do is marshall data into and out of
// the AbiWord application

#define EM_NAME(n) abi_em_##n

#define EM_CHARPTR_INT_INT__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, const char * str, gint32 x, gint32 y) \
{ \
return abi_widget_invoke_ex (w, #n, str, x, y); \
}

#define EM_VOID__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w)\
{ \
return abi_widget_invoke_ex (w, #n, 0, 0, 0); \
}

#define EM_INT_INT__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, gint32 x, gint32 y) \
{ \
return abi_widget_invoke_ex (w, #n, 0, x, y); \
}

#define EM_CHARPTR__BOOL(n) \
static gboolean EM_NAME(n) (AbiWidget * w, const char * str) \
{ \
return abi_widget_invoke_ex (w, #n, str, 0, 0); \
}

/**************************************************************************/
/**************************************************************************/

// Here we define our EditMethods which will later be mapped back onto
// Our AbiWidgetClass' member functions

EM_VOID__BOOL(alignCenter);
EM_VOID__BOOL(alignLeft);
EM_VOID__BOOL(alignRight);
EM_VOID__BOOL(alignJustify);

EM_VOID__BOOL(copy);
EM_VOID__BOOL(cut);
EM_VOID__BOOL(paste);
EM_VOID__BOOL(pasteSpecial);
EM_VOID__BOOL(selectAll);
EM_VOID__BOOL(selectBlock);
EM_VOID__BOOL(selectLine);
EM_VOID__BOOL(selectWord);

EM_VOID__BOOL(undo);
EM_VOID__BOOL(redo);

EM_CHARPTR__BOOL(insertData);
EM_VOID__BOOL(insertSpace);

EM_VOID__BOOL(delBOB);
EM_VOID__BOOL(delBOD);
EM_VOID__BOOL(delBOL);
EM_VOID__BOOL(delBOW);
EM_VOID__BOOL(delEOB);
EM_VOID__BOOL(delEOD);
EM_VOID__BOOL(delEOL);
EM_VOID__BOOL(delEOW);
EM_VOID__BOOL(delLeft);
EM_VOID__BOOL(delRight);

EM_VOID__BOOL(editHeader);
EM_VOID__BOOL(editFooter);
EM_VOID__BOOL(removeHeader);
EM_VOID__BOOL(removeFooter);

EM_VOID__BOOL(extSelBOB);
EM_VOID__BOOL(extSelBOD);
EM_VOID__BOOL(extSelBOL);
EM_VOID__BOOL(extSelBOW);
EM_VOID__BOOL(extSelEOB);
EM_VOID__BOOL(extSelEOD);
EM_VOID__BOOL(extSelEOL);
EM_VOID__BOOL(extSelEOW);
EM_VOID__BOOL(extSelLeft);
EM_VOID__BOOL(extSelNextLine);
EM_VOID__BOOL(extSelPageDown);
EM_VOID__BOOL(extSelPageUp);
EM_VOID__BOOL(extSelPrevLine);
EM_VOID__BOOL(extSelRight);
EM_VOID__BOOL(extSelScreenDown);
EM_VOID__BOOL(extSelScreenUp);
EM_INT_INT__BOOL(extSelToXY);

EM_VOID__BOOL(toggleBold);
EM_VOID__BOOL(toggleBottomline);
EM_VOID__BOOL(toggleInsertMode);
EM_VOID__BOOL(toggleItalic);
EM_VOID__BOOL(toggleOline);
EM_VOID__BOOL(togglePlain);
EM_VOID__BOOL(toggleStrike);
EM_VOID__BOOL(toggleSub);
EM_VOID__BOOL(toggleSuper);
EM_VOID__BOOL(toggleTopline);
EM_VOID__BOOL(toggleUline);
EM_VOID__BOOL(toggleUnIndent);

EM_VOID__BOOL(viewPara);
EM_VOID__BOOL(viewPrintLayout);
EM_VOID__BOOL(viewNormalLayout);
EM_VOID__BOOL(viewWebLayout);

EM_VOID__BOOL(warpInsPtBOB);
EM_VOID__BOOL(warpInsPtBOD);
EM_VOID__BOOL(warpInsPtBOL);
EM_VOID__BOOL(warpInsPtBOP);
EM_VOID__BOOL(warpInsPtBOW);
EM_VOID__BOOL(warpInsPtEOB);
EM_VOID__BOOL(warpInsPtEOD);
EM_VOID__BOOL(warpInsPtEOL);
EM_VOID__BOOL(warpInsPtEOP);
EM_VOID__BOOL(warpInsPtEOW);
EM_VOID__BOOL(warpInsPtLeft);
EM_VOID__BOOL(warpInsPtNextLine);
EM_VOID__BOOL(warpInsPtNextPage);
EM_VOID__BOOL(warpInsPtNextScreen);
EM_VOID__BOOL(warpInsPtPrevLine);
EM_VOID__BOOL(warpInsPtPrevPage);
EM_VOID__BOOL(warpInsPtPrevScreen);
EM_VOID__BOOL(warpInsPtRight);
EM_INT_INT__BOOL(warpInsPtToXY);

EM_VOID__BOOL(zoom100);
EM_VOID__BOOL(zoom200);
EM_VOID__BOOL(zoom50);
EM_VOID__BOOL(zoom75);
EM_VOID__BOOL(zoomWhole);
EM_VOID__BOOL(zoomWidth);

/**************************************************************************/
/**************************************************************************/

// cleanup - keep EM_NAME around because it's actually useful elsewhere

#undef EM_VOID__BOOL
#undef EM_CHARPTR__BOOL
#undef EM_INT_INT__BOOL
#undef EM_CHARPTR_INT_INT__BOOL

/**************************************************************************/
/**************************************************************************/

static const guint32 ABI_DEFAULT_WIDTH  = 250 ;
static const guint32 ABI_DEFAULT_HEIGHT = 250 ;

/**************************************************************************/
/**************************************************************************/

//
static bool
abi_widget_load_file(AbiWidget * abi, const char * pszFile)
{
	if(abi->priv->m_szFilename)
		free(abi->priv->m_szFilename);
	abi->priv->m_szFilename = UT_strdup(pszFile);
	if(!abi->priv->m_bMappedToScreen)
	{
	  abi->priv->m_bPendingFile = true;
	  return false;
	}
	if(abi->priv->m_iNumFileLoads > 0)
	{
		return false;
	}
#ifdef LOGFILE
	UT_uint32 j =0;
	for(j=0; j<vecAbi.getItemCount(); j++)
	{
		AbiWidget * pAbi = (AbiWidget *) vecAbi.getNthItem(j);
		fprintf(getlogfile(),"Refcount abi %d is %d at next file load \n",j,G_OBJECT(pAbi)->ref_count);
	}
#endif
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
		return false;

	bool res= ( UT_OK == pFrame->loadDocument(abi->priv->m_szFilename,IEFT_Unknown ,true));
	abi->priv->m_bPendingFile = false;
	abi->priv->m_iNumFileLoads += 1;
	if(abi->priv->m_bUnlinkFileAfterLoad)
	{
	  unlink(pszFile);
	  abi->priv->m_bUnlinkFileAfterLoad = false;
	}
	return FALSE;
}

static gint s_abi_widget_load_file(gpointer p)
{
  AbiWidget * abi = (AbiWidget *) p;

  if(!abi->priv->m_bMappedToScreen)
	abi_widget_map_to_screen(abi);
  if(abi->priv->m_bPendingFile)
  {
	char * pszFile = UT_strdup(abi->priv->m_szFilename);
	abi_widget_load_file(abi, pszFile);
	free(pszFile);
  }
  return FALSE;
}

static void s_abi_widget_map_cb(GObject * w,  GdkEvent *event,gpointer p)
{
  AbiWidget * abi = ABI_WIDGET(p);

  if(!abi->priv->m_bMappedEventProcessed)
  {
	  abi->priv->m_bMappedEventProcessed = true;
//
// Can't load until this event has finished propagating
//
	  g_idle_add(static_cast<GSourceFunc>(s_abi_widget_load_file),static_cast<gpointer>(abi));
  }
}


static void s_abi_widget_destroy(GObject * w, gpointer p)
{

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget destroyed \n");
#endif
  abi_widget_destroy(G_OBJECT(p));
}


static void s_abi_widget_delete(GObject * w, gpointer p)
{

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget deleted\n");
#endif

  abi_widget_destroy(G_OBJECT(p));
}

//
// arguments to abiwidget
//
static void abi_widget_get_prop (GObject  *object,
								 guint arg_id,
								 GValue     *arg,
								 GParamSpec *pspec)
{
    AbiWidget * abi = ABI_WIDGET(object);
	switch(arg_id)
	{
	    case MAP_TO_SCREEN:
		{
			g_value_set_boolean(arg, (gboolean) abi->priv->m_bMappedToScreen);
			break;
		}
	    case IS_ABI_WIDGET:
		{
			g_value_set_boolean(arg,(gboolean) true);
			break;
		}
	    case UNLINK_AFTER_LOAD:
		{
			g_value_set_boolean(arg,(gboolean) abi->priv->m_bUnlinkFileAfterLoad);
			break;
		}
	    default:
			break;
	}
}

void abi_widget_get_property(GObject  *object,
								 guint arg_id,
								 GValue     *arg,
								 GParamSpec *pspec)
{
	abi_widget_get_prop(object,	arg_id,	arg, pspec);
}

//
// props to abiwidget
//
static void abi_widget_set_prop (GObject  *object,
								 guint	arg_id,
								 const GValue *arg,
								 GParamSpec *pspec)
{
  g_return_if_fail (object != 0);

  AbiWidget * abi = ABI_WIDGET (object);
  AbiWidgetClass * abi_klazz = ABI_WIDGET_CLASS (G_OBJECT_GET_CLASS(object));

#ifdef LOGFILE
	fprintf(getlogfile(),"setArg %d\n",arg_id);
#endif

	switch(arg_id)
	{
	    case CURSOR_ON:
		{
		     if(g_value_get_boolean(arg) == TRUE)
			      abi_widget_turn_on_cursor(abi);
			 break;
		}
	    case INVOKE_NOARGS:
		{
		     const char * psz= g_value_get_string( arg);
		     abi_widget_invoke_ex(abi,psz,0,0,0);
			 break;
		}
	    case MAP_TO_SCREEN:
		{
		     if(g_value_get_boolean(arg) == TRUE)
			      abi_widget_map_to_screen(abi);
			 break;
		}
	    case UNLINK_AFTER_LOAD:
		{
		     if(g_value_get_boolean(arg) == TRUE)
			      abi->priv->m_bUnlinkFileAfterLoad = true;
			 else
			      abi->priv->m_bUnlinkFileAfterLoad = false;
			 break;
		}
	    case DRAW:
		{
		     if(g_value_get_boolean(arg) == TRUE)
			      abi_widget_draw(abi);
			 break;
		}
	    case LOAD_FILE:
		{
		     const char * pszFile= g_value_get_string(arg);
			 abi_widget_load_file(abi,pszFile);
			 break;
		}
	    case ALIGNCENTER:
		{
		  abi_klazz->align_center (abi);
		  break;
		}
	    case ALIGNLEFT:
		{
		  abi_klazz->align_left (abi);
		  break;
		}
	    case ALIGNRIGHT:
	      {
			  abi_klazz->align_right (abi);
			  break;
	      }
	    case ALIGNJUSTIFY:
		{
		  abi_klazz->align_justify (abi);
		  break;
		}
	    case COPY:
		{
		  abi_klazz->copy (abi);
		  break;
		}
	    case CUT:
	      {
		abi_klazz->cut (abi);
		break;
	      }
	    case PASTE:
		{
		  abi_klazz->paste (abi);
		  break;
		}
	    case PASTESPECIAL:
		{
		  abi_klazz->paste_special (abi);
		  break;
		}
	    case SELECTALL:
		{
		  abi_klazz->select_all (abi);
		  break;
		}
	    case SELECTBLOCK:
		{
		  abi_klazz->select_block (abi);
		  break;
		}
	    case SELECTLINE:
		{
		  abi_klazz->select_line (abi);
		  break;
		}
	    case SELECTWORD:
		{
		  abi_klazz->select_word (abi);
		  break;
		}
	    case INSERTDATA:
		{
		     const char * pszstr= g_value_get_string(arg);
			 abi_klazz->insert_data (abi, pszstr);
			 break;
		}
	    case  INSERTSPACE:
		{
		  abi_klazz->insert_space (abi);
		  break;
		}
	    case DELBOB:
		{
		  abi_klazz->delete_bob (abi);
		  break;
		}
	    case DELBOD:
		{
		  abi_klazz->delete_bod (abi);
		  break;
		}
	    case DELBOL:
		{
		  abi_klazz->delete_bol (abi);		  
		  break;
		}
	    case DELBOW:
		{
		  abi_klazz->delete_bow (abi);
		  break;
		}
	case DELEOB:
	  {
	    abi_klazz->delete_eob (abi);
	    break;
	  }
	case DELEOD:
	  {
	    abi_klazz->delete_eod (abi);
	    break;
	  }
	case DELEOL:
	  {
	    abi_klazz->delete_eol (abi);
	    break;
	  }
	case DELEOW:
	  {
	    abi_klazz->delete_eow (abi);
	    break;
	  }
	case DELLEFT:
	  {
	    abi_klazz->delete_left (abi);
	    break;
	  }
	    case DELRIGHT:
	      {
		abi_klazz->delete_right (abi);
		break;
	      }
	    case EDITHEADER:
		{
		  abi_klazz->edit_header (abi);
		  break;
		}
	    case EDITFOOTER:
		{
		  abi_klazz->edit_footer (abi);
		  break;
		}
	    case REMOVEHEADER:
		{
		  abi_klazz->remove_header (abi);
		  break;
		}
	    case REMOVEFOOTER:
		{
		  abi_klazz->remove_footer (abi);
		  break;
		}
	    case EXTSELBOB:
		{
		  abi_klazz->select_bob (abi);
		  break;
		}
	    case EXTSELBOD:
		{
		  abi_klazz->select_bod (abi);
		  break;
		}
	    case EXTSELBOL:
		{
		  abi_klazz->select_bol (abi);
		  break;
		}
	    case EXTSELBOW:
		{
		  abi_klazz->select_bow (abi);
		  break;
		}
	    case EXTSELEOB:
		{
		  abi_klazz->select_eob (abi);
		  break;
		}
	    case EXTSELEOD:
		{
		  abi_klazz->select_eod (abi);
		  break;
		}
	    case EXTSELEOL:
		{
		  abi_klazz->select_eol (abi);
		  break;
		}
	    case EXTSELEOW:
		{
		  abi_klazz->select_eow (abi);
		  break;
		}
	    case EXTSELLEFT:
		{
		  abi_klazz->select_left (abi);
		  break;
		}
	case EXTSELRIGHT:
	  {
	    abi_klazz->select_right (abi);
	    break;
	  }
	    case EXTSELNEXTLINE:
		{
		  abi_klazz->select_next_line (abi);
		  break;
		}
	    case EXTSELPAGEDOWN:
		{
		  abi_klazz->select_page_down (abi);
		  break;
		}
	    case EXTSELPAGEUP:
		{
		  abi_klazz->select_page_up (abi);
		  break;
		}
	    case EXTSELPREVLINE:
		{
		  abi_klazz->select_prev_line (abi);
		  break;
		}
	    case EXTSELSCREENDOWN:
		{
		  abi_klazz->select_screen_down (abi);
		  break;
		}
	    case EXTSELSCREENUP:
		{
		  abi_klazz->select_screen_up (abi);
		  break;
		}
	    case TOGGLEBOLD:
		{
		  abi_klazz->toggle_bold (abi);
		  break;
		}
	    case TOGGLEBOTTOMLINE:
		{
		  abi_klazz->toggle_bottomline (abi);
		  break;
		}
	    case TOGGLEINSERTMODE:
		{
		  abi_klazz->toggle_insert_mode (abi);
		  break;
		}
	    case TOGGLEITALIC:
		{
		  abi_klazz->toggle_italic (abi);
		  break;
		}
	    case TOGGLEOLINE:
		{
		  abi_klazz->toggle_overline (abi);
		  break;
		}
	    case TOGGLEPLAIN:
		{
		  abi_klazz->toggle_plain (abi);
		  break;
		}
	    case TOGGLESTRIKE:
		{
		  abi_klazz->toggle_strike (abi);
		  break;
		}
	    case TOGGLESUB:
	      {
		abi_klazz->toggle_sub (abi);
		break;
	      }
	    case TOGGLESUPER:
		{
		  abi_klazz->toggle_super (abi);
		  break;
		}
	    case TOGGLETOPLINE:
	      {
		abi_klazz->toggle_topline (abi);
		break;
		}
	    case TOGGLEULINE:
		{
		  abi_klazz->toggle_underline (abi);
		  break;
		}
	    case TOGGLEUNINDENT:
		{
		  abi_klazz->toggle_unindent (abi);
		  break;
		}
	    case VIEWPARA:
		{
		  abi_klazz->view_formatting_marks (abi);
		  break;
		}
	    case VIEWPRINTLAYOUT:
		{
		  abi_klazz->view_print_layout (abi);
		  break;
		}
	    case VIEWNORMALLAYOUT:
		{
		  abi_klazz->view_normal_layout (abi);
		  break;
		}
	    case VIEWWEBLAYOUT:
	      {
		abi_klazz->view_online_layout (abi);
		break;
		}
	    case UNDO:
	      {
		  abi_klazz->undo (abi);
		  break;
		}
	    case REDO:
		{
		  abi_klazz->redo (abi);
		  break;
		}

	    case WARPINSPTBOB:
		{
		  abi_klazz->moveto_bob (abi);
		  break;
		}
	    case WARPINSPTBOD:
		{
		  abi_klazz->moveto_bod (abi);
		  break;
		}
	    case WARPINSPTBOL:
		{
		  abi_klazz->moveto_bol (abi);
		  break;
		}
	    case WARPINSPTBOP:
		{
		  abi_klazz->moveto_bop (abi);
		  break;
		}
	    case WARPINSPTBOW:
		{
		  abi_klazz->moveto_bow (abi);
		  break;
		}
	    case WARPINSPTEOB:
		{
		  abi_klazz->moveto_eob (abi);
		  break;
		}
	    case WARPINSPTEOD:
		{
		  abi_klazz->moveto_eod (abi);
		  break;
		}
	    case WARPINSPTEOL:
		{
		  abi_klazz->moveto_eol (abi);
		  break;
		}
	    case WARPINSPTEOP:
		{
		  abi_klazz->moveto_eop (abi);
		  break;
		}
	    case WARPINSPTEOW:
		{
		  abi_klazz->moveto_eow (abi);
		  break;
		}
	    case WARPINSPTLEFT:
		{
		  abi_klazz->moveto_left (abi);
		  break;
		}
	    case WARPINSPTNEXTLINE:
		{
		  abi_klazz->moveto_next_line (abi);
		  break;
		}
	    case WARPINSPTNEXTPAGE:
		{
		  abi_klazz->moveto_next_page (abi);
		  break;
		}
	    case WARPINSPTNEXTSCREEN:
		{
		  abi_klazz->moveto_next_screen (abi);
		  break;
		}
	    case WARPINSPTPREVLINE:
		{
		  abi_klazz->moveto_prev_line (abi);
		  break;
		}
	    case WARPINSPTPREVPAGE:
		{
		  abi_klazz->moveto_prev_page (abi);
		  break;
		}
	    case WARPINSPTPREVSCREEN:
		{
		  abi_klazz->moveto_prev_screen (abi);
		  break;
		}
	    case WARPINSPTPREVRIGHT:
		{
		  abi_klazz->moveto_right (abi);
		  break;
		}
	    case ZOOM100:
		{
		  abi_klazz->zoom_100 (abi);
		  break;
		}
	    case ZOOM200:
		{
		  abi_klazz->zoom_200 (abi);
		  break;
		}
	    case ZOOM50:
		{
		  abi_klazz->zoom_50 (abi);
		  break;
		}
	    case ZOOM75:
		{
		  abi_klazz->zoom_75 (abi);
		  break;
		}
	    case ZOOMWHOLE:
		{
		  abi_klazz->zoom_whole (abi);		  
		  break;
		}
	    case ZOOMWIDTH:
		{
		  abi_klazz->zoom_width (abi);
		  break;
		}
	    default:
			break;
	}
}

void abi_widget_set_property(GObject  *object,
							 guint	arg_id,
							 const GValue *arg,
							 GParamSpec *pspec)
{
	abi_widget_set_prop(object, arg_id, arg, pspec);
}

static void 
abi_widget_size_request (GtkWidget      *widget,
			 GtkRequisition *requisition)
{
	requisition->width = ABI_DEFAULT_WIDTH;
	requisition->height = ABI_DEFAULT_HEIGHT;

	if (ABI_WIDGET(widget)->child)
	  {
		GtkRequisition child_requisition;
      
		gtk_widget_size_request (ABI_WIDGET(widget)->child, &child_requisition);

		requisition->width = child_requisition.width;
		requisition->height = child_requisition.height;
	  }
}

//
// Needed for the gtkbin class
//
static void
abiwidget_add(GtkContainer *container,
	      GtkWidget    *widget)
{
  g_return_if_fail (container != NULL);
  g_return_if_fail (widget != NULL);

  if (GTK_CONTAINER_CLASS (parent_class)->add)
	  GTK_CONTAINER_CLASS (parent_class)->add (container, widget);

  ABI_WIDGET(container)->child = GTK_BIN (container)->child;
}

//
// Needed for the gtkbin class
//
static void
abiwidget_remove (GtkContainer *container,
		  GtkWidget    *widget)
{
  g_return_if_fail (container != NULL);
  g_return_if_fail (widget != NULL);

  if (GTK_CONTAINER_CLASS (parent_class)->remove)
    GTK_CONTAINER_CLASS (parent_class)->remove (container, widget);

  ABI_WIDGET(container)->child = GTK_BIN (container)->child;
}

//
// Needed for the gtkbin class
//
static GtkType
abiwidget_child_type  (GtkContainer     *container)
{
  if (!GTK_BIN (container)->child)
    return GTK_TYPE_WIDGET;
  else
    return GTK_TYPE_NONE;
}

static void
abi_widget_init (AbiWidget * abi)
{
	// this isn't really needed, since each widget is
	// guaranteed to be created with g_new0 and we just
	// want everything to be 0/NULL/FALSE anyway right now
	// but i'm keeping it around anyway just in case that changes
  GTK_WIDGET_SET_FLAGS (abi, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT |GTK_CAN_DEFAULT );
  GTK_WIDGET_UNSET_FLAGS (abi, GTK_NO_WINDOW);
}

static void
abi_widget_size_allocate (GtkWidget     *widget,
						  GtkAllocation *allocation)
{
	AbiWidget *abi;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_ABI_WIDGET (widget));
	g_return_if_fail (allocation != NULL);

	GtkAllocation child_allocation;
	widget->allocation = *allocation;

	gint border_width = GTK_CONTAINER (widget)->border_width;
	gint xthickness = GTK_WIDGET (widget)->style->xthickness;
	gint ythickness = GTK_WIDGET (widget)->style->ythickness;
 	if (GTK_WIDGET_REALIZED (widget))
    {
		// only allocate on realized widgets

		abi = ABI_WIDGET(widget);
		gdk_window_move_resize (widget->window,
					allocation->x+border_width, 
					allocation->y+border_width,
					allocation->width - border_width*2, 
					allocation->height - border_width*2);
		
		if (abi->child)
		{
		     child_allocation.x = xthickness;
			 child_allocation.y = ythickness;

			 child_allocation.width = MAX (1, 
										   (gint)widget->allocation.width - child_allocation.x * 2 - border_width * 2);
			 child_allocation.height = MAX (1, 
											(gint)widget->allocation.height - child_allocation.y * 2 - border_width * 2);
			 gtk_widget_size_allocate (ABI_WIDGET (widget)->child, &child_allocation);
		}
    }
}

static void
abi_widget_realize (GtkWidget * widget)
{
	AbiWidget * abi;
	GdkWindowAttr attributes;
	gint attributes_mask;

	// we *must* ensure that we get a GdkWindow to draw into
	// this here is just boilerplate GTK+ code

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_ABI_WIDGET(widget));

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	abi = ABI_WIDGET(widget);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = ABI_DEFAULT_WIDTH;
	attributes.height = ABI_DEFAULT_HEIGHT;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | 
	  GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK |
	  GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
	  GDK_POINTER_MOTION_HINT_MASK | GDK_ENTER_NOTIFY_MASK |
	  GDK_LEAVE_NOTIFY_MASK |
	  GDK_FOCUS_CHANGE_MASK |
	  GDK_STRUCTURE_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);
	
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, abi);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

	//
	// connect a signal handler to load files after abiword is in a stable
	// state.
	//
	g_signal_connect_after(G_OBJECT(widget),"map_event", 
			       G_CALLBACK (s_abi_widget_map_cb),
			       (gpointer) abi);
	//
	// connect a signal handler to the destroy signal of the window
	//
// 	g_signal_connect(G_OBJECT(widget),"delete_event", 
//					 G_CALLBACK (s_abi_widget_delete),
//					 (gpointer) abi);
//	g_signal_connect(G_OBJECT(widget),"destroy", 
//					 G_CALLBACK (s_abi_widget_delete),
//					 (gpointer) abi);
}

#ifdef HAVE_GNOME
static void
abi_widget_finalize(GObject *object)
{
	AbiWidget * abi;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ABI_WIDGET(object));

	// here we free any self-created data
	abi = ABI_WIDGET(object);

	// order of deletion is important here

	if(abi->priv->m_pApp)
	{
		if(abi->priv->m_pFrame)
		{
			abi->priv->m_pApp->forgetFrame(abi->priv->m_pFrame);
			delete abi->priv->m_pFrame;
		}
		if(!abi->priv->externalApp)
		{
			abi->priv->m_pApp->shutdown();
			delete abi->priv->m_pApp;
		}
		abi->priv->m_pApp = NULL;
	}
	g_free (abi->priv->m_szFilename);

	g_free (abi->priv);

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget finalized\n");
#endif
	// chain up
	BONOBO_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}
#endif



static void
abi_widget_destroy (GObject *object)
{
	AbiWidget * abi;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ABI_WIDGET(object));

	// here we free any self-created data
	abi = ABI_WIDGET(object);

	// order of deletion is important here

	if(abi->priv->m_pApp)
	{
		if(abi->priv->m_pFrame)
		{
			abi->priv->m_pApp->forgetFrame(abi->priv->m_pFrame);
			delete abi->priv->m_pFrame;
		}
		if(!abi->priv->externalApp)
		{
			abi->priv->m_pApp->shutdown();
			delete abi->priv->m_pApp;
		}
		abi->priv->m_pApp = NULL;
		g_free (abi->priv->m_szFilename);
		g_free (abi->priv);
		if (GTK_OBJECT_CLASS(parent_class)->destroy)
			GTK_OBJECT_CLASS(parent_class)->destroy (GTK_OBJECT(object));
	}

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget destroyed in abi_widget_destroy \n");
#endif
}


static void
abi_widget_destroy_gtk (GtkObject *object)
{
	AbiWidget * abi;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ABI_WIDGET(object));

	// here we free any self-created data
	abi = ABI_WIDGET(object);

	// order of deletion is important here
	bool bBonobo = false;
	bool bKillApp = false;
	if(abi->priv->m_pApp)
	{
		bBonobo = abi->priv->m_pApp->isBonoboRunning();
		if(abi->priv->m_pFrame)
		{
#ifdef LOGFILE
	fprintf(getlogfile(),"frame count before forgetting = %d \n",abi->priv->m_pApp->getFrameCount());
#endif
			if(abi->priv->m_pApp->getFrameCount() <= 1)
			{
				bKillApp = true;
			}
			abi->priv->m_pApp->forgetFrame(abi->priv->m_pFrame);
			abi->priv->m_pFrame->close();
			delete abi->priv->m_pFrame;
#ifdef LOGFILE
	fprintf(getlogfile(),"frame count = %d \n",abi->priv->m_pApp->getFrameCount());
#endif
		}
		if(!abi->priv->externalApp)
		{
			abi->priv->m_pApp->shutdown();
			delete abi->priv->m_pApp;
			bKillApp = true;
		}
	}
	g_free (abi->priv->m_szFilename);

	g_free (abi->priv);

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget destroyed in abi_widget_destroy_gtk\n");
#endif
	if(!bBonobo)
	{
		if (GTK_OBJECT_CLASS(parent_class)->destroy)
			GTK_OBJECT_CLASS(parent_class)->destroy (GTK_OBJECT(object));
		if(bKillApp)
		{
			gtk_main_quit();
		}
	}
#ifdef HAVE_GNOME
	else
	{
		
		if (GTK_OBJECT_CLASS(parent_class)->destroy)
			GTK_OBJECT_CLASS(parent_class)->destroy (GTK_OBJECT(object));
	// chain up
		BONOBO_CALL_PARENT (BONOBO_OBJECT_CLASS, destroy, BONOBO_OBJECT(object));
		if(bKillApp)
		{
			bonobo_main_quit();
		}
	}
#endif
}

#ifdef HAVE_GNOME

static void
abi_widget_bonobo_destroy (BonoboObject *object)
{
	AbiWidget * abi;
	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ABI_WIDGET(object));

	// here we free any self-created data
	abi = ABI_WIDGET(object);

	// order of deletion is important here

	if(abi->priv->m_pApp)
	{
		if(abi->priv->m_pFrame)
		{
			abi->priv->m_pApp->forgetFrame(abi->priv->m_pFrame);
			delete abi->priv->m_pFrame;
		}
		if(!abi->priv->externalApp)
		{
			abi->priv->m_pApp->shutdown();
			delete abi->priv->m_pApp;
		}
		abi->priv->m_pApp = NULL;
	}
	g_free (abi->priv->m_szFilename);

	g_free (abi->priv);

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget destroyed in bonobo_destroy \n");
#endif
	// chain up
	BONOBO_CALL_PARENT (BONOBO_OBJECT_CLASS, destroy, BONOBO_OBJECT(object));
}
#endif


static void
abi_widget_class_init (AbiWidgetClass *abi_class)
{

#ifdef LOGFILE
	fprintf(getlogfile(),"Abi_widget class init \n");
#endif

	GtkObjectClass * object_class;
	GtkWidgetClass * widget_class;
	GtkContainerClass *container_class;
	container_class = (GtkContainerClass*) abi_class;

	object_class = (GtkObjectClass *)abi_class;
	widget_class = (GtkWidgetClass *)abi_class;
//
// Next to install properties to the object class too
	GObjectClass *gobject_class = G_OBJECT_CLASS(abi_class);

	// we need our own special destroy function
	XAP_App * pApp = XAP_App::getApp();
#ifdef HAVE_GNOME
	if(pApp->isBonoboRunning())
	{
		BonoboObjectClass *bonobo_object_class = (BonoboObjectClass *)abi_class;
		bonobo_object_class->destroy = abi_widget_bonobo_destroy;
//		gobject_class->finalize = abi_widget_finalize;
	}
#endif

	// set our parent class
	parent_class = (GtkBinClass *)
		gtk_type_class (gtk_bin_get_type());
	
	// set our custom destroy method
	object_class->destroy = abi_widget_destroy_gtk;
	gobject_class->set_property = abi_widget_set_prop;
	gobject_class->get_property = abi_widget_get_prop;

	// set our custom class methods
	widget_class->realize       = abi_widget_realize;
	widget_class->size_request  = abi_widget_size_request;
   	widget_class->size_allocate = abi_widget_size_allocate; 

	// For the container methods
	container_class->add = abiwidget_add;
	container_class->remove = abiwidget_remove;
	container_class->child_type = abiwidget_child_type;


	// AbiWidget's master "invoke" method
	abi_class->invoke    = abi_widget_invoke;
	abi_class->invoke_ex = abi_widget_invoke_ex;

	// assign handles to Abi's edit methods
	abi_class->align_center  = EM_NAME(alignCenter);
	abi_class->align_justify = EM_NAME(alignLeft);
	abi_class->align_left    = EM_NAME(alignRight);
	abi_class->align_right   = EM_NAME(alignJustify);
	
	abi_class->copy          = EM_NAME(copy);
	abi_class->cut           = EM_NAME(cut);
	abi_class->paste         = EM_NAME(paste);
	abi_class->paste_special = EM_NAME(pasteSpecial);
	abi_class->select_all    = EM_NAME(selectAll);
	abi_class->select_block  = EM_NAME(selectBlock);
	abi_class->select_line   = EM_NAME(selectLine);	
	abi_class->select_word   = EM_NAME(selectWord);	
	
	abi_class->undo = EM_NAME(undo);
	abi_class->redo = EM_NAME(redo);
	
	abi_class->insert_data  = EM_NAME(insertData);
	abi_class->insert_space = EM_NAME(insertSpace);		
        
	abi_class->delete_bob   = EM_NAME(delBOB);
	abi_class->delete_bod   = EM_NAME(delBOD);
	abi_class->delete_bol   = EM_NAME(delBOL);
	abi_class->delete_bow   = EM_NAME(delBOW);
	abi_class->delete_eob   = EM_NAME(delEOB);
	abi_class->delete_eod   = EM_NAME(delEOD);
	abi_class->delete_eol   = EM_NAME(delEOL);
	abi_class->delete_eow   = EM_NAME(delEOW);
	abi_class->delete_left  = EM_NAME(delLeft);
	abi_class->delete_right = EM_NAME(delRight);
	
	abi_class->edit_header   = EM_NAME(editHeader);
	abi_class->edit_footer   = EM_NAME(editFooter);
	abi_class->remove_header = EM_NAME(removeHeader);
	abi_class->remove_footer = EM_NAME(removeFooter);
	
	abi_class->select_bob         = EM_NAME(extSelBOB);
	abi_class->select_bod         = EM_NAME(extSelBOD);
	abi_class->select_bol         = EM_NAME(extSelBOL);
	abi_class->select_bow         = EM_NAME(extSelBOW);
	abi_class->select_eob         = EM_NAME(extSelEOB);
	abi_class->select_eod         = EM_NAME(extSelEOD);
	abi_class->select_eol         = EM_NAME(extSelEOL);
	abi_class->select_eow         = EM_NAME(extSelEOW);    
	abi_class->select_left        = EM_NAME(extSelLeft);
	abi_class->select_next_line   = EM_NAME(extSelNextLine);
	abi_class->select_page_down   = EM_NAME(extSelPageDown);
	abi_class->select_page_up     = EM_NAME(extSelPageUp);
	abi_class->select_prev_line   = EM_NAME(extSelPrevLine);
	abi_class->select_right       = EM_NAME(extSelRight);
	abi_class->select_screen_down = EM_NAME(extSelScreenDown);
	abi_class->select_screen_up   = EM_NAME(extSelScreenUp);
	abi_class->select_to_xy       = EM_NAME(extSelToXY);
	
	abi_class->toggle_bold        = EM_NAME(toggleBold);
	abi_class->toggle_bottomline  = EM_NAME(toggleBottomline);
	abi_class->toggle_insert_mode = EM_NAME(toggleInsertMode);
	abi_class->toggle_italic      = EM_NAME(toggleItalic);
	abi_class->toggle_overline    = EM_NAME(toggleOline);
	abi_class->toggle_plain       = EM_NAME(togglePlain);
	abi_class->toggle_strike      = EM_NAME(toggleStrike);
	abi_class->toggle_sub         = EM_NAME(toggleSub);
	abi_class->toggle_super       = EM_NAME(toggleSuper);
	abi_class->toggle_topline     = EM_NAME(toggleTopline);
	abi_class->toggle_underline   = EM_NAME(toggleUline);
	abi_class->toggle_unindent    = EM_NAME(toggleUnIndent);
	
	abi_class->view_formatting_marks = EM_NAME(viewPara);
	abi_class->view_print_layout     = EM_NAME(viewPrintLayout);
	abi_class->view_normal_layout    = EM_NAME(viewNormalLayout);
	abi_class->view_online_layout    = EM_NAME(viewWebLayout);
	
	abi_class->moveto_bob         = EM_NAME(warpInsPtBOB);
	abi_class->moveto_bod         = EM_NAME(warpInsPtBOD);
	abi_class->moveto_bol         = EM_NAME(warpInsPtBOL);
	abi_class->moveto_bop         = EM_NAME(warpInsPtBOP);
	abi_class->moveto_bow         = EM_NAME(warpInsPtBOW);
	abi_class->moveto_eob         = EM_NAME(warpInsPtEOB);
	abi_class->moveto_eod         = EM_NAME(warpInsPtEOD);
	abi_class->moveto_eol         = EM_NAME(warpInsPtEOL);
	abi_class->moveto_eop         = EM_NAME(warpInsPtEOP);
	abi_class->moveto_eow         = EM_NAME(warpInsPtEOW);
	abi_class->moveto_left        = EM_NAME(warpInsPtLeft);
	abi_class->moveto_next_line   = EM_NAME(warpInsPtNextLine);
	abi_class->moveto_next_page   = EM_NAME(warpInsPtNextPage);
	abi_class->moveto_next_screen = EM_NAME(warpInsPtNextScreen);
	abi_class->moveto_prev_line   = EM_NAME(warpInsPtPrevLine);
	abi_class->moveto_prev_page   = EM_NAME(warpInsPtPrevPage);
	abi_class->moveto_prev_screen = EM_NAME(warpInsPtPrevScreen);
	abi_class->moveto_right       = EM_NAME(warpInsPtRight);
	abi_class->moveto_to_xy       = EM_NAME(warpInsPtToXY);
	
	abi_class->zoom_100   = EM_NAME(zoom100);
	abi_class->zoom_200   = EM_NAME(zoom200);
	abi_class->zoom_50    = EM_NAME(zoom50);
	abi_class->zoom_75    = EM_NAME(zoom75);
	abi_class->zoom_whole = EM_NAME(zoomWhole);
	abi_class->zoom_width = EM_NAME(zoomWidth);


// now install GObject properties

	g_object_class_install_property (gobject_class,
									 CURSOR_ON,
									 g_param_spec_boolean("AbiWidget--cursoron",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 INVOKE_NOARGS,
									 g_param_spec_string("AbiWidget--invoke_noargs",
														 NULL,
														 NULL,
														 NULL,
														 (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 MAP_TO_SCREEN,
									 g_param_spec_boolean("AbiWidget--map_to_screen",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 UNLINK_AFTER_LOAD,
									 g_param_spec_boolean("AbiWidget--unlink_after_load",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 IS_ABI_WIDGET,
									 g_param_spec_boolean("AbiWidget--is_abi_widget",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 DRAW,
									 g_param_spec_boolean("AbiWidget--draw",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 LOAD_FILE,
									 g_param_spec_string("AbiWidget--load-file",
														 NULL,
														 NULL,
													   NULL,
														 (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 ALIGNCENTER,
									 g_param_spec_boolean("AbiWidget--aligncenter",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 ALIGNLEFT,
									 g_param_spec_boolean("AbiWidget--alignleft",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 ALIGNRIGHT,
									 g_param_spec_boolean("AbiWidget--alignright",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 ALIGNJUSTIFY,
									 g_param_spec_boolean("AbiWidget--alignjustify",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 COPY,
									 g_param_spec_boolean("AbiWidget--copy",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 CUT,
									 g_param_spec_boolean("AbiWidget--cut",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 PASTE,
									 g_param_spec_boolean("AbiWidget--paste",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 PASTESPECIAL,
									 g_param_spec_boolean("AbiWidget--pastespecial",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 SELECTBLOCK,
									 g_param_spec_boolean("AbiWidget--selectblock",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 SELECTLINE,
									 g_param_spec_boolean("AbiWidget--selectline",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 SELECTWORD,
									 g_param_spec_boolean("AbiWidget--selectword",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 SELECTALL,
									 g_param_spec_boolean("AbiWidget--selectall",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));

	g_object_class_install_property (gobject_class,
									 INSERTDATA,
									 g_param_spec_string("AbiWidget--insertdata",
														 NULL,
														 NULL,
													   NULL,
														 (GParamFlags) G_PARAM_READWRITE));

	  g_object_class_install_property(gobject_class,
									  INSERTSPACE,
									  g_param_spec_boolean("AbiWidget--insertspace",
														   NULL,
														   NULL,
														   FALSE,
														   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELBOB,
								  g_param_spec_boolean("AbiWidget--delbob",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELBOD,
								  g_param_spec_boolean("AbiWidget--delbod",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELBOL,
								  g_param_spec_boolean("AbiWidget--delbol",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELBOW,
								  g_param_spec_boolean("AbiWidget--delbow",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELEOB,
								  g_param_spec_boolean("AbiWidget--deleob",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELEOD,
								  g_param_spec_boolean("AbiWidget--deleod",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELEOL,
								  g_param_spec_boolean("AbiWidget--deleol",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELEOW,
								  g_param_spec_boolean("AbiWidget--deleow",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELLEFT,
								  g_param_spec_boolean("AbiWidget--delleft",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  DELRIGHT,
								  g_param_spec_boolean("AbiWidget--delright",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EDITHEADER,
								  g_param_spec_boolean("AbiWidget--editheader",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EDITFOOTER,
								  g_param_spec_boolean("AbiWidget--editfooter",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  REMOVEHEADER,
								  g_param_spec_boolean("AbiWidget--removeheader",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  REMOVEFOOTER,
								  g_param_spec_boolean("AbiWidget--removefooter",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELBOB,
								  g_param_spec_boolean("AbiWidget--extselbob",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELBOL,
								  g_param_spec_boolean("AbiWidget--extselboL",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELBOD,
								  g_param_spec_boolean("AbiWidget--extselbod",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELBOW,
								  g_param_spec_boolean("AbiWidget--extselbow",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELEOB,
								  g_param_spec_boolean("AbiWidget--extseleob",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELEOD,
								  g_param_spec_boolean("AbiWidget--extseleod",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELEOL,
								  g_param_spec_boolean("AbiWidget--extseleol",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELEOW,
								  g_param_spec_boolean("AbiWidget--extseleow",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELLEFT,
								  g_param_spec_boolean("AbiWidget--extselleft",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELRIGHT,
								  g_param_spec_boolean("AbiWidget--extselright",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELNEXTLINE,
								  g_param_spec_boolean("AbiWidget--extselnextline",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELPAGEDOWN,
								  g_param_spec_boolean("AbiWidget--extselpagedown",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELPAGEUP,
								  g_param_spec_boolean("AbiWidget--extselpageup",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELPREVLINE,
								  g_param_spec_boolean("AbiWidget--extselprevline",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELSCREENDOWN,
								  g_param_spec_boolean("AbiWidget--extselscreendown",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  EXTSELSCREENUP,
								  g_param_spec_boolean("AbiWidget--extselscreenup",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLEBOLD,
								  g_param_spec_boolean("AbiWidget--togglebold",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLEBOTTOMLINE,
								  g_param_spec_boolean("AbiWidget--togglebottomline",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLEINSERTMODE,
								  g_param_spec_boolean("AbiWidget--toggleinsertmode",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLEITALIC,
								  g_param_spec_boolean("AbiWidget--toggleitalic",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLEOLINE,
								  g_param_spec_boolean("AbiWidget--toggleoline",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLEPLAIN,
								  g_param_spec_boolean("AbiWidget--toggleplain",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLESTRIKE,
								  g_param_spec_boolean("AbiWidget--togglestrike",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLESUB,
								  g_param_spec_boolean("AbiWidget--togglesub",
													   NULL,
													   NULL,
													   FALSE,
													   (GParamFlags) G_PARAM_READWRITE));
  g_object_class_install_property(gobject_class,
								  TOGGLESUPER,
								  g_param_spec_boolean("AbiWidget--togglesuper",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  TOGGLETOPLINE,
								  g_param_spec_boolean("AbiWidget--toggletopline",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  TOGGLEULINE,
								  g_param_spec_boolean("AbiWidget--toggleuline",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  TOGGLEUNINDENT,
								  g_param_spec_boolean("AbiWidget--toggleunindent",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  VIEWPARA,
								  g_param_spec_boolean("AbiWidget--viewpara",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  VIEWPRINTLAYOUT,
								  g_param_spec_boolean("AbiWidget--viewprintlayout",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  VIEWNORMALLAYOUT,
								  g_param_spec_boolean("AbiWidget--viewnormallayout",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  VIEWWEBLAYOUT,
								  g_param_spec_boolean("AbiWidget--viewweblayout",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  UNDO,
								  g_param_spec_boolean("AbiWidget--undo",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  REDO,
								  g_param_spec_boolean("AbiWidget--redo",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTBOB,
								  g_param_spec_boolean("AbiWidget--warpinsptbob",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTBOD,
								  g_param_spec_boolean("AbiWidget--warpinsptbod",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTBOL,
								  g_param_spec_boolean("AbiWidget--warpinsptbol",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTBOP,
								  g_param_spec_boolean("AbiWidget--warpinsptbop",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTBOW,
								  g_param_spec_boolean("AbiWidget--warpinsptbow",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTEOB,
								  g_param_spec_boolean("AbiWidget--warpinspteob",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTEOD,
								  g_param_spec_boolean("AbiWidget--warpinspteod",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTEOL,
								  g_param_spec_boolean("AbiWidget--warpinspteol",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTEOP,
								  g_param_spec_boolean("AbiWidget--warpinspteop",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTEOW,
								  g_param_spec_boolean("AbiWidget--warpinspteow",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTLEFT,
								  g_param_spec_boolean("AbiWidget--warpinsptleft",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTNEXTLINE,
								  g_param_spec_boolean("AbiWidget--warpinsptnextline",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTNEXTPAGE,
								  g_param_spec_boolean("AbiWidget--warpinsptnextpage",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTNEXTSCREEN,
								  g_param_spec_boolean("AbiWidget--warpinsptnextscreen",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTPREVLINE,
								  g_param_spec_boolean("AbiWidget--warpinsptprevline",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTPREVPAGE,
								  g_param_spec_boolean("AbiWidget--warpinsptprevpage",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTPREVSCREEN,
								  g_param_spec_boolean("AbiWidget--warpinsptprevscreen",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  WARPINSPTPREVRIGHT,
								  g_param_spec_boolean("AbiWidget--warpinsptprevright",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  ZOOM100,
								  g_param_spec_boolean("AbiWidget--zoom100",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  ZOOM200,
								  g_param_spec_boolean("AbiWidget--zoom200",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  ZOOM50,
								  g_param_spec_boolean("AbiWidget--zoom50",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  ZOOM75,
								  g_param_spec_boolean("AbiWidget--zoom75",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  ZOOMWHOLE,
								  g_param_spec_boolean("AbiWidget--zoomwhole",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
  g_object_class_install_property(gobject_class,
								  ZOOMWIDTH,
								  g_param_spec_boolean("AbiWidget--zoomwidth",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
}

static void
abi_widget_construct (AbiWidget * abi, const char * file, AP_UnixApp * pApp)
{
	AbiPrivData * priv = g_new0 (AbiPrivData, 1);
	priv->m_pFrame = NULL;
	priv->m_szFilename = NULL;
	priv->m_bMappedToScreen = false;
	priv->m_bPendingFile = false;
	priv->m_bMappedEventProcessed = false;
	priv->m_bUnlinkFileAfterLoad = false;
	priv->m_iNumFileLoads = 0;
	if(pApp == NULL)
	{
		priv->m_pApp = NULL;
		priv->externalApp = false;
	}
	else
	{
		priv->m_pApp = pApp;
		priv->externalApp = true;
	}
	// this is all that we can do here, because we can't draw until we're
	// realized and have a GdkWindow pointer

	if (file)
		priv->m_szFilename = g_strdup (file);

	abi->priv = priv;
#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget Constructed %x \n",abi);
#endif
	vecAbi.addItem((void *) abi);
}

/**************************************************************************/
/**************************************************************************/

extern "C" void 
abi_widget_map_to_screen(AbiWidget * abi)
{
  g_return_if_fail (abi != 0);
  UT_DEBUGMSG(("Doing map_to_screen \n"));
  GtkWidget * widget = GTK_WIDGET(abi);

  // now we can set up Abi inside of this GdkWindow

#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget about to map_to_screen \n");
	fprintf(getlogfile(),"AbiWidget about to map_to_screen ref_count %d \n",G_OBJECT(abi)->ref_count);
#endif

	XAP_Args *pArgs = 0;
	abi->priv->m_bMappedToScreen = true;
	if(!abi->priv->externalApp)
	{
		if (abi->priv->m_szFilename)
			// C++ casts don't seem to work here
			pArgs = new XAP_Args (1, (const char **)(&abi->priv->m_szFilename));
		else
			pArgs = new XAP_Args(0, 0);

		AP_UnixApp * pApp = static_cast<AP_UnixApp*>(XAP_App::getApp());

		if ( !pApp )
		    pApp = new AP_UnixApp (pArgs, "AbiWidget");

		UT_ASSERT(pApp);
		pApp->initialize(true);
		abi->priv->m_pApp     = pApp;
	}

	AP_UnixFrame * pFrame  = new AP_UnixFrame(abi->priv->m_pApp);

	UT_ASSERT(pFrame);
	static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl())->setTopLevelWindow(widget);
	pFrame->initialize(XAP_NoMenusWindowLess);
	abi->priv->m_pFrame   = pFrame;
	if(!abi->priv->externalApp)
		delete pArgs;

	abi->priv->m_pApp->rememberFrame ( pFrame ) ;
	abi->priv->m_pApp->rememberFocussedFrame ( pFrame ) ;

#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget After Finished map_to_screen ref_count %d \n",G_OBJECT(abi)->ref_count);
#endif
}

extern "C" void 
abi_widget_turn_on_cursor(AbiWidget * abi)
{
	if(abi->priv->m_pFrame)
	{
		g_return_if_fail (abi != 0);
		FV_View * pV = static_cast<FV_View*>(abi->priv->m_pFrame->getCurrentView());
		if(pV)
			pV->focusChange(AV_FOCUS_HERE);
	}
}

extern "C" GtkType
abi_widget_get_type (void)
{
	// boilerplate code

	static GtkType abi_type = 0;

	if (!abi_type){
		GtkTypeInfo info = {
			"AbiWidget",
			sizeof (AbiWidget),
			sizeof (AbiWidgetClass),
			reinterpret_cast<GtkClassInitFunc>(abi_widget_class_init),
			reinterpret_cast<GtkObjectInitFunc>(abi_widget_init),
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			static_cast<GtkClassInitFunc>(NULL)
		};
		
		abi_type = gtk_type_unique (gtk_bin_get_type (), &info);
	}
	
	return abi_type;
}

/**
 * abi_widget_new
 *
 * Creates a new AbiWord widget using an internal Abiword App
 */
extern "C" GtkWidget *
abi_widget_new (void)
{
	AbiWidget * abi;

	abi = static_cast<AbiWidget *>(gtk_type_new (abi_widget_get_type ()));
	abi_widget_construct (abi, 0, NULL);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_file
 *
 * Creates a new AbiWord widget and tries to load the file
 * This uses an internal Abiword App
 *
 * \param file - A file on your HD
 */
extern "C" GtkWidget *
abi_widget_new_with_file (const gchar * file)
{
	AbiWidget * abi;

	g_return_val_if_fail (file != 0, 0);

	abi = static_cast<AbiWidget *>(gtk_type_new (abi_widget_get_type ()));
	abi_widget_construct (abi, file,NULL);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_app
 *
 * Creates a new AbiWord widget using an external Abiword App
 */
extern "C" GtkWidget *
abi_widget_new_with_app (AP_UnixApp * pApp)
{
	AbiWidget * abi;

	g_return_val_if_fail (pApp != 0, 0);

	abi = static_cast<AbiWidget *>(gtk_type_new (abi_widget_get_type ()));
	abi_widget_construct (abi, 0, pApp);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_app_file
 *
 * Creates a new AbiWord widget and tries to load the file
 * This uses an external Abiword App
 *
 * \param file - A file on your HD
 * \param pApp - pointer to a valid AbiWord XAP_UnixApp
 */
extern "C" GtkWidget *
abi_widget_new_with_app_file (AP_UnixApp * pApp, const gchar * file)
{
	AbiWidget * abi;

	g_return_val_if_fail (file != 0, 0);
	g_return_val_if_fail (pApp != 0, 0);

	abi = static_cast<AbiWidget *>(gtk_type_new (abi_widget_get_type ()));
	abi_widget_construct (abi, file, pApp);

	return GTK_WIDGET (abi);
}

extern "C" XAP_Frame * 
abi_widget_get_frame ( AbiWidget * w )
{
  g_return_val_if_fail ( w != NULL, NULL ) ;
  return w->priv->m_pFrame ;
}

#ifdef HAVE_GNOME
extern "C" void
abi_widget_set_Bonobo_uic(AbiWidget * w, BonoboUIComponent * uic)
{
	w->priv->m_uic = uic;
}

extern "C" BonoboUIComponent *
abi_widget_get_Bonobo_uic(AbiWidget * w)
{
	return 	w->priv->m_uic;
}

#endif
/**
 * abi_widget_invoke()
 *
 * Invoke any of abiword's edit methods by name
 *
 * \param w - An AbiWord widget
 * \param mthdName - A null-terminated string representing the method's name
 *
 * \return FALSE if any preconditions fail
 * \return TRUE|FALSE depending on the result of the EditMethod's completion
 *
 * example usage:
 *
 * gboolean b = FALSE; 
 * GtkWidget * w = abi_widget_new ();
 *
 * b = abi_widget_invoke (ABI_WIDGET(w), "alignCenter");
 *
 */
extern "C" gboolean
abi_widget_invoke (AbiWidget * w, const char * mthdName)
{
  return abi_widget_invoke_ex ( w, mthdName, NULL, 0, 0 ) ;
}

/**
 * abi_widget_invoke_ex()
 *
 * Invoke any of abiword's edit methods by name
 *
 * \param w - An AbiWord widget
 * \param mthdName - A null-terminated string representing the method's name
 * \param data - an optional null-terminated string data to pass to the method
 * \param x - an optional x-coordinate to pass to the method (usually 0)
 * \param y - an optional y-coordinate to pass to the method (usuall 0)
 *
 * \return FALSE if any preconditions fail
 * \return TRUE|FALSE depending on the result of the EditMethod's completion
 *
 * example usage:
 *
 * gboolean b = FALSE; 
 * GtkWidget * w = abi_widget_new ();
 *
 * b = abi_widget_invoke_ex (ABI_WIDGET(w), "insertData", "Hello World!", 0, 0);
 * b = abi_widget_invoke_ex (ABI_WIDGET(w), "alignCenter", 0, 0, 0);
 *
 */
extern "C" gboolean
abi_widget_invoke_ex (AbiWidget * w, const char * mthdName, 
		      const char * data, gint32 x, gint32 y)
{
	EV_EditMethodContainer * container;
	EV_EditMethod          * method;
	AV_View                * view;

	// lots of conditional returns - code defensively
	g_return_val_if_fail (w != 0, FALSE);
	g_return_val_if_fail (mthdName != 0, FALSE);
	g_return_val_if_fail (w->priv->m_pApp != 0, FALSE);

	// get the method container
	container = w->priv->m_pApp->getEditMethodContainer();
	g_return_val_if_fail (container != 0, FALSE);

	// get a handle to the actual EditMethod
	method = container->findEditMethodByName (mthdName);
	g_return_val_if_fail (method != 0, FALSE);

	// get a valid frame
	g_return_val_if_fail (w->priv->m_pFrame != 0, FALSE);

	// obtain a valid view
	view = w->priv->m_pFrame->getCurrentView();
	g_return_val_if_fail (view != 0, FALSE);

	// construct the call data
	EV_EditMethodCallData calldata(data, (data ? strlen (data) : 0));
	calldata.m_xPos = x;
	calldata.m_yPos = y;

	// actually invoke
	return (method->Fn(view, &calldata) ? TRUE : FALSE);
}

extern "C" void
abi_widget_draw (AbiWidget * w)
{
	// obtain a valid view
	if(w->priv->m_pFrame)
	{
		// obtain a valid view
		g_return_if_fail (w != NULL);
		FV_View * view = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
		if(view)
			view->draw();
	}
}

extern "C" gboolean
abi_widget_save ( AbiWidget * w, const char * fname )
{
  return abi_widget_save_ext ( w, fname, ".abw" ) ;
}

extern "C" gboolean 
abi_widget_save_ext ( AbiWidget * w, const char * fname,
		      const char * extension )
{
  g_return_val_if_fail ( w != NULL, FALSE );
  g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
  g_return_val_if_fail ( fname != NULL, FALSE );

  FV_View * view = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
  if(view == NULL)
	  return false;
  PD_Document * doc = view->getDocument () ;

  // start out saving as abiword format by default
  IEFileType ieft = IE_Exp::fileTypeForSuffix ( ".abw" ) ;

  if ( extension != NULL && strlen ( extension ) > 0 && extension[0] == '.' )
    ieft = IE_Exp::fileTypeForSuffix ( extension ) ;

  return ( doc->saveAs ( fname, ieft ) == UT_OK ? TRUE : FALSE ) ;
}
