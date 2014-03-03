/*
 * AiksaurusGTK - A GTK interface to the Aiksaurus library
 * Copyright (C) 2001 by Jared Davis
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

#include "AiksaurusGTK.h"
#include "DialogMediator.h"
#include "Toolbar.h"
#include "Display.h"
#include "Replacebar.h"
#include "Exception.h"
#include <iostream>
#include <string>
#include <new>

namespace AiksaurusGTK_impl
{
    class DialogImpl : public DialogMediator
    {
        private: 
            GtkWidget* d_window_ptr;
            GtkWidget* d_layout_ptr;
            Toolbar* d_toolbar_ptr;
            Display* d_display_ptr;
            Replacebar* d_replacebar_ptr;
            
            std::string d_replacement;
            std::string d_title;
            std::string d_initialMessage;
            bool d_showreplacebar;
            
            static gint _closeDialog(GtkWidget *w, GdkEventAny *e, gpointer data);
            void _init();
           
        public:
            DialogImpl();
            virtual ~DialogImpl();
            
            const char* runThesaurus(const char* word) throw();
            void setTitle(const char* title) throw();
            void setReplacebar(bool replacebar) throw();
            void setInitialMessage(const char* message) throw(std::bad_alloc);
            
            void eventCancel() throw();
            void eventReplace(const char* replacement) throw();
            void eventSelectWord(const char* word) throw();
            void eventSearch(const char* word) throw();
    };


    DialogImpl::DialogImpl()
        : d_window_ptr(0), 
          d_title("Aiksaurus"),
          d_showreplacebar(true)
    {
    
    }

          
    DialogImpl::~DialogImpl()
    {

    }
 

    void DialogImpl::setReplacebar(bool replacebar) throw()
    {
        d_showreplacebar = replacebar;
    }

    
    void DialogImpl::setInitialMessage(const char* message) throw(std::bad_alloc)
    {
        d_initialMessage = message;
    }
    
    
    gint DialogImpl::_closeDialog(GtkWidget *w, GdkEventAny *e, gpointer data)
    {
        DialogImpl* di = static_cast<DialogImpl*>(data);
        di->d_window_ptr = 0;
        gtk_main_quit();
        return 0;
    }
    
          
    void DialogImpl::_init()
    {
        if (d_window_ptr)
        {
            gtk_widget_show(d_window_ptr);
        }

        else
        {
            d_window_ptr = gtk_dialog_new();
            gtk_widget_realize(d_window_ptr);
            
            d_layout_ptr = gtk_dialog_get_content_area (GTK_DIALOG(d_window_ptr));

            d_toolbar_ptr = new Toolbar(*this, d_window_ptr);
            d_toolbar_ptr->focus();
            d_display_ptr = new Display(*this);


            gtk_box_pack_start(
                GTK_BOX(d_layout_ptr), d_toolbar_ptr->getToolbar(), 0, 0, 4
            );
            gtk_box_pack_start(
                GTK_BOX(d_layout_ptr), d_display_ptr->getDisplay(), 1, 1, 0
            );
            
            if (d_showreplacebar)
            {
                d_replacebar_ptr = new Replacebar(*this);
                gtk_box_pack_start(
                    GTK_BOX(d_layout_ptr), d_replacebar_ptr->getReplacebar(), 0, 0, 4
                );
            }
            else
            {
                d_replacebar_ptr = 0;
            }
            
            //gtk_container_add(GTK_CONTAINER(d_window_ptr), d_layout_ptr);

            gtk_window_set_title(GTK_WINDOW(d_window_ptr), d_title.c_str());

            g_signal_connect(
                G_OBJECT(d_window_ptr), "delete_event", 
                G_CALLBACK(_closeDialog), this
            );
        }
    }
    
       
    const char* DialogImpl::runThesaurus(const char* word) throw()
    {
        try {
            
            _init();
           
            if (word)
               eventSearch(word);

            else if (d_initialMessage != "")
                d_display_ptr->showMessage(d_initialMessage.c_str());
           
            gtk_widget_show_all(d_window_ptr);
            gtk_window_set_modal(GTK_WINDOW(d_window_ptr), true);

            gtk_main();
    
            if (d_window_ptr)
            {    
                gtk_window_set_modal(GTK_WINDOW(d_window_ptr), false);
                gtk_widget_hide(d_window_ptr);
            }
        }
        catch(...) {
            std::cerr << "[AiksaurusGTK] runThesaurus() error, ignoring." << std::endl;
        }
      
        return ((d_replacement == "") ? (word) : (d_replacement.c_str()));
    }

    
    void DialogImpl::setTitle(const char* word) throw()
    {
        try {
            d_title = (word) ? (word) : ("");
        }
        catch(...) {
            std::cerr << "[AiksaurusGTK] DialogImpl::setTitle() error, ignoring." << std::endl;
        }
    }
    
    
    void DialogImpl::eventCancel() throw()
    {
        gtk_main_quit();
    }
   
    
    void DialogImpl::eventReplace(const char* replacement) throw()
    {
        try {
            d_replacement = replacement;
        }
        catch(std::bad_alloc) {
            std::cerr << Exception::CANNOT_ALLOCATE_MEMORY;
        }
        gtk_main_quit();
    }
   
    
    void DialogImpl::eventSelectWord(const char* word) throw()
    {
        if (d_replacebar_ptr)
            d_replacebar_ptr->setText(word);
    }
   
    
    void DialogImpl::eventSearch(const char* word) throw()
    {
        try {
            std::string w( (word) ? (word) : ("") );

            if (w == "")
                d_display_ptr->showMessage(d_initialMessage.c_str());
           
            else {
                d_toolbar_ptr->search(w.c_str());
                d_display_ptr->search(w.c_str());
   
                if (d_replacebar_ptr)
                    d_replacebar_ptr->setText(w.c_str());
            }
        }

        catch(std::bad_alloc) {
            std::cerr << Exception::CANNOT_ALLOCATE_MEMORY;
        }
    }

    
    AiksaurusGTK::AiksaurusGTK() 
        : d_impl_ptr(new DialogImpl())
    {
        
    }


    AiksaurusGTK::~AiksaurusGTK() 
    {
        delete d_impl_ptr;
    }

    
    void AiksaurusGTK::setTitle(const char* title) 
    {
        d_impl_ptr->setTitle(title);
    }
    

    void AiksaurusGTK::hideReplacebar() 
    {
        d_impl_ptr->setReplacebar(false);
    }
    

    void AiksaurusGTK::showReplacebar()
    {
        d_impl_ptr->setReplacebar(true);
    }

    
    void AiksaurusGTK::setInitialMessage(const char* message)
    {
        d_impl_ptr->setInitialMessage(message);
    }
    
        
    const char* AiksaurusGTK::runThesaurus(const char* word)
    {
        return d_impl_ptr->runThesaurus(word);
    }
}
