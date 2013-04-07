/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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

#ifndef AP_DIALOG_MODELESS_H
#define AP_DIALOG_MODELESS_H

#include <string>

#include "xap_Dialog.h"
#include "fv_View.h"
#include "pd_DocumentRDF.h"
#include "xap_Strings.h"
class XAP_Frame;

class ABI_EXPORT AP_Dialog_Modeless : public XAP_Dialog_Modeless
{
  protected:
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /*** These methods rely on the later pure virtual ones to provide */
    /*** reasonable defaults. They are virtual so that subclasses can */
    /*** change them if specialized behavior is desired.              */
    /************************************************************/
    /************************************************************/
    /************************************************************/

	virtual void ConstructWindowName();

    /************************************************************/
    /************************************************************/
    /************************************************************/
    /*** pure virtual methods used by the above methods *********/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    //
    virtual XAP_String_Id getWindowTitleStringId() = 0;
    virtual void setStatus( const std::string& msg );

public:
	AP_Dialog_Modeless(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~AP_Dialog_Modeless();

	// these are kinda screwy now, but we never return anything but on
	// "cancel" or "close"
	typedef enum {
		a_CLOSE
	} tAnswer;

	AP_Dialog_Modeless::tAnswer getAnswer() const;
	// These are called from edit methods or from dialogs
	// to set or read the variables in the current
	// instance of the dialog.  These do not read the persistent
	// values.
  	bool						setView(FV_View * view);
  	FV_View * 					getView() const;
	void						setActiveFrame(XAP_Frame *pFrame);

    virtual void                maybeClosePopupPreviewBubbles();
    void                        closePopupPreviewBubbles();
    virtual void                maybeReallowPopupPreviewBubbles();

protected:
	// These are the "current use" dialog data items,
	// which are liberally read and set by the
	// accessor methods above.
  	FV_View * 					m_pView;
    std::string                 m_WindowName;
	// is this used in a modeless dialog like this?
	tAnswer						m_answer;

    FV_View_BubbleBlocker       m_bubbleBlocker;
};

#endif
