
/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <glib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixWidget.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Latex_Advanced.h"
#include "xap_Dlg_MessageBox.h"

data d;

static gboolean s_delete_clicked(GtkWidget * /*widget*/, GdkEvent * /*event*/, AP_UnixDialog_Latex_Advanced * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return TRUE;
}

static void s_close_clicked(GtkWidget * /*widget*/,AP_UnixDialog_Latex_Advanced * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Close();
}

static void s_destroy_clicked(GtkWidget * /*widget*/,AP_UnixDialog_Latex_Advanced * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Close();
}

static void s_insert_clicked(GtkWidget * /*widget*/,AP_UnixDialog_Latex_Advanced * dlg)
{
	UT_ASSERT(dlg);
	dlg->event_Insert();
}

static void s_set_data(GtkWidget * /**widget*/, gpointer d1)
{
	d.s = (SymbolButton *)d1;
}

int flag =0;

static void updateText ()
{
	GString *itex;
	char *itex_iter;
	char *prev_char = '\0';
	size_t size_utf8;
	unsigned int i;
	int n_unclosed_braces = 0;
	int j;
	gboolean add_dash = FALSE;

	gchar * sz = NULL;
	GtkTextBuffer * buffer = d.buffer;//gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	GtkTextIter startIter,endIter;
	gtk_text_buffer_get_start_iter  (buffer,&startIter);
	gtk_text_buffer_get_end_iter    (buffer,&endIter);
	sz = gtk_text_buffer_get_text   (buffer,&startIter,&endIter,TRUE);


	GtkTextMark *  initialPos = gtk_text_buffer_get_insert(buffer);
	UT_DEBUGMSG(("Got initial pos\n"));
	GtkTextIter * temp;
	gtk_text_buffer_get_iter_at_mark (buffer, temp ,initialPos);
	UT_DEBUGMSG(("Got temp\n"));


	if (sz != NULL && !g_utf8_validate (sz, -1, NULL)) 
	{
		g_free (sz);
		sz = NULL;
	}

	if (sz != NULL) 
	{
		size_utf8 = g_utf8_strlen (sz, -1);

		if (size_utf8 > 0) 
		{
			for (i = 0, itex_iter = sz; i < size_utf8; i++, itex_iter = g_utf8_next_char (itex_iter)) 
			{
				if (*itex_iter != ' ') 
				{
					if (*itex_iter == '{' && (prev_char == NULL || *prev_char != '\\'))
						n_unclosed_braces++;
					else if (*itex_iter == '}' && (prev_char != NULL || *prev_char != '\\'))
						n_unclosed_braces--;
				}

				prev_char = itex_iter;
			}
			if (prev_char != NULL && (*prev_char == '^' || *prev_char == '_'))
				add_dash = TRUE;

		}
	}

	itex = g_string_new (sz);
	if (add_dash)
		g_string_append_c (itex, '-');
	for (j = 0; j < n_unclosed_braces; j++)
		g_string_append_c (itex, '}');

	printf("%s\n",itex->str);
	gtk_text_buffer_set_text(buffer,itex->str,-1);

	g_string_free (itex, TRUE);
	gtk_text_buffer_move_mark(buffer, initialPos, temp);
	UT_DEBUGMSG(("Moved mark\n"));
	flag =0;
}

static void update (GtkTextBuffer * /*textbuffer*/, gpointer user_data)
{
	if(flag==0)
	{
		flag=1;
		updateText();
	}
}
static void insert_symbol(GtkWidget * /*widget*/, gpointer d1)
{
	//update();
	gtk_text_buffer_insert_at_cursor(d.buffer,d.s->itex,-1);//directly access buffer
	
}
gboolean is_new_category(gchar* line)
{
	if(strncmp(line,"GREEK",5)==0)
		return TRUE;
	if(strncmp(line,"MATH",4)==0)
		return TRUE;
	if(strncmp(line,"OPERATORS",9)==0)
		return TRUE;
	if(strncmp(line,"ARROWS",6)==0)
		return TRUE;
	if(strncmp(line,"OTHER",5)==0)
		return TRUE;
	else
		return FALSE;
}

Category get_category(gchar* line)
{
	if(strncmp(line,"GREEK",5)==0)
		return GREEK;
	if(strncmp(line,"MATH",4)==0)
		return MATH;
	if(strncmp(line,"OPERATORS",9)==0)
		return OPERATORS;
	if(strncmp(line,"ARROWS",6)==0)
		return ARROWS;
	if(strncmp(line,"OTHER",5)==0)
		return OTHER;
	else
		return OTHER;
}

char *category_heading(Category c)
{
    char *headings[] = {"Greek","Math","Operators","Arrows","Others" };
    return headings[c];
}


void load_list(GPtrArray *list)
{
	UT_UTF8String path(XAP_App::getApp()->getAbiSuiteLibDir());
	path += "/xsltml/symbols.txt";

	char ** toks;

  	Category current=OTHER;
	FILE *file = fopen ( path.utf8_str(), "r" );
	 if (file != NULL) 
  	 {
   	 	char line [1000];
		gchar* copy;
   	 	while(fgets(line,sizeof line,file)!= NULL) 
   	 	{	
			copy = g_strdup(line);
			toks = (char**) strsep(&copy,";");

			if(is_new_category(line))
				current = get_category(line);
			else
			{
				struct SymbolButton *s = g_new0(struct SymbolButton, 1);
				s->itex = g_strdup((gchar*)toks);

				gchar *label_name = g_strdup((gchar*)copy);
				s->button = gtk_button_new_with_label(label_name);			
										
				s->cat = current;
				gtk_widget_set_size_request(s->button, 30, 30);
				g_ptr_array_add(list, s);
			}
  	 	}
  	 	UT_DEBUGMSG(("Entities Loaded\n"));
  		fclose(file);
  	}
  	else
  	{
   		 perror(path.utf8_str());
  	}
}


XAP_Dialog * AP_UnixDialog_Latex_Advanced::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Latex_Advanced(pFactory,id);
}

AP_UnixDialog_Latex_Advanced::AP_UnixDialog_Latex_Advanced(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : 
  AP_Dialog_Latex_Advanced(pDlgFactory,id),
  m_wClose(NULL), 
  m_wInsert(NULL),
  m_wText(NULL),
  m_windowMain(NULL)
{
  UT_DEBUGMSG(("Constructing Latex dialog %p \n",this));
}

AP_UnixDialog_Latex_Advanced::~AP_UnixDialog_Latex_Advanced(void)
{
}

void  AP_UnixDialog_Latex_Advanced::activate(void)
{
	// FIXME move to XP
	UT_ASSERT (m_windowMain);
	
	ConstructWindowName();

	gdk_window_raise (gtk_widget_get_window(m_windowMain));
}

void AP_UnixDialog_Latex_Advanced::runModeless(XAP_Frame * pFrame)
{
	constructDialog();
	UT_return_if_fail(m_windowMain);

	abiSetupModelessDialog(GTK_DIALOG(m_windowMain), pFrame, this, 
						   GTK_RESPONSE_CLOSE);
	gtk_widget_show(m_windowMain);
}


void AP_UnixDialog_Latex_Advanced::event_Insert(void)
{
	getLatexFromGUI();
	if (convertLatexToMathML())
	{
		insertIntoDoc();
	}
}

void AP_UnixDialog_Latex_Advanced::event_Close(void)
{
	destroy();
}

void AP_UnixDialog_Latex_Advanced::event_WindowDelete(void)
{
	destroy();
}

void AP_UnixDialog_Latex_Advanced::notifyActiveFrame(XAP_Frame * /*pFrame*/)
{
	// FIXME put that in XP code
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW(m_windowMain), m_sWindowName.utf8_str());
}

void AP_UnixDialog_Latex_Advanced::destroy(void)
{
	m_answer = AP_Dialog_Latex_Advanced::a_CANCEL;	
	modeless_cleanup();
	if (m_windowMain != NULL)
	{
		gtk_widget_destroy(m_windowMain);
		m_windowMain = NULL;
	}
}

void AP_UnixDialog_Latex_Advanced::setLatexInGUI(void)
{
	UT_UTF8String sLatex;
	getLatex(sLatex);
	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	gtk_text_buffer_set_text (buffer, sLatex.utf8_str(), -1);
}

bool AP_UnixDialog_Latex_Advanced::getLatexFromGUI(void)
{
	UT_UTF8String sLatex;

	//
	// Get the chars from the widget
	//
	gchar * sz = NULL;
	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	GtkTextIter startIter,endIter;
	gtk_text_buffer_get_start_iter  (buffer,&startIter);
	gtk_text_buffer_get_end_iter    (buffer,&endIter);
	sz = gtk_text_buffer_get_text   (buffer,&startIter,&endIter,TRUE);
	sLatex = sz;
	g_free(sz);
	UT_DEBUGMSG(("LAtex from widget is %s \n",sLatex.utf8_str()));
	setLatex(sLatex);

	return true;
}


/*****************************************************************/
GtkWidget*	AP_UnixDialog_Latex_Advanced::getDialog()
{
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Latex_Advanced.ui");
	m_windowMain   = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Latex"));
	return m_windowMain;
}
void AP_UnixDialog_Latex_Advanced::constructDialog(void)
{	
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	// load the dialog from the UI file
#if GTK_CHECK_VERSION(3,0,0)
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_Latex_Advanced.ui");
#else
	GtkBuilder* builder = newDialogBuilder("AP_UnixDialog_Latex_Advanced-2.ui");
#endif

        // Update our member variables with the important widgets that
        // might need to be queried or altered later
	m_windowMain   = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Latex"));
	m_wClose = GTK_WIDGET(gtk_builder_get_object(builder, "wClose"));
	m_wInsert =  GTK_WIDGET(gtk_builder_get_object(builder, "wInsert"));
	m_wText = GTK_WIDGET(gtk_builder_get_object(builder, "wTextView"));

	d.buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (m_wText));
	d.view = m_wText; 
	//d.buffer = buffer;

	// localize the strings in our dialog, and set tags for some widgets

	localizeButtonUnderline(m_wInsert, pSS, AP_STRING_ID_DLG_InsertButton);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbLatexEquation")), pSS, AP_STRING_ID_DLG_Latex_LatexEquation);
	
	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbExample")), pSS, AP_STRING_ID_DLG_Latex_Example);

	ConstructWindowName();
	gtk_window_set_title (GTK_WINDOW(m_windowMain), m_sWindowName.utf8_str());

	GPtrArray *list =  g_ptr_array_new ();  
	//Load entities
	load_list(list);

	GtkWidget* notebook = GTK_WIDGET(gtk_builder_get_object(builder, "wNoteBook"));
	//GtkWidget* label, grid;
	int j=0,pos=0;
	Category prev=GREEK;
	GtkWidget* label = gtk_label_new (category_heading(prev));
	GtkWidget* grid = gtk_grid_new();
	for(j=0;j<224;j++)
	{
		struct SymbolButton *s = (SymbolButton*) g_ptr_array_index(list,j);
		if(s->cat!=prev)
		{	
			//display the current grid
			label = gtk_label_new (category_heading(prev));
			
			gtk_widget_show (grid);
			gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, label);	
			//UT_DEBUGMSG((category_heading(prev)));		
			//New category elements in new grid/page			
			grid = gtk_grid_new();
			prev = s->cat;
			pos=0;
		}
		gtk_widget_set_size_request(s->button, 30, 30);
		gtk_grid_attach(GTK_GRID (grid),s->button,(pos)%15,pos/15,1,1);
		pos++;
	}
	//display last category
	label = gtk_label_new (category_heading(prev));
	gtk_widget_show (grid);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, label);			
	UT_DEBUGMSG(("prev"));
	UT_DEBUGMSG((category_heading(prev)));
	gint i;
	for(i=0;i<224;i++)
	{
		struct SymbolButton *s = (SymbolButton *)g_ptr_array_index(list,i);
		g_signal_connect(G_OBJECT(s->button), "clicked", G_CALLBACK(s_set_data), reinterpret_cast<gpointer>(s));
		g_signal_connect(G_OBJECT(s->button), "clicked", G_CALLBACK(insert_symbol), reinterpret_cast<gpointer>(&d));
	}

	g_signal_connect(G_OBJECT(m_windowMain), "delete_event",
					   G_CALLBACK(s_delete_clicked),
					   reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_windowMain), "destroy",
					   G_CALLBACK(s_destroy_clicked),
					   reinterpret_cast<gpointer>(this));
	g_signal_connect(G_OBJECT(m_wClose), "clicked",
					   G_CALLBACK(s_close_clicked),
					   reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wInsert), "clicked",
					   G_CALLBACK(s_insert_clicked),
					   reinterpret_cast<gpointer>(this));

	g_signal_connect(d.buffer, "end-user-action",
					   G_CALLBACK(update),
					   NULL);


	gtk_widget_show_all (m_windowMain);

	g_object_unref(G_OBJECT(builder));
}

