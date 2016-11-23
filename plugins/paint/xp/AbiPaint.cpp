/*
 * AbiPaint plugin
 *
 * AbiWord plugin to ease editing embedded images via an
 * External Image Editing program.  The image editing program
 * used and optional image format conversion may be specified,
 * though sensible (platform specific) defaults are defined.
 *
 */

/* The default image editor and exported image type depends
 * on the platform.
 *
 * For Windows:
 *   The PNG images by default are converted to BMP files
 *   and the default image editor is Microsoft's (R) Paint (MSPAINT.EXE)
 *     PNG to BMP conversion done using:
 *       PNGDIB - a mini DIB-PNG conversion library for Win32
 *       By Jason Summers  <jason1@pobox.com>
 *
 * For Unix Systems (and similar)
 *   The images are exported as PNG files
 *   and the default image editor is the GIMP (gimp)
 *   GNU Image Manipulation Program - see http://www.gimp.org/
 */

/*
 * Based on AbiGimp copyright Martin Sevior which in turn
 *   is based on AiksaurusABI - Abiword plugin for Aiksaurus
 *   Copyright (C) 2001 by Jared Davis
 * Also tidbits taken from ImageMagick plugin, copyright 2002
 *   by Dom Lachowicz
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#define ABI_PLUGIN_NAME AbiPaint	/* Very important, do this before including AbiGeneric */
#include "AbiPaint.h"			/* includes "AbiGeneric.h" */

#ifdef _WIN32
#include <io.h>
#endif

// use preference file instead of registry
XAP_PrefsScheme * prefsScheme = NULL;
/* our Plugin Scheme name in preference file,  e.g. "AbiGeneric" */
const gchar * szAbiPluginSchemeName = ABI_PLUGIN_SCHEME_NAME ;
// settings within our plugin scheme
const gchar * ABIPAINT_PREF_KEY_bLeaveImageAsPNG = "bLeaveImageAsPNG";
const char* ABIPAINT_PREF_KEY_szProgramName = "szImageEditor";


/*
 * Declare image editor methods
 */
static DECLARE_ABI_PLUGIN_METHOD(editImage);
ABI_GRAYABLE_MENUITEM_PROTOTYPE(editImage);
Defun_EV_GetMenuItemComputedLabel_Fn(getEditImageMenuName);
#ifdef ENABLE_BMP
static DECLARE_ABI_PLUGIN_METHOD(saveAsBmp);
static DECLARE_ABI_PLUGIN_METHOD(useBmp);
#endif
static DECLARE_ABI_PLUGIN_METHOD(specify);
ABI_TOGGLEABLE_MENUITEM_PROTOTYPE(useBmp);


/* Note:  Make sure the methodName field is Not NULL, otherwise
 *        AbiWord.exe will probably segfault -- actual results 
 *        depend on where other plugins add menu items.
 * Note2: Make sure the label is unique across ALL plugins (and
 *        internal menu labels), any duplicates [including NULL]
 *        may cause the menu to display incorrectly.  This is
 *        most notable on the end of a submenu; its probably ok
 *        for separators to share a NULL label.
 */
static AbiMenuOptions amo [] = 
{
  { ABI_PLUGIN_METHOD_STR(submenu_start), NULL,                           "AbiPaint",                   "Allows in place editing of image via external program.", EV_MLF_BeginSubMenu, true, false, false, NULL, NULL, true, false, 0 },
  { ABI_PLUGIN_METHOD_STR(editImage),     ABI_PLUGIN_METHOD(editImage),   "(AbiPaint) &Edit Image",     "Opens the selected image for modification (in specified image editing program).", EV_MLF_Normal, false, true, false, ABI_GRAYABLE_MENUITEM(editImage), getEditImageMenuName, true, true, 0 },
#ifdef ENABLE_BMP
  { ABI_PLUGIN_METHOD_STR(saveAsBmp),     ABI_PLUGIN_METHOD(saveAsBmp),   "Save Image &As BMP",         "Saves the selected image as a BMP file.", EV_MLF_Normal, false, true, false, ABI_GRAYABLE_MENUITEM(editImage), NULL, true, true, 0 },
#endif
  { ABI_PLUGIN_METHOD_STR(separator1),    NULL,                           NULL,                         NULL, EV_MLF_Separator, false, false, false, NULL, NULL, true, false, 0 },
  { ABI_PLUGIN_METHOD_STR(specify),       ABI_PLUGIN_METHOD(specify),     "&Specify Image Editor",      "Allows you to specify what image editing program to use, results stored in registry.", EV_MLF_Normal, false, true, false, NULL, NULL, true, false, 0 },
#ifdef ENABLE_BMP
  { ABI_PLUGIN_METHOD_STR(useBmp),        ABI_PLUGIN_METHOD(useBmp),      "Image Editor Requires &BMP", "Indicates the specified image editing program must use a BMP file instead of PNG (default is enabled).", EV_MLF_Normal, false, false, true, ABI_TOGGLEABLE_MENUITEM(useBmp), NULL, true, false, 0 },
#endif
  { ABI_PLUGIN_METHOD_STR(submenu_end),   NULL,                           "AbiPaint Submenu End",       NULL, EV_MLF_EndSubMenu, true, false, false, NULL, NULL, true, false, 0 },
} ;
#define NUM_MENUITEMS G_N_ELEMENTS(amo)
    
// -----------------------------------------------------------------------
//
//      Implement AbiGeneric Plugin Interface 
//
// -----------------------------------------------------------------------


static XAP_ModuleInfo AbiPaintModuleInfo = 
{
    "AbiPaint",											/* name */
    "Allows editing an embedded image via external image editing program.",	/* desc */
    ABI_PLUGIN_mkstr(ABI_PLUGIN_VERSION) " for AbiWord " ABI_BUILD_VERSION , 	/* version */
    "Abi the Ant",										/* author */
    "Select Image 1st, then select the action from AbiPaint menu.  ;-)",	/* usage */
} ;
    
XAP_ModuleInfo * getModuleInfo(void)
{
	return &AbiPaintModuleInfo;
}

bool doRegistration(void)
{
    // Get XAP_Prefs object for retrieving/storing image editor and related preferences
    UT_return_val_if_fail(prefs != NULL, false);
    if ((prefsScheme = prefs->getPluginScheme(szAbiPluginSchemeName)) == nullptr) {
      // it may not exist so try creating & adding it.
      prefs->addPluginScheme(new XAP_PrefsScheme(prefs, szAbiPluginSchemeName));
	// if it still isn't there then fail
      if ((prefsScheme = prefs->getPluginScheme(szAbiPluginSchemeName)) == nullptr) {
        return false;
      }

      // go ahead and set our default values
      std::string szProgramName;
      bool bLeaveImageAsPNG;
      getDefaultApp(szProgramName, bLeaveImageAsPNG);
      prefsScheme->setValue(ABIPAINT_PREF_KEY_szProgramName, szProgramName.c_str());
      prefsScheme->setValueBool(ABIPAINT_PREF_KEY_bLeaveImageAsPNG, bLeaveImageAsPNG);
    }


    // Add the image editor to AbiWord's menus.
    addToMenus(amo, NUM_MENUITEMS, AP_MENU_ID_TOOLS_WORDCOUNT, AP_MENU_ID_CONTEXT_IMAGE);


    return true;
}

void doUnregistration(void)
{
    removeFromMenus(amo, NUM_MENUITEMS);
}


//
// AbiPaint specify image editor
// -------------------
//   This is the function sets which image editor will (at least attempted) be invoked.
//
//   parameters are:
//     AV_View* v
//     EV_EditMethodCallData *d)
//
static DECLARE_ABI_PLUGIN_METHOD(specify)
{
	UT_UNUSED(v);
	UT_UNUSED(d);
	// get current value
	std::string szProgramName;
	prefsScheme->getValue(ABIPAINT_PREF_KEY_szProgramName, szProgramName);

	// Get a frame in case we need to show an error message
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();

	{
		const char * szDescList[3];
		const char * szSuffixList[3];
		int ft[3];
		szDescList[0] = szProgramsDesc;
		szSuffixList[0] = szProgramSuffix;
		szDescList[1] = szSuffixList[1] = NULL;
		ft[0] = ft[1] = ft[2] = IEGFT_Unknown;

		if (getFileName(szProgramName, pFrame, XAP_DIALOG_ID_FILE_OPEN, szDescList, szSuffixList, ft))
			return false;

		UT_DEBUGMSG(("ABIPAINT: szProgramName to use is  %s\n", szProgramName.c_str()));
	}

	// now write it to the preference
	prefsScheme->setValue(ABIPAINT_PREF_KEY_szProgramName, szProgramName.c_str());

	return true;
}


//   When no image is selected, we gray out this menu item
DECLARE_ABI_GRAYABLE_MENUITEM(editImage,isImageSelected)


#ifdef ENABLE_BMP

// This function returns current settings, ie whether image editor requires BMP or can work with PNG
bool leaveImageAsPng(void)
{
	bool bLeaveImageAsPNG = false;  // if value not found, then assume we must convert
	prefsScheme->getValueBool(ABIPAINT_PREF_KEY_bLeaveImageAsPNG, &bLeaveImageAsPNG);
	return bLeaveImageAsPNG;
}

// Function that returns current menu state (toggled or not)
DECLARE_ABI_TOGGLEABLE_MENUITEM(useBmp,leaveImageAsPng)


//
// AbiPaint useBmp
// ---------------
//   This function sets whether image editor can work with PNG files
//   or requires conversion to platforms default image format (eg requires BMP on Windows)
//   Note: the conversion may advserly alter the image
//
//   parameters are:
//     AV_View* v
//     EV_EditMethodCallData *d)
//
static DECLARE_ABI_PLUGIN_METHOD(useBmp)
{
	// get and toggle current state
	bool bLeaveImageAsPNG = !leaveImageAsPng();

	// now write it to the preference
	prefsScheme->setValueBool(ABIPAINT_PREF_KEY_bLeaveImageAsPNG, bLeaveImageAsPNG);

	return true;
}


//
// AbiPaint saveAsBmp
// ------------------
//   This is the function exports selected image as a Windows BMP file.
//
//   parameters are:
//     AV_View* v
//     EV_EditMethodCallData *d)
//
static DECLARE_ABI_PLUGIN_METHOD(saveAsBmp)
{
	// Get a frame (for error messages) and (to) get the current view that the user is in.
	XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
	FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());

	char *szTempFileName = NULL;
	GError *err = NULL;
	gint fp = g_file_open_tmp ("XXXXXX", &szTempFileName, &err);
	if (err) {
		g_warning (err->message);
		g_error_free (err); err = NULL;
		return FALSE;
	}
	close(fp);

	std::string szTmpPng = szTempFileName;
	szTmpPng += ".png";
	remove(szTempFileName);
	g_free (szTempFileName); szTempFileName = NULL;

	PT_DocPosition pos = pView->saveSelectedImage(szTmpPng.c_str());
	if(pos == 0)
	{
		pFrame->showMessageBox("You must select an Image before trying to save it as a BMP file!", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
		return false;
	}

	//
	// Convert png into bmp
	// NOTE: probably looses detail/information though!!!
	//

	std::string szBMPFile = pFrame->getFilename(); // perhaps a different default directory should be used???
	{
		const char * szDescList[2];
		const char * szSuffixList[2];
		IEGraphicFileType ft[2];
		{
			// IE_ImpGraphicBMP_Sniffer tmp;
			// tmp.getDlgLabels(szDescList, szSuffixList, ft);
			szDescList[0] = "Windows Bitmap (*.bmp)";
			szSuffixList[0] = "*.bmp";
			ft[0] = IEGFT_BMP;
		}
		szDescList[1] = szSuffixList[1] = NULL;
		ft[1] = IEGFT_Unknown;

		if (getFileName(szBMPFile, pFrame, XAP_DIALOG_ID_FILE_SAVEAS, szDescList, szSuffixList, ft))
		{
			// user canceled
			remove(szTmpPng.c_str());
			return true;
		}
	}

	if (convertPNG2BMP(szTmpPng.c_str(), szBMPFile.c_str()))
	{
		pFrame->showMessageBox("Unable to convert PNG image data to BMP.", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

		remove(szTmpPng.c_str());
		return false;
	}
	remove(szTmpPng.c_str());
	return true;
}

#endif /* ENABLE_BMP */


//
// getEditImageMenuName
// ----------------------
// returns menu name to edit image with (eg Edit Image via <program>)
// Note: While this allows us to optionally include the program or whatever,
//       the primary purposes is to allow us to use the same name as
//       any other plugin (specifically AbiGimp) without them colliding.
//       [we use a different static name and possibly identical dynamic name]
// const char * getEditImageMenuName(XAP_Frame * pFrame, const EV_Menu_Label * pLabel, XAP_Menu_Id id)
Defun_EV_GetMenuItemComputedLabel_Fn(getEditImageMenuName)
{
	UT_UNUSED(pLabel);
	UT_UNUSED(id);

	std::string szProgramName;
	static std::string MenuName;
	MenuName = "&Edit Image";

	// give user some indication of program that will be executed
	if (prefsScheme->getValue(ABIPAINT_PREF_KEY_szProgramName, szProgramName))
	{
		// we now have the full program name (with path & extension), so prune
		MenuName += " via ";
		MenuName += UT_basename(szProgramName.c_str());

		// limit menu length to max of 33 (31 characters + two dots ..)
		if (MenuName.size() > 33)
		{
			MenuName = MenuName.substr(0, 31);
			MenuName += ".. ";  // note the space is to separate these dots from the menu dots
		}
	}

	return MenuName.c_str();
}


//
// AbiPaint editImage
// ------------------
//   This is the function that we actually call to invoke the image editor.
//
//   parameters are:
//     AV_View* v
//     EV_EditMethodCallData *d
//
static DECLARE_ABI_PLUGIN_METHOD(editImage)
{
	UT_UNUSED(v);
    // Get the current view that the user is in.
    XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
    FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());

//
// get values from preference (initial plugin execution should have set sensible defaults)
//
    std::string imageApp;  // holds MAXPATH\appName <space> MAXPATH\imagefilename
	bool bLeaveImageAsPNG;

	// read stuff from the preference value
	if (!prefsScheme->getValue(ABIPAINT_PREF_KEY_szProgramName, imageApp))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		getDefaultApp(imageApp, bLeaveImageAsPNG);
	}

	// now that we have program name, try to get other flag (allows overriding default value)
	// Note: we allow overriding, otherwise if we don't adhere to user's setting
	//       then the use BMP or not menu should be greyed to note it has no effect
	prefsScheme->getValueBool(ABIPAINT_PREF_KEY_bLeaveImageAsPNG, &bLeaveImageAsPNG);


//
// generate a temp file name...
//
	char *szTempFileName = NULL;
	GError *err = NULL;
	gint fp = g_file_open_tmp ("XXXXXX", &szTempFileName, &err);
	if (err) {
		g_warning ("%s", err->message);
		g_error_free (err); err = NULL;
		return FALSE;
	}
	close(fp);

	std::string szTmpPng = szTempFileName;
	szTmpPng += ".png";
	std::string szTmp = szTmpPng; // default: our temp file is the created png file

	PT_DocPosition pos = pView->saveSelectedImage((const char *)szTmpPng.c_str());
	if(pos == 0)
	{
		remove(szTempFileName);
		g_free (szTempFileName); szTempFileName = NULL;
		pFrame->showMessageBox("You must select an Image before editing it", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
		return false;
	}

#ifdef ENABLE_BMP
//
// Convert png into bmp for best compatibility with Windows programs
// NOTE: probably looses detail/information though!!! so if possible use PNG
//
	if (!bLeaveImageAsPNG)
	{
		szTmp = szTempFileName;
		szTmp += ".bmp";	// our temp file is a bmp file

		if (convertPNG2BMP(szTmpPng.c_str(), szTmp.c_str()))
		{
			pFrame->showMessageBox("Unable to convert PNG image data to BMP for external program use!", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

			remove(szTempFileName);
			g_free (szTempFileName); szTempFileName = NULL;
			remove(szTmpPng.c_str());
			return false;
		}
		// remove(szTmpPng.c_str());

	}
#endif
	
	// remove the temp file (that lacks proper extension)
	remove(szTempFileName);
	g_free (szTempFileName); szTempFileName = NULL;

//
// Get the initial file status.
//
	struct stat myFileStat;
	int ok = stat(szTmp.c_str(),&myFileStat);
	if(ok < 0)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		remove(szTmpPng.c_str());
		remove(szTmp.c_str());	// should silently fail if exporting as PNG file
		return false;
	}
	time_t mod_time = myFileStat.st_mtime;

//	
// Fire up the image editor...
//
	ProcessInfo procInfo;
	if (!createChildProcess(imageApp.c_str(), szTmp.c_str(), &procInfo))

	{
		std::string msg = "Unable to run program: ";  
		msg += imageApp + " " + szTmp;
		pFrame->showMessageBox(msg.c_str(), XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);

		// failed to spawn stuff, so do some cleanup and return failure
		remove(szTmpPng.c_str());
		remove(szTmp.c_str());	// should silently fail if exporting as PNG file
		return false;
	}

	lockGUI(d);

	while (isProcessStillAlive(procInfo))
	{
		UT_usleep(10000); // wait 10 milliseconds
		pFrame->nullUpdate();
		ok = stat(szTmp.c_str(),&myFileStat);
		if(ok == 0)
		{
			if(myFileStat.st_mtime != mod_time)
			{
				// wait for changes to settle (program done writing changes)
				// we use both modified time & file size, but really we
				// could just use file size as mod time doesn't appear to change for small images
				mod_time = myFileStat.st_mtime;
				off_t size = myFileStat.st_size;
				UT_usleep(100000); // wait 100 milliseconds (so program may have time to write something)
				ok = stat(szTmp.c_str(),&myFileStat);
				while((mod_time != myFileStat.st_mtime) || !size || (size > 0 && size != myFileStat.st_size))
				{
					mod_time = myFileStat.st_mtime;
					size = myFileStat.st_size;
					ok = stat(szTmp.c_str(),&myFileStat);
					UT_usleep(500000); // wait a while, let program write its data

					// just make sure the program is still running, otherwise we could get stuck in a loop
					if (!isProcessStillAlive(procInfo))
					{
						pFrame->showMessageBox("External image editor appears to have been terminated unexpectedly.", 
								XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
						//procInfo.hProcess = 0;
						goto Cleanup;
					}
				}
				mod_time = myFileStat.st_mtime;
				UT_usleep(100000); // wait a while just to make sure program is done with file

//
// OK replace the current image with this.
//
				IEGraphicFileType iegft = IEGFT_Unknown;
				FG_ConstGraphicPtr pFG;
		
				UT_Error errorCode;
		
#ifdef ENABLE_BMP
//
// Convert bmp back to png (as we can not assume AbiWord has builtin BMP support [as its now an optional plugin])
// NOTE: probably looses detail/information though!!! so if possible use only PNG
//
				if (!bLeaveImageAsPNG)
				{
					if (convertBMP2PNG(szTmp.c_str(), szTmpPng.c_str()))
					{
						pFrame->showMessageBox("Unable to convert BMP image data back to PNG for AbiWord to import!", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
						goto Cleanup;
					}
				}
#endif

				errorCode = IE_ImpGraphic::loadGraphic(szTmpPng.c_str(), iegft, pFG);
				if(errorCode)
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					pFrame->showMessageBox("Error making pFG. Could not put image back into Abiword", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
					goto Cleanup;
				}

				unlockGUI(d);

				pView->cmdUnselectSelection();
				pView->setPoint(pos);
				pView->extSelHorizontal(true, 1); // move point forward one
				errorCode = pView->cmdInsertGraphic(pFG);
				if (errorCode)
				{
					pFrame->showMessageBox("Could not put image back into Abiword", XAP_Dialog_MessageBox::b_O,XAP_Dialog_MessageBox::a_OK);
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					goto Cleanup;
				}
//
// Reselect the image
//
				pView->setPoint(pos);
				pView->extSelHorizontal(true, 1); // move point forward one

				lockGUI(d);
			}
		}
	}

//
// Normal exit, delete the tempfile and return success
//
	remove(szTmpPng.c_str());
	remove(szTmp.c_str());	// should silently fail if exporting as PNG file
	unlockGUI(d);
	return true;

//
// Something went wrong.
//
 Cleanup: 
	remove(szTmpPng.c_str());
	remove(szTmp.c_str());	// should silently fail if exporting as PNG file
	unlockGUI(d);
//
// Kill the image editor.
//
	endProcess(procInfo);
	return false;
}


static void getDefaultApp(std::string &imageApp, bool &bLeaveImageAsPNG)
{
#ifdef _WIN32
	bLeaveImageAsPNG = false;
	imageApp.clear();

	char buffer[MAX_PATH];
	// for WinNT mspaint is most likely in the system directory (eg C:\WINNT\SYSTEM32\MSPAINT.EXE)
	if (GetSystemDirectoryA(buffer, MAX_PATH))
	{
		imageApp = buffer;
		imageApp += "\\MSPAINT.EXE";
		UT_DEBUGMSG(("ABIPAINT: Checking if %s exists\n", imageApp.c_str()));
		if (!UT_isRegularFile(imageApp.c_str()))
			imageApp.clear();
	}
	// if not there, try in Win95b directory (eg %PROGRAMFILES%\ACCESSORIES\MSPAINT.EXE)
	if (imageApp.empty())
	{
		HKEY hKey;
		unsigned long lType;
		DWORD dwSize;
		unsigned char* szValue = NULL;
		if( ::RegOpenKeyExA( HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS )
		{
			if( ::RegQueryValueExA( hKey, "ProgramFilesDir", NULL, &lType, NULL, &dwSize) == ERROR_SUCCESS )
			{
				szValue = new unsigned char[dwSize + 1];
				::RegQueryValueExA( hKey, "ProgramFilesDir", NULL, &lType, szValue, &dwSize);
				imageApp = (char*) szValue;
				delete[] szValue;
				imageApp += "\\ACCESSORIES\\MSPAINT.EXE";
				UT_DEBUGMSG(("ABIPAINT: Checking if %s exists\n", imageApp.c_str()));
				if (!UT_isRegularFile(imageApp.c_str()))
					imageApp.clear();
			}
			::RegCloseKey(hKey);
		}
	}
	// if we still haven't found the file, then simply try mspaint.exe
	if (imageApp.empty())
	{
		imageApp = "mspaint.exe";
		UT_DEBUGMSG(("ABIPAINT: Falling back to %s (will probably fail)\n", imageApp.c_str()));
	}
#else
	// for other platforms default to the GIMP, assume in path
	bLeaveImageAsPNG = true;
	imageApp = "gimp";
#endif
}


