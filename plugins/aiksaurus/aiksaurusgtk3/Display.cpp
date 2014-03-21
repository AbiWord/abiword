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

#include <strings.h>

#include "Display.h"
#include "Meaning.h"
#include "DialogMediator.h"
#include "Exception.h"
using namespace std;

namespace AiksaurusGTK_impl
{

    Display::Display(DialogMediator& mediator) throw()
        : d_mediator(mediator)
    {
        // ensure that styles are set up.
 //       _initResources();

        // create our scrollbar. we'll wrap this around all of our
        // meanings to create a really nice look.  We'll also set up
        // an initial size (the usize call) so that it won't start
        // out all squnched together.
        d_scroller = gtk_scrolled_window_new(0, 0);
        gtk_widget_set_size_request(d_scroller, 360, 240);
        gtk_scrolled_window_set_policy(
            GTK_SCROLLED_WINDOW(d_scroller),
            GTK_POLICY_NEVER,       // no horizontal scrollbar.
            GTK_POLICY_AUTOMATIC    // auto vertical scrollbar
        );

        // our main scrolling widget will be an event box with a white
        // background.  this way, when the window is larger than all the
        // meanings, we don't get a big ugly gray blob at the bottom.
        // we must use an event box rather than setting d_layout to
        // white because boxes are transparent and you can't set a
        // background color on them.
        d_white = gtk_event_box_new();
        gtk_widget_set_name(d_white, "wbg");
        gtk_container_add(GTK_CONTAINER(d_scroller), d_white);

        d_layout = 0;
    }


    Display::~Display() throw()
    {
        for(int i = 0;i < static_cast<int>(d_meanings.size());++i)
        {
            delete d_meanings[i];
        }
    }


    void Display::_createMeaning(const string& title, vector<string>& words)
        throw(std::bad_alloc)
    {
        Meaning *mean = new Meaning(title, words, *this);
        d_meanings.push_back(mean);
        gtk_box_pack_start(GTK_BOX(d_layout), mean->getLayout(), 0, 0, 0);
    }


    void Display::_resetDisplay() throw()
    {
        // Recreate our layout widget.
        if (d_layout)
            gtk_container_remove(GTK_CONTAINER(d_white), d_layout);

        d_layout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(d_white), d_layout);

        // Clear out meanings.
        for(int i = 0;i < static_cast<int>(d_meanings.size());++i)
        {
            delete d_meanings[i];
        }
        d_meanings.clear();
    }

    void Display::_displayResults(const char* word) throw(Exception, std::bad_alloc)
    {
        _checkThesaurus();

        string title;
        vector<string> words;

        int meaning, prev_meaning = -1;
        for(const char* r = d_thesaurus.next(meaning);
            r[0]; r = d_thesaurus.next(meaning))
        {
            _checkThesaurus();
            if (meaning != prev_meaning) // new meaning
            {
                if (prev_meaning != -1)
                {
                    _createMeaning(title, words);
                    words.clear();
                }

                prev_meaning = meaning;

                string option1(r);
                string option2(d_thesaurus.next(meaning));
                title = strcasecmp(option1.c_str(), word)
                    ? (option1) : (option2);

                r = d_thesaurus.next(meaning);
                _checkThesaurus();
            }

            words.push_back(r);
        }

        _createMeaning(title, words);
    }



    void Display::_checkThesaurus() throw(Exception)
    {
        if (d_thesaurus.error()[0])
        {
            showMessage(d_thesaurus.error());
            /*
            GtkWidget* err = gtk_label_new(d_thesaurus.error());
            gtk_label_set_justify(GTK_LABEL(err), GTK_JUSTIFY_LEFT);
            gtk_box_pack_start(GTK_BOX(d_layout), err, 0, 0, 0);
            gtk_widget_show_all(d_layout);
            */
            throw Exception(d_thesaurus.error());
        }
    }

    void Display::_displayAlternatives()
        throw(Exception, std::bad_alloc)
    {
        _checkThesaurus();
        vector<string> words;
        for(const char* r = d_thesaurus.similar(); r[0]; r = d_thesaurus.similar())
        {
            _checkThesaurus();
            words.push_back(r);
        }

        _createMeaning("No Synonyms Known.  Nearby words:", words);
    }


    void Display::showMessage(const char* message) throw()
    {
        _resetDisplay();
        GtkWidget* label = gtk_label_new(message);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_box_pack_start(GTK_BOX(d_layout), label, 1, 1, 0);
        gtk_widget_show_all(d_layout);
    }

    void Display::search(const char* word) throw(std::bad_alloc)
    {
        try
        {
            _resetDisplay();
            _checkThesaurus();

            if (d_thesaurus.find(word))
                _displayResults(word);
            else
                _displayAlternatives();

            gtk_widget_show_all(d_layout);
        }
        catch(Exception& e)
        {
          // ignore, we've processed it already.
        }
    }


    void Display::_handleClick(bool isDoubleClick, const char* text) throw(std::bad_alloc)
    {
        string str(text); // might throw

        if (isDoubleClick)
            d_mediator.eventSearch(str.c_str());
        else
            d_mediator.eventSelectWord(str.c_str());
    }


    void Display::_handleSelection(GtkWidget* list) throw()
    {
        for(int i = 0;i < static_cast<int>(d_meanings.size());++i)
        {
            d_meanings[i]->unselectListsExcept(list);
        }
    }


    GtkWidget* Display::getDisplay() throw()
    {
        return d_scroller;
    }


    const Aiksaurus& Display::getThesaurus() const throw()
    {
        return d_thesaurus;
    }


#if 0
	//
    // We use GTK styles to get the nice colors for our display.
    // To do this, we have to set up a resource first so that the
    // styles mean what we want them to mean.
    //
    void Display::_initResources() throw()
    {
        // Execute this code only once.
        static bool done = false;
        if (done) return;
        done = true;

        // Resource string to control colors and fonts of display pane.
        const char* rcstring =
            "style \"white_background\" {\n"
            "   bg[NORMAL] = \"#ffffff\" \n"
            "}\n"
            "style \"meaningset_title\" {\n"
            "   fg[NORMAL] = \"#600000\" \n"
            "   font = \"-*-Arial-bold-r-*-*-14-*-*-*-*-*-*-*\" \n"
            "}\n"
            "style \"yellow_background\" {\n"
            "   bg[NORMAL] = \"#f0f0e0\" \n"
            "}\n"
            "widget \"*wbg\" style \"white_background\"\n"
            "widget \"*ybg\" style \"yellow_background\"\n"
            "widget \"*mst\" style \"meaningset_title\"\n";

        // Parse this resource string.
        gtk_rc_parse_string(rcstring);
    }
#endif

}
