/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_dialogHelper.h"

#include "xap_UnixApp.h"
#include "xap_UnixFontManager.h"
#include "xap_UnixFontXLFD.h"
#include "xap_EncodingManager.h"
#include "ut_string_class.h"

// TODO get this from some higher-level place
#define FONTS_DIR_FILE	"/fonts.dir"

XAP_UnixFontManager::XAP_UnixFontManager(void) : m_fontHash(256)
{
}

XAP_UnixFontManager::~XAP_UnixFontManager(void)
{
	UT_VECTOR_PURGEALL(char *, m_searchPaths);
	UT_HASH_PURGEDATA(XAP_UnixFont *, m_fontHash);
}

bool XAP_UnixFontManager::setFontPath(const char * searchpath)
{
	gchar ** table = g_strsplit(searchpath, ";", 0);

	for (gint i = 0; table[i]; i++)
		m_searchPaths.addItem(table[i]);

	// free the table but not its contents
	g_free(table);
	
	return true;
}

bool XAP_UnixFontManager::scavengeFonts(void)
{
	UT_uint32 i = 0;
	UT_uint32 count = m_searchPaths.getItemCount();

	UT_uint32 totaldirs = 0;
	UT_uint32 totalfonts = 0;
	
	const char** subdirs = localeinfo_combinations("","","");
	/* If j is even, we open fonts.dir in encoding-specific directory */
	for (i = 0; i < count; i++)
	{
	    UT_ASSERT(m_searchPaths.getNthItem(i));	
	    char* basedirname = (char *) m_searchPaths.getNthItem(i);
	    int basedirname_len = strlen(basedirname);
	    char* filename;
	    
	    for(const char** subdir = subdirs;*subdir;++subdir)
	    {	    	
		filename = (char *) calloc(basedirname_len + 1 + strlen(*subdir) + strlen((char *) FONTS_DIR_FILE) + 1, sizeof(char));
		sprintf(filename, "%s/%s%s", basedirname, *subdir, FONTS_DIR_FILE);

		FILE * file;

		file = fopen(filename, "r");
		if (!file)
		{
			// silently print this message out, since lots of users will have a "default"
			// path which will probably contain bogus directories, but there's always
			// hope later on in the path
			UT_DEBUGMSG(("Cannot open [%s] to read fonts list.\n", filename));
		}
		else
		{
			char buffer[512];

			// first line is a count
			fgets(buffer, 512, file);

			UT_sint32 fontcount = atol(buffer);

			// these should probably not be DEBUG-build only, but reworked
			// for real error messages in a release version (perhaps with
			// friendly messages telling the users where to look to fix
			// a busted font path problem)
#ifdef DEBUG
			if (fontcount < 0)
			{
				char message[512];
				g_snprintf(message, 512, "WARNING: Font index file [%s]\ndeclares an invalid number of fonts (%d).",
						   filename, fontcount);
				messageBoxOK(message);
			}
			else
			{
				UT_DEBUGMSG(("File says %d fonts should follow...\n", fontcount));
			}
#endif

			char* lastslash = strrchr(filename,'/');
			if (lastslash)
				*lastslash = '\0';
			
			// every line after is a font name / XLFD pair
			UT_sint32 line;
			for (line = 0; line < fontcount; line++)
			{
				if (!fgets(buffer, 512, file))
				{
					// premature EOF (it's always premature if there are more
					// fonts specified than found)
					char message[512];
					g_snprintf(message, 512, "Premature end of file from font index file [%s];\n"
							   "%d fonts were supposed to be declared, but I only got %d.\n"
							   "I will continue, but things may not work correctly.\n",
							   filename, fontcount, line);
					messageBoxOK(message);
					fclose(file);
					return true;
				}
				if (subdir != subdirs && XAP_EncodingManager::instance->cjk_locale())
				    _allocateCJKFont((const char *) buffer,line);
				else
				    _allocateThisFont((const char *) buffer, filename,line);
			}

			totalfonts += line;

			if (line >= 0)
				totaldirs++;
		  
			UT_DEBUGMSG(("Read %ld fonts from directory [%s].\n", line, filename));
		}
		FREEP(filename);
		if (file)
			fclose(file);
	    }
	}

	if (totaldirs == 0)
	{
		// TODO this is not big enough for a really big list of fonts!
	        UT_String message = "AbiWord could not find any local font files in its font path.  Often this error is the\n"
				   "result of invoking AbiWord directly instead of through its wrapper\n"
				   "shell script.  The script sets the environment variable $ABISUITE_HOME which\n"
				   "should point to the directory where AbiSuite components reside.\n"
				   "\n"
				   "The current font path contains the following directories:\n\n";
		{
			UT_uint32 dircount = m_searchPaths.getItemCount();
			for (i = 0; i < dircount; i++)
			{
				UT_ASSERT(m_searchPaths.getNthItem(i));
				message += "    [";
				message += (const char *) m_searchPaths.getNthItem(i);
				message += "]\n";
			}
		}
		message += "\nPlease visit http://www.abisource.com/ for more information.";
		messageBoxOK(message);
		return false;
	}

	if (totalfonts <= 0)
	{
		// we have no fonts, just quit
		char message[1024];
		g_snprintf(message, 1024, "AbiWord found no actual font data files ('*.pfa', '*.afm') in\n"
				   "the font path even though I found [%d] font index files ('fonts.dir').\n"
				   "\n"
				   "Please visit http://www.abisource.com/ for more information.",
				   totaldirs);
		messageBoxOK(message);
		return false;
	}

	// since we now have a good list of fonts totalling more than 0,
	// verify that their metrics data can be loaded by parsing the files
	// now.

	// NOTE this adds more time to program startup and steals it from the
	// NOTE first printing

	// for the above reason (speed) we don't do this now
#if 0	
	XAP_UnixFont ** allfonts = getAllFonts();
	for (UT_uint32 k = 0; k < getCount(); k++)
	{
		// if any of these fails, the user will know about it
		if (!allfonts[k]->getMetricsData())
			return false;
	}
	DELETEP(allfonts);
#endif
	
	return true;
}

UT_uint32 XAP_UnixFontManager::getCount(void)
{
	return (UT_uint32) m_fontHash.getEntryCount();
}

XAP_UnixFont ** XAP_UnixFontManager::getAllFonts(void)
{
	UT_uint32 count = getCount();

	UT_DEBUGMSG(("XAP_UnixFontManager::getAllFonts [count %d]\n", count));
		
	XAP_UnixFont ** table = new XAP_UnixFont * [count];

	UT_ASSERT(table);

	UT_HashEntry * entry = NULL;
	for (UT_uint32 i = 0; i < count; i++)
	{
		entry = m_fontHash.getNthEntry(i);
		UT_ASSERT(entry && entry->pData);
		
		table[i] = (XAP_UnixFont *) entry->pData;
	}

	return table;
}

XAP_UnixFont * XAP_UnixFontManager::getDefaultFont(void)
{
	// this function (perhaps incorrectly) always assumes
	// it will be able to find this font on the display.
	// this is probably not such a bad assumption, since
	// gtk itself uses it all over (and it ships with every
	// X11R6 I can think of)
	UT_DEBUGMSG(("XAP_UnixFontManager::getDefaultFont\n"));

	XAP_UnixFont * f = new XAP_UnixFont();

	// do some manual behind-the-back construction
	f->setName("Default");
	f->setStyle(XAP_UnixFont::STYLE_NORMAL);
	f->setXLFD("-*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*");

	return f;
}

XAP_UnixFont * XAP_UnixFontManager::getFont(const char * fontname,
											XAP_UnixFont::style s)
{
	// We use a static buffer because this function gets called a lot.
	// Doing an allocation is really slow on many machines.
	static char keyBuffer[512];

	char * copy;
	UT_cloneString(copy, fontname);
	UT_upperString(copy);
	
	g_snprintf(keyBuffer, 512, "%s@%d", copy, s);

	FREEP(copy);
	
	UT_HashEntry * entry = m_fontHash.findEntry(keyBuffer);

	//UT_DEBUGMSG(("Found font [%p] in table.\n", entry));
	
	return (entry && entry->pData) ? ((XAP_UnixFont *) entry->pData) : NULL;
}

void XAP_UnixFontManager::_allocateThisFont(const char * line,
											const char * workingdir,int iLine)
{
	//UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont\n"));

	/*
	  Each line comes in as:
	  
	  "myfile.pfa  -foundry-family-weight-slant-blah--0-0-0-0-p-0-iso8859-1"

	  We consider everything up to the first chunk of whitespace to be the
	  actual font name.  This name, minus the "pfa" extension is the name
	  of the file containing the font metrics.  These files are both
	  considered to be found in the working directory (workingdir).
	  Since fields in an XLFD can have white space (like the family
	  name--"Times New Roman"), the rest of the string (minus that whitespace)
	  is considered the complete XLFD.

	  The XLFD string also holds the style attributes, which we parse out
	  and use to create the font.

	  We then do our best shot to allocate this font.
	*/

	char * linedup = UT_strdup(line);

	// Look for either first space or first tab
	char * firstspace = strchr(linedup, ' ');
	char * firsttab = strchr(linedup, '\t');

	UT_uint32 whitespace;

	if (!firstspace && !firsttab)
	{
		// We have a problem, there's only one thing here
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - missing seperator "
					 "between file name component and XLFD.\n"));
		FREEP(linedup);
		return;
	}
	if (!firstspace)
		whitespace = UT_uint32 (firsttab - linedup);
	else if (!firsttab)
		whitespace = UT_uint32 (firstspace - linedup);
	else
		whitespace = (firstspace < firsttab) ?
			(UT_uint32) (firstspace - linedup) : (UT_uint32) (firsttab - linedup);

	// damage the duplicated string with a NULL, strtok() style
	linedup[whitespace] = 0;
	
	// point fontfile to this new token
	char * fontfile = linedup;
	if (!fontfile)
	{
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - missing font "
					 "file name at first position in line.\n"));
		FREEP(linedup);
		return;
	}
	// allocate a real buffer for this
	fontfile = UT_strdup(fontfile);
	// tack on the working directory to get the full path to the font file
	char * newstuff = (char *) calloc(strlen(workingdir) + 1 + strlen(fontfile) + 1,
									  sizeof(char));
	sprintf(newstuff, "%s/%s", workingdir, fontfile);
	FREEP(fontfile);
	fontfile = newstuff;
		
	char * xlfd = linedup + whitespace + 1;
	if (!xlfd)
	{
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - missing XLFD "
					 "file name at second position in line.\n"));
		FREEP(linedup);
		return;		
	}
	// clean up that XLFD by removing surrounding spaces
	xlfd = g_strstrip(xlfd);

	// let this class munch on all the fields
	XAP_UnixFontXLFD descriptor(xlfd);

	// get the weight (should be "regular" or "bold") and the slant (should be
	// "r" for Roman or "i" for Italic) to discern an internal "style"
	const char * weight = descriptor.getWeight();
	UT_ASSERT(weight);
	const char * slant = descriptor.getSlant();
	UT_ASSERT(slant);

	XAP_UnixFont::style s = XAP_UnixFont::STYLE_NORMAL;
	if(!UT_strcmp(slant, "r"))
	  {
		if(!UT_strcmp(weight, "bold"))
		  s = XAP_UnixFont::STYLE_BOLD;
		else
		  s = XAP_UnixFont::STYLE_NORMAL;
	  }
	else if(!UT_strcmp(slant, "i"))
	  {
		if(!UT_strcmp(weight, "bold"))
		  s = XAP_UnixFont::STYLE_BOLD_ITALIC;
		else
		  s= XAP_UnixFont::STYLE_ITALIC;
	  }
	else if(!UT_strcmp(slant, "o"))
	  {
		if(!UT_strcmp(weight, "bold"))
		  s = XAP_UnixFont::STYLE_BOLD_OUTLINE;
		else
		  s= XAP_UnixFont::STYLE_OUTLINE;
	  }
    else
    {
        UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - can't guess "
                     "font style from XLFD.\n"));
        FREEP(linedup);
        return;
    }
	
	// do some voodoo to get the AFM file from the file name
	char * dot = strrchr(fontfile, '.');
	if (!dot)
	{
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - can't guess "
					 "at font metrics file from font file name.\n"));
		FREEP(linedup);
		return;
	}

	// where's the last dot at in the filename?
	UT_ASSERT(dot > fontfile);
	unsigned int stemchars = (dot - fontfile) + 1;
	UT_ASSERT(stemchars <= strlen(fontfile));

	// allocate a new buffer for the AFM name
	size_t len = stemchars + 4 + 1;
	char * metricfile = (char *) calloc(len, sizeof(char));
	g_snprintf(metricfile, stemchars, "%s", fontfile);
	strcat(metricfile, ".afm");

	// build a font and load it up
	XAP_UnixFont * font = new XAP_UnixFont;
	font->set_CJK_font(0);
	if (font->openFileAs((const char *) fontfile,
						 (const char *) metricfile,
						 (const char *) xlfd,
						 s))
	{
		UT_ASSERT(iLine>=0);
		if(iLine<4)
			XAP_UnixFont::s_defaultNonCJKFont[iLine]=font;
		_addFont(font);
	}
	else
	{
		DELETEP(font);
	}

	FREEP(newstuff);
	FREEP(metricfile);
	FREEP(linedup);
}
	  
void XAP_UnixFontManager::_addFont(XAP_UnixFont * newfont)
{
	// we index fonts by a combined "name" and "style"
	const char* fontkey = newfont->getFontKey();
	UT_HashEntry* curfont_entry = m_fontHash.findEntry(fontkey);
	if (!curfont_entry || !curfont_entry->pData)
	{
		m_fontHash.addEntry(fontkey, NULL, (void *) newfont);
	} 
	else 
	{
		/* 
                  since "standard fonts" folder is read first and then
                  current charset-specific subdirectory, it's obvious that
                  the recent version is current charset-specific font, 
                  so we replace original font (that is standard) 
                  unconditionally.
		*/
                XAP_UnixFont* curfont = static_cast<XAP_UnixFont*>(curfont_entry->pData);               
                delete curfont;
                curfont_entry->pData = (void*)newfont;

	       
	}
}

void XAP_UnixFontManager::_allocateCJKFont(const char * line,
											   int iLine)
{
	gchar **sa= g_strsplit(line,",",5);

	g_strstrip(sa[0]);
		
	gchar * xlfd =sa[1];
	if (!xlfd)
	{
		UT_DEBUGMSG(("XAP_UnixFontManager::_allocateThisFont() - missing XLFD "
					 "file name at second position in line.\n"));
		g_strfreev(sa);
		return;		
	}
	// clean up that XLFD by removing surrounding spaces
	xlfd = g_strstrip(xlfd);

	// let this class munch on all the fields
	XAP_UnixFontXLFD descriptor(xlfd);

	// get the weight (should be "regular" or "bold") and the slant (should be
	// "r" for Roman or "i" for Italic) to discern an internal "style"
	const char * weight = descriptor.getWeight();
	UT_ASSERT(weight);
	const char * slant = descriptor.getSlant();
	UT_ASSERT(slant);

	XAP_UnixFont::style s = XAP_UnixFont::STYLE_NORMAL;
	// sort from most common down

	if(!UT_strcmp(slant, "r"))
	  {
		if(!UT_strcmp(weight, "bold"))
		  s = XAP_UnixFont::STYLE_BOLD;
		else
		  s = XAP_UnixFont::STYLE_NORMAL;
	  }
	else if(!UT_strcmp(slant, "i"))
	  {
		if(!UT_strcmp(weight, "bold"))
		  s = XAP_UnixFont::STYLE_BOLD_ITALIC;
		else
		  s= XAP_UnixFont::STYLE_ITALIC;
	  }
	
	// build a font and load it up
	XAP_UnixFont * font = new XAP_UnixFont;
	font->set_CJK_Ascent(atoi(sa[2]));
	font->set_CJK_Descent(atoi(sa[3]));
	font->set_CJK_Width(atoi(sa[4]));
	font->set_CJK_font(1);
	if (font->openFileAs((const char *) sa[0],
						 (const char *) sa[0],
						 (const char *) xlfd,
						 s))
	{
	  
	  UT_ASSERT(iLine>=0);
	  if(iLine<4)
		XAP_UnixFont::s_defaultCJKFont[iLine]=font;
	  _addFont(font);
	}
	else
	{
		DELETEP(font);
	}

	g_strfreev(sa);
}

