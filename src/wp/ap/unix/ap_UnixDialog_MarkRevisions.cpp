/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include <stdlib.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MarkRevisions.h"
#include "ap_UnixDialog_MarkRevisions.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MarkRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_MarkRevisions * p = new AP_UnixDialog_MarkRevisions(pFactory,id);
	return p;
}

AP_UnixDialog_MarkRevisions::AP_UnixDialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MarkRevisions(pDlgFactory,id)
{
}

AP_UnixDialog_MarkRevisions::~AP_UnixDialog_MarkRevisions(void)
{
}

void AP_UnixDialog_MarkRevisions::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	/*
	   This is the only function you need to implement, and the MarkRevisions
	   dialogue should look like this:

	   ----------------------------------------------------
	   | Title                                            |
       ----------------------------------------------------
	   |                                                  |
	   | O Radio1                                         |
	   |    Comment1 (a label)                            |
	   |                                                  |
	   | O Radio2                                         |
	   |    Comment2Label                                 |
	   |    Comment2 (an edit control)                    |
       |                                                  |
       |                                                  |
       |     OK_BUTTON              CANCEL_BUTTON         |
	   ----------------------------------------------------

	   Where: Title, Comment1 and Comment2Label are labels, Radio1-2
	   is are radio buttons, Comment2 is an Edit control.

	   Use getTitle(), getComment1(), getComment2Label(), getRadio1Label()
	   and getRadio2Label() to get the labels (the last two for the radio
	   buttons), note that you are responsible for freeing the
	   pointers returned by getLable1() and getComment1() using FREEP
	   (but not the rest!)

	   if getLabel1() returns NULL, hide the radio buttons and enable
	   the Edit box; otherwise the Edit box should be only enabled when
	   Radio2 is selected.

	   Use setComment2(const char * pszString) to store the contents of the Edit control
       when the dialogue closes; make sure that you freee pszString afterwards.


	*/

	UT_ASSERT(UT_NOT_IMPLEMENTED);
}
