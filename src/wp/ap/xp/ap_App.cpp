/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
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

#include "ap_App.h"
#include "ap_Args.h"
#include "xap_Frame.h"

#include "ie_imp.h"

#if defined(WIN32)
AP_App::AP_App (HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName)
  : XAP_App_BaseClass ( hInstance, pArgs, szAppName )
#else
AP_App::AP_App (XAP_Args * pArgs, const char * szAppName)
  : XAP_App_BaseClass ( pArgs, szAppName )
#endif
{
}

AP_App::~AP_App ()
{
}

/*!
 *  Open windows requested on commandline.
 * 
 * \return False if an unknown command line option was used, true
 * otherwise.  
 */
bool AP_App::openCmdLineFiles(poptContext poptcon)
{
	int kWindowsOpened = 0;
	const char *file = NULL;

	while ((file = poptGetArg (poptcon)) != NULL) {
		XAP_Frame * pFrame = newFrame(this);

		UT_Error error = pFrame->loadDocument
			(file, IEFT_Unknown, true);
		if (!error)
		{
			kWindowsOpened++;
		}
		else
		{
			// TODO we crash if we just delete this without putting something
			// TODO in it, so let's go ahead and open an untitled document
			// TODO for now.  this would cause us to get 2 untitled documents
			// TODO if the user gave us 2 bogus pathnames....

			// Because of the incremental loader, we should not crash anymore;
			// I've got other things to do now though.
			kWindowsOpened++;
			pFrame->loadDocument(NULL, IEFT_Unknown);
			pFrame->raise();

			errorMsgBadFile (pFrame, file, error);
		}
	}

	if (kWindowsOpened == 0)
	{
		// no documents specified or openable, open an untitled one
		
		XAP_Frame * pFrame = newFrame(this);
		pFrame->loadDocument(NULL, IEFT_Unknown);
	}

	return true;
}

/*! Processes all the command line options and puts them in AP_Args.
 * Leaves the files to open in the poptContext for ::openCmdLineFiles
 * to handle.
 */
void AP_App::initPopt (AP_Args * Args)
{
	int nextopt, v, i;

	for (i = 0; Args->const_opts[i].longName != NULL; i++)
		;

	v = i;

	struct poptOption * opts = (struct poptOption *)
		UT_calloc(v+1, sizeof(struct poptOption));
	for (int j = 0; j < v; j++)
		opts[j] = Args->const_opts[j];

	Args->options = opts;
	Args->poptcon = poptGetContext("AbiWord", 
				       Args->XArgs->m_argc, Args->XArgs->m_argv, 
				       Args->options, 0);

    while ((nextopt = poptGetNextOpt (Args->poptcon)) > 0 &&
		   nextopt != POPT_ERROR_BADOPT)
        /* do nothing */ ;

    if (nextopt != -1) 
	{
		errorMsgBadArg(Args, nextopt);
        exit (1);
    }
}

void AP_App::errorMsgBadArg (AP_Args *, int)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void AP_App::errorMsgBadFile(XAP_Frame *, const char *, UT_Error)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

bool AP_App::doWindowlessArgs (const AP_Args *)
{
	return false;
}

XAP_Frame * AP_App::newFrame(AP_App *)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}
