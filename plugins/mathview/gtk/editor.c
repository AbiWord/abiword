#include <gtk/gtk.h>
#include <glib.h>

typedef enum {GREEK,MATH,OTHER} Category; //to be expanded

struct SymbolButton
{
	gchar* unicode;
	gchar* itex;
	gchar* tooltip_text;
	Category cat;
	GtkWidget *button;
};

typedef struct 
{
	GtkWidget  *view;
	GtkTextBuffer *buffer;
	struct SymbolButton *s;
}data;
GtkWidget *view;
data d;
void update ()
{
	gchar* curItex = NULL;
	gchar * sz = NULL;
	GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	GtkTextIter startIter,endIter;
	gtk_text_buffer_get_start_iter  (buffer,&startIter);
	gtk_text_buffer_get_end_iter    (buffer,&endIter);
	curItex = gtk_text_buffer_get_text   (buffer,&startIter,&endIter,TRUE); 

	GString *itex;
	char *itex_iter;
	char *prev_char = '\0';
	size_t size_utf8;
	unsigned int i, n_unclosed_braces =0;
	int j;
	gboolean add_dash = FALSE;

	if (curItex != NULL && !g_utf8_validate (curItex, -1, NULL)) {
		g_free (curItex);
		curItex = NULL;
	}

	if (curItex != NULL) {
		size_utf8 = g_utf8_strlen (curItex, -1);

		if (size_utf8 > 0) {
			for (i = 0, itex_iter = curItex;
			     i < size_utf8;
			     i++, itex_iter = g_utf8_next_char (itex_iter)) {
				if (*itex_iter != ' ') {
					if (*itex_iter == '{' && (prev_char == NULL || *prev_char != '\\'))
						n_unclosed_braces++;
					else if (*itex_iter == '}' && (prev_char != NULL || *prev_char != '\\'))
						n_unclosed_braces--;
				}

				prev_char = itex_iter;
			}
			
		}
	}

	itex = g_string_new (curItex);
	for (j = 0; j < n_unclosed_braces; j++)
		g_string_append_c (itex, '}');
	
	//UT_UTF8String sLatex;
	GtkTextBuffer * buffer2 = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
	gtk_text_buffer_set_text (buffer2, itex, -1);

	g_string_free (itex, TRUE);
}


void insert_into_doc(GtkWidget *widget, gpointer view)
{

}

void set_data(GtkWidget *widget, gpointer d1)
{
	d.s = d1;
}


void insert_symbol(GtkWidget *widget, data *d)
{
	update();
	gtk_text_buffer_insert_at_cursor(d->buffer,d->s->itex,-1);
	
}

void close_window(GtkWidget *widget)
{
	gtk_main_quit();
}

gboolean is_new_category(gchar* line)
{
	if(strncmp(line,"GREEK",5)==0)
		return TRUE;
	if(strncmp(line,"MATH",4)==0)
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
	if(strncmp(line,"OTHER",5)==0)
		return OTHER;
	else
		return OTHER;
}

char *category_heading(Category c)
{
    char *headings[] = {"Greek Letters","Math symbols","Others" };
    return headings[c];
}


void load_list(GPtrArray *list)
{
	char filename[] = "symbols.txt";
	char ** toks;
	const char delim[] = ";";
  	Category current=OTHER;
	FILE *file = fopen ( filename, "r" );
	 if (file != NULL) 
  	 {
   	 	char line [1000];
		gchar* copy;
   	 	while(fgets(line,sizeof line,file)!= NULL) 
   	 	{	
			copy = g_strdup(line);
			toks = strsep(&copy,";");
			
			if(is_new_category(line))
				current = get_category(line);
			else
			{
				struct SymbolButton *s = g_new0(struct SymbolButton, 1);
						
				//gchar* a = "\u2135";
				//printf("%s %s Test : %d\n",a,toks, strncmp(a,toks,1));
			
				gchar *label_name = g_strdup((gchar*)toks);
				toks++;
				toks=strsep(&toks,";");
				s->button = gtk_button_new_with_label(label_name);			
				s->itex = g_strdup((gchar*)toks);						
				s->cat = current;
			
				gtk_widget_set_size_request(s->button, 30, 30);
				g_ptr_array_add(list, s);
			}
  	 	}

  		fclose(file);
  	}
  	else
  	{
   		 perror(filename);
  	}
}

GtkTextBuffer *buffer;
  	GtkTextIter start, end;
  	GtkTextIter iter;

int main(int argc, char** argv) 
{

	//GtkWidget *view;
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *latex_label;
	GtkWidget *insert;
	GtkWidget *close;
	GtkWidget *notebook;
	GtkWidget *label;
	GtkWidget *grid;		

	GtkTextBuffer *buffer;
  	GtkTextIter start, end;
  	GtkTextIter iter;
	
	GPtrArray *list;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 360);
	gtk_window_set_title(GTK_WINDOW(window), "Advanced Editor");


	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);

	list =  g_ptr_array_new ();  
            
	//Load entities
	load_list(list);

	latex_label = gtk_label_new("Latex Equation :");
	gtk_widget_set_size_request(latex_label, 100, 20);
	gtk_fixed_put(GTK_FIXED(frame), latex_label, 20, 20);

	insert = gtk_button_new_with_label("Insert");
	gtk_widget_set_size_request(insert, 80, 35);
	gtk_fixed_put(GTK_FIXED(frame), insert, 50, 300);

	close = gtk_button_new_with_label("Close");
	gtk_widget_set_size_request(close, 80, 35);
	gtk_fixed_put(GTK_FIXED(frame), close, 240, 300);
	

	view = gtk_text_view_new();
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (view), GTK_WRAP_WORD);
	
	GtkWidget* scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow), 5);
   	gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolledwindow,GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrolledwindow), view);
	

	gtk_fixed_put(GTK_FIXED(frame), scrolledwindow, 30, 50);
	gtk_widget_set_size_request(scrolledwindow,450,80);
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));



	d.view = view; 
	d.buffer = buffer;
	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_fixed_put(GTK_FIXED(frame), notebook, 20, 140);

	int j=0,pos=0;
	Category prev=GREEK;

	grid = gtk_grid_new();
	for(j=0;j<5;j++)
	{
		struct SymbolButton *s = g_ptr_array_index(list,j);
		if(s->cat!=prev)
		{	
			//display the current grid
			label = gtk_label_new (category_heading(prev));
			gtk_widget_show (grid);
			gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, label);			
			//New category elements in new grid/page			
			grid = gtk_grid_new();
			prev = s->cat;
			pos=0;
		}
		gtk_widget_set_size_request(s->button, 30, 30);
		gtk_grid_attach(GTK_GRID (grid),s->button,pos++,0,1,1);
	}	
	//display last category
	label = gtk_label_new (category_heading(prev));
	gtk_widget_show (grid);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, label);			

	gtk_widget_show_all(window);

	g_signal_connect(window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	g_signal_connect(insert, "clicked", 
			G_CALLBACK(insert_into_doc), view);

	g_signal_connect(close, "clicked", 
			G_CALLBACK(close_window), view);
	

	gint i;
	for(i=0;i<5;i++)
	{
		struct SymbolButton *s = g_ptr_array_index(list,i);
		g_signal_connect(s->button, "clicked", 
				G_CALLBACK(set_data), s);
		g_signal_connect(s->button, "clicked", 
				G_CALLBACK(insert_symbol), &d);
	}
	
	gtk_main();
	return 0;
}
