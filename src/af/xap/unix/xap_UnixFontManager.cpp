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
#include "xap_UnixDialogHelper.h"

#include "xap_UnixFontManager.h"
#include "xap_UnixFontXLFD.h"
#include "xap_EncodingManager.h"
#include "ut_string_class.h"
#include "ut_sleep.h"
#include <sys/stat.h>
#include "xap_Strings.h"
#include "xap_Prefs.h"
// TODO get this from some higher-level place
#define FONTS_DIR_FILE	"/fonts.dir"

static char **	s_oldFontPath = NULL;
static int		s_oldFontPathCount = 0;
static int		(*s_oldErrorHandler)(Display *dsp, XErrorEvent *e);
static int		s_errorDepth = 0;

static int s_xerror_handler(Display *dsp, XErrorEvent *e)
{
	
	if(++s_errorDepth > 1)
	{
		// we have been caught in a loop; this happens when the old font path
		// we got from XGetFontPath is invalid, so that the attempt of this
		// error handler to restore it fails. There is nothing we can do
		// but treat this a unhandled error
		// (we do some tests in the main code to avoid this situation,
		// this is a last resort)
		goto unhandled;
	}
	
	if(e->error_code == 86)
	{
		// BadFontPath, we handle this ourselves
		UT_DEBUGMSG(("Caught BadFontPath Error, reverting to old path\n"));
		
		if(s_oldFontPath)
		{
			XSetFontPath(dsp, s_oldFontPath, s_oldFontPathCount);
			
			bool bShowWarning = true;
			XAP_App * pApp = XAP_App::getApp();
			UT_ASSERT(pApp);
			pApp->getPrefsValueBool(XAP_PREF_KEY_ShowUnixFontWarning, &bShowWarning);
			bool bBonobo = ((XAP_UnixApp *)pApp)->isBonoboRunning();
			if(bBonobo)
			{
				bShowWarning = false;
			}
#if 1 
			if(bShowWarning)
			{
				if(pApp->getDisplayStatus())
				{
					const XML_Char * msg = pApp->getStringSet()->getValue(XAP_STRING_ID_MSG_ShowUnixFontWarning);
					UT_ASSERT(msg);
					messageBoxOK(msg);
				}
       			else
       				fprintf(stderr, "AbiWord WARNING: unable to modify font path\n");
				
			}
			
			
			
#else			
			if(bShowWarning)			
				messageBoxOK("WARNING: AbiWord could not add its fonts to the X font path.\n"
				 " See \"Unix Font Warning\" in the FAQ section of AbiWord help.");
#endif
				
			//s_errorDepth--;
			return (0);
		}
		
		UT_DEBUGMSG(("!!! No Old Path available !!!\n"));
		
	}

unhandled:
	// on everything else we will call the previous handler.
	UT_DEBUGMSG(("Unhandled X error: %d\n"
				 "          display %s, serial %lu, request %d:%d, XID %lu\n"
				 "          AbiWord will terminate\n",
				 (int) e->error_code, DisplayString (e->display), e->serial, (int) e->request_code, (int) e->minor_code, (unsigned long) e->resourceid));
	
	if(!s_oldErrorHandler)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		// we cannot handle this ourselves, nor is there any handler available
		// what can we do but exit?
		exit(1);
	}

	s_errorDepth--;	
	return s_oldErrorHandler(dsp,e);
}

XAP_UnixFontManager::XAP_UnixFontManager(void) : m_fontHash(256), m_pExtraXFontPath(0),
m_iExtraXFontPathCount(0)
{
}

XAP_UnixFontManager::~XAP_UnixFontManager(void)
{
	UT_VECTOR_FREEALL(char *, m_searchPaths);

	UT_HASH_PURGEDATA(XAP_UnixFont *, &m_fontHash, delete);

	// we have to remove what we have added to the X font path
	// the code below is not perfect, because if some other
	// program adds one of our paths while we are running,
	// we will remove it as well.	
	if(m_pExtraXFontPath)
	{
		Display *dsp  = XOpenDisplay(gdk_get_display());
		UT_sint32 pathSize;
		char ** oldPath = XGetFontPath(dsp, &pathSize);
		
		char ** newPath = new char *[pathSize];
		UT_ASSERT(newPath);
		
		char ** newPath_ptr = newPath;
		
		xxx_UT_DEBUGMSG(("current path size %d, m_iExtraXFontPathCount %d\n", pathSize,m_iExtraXFontPathCount));
		
		UT_sint32 i;
		UT_sint32 j;
		
		for(i = 0; i < pathSize; i++)
		{
			bool bFound = false;
			for(j = 0; j < m_iExtraXFontPathCount; j++)
				if(!UT_strcmp(m_pExtraXFontPath[j], oldPath[i]))
				{
					bFound = true;
					break;
				}
				
			if(bFound)
				continue;
				
			*newPath_ptr++ = oldPath[i];
		}
		
		pathSize = newPath_ptr - newPath;

#ifdef DEBUG		
		UT_DEBUGMSG(("--- new path (size %d) ---\n", pathSize));
		for(j = 0; j < pathSize; j++)
			UT_DEBUGMSG(("\tfontpath[%d]: \"%s\"\n", j, newPath[j]));
		UT_DEBUGMSG(("-------------------------------------------------\n\n"));
#endif
		
		XSetFontPath(dsp, newPath, pathSize);
		for(j = 0; j < m_iExtraXFontPathCount; j++)
		{
		    xxx_UT_DEBUGMSG(("freeing indx %d\n", j));
			FREEP(m_pExtraXFontPath[j]);
		}
		xxx_UT_DEBUGMSG(("deleting m_pExtraXFontPath\n"));
		delete [] m_pExtraXFontPath;

		xxx_UT_DEBUGMSG(("freeing old X font path\n"));
		XFreeFontPath(oldPath);
		xxx_UT_DEBUGMSG(("closing X display\n"));
		XCloseDisplay(dsp);
		xxx_UT_DEBUGMSG(("done.\n"));

		delete[] newPath ;
	}
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

#if 1
	bool bModifyPath = true;
	XAP_App * pApp = XAP_App::getApp();
	UT_ASSERT(pApp);
	pApp->getPrefsValueBool(XAP_PREF_KEY_ModifyUnixFontPath, &bModifyPath);

	if(bModifyPath)
	{
	    const char** subdir = subdirs;
    	UT_uint32 subdircount  = 0;
	
		while(*subdir++)
			subdircount++;
	
		Display *dsp  = XOpenDisplay(gdk_get_display());
		UT_uint32 fontPathDirCount, realFontPathDirCount = 0;
	
		if(m_pExtraXFontPath)
		{
			// this should not happen, if it does, we will not be able to restore
			// font path when we exit
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			UT_ASSERT(m_iExtraXFontPathCount >= 0);
		
		    for(i = 0; i < (UT_uint32)m_iExtraXFontPathCount; i++)
		    	FREEP(m_pExtraXFontPath[i]);
	    	delete [] m_pExtraXFontPath;
		}
		
		char ** oldFontPath = XGetFontPath(dsp,(int*)&fontPathDirCount);
	
		// set stuff for our error handler
		s_oldFontPath = oldFontPath;
		s_oldFontPathCount = fontPathDirCount;
	
		realFontPathDirCount = fontPathDirCount;
		m_iExtraXFontPathCount = -fontPathDirCount;
	
		fontPathDirCount += count*subdircount;
		UT_DEBUGMSG(("XDisplay [%s], current X font path count %d, AW path count %d, subdircount %d\n",
				gdk_get_display(),realFontPathDirCount,count, subdircount));
				
		char ** newFontPath = new char*[fontPathDirCount];
		UT_ASSERT(newFontPath);
		char ** newFontPath_ptr = newFontPath;
		char ** oldFontPath_ptr	= oldFontPath;
		for(i = 0; i < realFontPathDirCount; i++)
		{
			// do some elementary tests on validity of the old path
			// basically, if it contains anything else than directories
			// we will abort our attempt to modify the path.
			
			// first of all, take care of the path elements that have size
			// specification;
			
			char * temp_str = UT_strdup(*oldFontPath_ptr);
			UT_ASSERT(temp_str);
			
			char * colon = strrchr(temp_str, ':');
			
			if(colon)
				*colon = 0;
				
	    	struct stat stat_data;
	    	if(!stat(temp_str,&stat_data) && S_ISDIR(stat_data.st_mode))
			{
    			xxx_UT_DEBUGMSG(("orig font path element: %s\n", *oldFontPath_ptr));
    			*newFontPath_ptr++ = UT_strdup(*oldFontPath_ptr++);
    			FREEP(temp_str);
    		}
    		else
    		{
    			// remember how many elements of the font path we have
    			// duplicated
    			realFontPathDirCount = i;
    			m_iExtraXFontPathCount = 0;
				bool bShowWarning = true;
				XAP_App * pApp = XAP_App::getApp();
				UT_ASSERT(pApp);
				pApp->getPrefsValueBool(XAP_PREF_KEY_ShowUnixFontWarning, &bShowWarning);
				bool bBonobo = ((XAP_UnixApp *)pApp)->isBonoboRunning();
				if(bBonobo)
				{
					bShowWarning = false;
				}

				UT_DEBUGMSG(("found non-directory entry in existing fontpath [%s]\n", *oldFontPath_ptr));
				if(bShowWarning)			
				{
					if(pApp->getDisplayStatus())
					{
#if 1
						const XML_Char * msg = pApp->getStringSet()->getValue(XAP_STRING_ID_MSG_ShowUnixFontWarning);
						UT_ASSERT(msg);
						messageBoxOK(msg);

#else					
						messageBoxOK("WARNING: Your current font path contains non-directory entries.\n"
							"AbiWord will not attempt to modify this path. You should see \"Unix Font Warning\"\n"
							"in the FAQ section of AbiWord help for further information and instructions on how to\n"
                        	"turn this warning off");
#endif
       				}
       				else
       					fprintf(stderr, "AbiWord WARNING: unable to modify font path\n");
				}
    			
    			FREEP(temp_str);
    			
    			goto invalid_old_path;
    		}
		}
		
		for(i = 0; i < count; i++)
		{
			UT_String str;
		    UT_ASSERT(m_searchPaths.getNthItem(i));	
		    char* basedirname = (char *) m_searchPaths.getNthItem(i);
	
	    	for(subdir = subdirs;*subdir;++subdir)
		    {
		    	str = basedirname;
	    		str += "/";
	    		str += *subdir;
		    	struct stat stat_data;
		    	if(!stat(str.c_str(),&stat_data))
	    		{
	    			if(S_ISDIR(stat_data.st_mode))
	    			{
	    				bool bFound = false;
		    			// only add this if it is not already in the path
		    			for(UT_uint32 j = 0; j < realFontPathDirCount; j++)
	    				{
	    					if(!UT_strcmp(newFontPath[j], str.c_str()))
	    					{
	    						UT_DEBUGMSG(("directory [%s] already in the X font path\n", str.c_str()));
	    						bFound = true;
	    						break;
		    				}
		    			}
	    			
	    				if(!bFound)
	    				{
	    					//str+= "rubbish"; // to force error while testing
	    					*newFontPath_ptr++ = UT_strdup(str.c_str());
		    				UT_DEBUGMSG(("adding \"%s\" to font path\n", str.c_str()));
			    			realFontPathDirCount++;
			    		}
	    			}
	    		}
			}
		}
	
		m_iExtraXFontPathCount += realFontPathDirCount;
		UT_ASSERT(m_iExtraXFontPathCount >= 0);
	    xxx_UT_DEBUGMSG(("m_iExtraXFontPathCount %d\n", m_iExtraXFontPathCount));
		
		m_pExtraXFontPath = new char *[m_iExtraXFontPathCount];
		UT_ASSERT(m_pExtraXFontPath);

		{	// the block is here because of the jump to invalid_old_path
			char ** pExtraXFontPath = m_pExtraXFontPath;
	
			for(i = realFontPathDirCount - m_iExtraXFontPathCount; i < realFontPathDirCount; i++)
				*pExtraXFontPath++ = newFontPath[i];
	    }
#ifdef DEBUG		
		UT_DEBUGMSG(("--- setting X font path (%d items)\n", realFontPathDirCount));
		for(UT_uint32 j = 0; j < realFontPathDirCount; j++)
			UT_DEBUGMSG(("\tfontpath[%d]: \"%s\"\n", j, newFontPath[j]));
		UT_DEBUGMSG(("-------------------------------------------------\n\n"));
#endif

		s_oldErrorHandler = XSetErrorHandler(s_xerror_handler);
		// force synchronic behaviour, so that any error is caught
		// immediately
		XSynchronize(dsp,1);
		XSetFontPath(dsp, newFontPath, (int)realFontPathDirCount);
	
		// now if we failed, our handler should have been called and if we are
		// here, we have things under control, co clean up
		XSynchronize(dsp,0);
		XSetErrorHandler(s_oldErrorHandler);
		s_oldErrorHandler = NULL;
		s_oldFontPath = 0;

invalid_old_path:	
		// now free what we do not need	
		XFreeFontPath(oldFontPath);
		XCloseDisplay(dsp);
	
		// free the directories that we got from the X server, not the bit
		// we appended
		for(i = 0; i < realFontPathDirCount - m_iExtraXFontPathCount; i++)
			FREEP(newFontPath[i]);
		delete [] newFontPath;
	}
#endif
	for (i = 0; i < count; i++)
	{
	    UT_ASSERT(m_searchPaths.getNthItem(i));	
	    char* basedirname = (char *) m_searchPaths.getNthItem(i);
	    int basedirname_len = strlen(basedirname);
	    char* filename;
	    
	    for(const char** subdir = subdirs;*subdir;++subdir)
	    {	    	
		filename = (char *) UT_calloc(basedirname_len + 1 + strlen(*subdir) + strlen((char *) FONTS_DIR_FILE) + 1, sizeof(char));
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
				if (subdir != subdirs && XAP_EncodingManager::get_instance()->cjk_locale())
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
		messageBoxOK(message.c_str());
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

/*!
 * compareFontNames this function is used to compare the char * strings names
 * of the Fonts with the qsort method on UT_Vector.
\params const void * vF1  - Pointer to XAP_UnixFont pointer
\params const void * vF2  - Pointer to a XAP_UnixFont pointer
\returns -1 if sz1 < sz2, 0 if sz1 == sz2, +1 if sz1 > sz2
*/
static UT_sint32 compareFontNames(const void * vF1, const void * vF2)
{
	XAP_UnixFont ** pF1 = (XAP_UnixFont **) const_cast<void *>(vF1);
	XAP_UnixFont ** pF2 = (XAP_UnixFont **) const_cast<void *>(vF2);
	return UT_strcmp((*pF1)->getName(), (*pF2)->getName());
}

UT_Vector * XAP_UnixFontManager::getAllFonts(void)
{
	UT_Vector * pVec = m_fontHash.enumerate();
	UT_ASSERT(pVec);
	if(pVec->getItemCount()> 1)
		pVec->qsort(compareFontNames);

	return pVec;
}

XAP_UnixFont * XAP_UnixFontManager::getDefaultFont(void)
{
	// this function (perhaps incorrectly) always assumes
	// it will be able to find this font on the display.
	// this is probably not such a bad assumption, since
	// gtk itself uses it all over (and it ships with every
	// X11R6 I can think of)
	UT_DEBUGMSG(("XAP_UnixFontManager::getDefaultFont\n"));

	static bool fontInitted = false ;
	static XAP_UnixFont m_f ;

	if ( !fontInitted )
	  {
	    // do some manual behind-the-back construction
	    m_f.setName("Default");
	    m_f.setStyle(XAP_UnixFont::STYLE_NORMAL);
	    m_f.setXLFD("-*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*");
	    fontInitted = true ;
	  }

	return &m_f;
}

XAP_UnixFont * XAP_UnixFontManager::getDefaultFont16Bit(void)
{
	UT_DEBUGMSG(("XAP_UnixFontManager::getDefaultFont16Bit\n"));

	XAP_UnixFont * f = NULL;
	
	if(XAP_EncodingManager::get_instance()->isUnicodeLocale())
	{
		XAP_UnixFont * f = new XAP_UnixFont();
		UT_ASSERT(f);
		// people runing utf-8 locale will probably have the
		// MS Arial ttf font ...
		f->setName("Default");
		f->setStyle(XAP_UnixFont::STYLE_NORMAL);
		f->setXLFD("-*-Arial-medium-r-*-*-*-*-*-*-*-*-iso10646-1");
		return f;
	}
    else
		return getDefaultFont();	
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
	
	const XAP_UnixFont * entry = static_cast<const XAP_UnixFont *>(m_fontHash.pick(keyBuffer));

	//UT_DEBUGMSG(("Found font [%p] in table.\n", entry));
	
	return const_cast<XAP_UnixFont *>(entry);
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
	char * newstuff = (char *) UT_calloc(strlen(workingdir) + 1 + strlen(fontfile) + 1,
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
	char * metricfile = (char *) UT_calloc(len, sizeof(char));
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
	const void * curfont_entry = m_fontHash.pick(fontkey);
	if (curfont_entry)
	{
		const XAP_UnixFont* curfont = static_cast<const XAP_UnixFont*>(curfont_entry);
		delete curfont;     
		m_fontHash.remove (fontkey, NULL);
	} 

	/* 
	   since "standard fonts" folder is read first and then
	   current charset-specific subdirectory, it's obvious that
	   the recent version is current charset-specific font, 
	   so we replace original font (that is standard) 
	   unconditionally.
	*/

	m_fontHash.insert(fontkey,
			  (void *) newfont);		
}

void XAP_UnixFontManager::_allocateCJKFont(const char * line,
											   int iLine)
{
	/*
	  Each line for cjk fonts.dir files comes in as:
	  
	  Song-Medium-GB-EUC-H, -default-song-medium-r-normal--0-0-0-0-c-0-gb2312.1980-0, 880, 120, 1000
	  
	  We consider everything up to the first comma to be the postscript
	  font name which we will use for postscript printing. 
	  The second field is the XLFD of the font. The next 3 fields are
	  the ascent, descent and width of the font.
	  
	  FIXME: We might want to change the format of the file to having 
	  the first field as the actually font filename. And insert the
	  postscript fontname to the 3rd field. This will agree better with
	  standard fonts.dir. Comma still has to be used as the seperator
	  since white spaces are valid XLFD characters.
	*/

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



