/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
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

#ifndef AP_STATUSBAR_H
#define AP_STATUSBAR_H

#include <limits.h>
// Class for dealing with the status bar at the bottom of
// the frame.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_units.h"
#include "ut_vector.h"
#include "ap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"
#include "xav_Listener.h"
#include "ut_string_class.h"

class XAP_Frame;
class GR_Graphics;

#define AP_MAX_MESSAGE_FIELD			(256*MB_LEN_MAX)

/*****************************************************************/
/*****************************************************************/

#define PROGRESS_CMD_MASK 0x3		/* 0,1,2,3 Operational values */
enum _progress_flags {
    PROGRESS_RESERVED1 	= 0x0,	
    PROGRESS_STARTBAR  	= 0x1,		/* Start using the progress bar */
    PROGRESS_STOPBAR	= 0x2,		/* Stop using the progress bar */	
    PROGRESS_RESERVED2	= 0x3,
    PROGRESS_SHOW_MSG	= 0x4,		/* Allow message to be displayed */
    PROGRESS_SHOW_RAW	= 0x8,		/* Allow raw value to be displayed */
    PROGRESS_SHOW_PERCENT = 0x10	/* Allow calculation of percent value */
};

#include "ut_timer.h"

// NOTE BY WILL LACHANCE (Tue. Oct 22/2002): This code is less of a mess than it used to be, but
// it is still far from ideal. We pretty much statically define the statusbar here (and
// have classes for each statusbar element, to boot). Ideally, we would somehow create a statusbar
// at run-time using an XML file. But it's too much work, for too little benefit, to do that right now.

class AP_StatusBarField;


class ABI_EXPORT AP_StatusBar : public AV_Listener
{
public:
    AP_StatusBar(XAP_Frame * pFrame);
    virtual ~AP_StatusBar(void);

    XAP_Frame *			getFrame(void) const;
    virtual void		setView(AV_View * pView);
    void			setStatusMessage(UT_UCSChar * pbufUCS, int redraw = true);
    void			setStatusMessage(const char * pbuf, int redraw = true);
    const UT_UTF8String & 	getStatusMessage(void) const;

    void			setStatusProgressType(int start, int end, int flags);
    void 			setStatusProgressValue(int value);

    virtual void		show(void) {} // It must be abstract, but I don't want to screw
    virtual void		hide(void) {} // the platforms that don't implement show/hide
	
    /* used with AV_Listener */
    virtual bool		    notify(AV_View * pView, const AV_ChangeMask mask);
    virtual AV_ListenerType getType(void) { return AV_LISTENER_STATUSBAR;}

    UT_GenericVector<AP_StatusBarField*> *             getFields() { return &m_vecFields; }
protected:

    XAP_Frame *			m_pFrame;
    AV_View *			m_pView;

    bool			m_bInitFields;
    UT_GenericVector<AP_StatusBarField*> m_vecFields;			/* vector of 'ap_sb_Field *' */
    void *			m_pStatusMessageField;	/* actually 'AP_StatusBarField_StatusMessage *' */
    void *			m_pStatusProgressField;	/* actually 'AP_StatusBarField_ProgressBar *' */

    
    UT_UTF8String		m_sStatusMessage;
};

// abstract class which "listens" for changes in the status bar fields in the base classes
// intended for platform specific code
class AP_StatusBarField; // fwd decl

class ABI_EXPORT AP_StatusBarFieldListener
{
public:
    AP_StatusBarFieldListener(AP_StatusBarField *pStatusBarField) { m_pStatusBarField = pStatusBarField; }
    virtual ~AP_StatusBarFieldListener() {}
    virtual void notify() = 0;

protected:
    AP_StatusBarField *m_pStatusBarField;
};

// alignment/fill properties for the upper level gui
enum _statusbar_element_fill_method {
    REPRESENTATIVE_STRING,
    MAX_POSSIBLE
};

enum _statusbar_textelement_alignment_method {
    LEFT,
    CENTER
};

// AP_StatusBarField: abstract base class for a status bar field
class ABI_EXPORT AP_StatusBarField
{
public:
    AP_StatusBarField(AP_StatusBar * pSB);
    virtual ~AP_StatusBarField(void);
		
    virtual void		notify(AV_View * pView, const AV_ChangeMask mask) = 0;
    void setListener(AP_StatusBarFieldListener *pStatusBarFieldListener) { m_pStatusBarFieldListener = pStatusBarFieldListener; }
    AP_StatusBarFieldListener * getListener() { return m_pStatusBarFieldListener; }

    _statusbar_element_fill_method getFillMethod() { return m_fillMethod; }

protected:
    AP_StatusBar *		m_pSB;
    AP_StatusBarFieldListener *m_pStatusBarFieldListener;
    _statusbar_element_fill_method m_fillMethod;
};

class ABI_EXPORT AP_StatusBarField_TextInfo : public AP_StatusBarField
{
public:
    AP_StatusBarField_TextInfo(AP_StatusBar * pSB); 
    //virtual ~AP_StatusBarField_TextInfo(void) {}
    const UT_UTF8String & getBuf() { return m_sBuf; }
    // getRepresentativeString: give a "guess" as to how long the string will be. it's not a big deal
    // if it's wrong; we should resize fixed-length status bar elements in platform specific code 
    // if they're not big enough to show the string correctly
    const char * getRepresentativeString(void) { return m_sRepresentativeString.utf8_str(); }
    _statusbar_textelement_alignment_method getAlignmentMethod() { return m_alignmentMethod; }

protected:
    UT_UTF8String m_sBuf;
    UT_UTF8String m_sRepresentativeString;
    _statusbar_textelement_alignment_method m_alignmentMethod;
};

// PROGRESSBAR. CURRENTLY UNUSED. MAY BE BROKEN. NEEDS TESTING.
class ABI_EXPORT AP_StatusBarField_ProgressBar : public AP_StatusBarField
{
public:
    AP_StatusBarField_ProgressBar(AP_StatusBar * pSB);
    virtual ~AP_StatusBarField_ProgressBar(void);

    virtual void		notify(AV_View * pView, const AV_ChangeMask mask);
    void setStatusProgressType(int start, int end, int flags);
    void setStatusProgressValue(int value);

protected:
    UT_sint32			m_ProgressStart;
    UT_sint32			m_ProgressEnd;
    UT_sint32			m_ProgressValue;
    UT_sint32			m_ProgressStartPoint;
    UT_uint32			m_ProgressFlags;
    UT_Timer			*m_ProgressTimer;
};
#endif /* AP_STATUSBAR_H */
