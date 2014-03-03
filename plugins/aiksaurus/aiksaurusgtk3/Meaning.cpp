// only for clist. todo: use treeview
#undef GTK_DISABLE_DEPRECATED

#include "Meaning.h"
#include "Display.h"
#include <cctype>  
using namespace std;

namespace AiksaurusGTK_impl 
{
    
    static void ucwords(string& str) throw()
    {
        bool ws = true;
        for(int i = 0;i < static_cast<int>(str.size());++i)
        {
            if (isspace(str[i]))
                ws = true;
                
            else if (ws)
            {
                str[i] = toupper(str[i]);
                ws = false;
            }
        }
    }

    Meaning::Meaning(const string& title, vector<string>& words, Display& display) 
    throw(bad_alloc)
        : d_title(title), d_words(words), d_display(display), d_lists(4), d_models(4)
    {
        d_masterLayout = gtk_event_box_new();
        
        ucwords(d_title);
        
        gtk_widget_set_name(d_masterLayout, "ybg");

        d_mainLayout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add( GTK_CONTAINER(d_masterLayout), d_mainLayout );

        d_labelLayout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
        gtk_box_pack_start( GTK_BOX(d_mainLayout), d_labelLayout, 0, 0, 2);

        d_label = gtk_label_new(d_title.c_str());
        gtk_widget_set_name(d_label, "mst");
        gtk_label_set_justify( GTK_LABEL(d_label), GTK_JUSTIFY_LEFT );
        gtk_box_pack_start( GTK_BOX(d_labelLayout), d_label, 0, 0, 4 );

        d_subLayout = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start( GTK_BOX(d_mainLayout), d_subLayout, 0, 0, 0 );

        int i;
        for(i = 0;i < 4;++i)
        {
			d_models[i] = gtk_list_store_new (1, G_TYPE_STRING);
            d_lists[i] = gtk_tree_view_new_with_model(GTK_TREE_MODEL(d_models[i]));
            gtk_container_set_border_width( GTK_CONTAINER(d_lists[i]), 0 );
            gtk_widget_set_name(d_lists[i], "wbg");  // <-- this one!!
            gtk_box_pack_start( GTK_BOX(d_subLayout), d_lists[i], 1, 1, 0 );
            g_signal_connect(
                G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(d_lists[i]))), "changed", 
                G_CALLBACK(_wordclick), this
            );
        }

        int n = d_words.size();
        int q = n / 4;
        int r = n % 4;
        int stop1, stop2, stop3;
        
        switch(r)
        {
            case 0:
                stop1 = q;
                stop2 = stop1 + q;
                stop3 = stop2 + q;
                break;    
                
            case 1:
                stop1 = q + 1;
                stop2 = stop1 + q;
                stop3 = stop2 + q;
                break;
                
            case 2:
                stop1 = q + 1;
                stop2 = stop1 + (q + 1);
                stop3 = stop2 + q;
                break;
                
            default: // r = 3
                stop1 = q + 1;
                stop2 = stop1 + (q + 1);
                stop3 = stop2 + (q + 1);
                break;
        }

        GtkTreeIter iter;
       for(i = 0;i < stop1;++i)
        {
            const char* str = d_words[i].c_str();
            gtk_list_store_append(d_models[0], &iter);
			gtk_list_store_set (d_models[0], &iter, 0, const_cast<char**>(&str), -1);
        }

        for(; i < stop2;++i)
        {
            const char* str = d_words[i].c_str();
            gtk_list_store_append(d_models[1], &iter);
			gtk_list_store_set (d_models[1], &iter, 0, const_cast<char**>(&str), -1);
        }

        for(; i < stop3;++i)
        {
            const char* str = d_words[i].c_str();
            gtk_list_store_append(d_models[2], &iter);
			gtk_list_store_set (d_models[2], &iter, 0, const_cast<char**>(&str), -1);
        }

        for(; i < static_cast<int>(d_words.size());++i)
        {
            const char* str = d_words[i].c_str();
            gtk_list_store_append(d_models[3], &iter);
			gtk_list_store_set (d_models[3], &iter, 0, const_cast<char**>(&str), -1);
        }
        
    }

    Meaning::~Meaning() throw()
    {
    
    }

    GtkWidget* Meaning::getLayout() throw()
    {
        return d_masterLayout;
    }


    gint Meaning::_wordclick
    (GtkTreeSelection *sel, gpointer data)
    throw(std::bad_alloc)
    {
        Meaning *m = static_cast<Meaning*>(data);
		GtkTreeView *tv = gtk_tree_selection_get_tree_view(sel);
		GtkTreeModel *model = gtk_tree_view_get_model(tv);
        m->d_display._handleSelection(GTK_WIDGET(tv));

		GtkTreeIter iter;
		if (gtk_tree_selection_get_selected (sel, &model, &iter))
		{
            char* text;    
            gtk_tree_model_get (model, 0, &text, -1);
		    GdkEvent *e = gtk_get_current_event ();
            m->d_display._handleClick((e->type == GDK_2BUTTON_PRESS), text);
		}

        return 0;
    }


    void Meaning::unselectListsExcept(GtkWidget* list) throw()
    {
        for(int i = 0;i < static_cast<int>(d_lists.size());++i)
        {
            if (d_lists[i] != list)
                gtk_tree_selection_unselect_all(gtk_tree_view_get_selection(GTK_TREE_VIEW(d_lists[i])));
        }
    }

}

