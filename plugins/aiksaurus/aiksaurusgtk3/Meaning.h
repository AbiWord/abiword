#ifndef INCLUDED_MEANING_H
#define INCLUDED_MEANING_H

#include "ut_compiler.h"
ABI_W_NO_CONST_QUAL
#include <gtk/gtk.h>
ABI_W_POP
#include <vector>
#include <string>
using namespace std;

namespace AiksaurusGTK_impl
{
    class Display;
    class Meaning
    {
        private:
            string d_title;
            vector<string> d_words;
            Display& d_display;
            vector<GtkWidget*> d_lists;
            vector<GtkListStore*> d_models;

            GtkWidget* d_masterLayout;
            GtkWidget* d_mainLayout;
            GtkWidget* d_labelLayout;
            GtkWidget* d_subLayout;
            GtkWidget* d_label;

            static gint _wordclick(GtkTreeSelection *sel,
                                   gpointer data);

        public:

            Meaning(const string& title, vector<string>& words, Display& display);

            ~Meaning();

            GtkWidget* getLayout() noexcept;

            void unselectListsExcept(GtkWidget* me) noexcept;
    };
}

#endif // INCLUDED_MEANING_H
