#ifndef INCLUDED_DISPLAY_H
#define INCLUDED_DISPLAY_H

#include <Aiksaurus.h>
#include <gtk/gtk.h>
#include <vector>
#include <string>
#include <new>
#include "Exception.h"

namespace AiksaurusGTK_impl 
{
    class DialogMediator;
    class Meaning;
    class Display
    {
        friend class Meaning;

        private:
            Display(const Display& rhs);
            Display& operator=(const Display& rhs);

            DialogMediator& d_mediator;
        
            Aiksaurus d_thesaurus;
            GtkWidget* d_scroller;
            GtkWidget* d_white;
            GtkWidget* d_layout;
            std::vector<Meaning*> d_meanings;

            void _handleSelection(GtkWidget* list) throw();
            void _handleClick(bool isDoubleClick, const char* text) throw(std::bad_alloc);
        
            void _resetDisplay() throw();
            
            void _createMeaning(const std::string& title, std::vector<std::string>& words) 
                throw(std::bad_alloc);
            
            void _displayResults(const char* word) throw(Exception, std::bad_alloc);
            void _displayAlternatives() throw(Exception, std::bad_alloc);
            
            void _checkThesaurus() throw(Exception);
            
            static void _initResources() throw();

        public:
            Display(DialogMediator& parent) throw();
            ~Display() throw();
        
            const Aiksaurus& getThesaurus() const throw();
            GtkWidget* getDisplay() throw();

            void search(const char* word) throw(std::bad_alloc);
            void showMessage(const char* message) throw();
    };

}

#endif // INCLUDED_DISPLAY_H
