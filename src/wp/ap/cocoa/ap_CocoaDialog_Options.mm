/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002-2003 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_CocoaGraphics.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#include "xap_Prefs.h"
#include "xap_CocoaDialog_Utilities.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#import "ap_CocoaDialog_Options.h"

/*!
	This class is a proxy class to allow accessing some members
	that are protected in AP_CocoaDialog_Options.
	
	Note: the whole class methods are inline static and not meant 
	to be constructed.
 */
class AP_CocoaDialog_OptionsController_proxy
{
public:
	static void _event_DictionaryEdit(AP_CocoaDialog_Options *obj)
		{
			obj->_event_DictionaryEdit();
		};
	static void _event_SetDefaults(AP_CocoaDialog_Options *obj)
		{
			obj->_event_SetDefaults();
		};
	static void _event_IgnoreEdit(AP_CocoaDialog_Options *obj)
		{
			obj->_event_IgnoreEdit();
		};
	static void _event_IgnoreReset(AP_CocoaDialog_Options *obj)
		{
			obj->_event_IgnoreReset();
		};
private:
	AP_CocoaDialog_OptionsController_proxy ();	//don't allow contruction
};

#if 0
/*****************************************************************/

//
// For Screen color picker
	enum
	{
		RED,
		GREEN,
		BLUE,
		OPACITY
	};

static void s_radio_toggled (GtkWidget * w, GtkWidget * c)
{
  GtkCList * clist = GTK_CLIST (c);
  gboolean b = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w));
  int row = GPOINTER_TO_INT (g_object_get_user_data (G_OBJECT (w)));

  xxx_UT_DEBUGMSG(("DOM: toggled row: %d val: %d\n", row, b));

  gtk_clist_set_row_data (clist, row, GINT_TO_POINTER(b));
}
#endif

XAP_Dialog * AP_CocoaDialog_Options::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id dlgid)
{
    AP_CocoaDialog_Options * p = new AP_CocoaDialog_Options(pFactory,dlgid);
    return p;
}

AP_CocoaDialog_Options::AP_CocoaDialog_Options(XAP_DialogFactory * pDlgFactory,
					     XAP_Dialog_Id dlgid)
  : AP_Dialog_Options(pDlgFactory, dlgid),
	m_dlg (nil)
{
}

AP_CocoaDialog_Options::~AP_CocoaDialog_Options(void)
{
}

/*****************************************************************/

void AP_CocoaDialog_Options::runModal(XAP_Frame * pFrame)
{
	if (m_dlg == nil) {
		m_dlg = [[AP_CocoaDialog_OptionsController alloc] initFromNib];
		[m_dlg setXAPOwner:this];
	}

    // save for use with event
    m_pFrame = pFrame;

	NSWindow *win = [m_dlg window];		// force the window to be loaded.
    // Populate the window's data items
    _populateWindowData();
	_initCocoaOnlyPrefs();

    do {
		[NSApp runModalForWindow:win];

		switch ( m_answer )
		{
		case AP_Dialog_Options::a_OK:
			_storeWindowData();
			break;
	
		case AP_Dialog_Options::a_APPLY:
			UT_DEBUGMSG(("Applying changes\n"));
			_storeWindowData();
			break;
	
		case AP_Dialog_Options::a_CANCEL:
			break;
	
		default:
			UT_ASSERT_NOT_REACHED();
			break;
		};

    } while ( m_answer == AP_Dialog_Options::a_APPLY );

	[m_dlg close];		// close before release because of the NSTableView data source
	[m_dlg release];
	m_dlg = nil;
}


#if 0

void AP_CocoaDialog_Options::event_clistClicked (int row, int col)
{
  GtkCList * clist = GTK_CLIST (m_toolbarClist);
  bool b = (bool)GPOINTER_TO_INT(gtk_clist_get_row_data (clist, row));

  g_object_set_user_data (G_OBJECT(m_checkbuttonViewShowTB), GINT_TO_POINTER(row));
  xxx_UT_DEBUGMSG (("DOM: setting row %d to %d\n", row, b));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonViewShowTB), (b ? TRUE : FALSE));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (m_checkbuttonViewHideTB), (b ? FALSE : TRUE));
}

///
/// All this color selection code is stolen from the ap_CocoaDialog_Background
/// dialog
///
#define CTI(c, v) (unsigned char)(c[v] * 255.0)

/* static */ void AP_CocoaDialog_Options::s_color_changed(GtkWidget * csel,
			    AP_CocoaDialog_Options * dlg)
{
  UT_ASSERT(csel && dlg);

  char color[10];

  GtkColorSelection * w = GTK_COLOR_SELECTION(csel);

  gdouble cur [4];

  gtk_color_selection_get_color (w, cur);
  sprintf(color,"#%02x%02x%02x",CTI(cur, RED), CTI(cur, GREEN), CTI(cur, BLUE));

  strncpy(dlg->m_CurrentTransparentColor,(const XML_Char *) color,9);
}

#undef CTI
#endif

void AP_CocoaDialog_Options::event_ChooseTransparentColor(void)
{
#if 0
//
// Run the Background dialog over the options? No the title is wrong.
//
  GtkWidget * dlg;
  GtkWidget * k;
  GtkWidget * actionarea;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  dlg = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW(dlg),
						pSS->getValue(AP_STRING_ID_DLG_Options_Label_ColorChooserLabel));

  actionarea = GTK_DIALOG (dlg)->action_area;

  k = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Close));
  gtk_widget_show(k);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dlg)->action_area),
                               k, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT(k), "clicked",
		      G_CALLBACK(gtk_main_quit), (gpointer)this);

  g_signal_connect_after(G_OBJECT(dlg),
			   "destroy",
			   NULL,
			   NULL);

  g_signal_connect(G_OBJECT(dlg),
		     "delete_event",
		     G_CALLBACK(gtk_main_quit),
		     (gpointer) this);

  GtkWidget *colorsel;

  colorsel = gtk_color_selection_new();
  gtk_widget_show (colorsel);
  UT_DEBUGMSG(("SEVIOR: About to add color selector to dialog window \n"));
  gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dlg)->vbox), colorsel);
  UT_DEBUGMSG(("SEVIOR: Added color selector to dialog window \n"));
  UT_RGBColor c;
  UT_parseColor(m_CurrentTransparentColor,c);

  gdouble currentColor[4] = { 0, 0, 0, 0 };
  currentColor[RED] = ((gdouble) c.m_red / (gdouble) 255.0);
  currentColor[GREEN] = ((gdouble) c.m_grn / (gdouble) 255.0);
  currentColor[BLUE] = ((gdouble) c.m_blu / (gdouble) 255.0);

  gtk_color_selection_set_color (GTK_COLOR_SELECTION(colorsel),
				 currentColor);

  g_signal_connect (G_OBJECT(colorsel), "color-changed",
		      G_CALLBACK(s_color_changed),
		      (gpointer) this);

//
// Do all the nice stuff and put the color selector on top of our current
// dialog.
//
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
	centerDialog(m_windowMain,dlg);

	// Show the top level dialog,
	gtk_widget_show(dlg);

	// Make it modal, and stick it up top
	gtk_grab_add(dlg);

	// run into the gtk main loop for this window
	gtk_main();

//
// Finish up here after a close or window delete signal.
//
	if(dlg && GTK_IS_WIDGET(dlg))
		gtk_widget_destroy(dlg);
#endif
}

#if 0
void AP_CocoaDialog_Options::event_AllowTransparentColor(void)
{
//
// If this button is up we do not allow transparent color
//
	if(!GTK_TOGGLE_BUTTON (m_checkbuttonTransparentIsWhite)->active)
	{
		strncpy(m_CurrentTransparentColor,(const XML_Char *) "ffffff",9);
		gtk_widget_set_sensitive(m_pushbuttonNewTransparentColor,FALSE);
	}
	else
		gtk_widget_set_sensitive(m_pushbuttonNewTransparentColor,TRUE);
}

#endif
void AP_CocoaDialog_Options::event_OK(void)
{
    m_answer = AP_Dialog_Options::a_OK;
	[NSApp stopModal];
}

void AP_CocoaDialog_Options::event_Cancel(void)
{
    m_answer = AP_Dialog_Options::a_CANCEL;
	[NSApp stopModal];
}

void AP_CocoaDialog_Options::event_Apply(void)
{
    m_answer = AP_Dialog_Options::a_APPLY;
	[NSApp stopModal];
}

/*!
	Enable a control
 */
void AP_CocoaDialog_Options::_controlEnable( tControl ctlid, bool value )
{
	UT_ASSERT (m_dlg);
	NSControl *w = [m_dlg _lookupWidget:ctlid];

	if (w) {
	  [w setEnabled:(value?YES:NO)];
	}
}


#define DEFINE_CLIST_GET_SET_BOOL(itm, row) \
bool AP_CocoaDialog_Options::_gather##itm(void) { \
		NSTableView * list = [m_dlg _lookupWidget:id_LIST_VIEW_TOOLBARS]; \
        UT_ASSERT (list); \
		bool b = [list isRowSelected:row]; \
        return b; \
} \
void AP_CocoaDialog_Options::_set##itm(bool b) { \
 		NSTableView * list = [m_dlg _lookupWidget:id_LIST_VIEW_TOOLBARS]; \
		UT_ASSERT (list); \
		if (b) { \
			[list selectRow:row byExtendingSelection:YES]; \
		} \
		else { \
			[list deselectRow:row]; \
		} \
}

DEFINE_CLIST_GET_SET_BOOL(ViewShowStandardBar, 0);
DEFINE_CLIST_GET_SET_BOOL(ViewShowFormatBar, 1);
DEFINE_CLIST_GET_SET_BOOL(ViewShowExtraBar, 2);
#undef DEFINE_CLIST_GET_SET_BOOL


#define DEFINE_GET_SET_BOOL(button, btnId) \
bool     AP_CocoaDialog_Options::_gather##button(void) {				\
	NSButton * btn = [m_dlg _lookupWidget:btnId]; \
	UT_ASSERT(btn); \
	return ([btn state] != NSOffState); }\
void        AP_CocoaDialog_Options::_set##button(bool b) {	\
	NSButton * btn = [m_dlg _lookupWidget:btnId]; \
	UT_ASSERT(btn); \
	[btn setState:(b?NSOnState:NSOffState)]; }

#define DEFINE_GET_SET_TEXT(widget, btnId) \
char *		AP_CocoaDialog_Options::_gather##widget() {				\
	NSButton * txt = [m_dlg _lookupWidget:btnId]; \
	UT_ASSERT(txt); \
	NSString * str = [txt textValue]; \
	return [str cString]; }			\
\
void		AP_CocoaDialog_Options::_set##widget(const char *t) {	\
	NSButton * txt = [m_dlg _lookupWidget:btnId]; \
	UT_ASSERT(txt); \
	NSString * str = [NSString stringWithUTF8String:t]; \
	[txt setTextValue:str]; \
}

DEFINE_GET_SET_BOOL(SpellCheckAsType, id_CHECK_SPELL_CHECK_AS_TYPE);
DEFINE_GET_SET_BOOL(SpellHideErrors, id_CHECK_SPELL_HIDE_ERRORS);
DEFINE_GET_SET_BOOL(SpellSuggest, id_CHECK_SPELL_SUGGEST);
DEFINE_GET_SET_BOOL(SpellMainOnly, id_CHECK_SPELL_MAIN_ONLY);
DEFINE_GET_SET_BOOL(SpellUppercase, id_CHECK_SPELL_UPPERCASE);
DEFINE_GET_SET_BOOL(SpellNumbers, id_CHECK_SPELL_NUMBERS);
DEFINE_GET_SET_BOOL(SpellInternet, id_CHECK_SPELL_INTERNET);
DEFINE_GET_SET_BOOL(SmartQuotesEnable, id_CHECK_SMART_QUOTES_ENABLE);

DEFINE_GET_SET_BOOL(OtherDirectionRtl, id_CHECK_OTHER_DEFAULT_DIRECTION_RTL);
DEFINE_GET_SET_BOOL(OtherUseContextGlyphs, id_CHECK_OTHER_USE_CONTEXT_GLYPHS);
DEFINE_GET_SET_BOOL(OtherSaveContextGlyphs, id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS);
DEFINE_GET_SET_BOOL(OtherHebrewContextGlyphs, id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS);

DEFINE_GET_SET_BOOL(AutoSaveFile, id_CHECK_AUTO_SAVE_FILE);
DEFINE_GET_SET_BOOL(ShowSplash, id_SHOWSPLASH);
DEFINE_GET_SET_BOOL(PrefsAutoSave, id_CHECK_PREFS_AUTO_SAVE);
DEFINE_GET_SET_BOOL(ViewShowRuler, id_CHECK_VIEW_SHOW_RULER);
DEFINE_GET_SET_BOOL(ViewShowStatusBar, id_CHECK_VIEW_SHOW_STATUS_BAR);

void AP_CocoaDialog_Options::_gatherAutoSaveFileExt(UT_String &stRetVal)
{
	NSTextField * txt = [m_dlg _lookupWidget:id_TEXT_AUTO_SAVE_FILE_EXT];
	NSString * str = [txt stringValue];
	stRetVal = [str cString];
}

void AP_CocoaDialog_Options::_setAutoSaveFileExt(const UT_String &stExt)
{
	NSTextField * txt = [m_dlg _lookupWidget:id_TEXT_AUTO_SAVE_FILE_EXT];
	NSString * str = [NSString stringWithUTF8String:stExt.c_str()];
	[txt setStringValue:str];
}


void AP_CocoaDialog_Options::_gatherAutoSaveFilePeriod(UT_String &stRetVal)
{
	NSTextField * txt = [m_dlg _lookupWidget:AP_Dialog_Options::id_TEXT_AUTO_SAVE_FILE_PERIOD];
	NSNumberFormatter * formatter = [txt formatter];
	
//	UT_ASSERT(m_textAutoSaveFilePeriod && GTK_IS_SPIN_BUTTON(m_textAutoSaveFilePeriod));
//	char nb[12];
//	int val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod));
//	g_snprintf(nb, 12, "%d", val);
//	stRetVal = nb;
}

void AP_CocoaDialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	NSTextField * txt = [m_dlg _lookupWidget:AP_Dialog_Options::id_TEXT_AUTO_SAVE_FILE_PERIOD];
	NSNumberFormatter * formatter = [txt formatter];

//	UT_ASSERT(m_textAutoSaveFilePeriod && GTK_IS_EDITABLE(m_textAutoSaveFilePeriod));
//	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod), atoi(stPeriod.c_str()));
}


UT_Dimension AP_CocoaDialog_Options::_gatherViewRulerUnits(void)
{
	NSPopUpButton * popup = [m_dlg _lookupWidget:AP_Dialog_Options::id_LIST_VIEW_RULER_UNITS];
	NSMenuItem * item = [popup itemAtIndex:[popup indexOfSelectedItem]];
	return (UT_Dimension)[item tag];
}

fp_PageSize::Predefined AP_CocoaDialog_Options::_gatherDefaultPageSize(void)
{
	NSPopUpButton * popup = [m_dlg _lookupWidget:AP_Dialog_Options::id_LIST_DEFAULT_PAGE_SIZE];
	NSMenuItem * item = [popup itemAtIndex:[popup indexOfSelectedItem]];
	return (fp_PageSize::Predefined)[item tag];
}

#if 0

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// This function will lookup a option box by the value stored in the
//	user data under the key WIDGET_MENU_VALUE_TAG
//
typedef struct {
	int index;
	int found;
	gchar *key;
	gpointer data;
} search_data;

static void search_for_value ( GtkWidget *widget, gpointer _value )
{
	search_data *value = (search_data *)_value;

	if ( !GTK_IS_MENU_ITEM(widget))
		return;

	value->index++;

	gint v = (gint) g_object_get_data( G_OBJECT(widget), value->key );
	if ( v == (gint)value->data )
	{
		// UT_DEBUGMSG(("search_for_value [%d]", (gint) value->data ));
		value->found = value->index;
	}
}

// returns -1 if not found
int option_menu_set_by_key ( GtkWidget *option_menu, gpointer value, gchar *key )
{
	UT_ASSERT( option_menu && key && GTK_IS_OPTION_MENU(option_menu));

	// at least make sure the value will be restored by the _gather
	g_object_set_data( G_OBJECT(option_menu), key, value);

	// lookup for the key with the value of dim
	search_data data = { -1, -1, key, value };

	GtkWidget *menu = gtk_option_menu_get_menu( GTK_OPTION_MENU(option_menu));
	UT_ASSERT(menu&&GTK_IS_MENU(menu));

	// iterate through all the values
	gtk_container_forall ( GTK_CONTAINER(menu), search_for_value, (gpointer) &data );

	// if we found a value that matches, then say select it
	if ( data.found >= 0 )
	{
		gtk_option_menu_set_history( GTK_OPTION_MENU(option_menu), data.found );
		//UT_DEBUGMSG(("search found %d\n", data.found ));
	}
	else
		UT_DEBUGMSG(("%s:%f search NOT found (searched %d indexes)\n", __FILE__, __LINE__, data.index ));

	return data.found;
}
#endif

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void    AP_CocoaDialog_Options::_setViewRulerUnits(UT_Dimension dim)
{
#if 0
	UT_ASSERT(m_listViewRulerUnits);

	int r = option_menu_set_by_key ( m_listViewRulerUnits, (gpointer)dim, WIDGET_MENU_VALUE_TAG );
	UT_ASSERT( r != -1 );
#endif
}

void AP_CocoaDialog_Options::_setDefaultPageSize(fp_PageSize::Predefined pre)
{
#if 0
	UT_ASSERT(m_listDefaultPageSize);

	int r = option_menu_set_by_key ( m_listDefaultPageSize, (gpointer)pre, WIDGET_MENU_VALUE_TAG );
	UT_ASSERT( r != -1 );
#endif
}


DEFINE_GET_SET_BOOL	(ViewCursorBlink, id_CHECK_VIEW_CURSOR_BLINK);

DEFINE_GET_SET_BOOL	(ViewAll, id_CHECK_VIEW_ALL);
DEFINE_GET_SET_BOOL	(ViewHiddenText, id_CHECK_VIEW_HIDDEN_TEXT);
DEFINE_GET_SET_BOOL	(ViewUnprintable, id_CHECK_VIEW_UNPRINTABLE);
DEFINE_GET_SET_BOOL (AllowCustomToolbars, id_CHECK_ALLOW_CUSTOM_TOOLBARS);
DEFINE_GET_SET_BOOL (AutoLoadPlugins, id_CHECK_AUTO_LOAD_PLUGINS);

#undef DEFINE_GET_SET_BOOL


int AP_CocoaDialog_Options::_gatherNotebookPageNum(void)
{
	NSTabView * tab = [m_dlg _lookupWidget:AP_Dialog_Options::id_NOTEBOOK];
	return [tab indexOfTabViewItem:[tab selectedTabViewItem]];
}

void    AP_CocoaDialog_Options::_setNotebookPageNum(int pn)
{
	NSTabView * tab = [m_dlg _lookupWidget:AP_Dialog_Options::id_NOTEBOOK];
	[tab selectTabViewItemAtIndex:pn];
}

#if 0

/*****************************************************************/

/*static*/ void AP_CocoaDialog_Options::s_delete_clicked(GtkWidget * /* widget */, GdkEvent * /*event*/, gpointer data )
{
	AP_CocoaDialog_Options * dlg = (AP_CocoaDialog_Options *)data;
	UT_ASSERT(dlg);
	UT_DEBUGMSG(("AP_CocoaDialog_Options::s_delete_clicked\n"));
	dlg->event_WindowDelete();
}



/*static*/ void AP_CocoaDialog_Options::s_allowTransparentColor( GtkWidget *widget, gpointer data )
{
	AP_CocoaDialog_Options * dlg = (AP_CocoaDialog_Options *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_AllowTransparentColor();
}


// these function will allow multiple widget to tie into the same logic
// function (at the AP level) to enable/disable stuff
/*static*/ void AP_CocoaDialog_Options::s_checkbutton_toggle( GtkWidget *w, gpointer data )
{
	AP_CocoaDialog_Options * dlg = (AP_CocoaDialog_Options *)data;
	UT_ASSERT(dlg);
	UT_ASSERT(w && GTK_IS_WIDGET(w));

	int i = (int) g_object_get_data( G_OBJECT(w), "tControl" );
	UT_DEBUGMSG(("s_checkbutton_toggle: control id = %d\n", i));
	dlg->_enableDisableLogic( (AP_Dialog_Options::tControl) i );
}

/*static*/ gint AP_CocoaDialog_Options::s_menu_item_activate(GtkWidget * widget, gpointer data )
{
	AP_CocoaDialog_Options * dlg = (AP_CocoaDialog_Options *)data;

	UT_ASSERT(widget && dlg);

	GtkWidget *option_menu = (GtkWidget *)g_object_get_data(G_OBJECT(widget),
												 WIDGET_MENU_OPTION_PTR);
	UT_ASSERT( option_menu && GTK_IS_OPTION_MENU(option_menu));

	gpointer p = g_object_get_data( G_OBJECT(widget),
												WIDGET_MENU_VALUE_TAG);

	g_object_set_data( G_OBJECT(option_menu), WIDGET_MENU_VALUE_TAG, p );

	//TODO: This code is now shared between RulerUnits and DefaultPaperSize
	//so anyone who wants to resurect this msg. needs to add a conditional
	//UT_DEBUGMSG(("s_menu_item_activate [%d %s]\n", p, UT_dimensionName( (UT_Dimension)((UT_uint32)p)) ) );

	return TRUE;
}

/* static */ void AP_CocoaDialog_Options::s_clist_clicked (GtkWidget *w, gint row, gint col,
							  GdkEvent *evt, gpointer d)
{
  AP_CocoaDialog_Options * dlg = static_cast <AP_CocoaDialog_Options *>(d);
  dlg->event_clistClicked (row, col);
}

#endif

void AP_CocoaDialog_Options::_initCocoaOnlyPrefs()
{
}

void AP_CocoaDialog_Options::_saveCocoaOnlyPrefs()
{
}

void AP_CocoaDialog_Options::_storeWindowData(void)
{
	_saveCocoaOnlyPrefs();
	AP_Dialog_Options::_storeWindowData();
}


@implementation AP_CocoaDialog_OptionsController

- (id)initFromNib
{
	self = [super initWithWindowNibName:@"ap_CocoaDialog_Options"];
	return self;
}

- (oneway void)dealloc
{
	// perhaps shall we dealloc super before release the source because that crash
	// therefore we close the dialog before release
	[m_tlbTlbListDataSource release];
	[super dealloc];
}

- (void)windowDidLoad
{
	XAP_Frame *pFrame = m_xap->_getFrame ();
	// we get all our strings from the application string set
	const XAP_StringSet * pSS = pFrame->getApp()->getStringSet();

	// insert translation code here.
	
	//set Tab label
	LocalizeControl([m_tab tabViewItemAtIndex:0], pSS, AP_STRING_ID_DLG_Options_Label_Toolbars);
	LocalizeControl(m_tlbTlbBox, pSS, AP_STRING_ID_DLG_Options_Label_Toolbars);
	// add the items
	
	m_tlbTlbListDataSource = [[XAP_StringListDataSource alloc] init];
	[m_tlbTlbListDataSource addString:[NSString stringWithUTF8String:pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewStandardTB)]];
	[m_tlbTlbListDataSource addString:[NSString stringWithUTF8String:pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewFormatTB)]];
	[m_tlbTlbListDataSource addString:[NSString stringWithUTF8String:pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewExtraTB)]];
	[m_tlbTlbList setDataSource:m_tlbTlbListDataSource];	// setDataSource DO NOT retain the data source
	
	LocalizeControl(m_tlbVisibleBox, pSS, AP_STRING_ID_DLG_Options_Label_Visible);
	LocalizeControl([m_tlbShowHideGroup cellAtRow:0 column:0], pSS, AP_STRING_ID_DLG_Options_Label_Show);
	LocalizeControl([m_tlbShowHideGroup cellAtRow:1 column:0], pSS, AP_STRING_ID_DLG_Options_Label_Hide);
	LocalizeControl(m_tlbBtnStylBox, pSS, AP_STRING_ID_DLG_Options_Label_Look);
	LocalizeControl([m_tlbBtnStylGroup cellAtRow:0 column:0], pSS, AP_STRING_ID_DLG_Options_Label_Icons);
	LocalizeControl([m_tlbBtnStylGroup cellAtRow:1 column:0], pSS, AP_STRING_ID_DLG_Options_Label_Text);
	LocalizeControl([m_tlbBtnStylGroup cellAtRow:2 column:0], pSS, AP_STRING_ID_DLG_Options_Label_Both);
	LocalizeControl(m_tlbViewTooltipBtn, pSS, AP_STRING_ID_DLG_Options_Label_ViewTooltips);
}

- (void)setXAPOwner:(AP_CocoaDialog_Options *)owner
{
	m_xap = owner;
}

- (NSView *)_lookupWidget:(AP_Dialog_Options::tControl)controlId
{
	switch (controlId)
	{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// spell
	case AP_Dialog_Options::id_CHECK_SPELL_CHECK_AS_TYPE:
		return m_spellCheckAsTypeBtn;

	case AP_Dialog_Options::id_CHECK_SPELL_HIDE_ERRORS:
		return m_spellHideErrBtn;

	case AP_Dialog_Options::id_CHECK_SPELL_SUGGEST:
		return m_spellAlwaysSuggBtn;

	case AP_Dialog_Options::id_CHECK_SPELL_MAIN_ONLY:
		return m_spellSuggFromMainDictBtn;

	case AP_Dialog_Options::id_CHECK_SPELL_UPPERCASE:
		return m_spellIgnoreUppercaseBtn;

	case AP_Dialog_Options::id_CHECK_SPELL_NUMBERS:
		return m_spellIgnoreWordsWithNumBtn;

	case AP_Dialog_Options::id_CHECK_SPELL_INTERNET:
		return m_spellIgnoreFileAddrBtn;

	case AP_Dialog_Options::id_LIST_DICTIONARY:
		return m_spellDictionaryPopup;

	case AP_Dialog_Options::id_BUTTON_DICTIONARY_EDIT:
		return m_spellDictEditBtn;	

	case AP_Dialog_Options::id_BUTTON_IGNORE_RESET:
		return m_spellResetDictBtn;

	case AP_Dialog_Options::id_BUTTON_IGNORE_EDIT:
		return m_spellIgnoreEditBtn;  

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// other
	case AP_Dialog_Options::id_CHECK_SMART_QUOTES_ENABLE:
		return m_layoutEnableSmartQuotesBtn;

	case AP_Dialog_Options::id_LIST_DEFAULT_PAGE_SIZE:
		return m_layoutDefaultPageSizePopup;

	case AP_Dialog_Options::id_SHOWSPLASH:
		return m_prefsShowSplashBtn;

	case AP_Dialog_Options::id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
		return m_prefsDefaultToRTLBtn;

	case AP_Dialog_Options::id_CHECK_OTHER_USE_CONTEXT_GLYPHS:
		return m_prefsOtherUseContextGlyphsBtn;

/* FIXME. Currently not implemented according to the ap_UnixDialog.
	case AP_Dialog_Options::id_CHECK_OTHER_SAVE_CONTEXT_GLYPHS:
		return m_checkbuttonOtherSaveContextGlyphs;
*/
	case AP_Dialog_Options::id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS:
		return m_prefsOtherHebrwContextGlyphBtn;

	case AP_Dialog_Options::id_CHECK_AUTO_SAVE_FILE:
		return m_prefsAutoSaveCurrentBtn;

	case AP_Dialog_Options::id_TEXT_AUTO_SAVE_FILE_EXT:
		return m_prefsWithExtField;

	case AP_Dialog_Options::id_TEXT_AUTO_SAVE_FILE_PERIOD:
		return m_prefsAutoSaveMinField;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// prefs
	case AP_Dialog_Options::id_CHECK_PREFS_AUTO_SAVE:
		return m_prefsAutoSavePrefsBtn;

	case AP_Dialog_Options::id_COMBO_PREFS_SCHEME:
		return m_prefsCurrentSetCombo;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// view
	case AP_Dialog_Options::id_CHECK_VIEW_SHOW_RULER:
		return m_layoutRulerBtn;

	case AP_Dialog_Options::id_LIST_VIEW_RULER_UNITS:
		return m_layoutUnitsPopup;

	case AP_Dialog_Options::id_CHECK_VIEW_CURSOR_BLINK:
		return m_layoutCursorBlinkBtn;

	case AP_Dialog_Options::id_CHECK_VIEW_SHOW_STATUS_BAR:
		return m_layoutStatusBarBtn;

	case AP_Dialog_Options::id_CHECK_VIEW_ALL:
		return m_layoutViewAllBtn;

	case AP_Dialog_Options::id_CHECK_VIEW_HIDDEN_TEXT:
		return m_layoutHiddenTextBtn;

	case AP_Dialog_Options::id_CHECK_VIEW_UNPRINTABLE:
		return m_layoutInvisbleMarksBtn;

	case AP_Dialog_Options::id_CHECK_ALLOW_CUSTOM_TOOLBARS:
		return m_layoutCustomToolbarBtn;

	case AP_Dialog_Options::id_CHECK_AUTO_LOAD_PLUGINS:
		return m_prefsLoadAllPluginsBtn;

	case AP_Dialog_Options::id_CHECK_COLOR_FOR_TRANSPARENT_IS_WHITE:
		return  m_layoutAllowScreenColorsBtn;

	case AP_Dialog_Options::id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT:
		return  m_layoutChooseScreenBtn;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// general

	case AP_Dialog_Options::id_BUTTON_DEFAULTS:
		return m_defaultsBtn;

	case AP_Dialog_Options::id_BUTTON_OK:
		return m_okBtn;

	case AP_Dialog_Options::id_BUTTON_CANCEL:
		return m_cancelBtn;

	case AP_Dialog_Options::id_BUTTON_APPLY:
		return m_applyBtn;

		// not implemented
	case AP_Dialog_Options::id_BUTTON_SAVE:
	case AP_Dialog_Options::id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR:
	case AP_Dialog_Options::id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR:
	case AP_Dialog_Options::id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR:
	  return nil;
	
	case AP_Dialog_Options::id_LIST_VIEW_TOOLBARS:
		return m_tlbTlbList;
	
	case AP_Dialog_Options::id_NOTEBOOK:
		return m_tab;
		
	default:
		UT_ASSERT("Unknown Widget");
		return 0;
	}

	UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
	return 0;
}


- (IBAction)applyAction:(id)sender
{
	m_xap->event_Apply();
}

- (IBAction)cancelAction:(id)sender
{
	m_xap->event_Cancel();	
}

- (IBAction)chooseDictAction:(id)sender
{
	AP_CocoaDialog_OptionsController_proxy::_event_DictionaryEdit(m_xap);
}

- (IBAction)chooseScreenAction:(id)sender
{
	m_xap->event_ChooseTransparentColor();
}

- (IBAction)defaultAction:(id)sender
{
	AP_CocoaDialog_OptionsController_proxy::_event_SetDefaults(m_xap);
}

- (IBAction)editDictAction:(id)sender
{
	AP_CocoaDialog_OptionsController_proxy::_event_IgnoreEdit(m_xap);
}

- (IBAction)increaseMinutesAction:(id)sender
{
}

- (IBAction)okAction:(id)sender
{
	m_xap->event_OK();	
}

- (IBAction)resetDictAction:(id)sender
{
	AP_CocoaDialog_OptionsController_proxy::_event_IgnoreReset(m_xap);
}

@end
