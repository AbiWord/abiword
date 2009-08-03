This was my attempt to port AbiWord to gettext, for Google Summer of Code 2009.  It worked, of course, but I was fired anyway.  My mentor (Robert Staudinger, robsta) wanted me to do something and Hubert Figuire (hub) wanted me to do the exact opposite entirely and criticised me for doing what I was told to do.  It's beyond me how they could have built it this far when the different developers have diametrically opposite ideas of what should be be done.  Ah well, if you want to take up where I left off, the best of luck (you're going to need it).

I will list below what happened.  I hope it doesn't scare you off.

robsta:  We haven't officially supported Cocoa builds in a long time.  Don't bother with it.
hub:  You broke Cocoa.  Dude, doesn't your editor have parentheses matching? (Yeah, he actually referred to me as "dude"; how patronizing!)

robsta: I want you to use intltool to internationalize the GtkBuilder files.
hub: Why are you putting GNOME dependencies in autogen.sh?  (Yeah, like a set of Perl scripts wrapping gettext functions is a GNOME dependency.)

robsta: Our current system of internationalizing the GtkBuilder files by setting the strings in the C++ files is hellish.  You are to rewrite them with the strings in the GtkBuilder files.
hub: A perfecly good internationalization system for GtkBuilder is in place.  You ruined it!

robsta: I want you to remove the *_StringSet classes.
hub: (After I did the first step by removing getValue() from the C++ files in ./plugins) I never told you to do that.  You're fired!
