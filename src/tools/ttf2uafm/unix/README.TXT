
Readme for ttf2uafm
-------------------

What is it?
-----------
A utility programme that can generate afm and u2g files from Windows TTF fonts.
In contrast to the ttf2afm utility from pdfTeX (on which it is based), it can
generata afm files for all glyphs in a Unicode font, not only the first 258.

(u2g file is required by AbiWord <www.abisource.com> for printing with ttf fonts;
it contains a table needed to translate the glyph names used by the font to
Unicode values.)

Copyright
---------
This utility is placed into public domain. Most of the code was taken over from the
original ttf2afm programme that is part of pdfTeX. Please address bugfixes, etc.,
to <tomas@frydrych.uklinux.net>

How to use it?
--------------
run
	ttf2uafm /myfontdir/myfont.ttf -a myfont.afm -u myfont.u2g
