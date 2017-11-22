#ifndef INCLUDED_DISPLAY_H
#define INCLUDED_DISPLAY_H

#include "ut_compiler.h"

#include <Aiksaurus.h>
ABI_W_NO_CONST_QUAL
#include <gtk/gtk.h>
ABI_W_POP
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

            void _handleSelection(GtkWidget* list) noexcept(false);
            void _handleClick(bool isDoubleClick, const char* text) noexcept(false);

            void _resetDisplay() noexcept(false);

            void _createMeaning(const std::string& title, std::vector<std::string>& words)
                noexcept(false);

            void _displayResults(const char* word) noexcept(false);
            void _displayAlternatives() noexcept(false);

            void _checkThesaurus() noexcept(false);

//            static void _initResources() noexcept(false);

        public:
            Display(DialogMediator& parent) noexcept(false);
            ~Display() noexcept(false);

            const Aiksaurus& getThesaurus() const noexcept(false);
            GtkWidget* getDisplay() noexcept(false);

            void search(const char* word) noexcept(false);
            void showMessage(const char* message) noexcept(false);
    };

}

#endif // INCLUDED_DISPLAY_H
