#ifndef INCLUDED_MEANING_H
#define INCLUDED_MEANING_H

#include <gtk/gtk.h>
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
                                   gpointer data) throw(std::bad_alloc);

        public:

            Meaning(const string& title, vector<string>& words, Display& display)
                throw(std::bad_alloc);

            ~Meaning() throw();

            GtkWidget* getLayout() throw();

            void unselectListsExcept(GtkWidget* me) throw();
    };
}

#endif // INCLUDED_MEANING_H
