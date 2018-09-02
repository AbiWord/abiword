/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_DIALOG_H
#define XAP_DIALOG_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include <glib.h>

#include "ut_misc.h"
#include "ut_assert.h"

#include "xap_Types.h"

class XAP_DialogFactory;
class XAP_App;
class XAP_Frame;
class XAP_Widget;
class UT_UTF8String;
class XAP_Dialog;

class AV_View;

/*****************************************************************
******************************************************************
** This file defines the base classes for cross-platform dialogs.
******************************************************************
*****************************************************************/

typedef enum _XAP_Dialog_Type
{
	XAP_DLGT_NON_PERSISTENT		= 1,
	XAP_DLGT_FRAME_PERSISTENT	= 2,
	XAP_DLGT_APP_PERSISTENT		= 3,
	XAP_DLGT_MODELESS		= 4

} XAP_Dialog_Type;

typedef int xap_widget_id;

typedef XAP_Dialog *(*pt2Constructor)(XAP_DialogFactory * pFactory, XAP_Dialog_Id id );

class ABI_EXPORT XAP_Dialog
{
public:

	XAP_Dialog(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id,
		   const char * helpUrl = NULL );
	virtual ~XAP_Dialog(void);

	virtual void				runModal(XAP_Frame * pFrame) = 0;

	XAP_Dialog_Id				getDialogId(void) const { return m_id; }
	XAP_App *				getApp(void) const { return m_pApp;}

	const std::string& getHelpUrl () const { return m_helpUrl ; }


	/** get a widget state (enabled/disabled) */
	bool getWidgetState(xap_widget_id wid);
	/** set a widget state (enable/disabled) */
	void setWidgetState(xap_widget_id wid, bool state);

	/** get a widget visible state */
	bool getWidgetVisible(xap_widget_id wid);
	/** set a widget visible state */
	void setWidgetVisible(xap_widget_id wid, bool visible);

	/** get a widget value int */
	int getWidgetValueInt(xap_widget_id wid);
	/** set a widget value int */
	void setWidgetValueInt(xap_widget_id wid, int value);

	/** get a widget value UTF-8 string */
	void getWidgetValueString(xap_widget_id wid, UT_UTF8String &str);
	/** set a widget value UTF-8 string */
	void setWidgetValueString(xap_widget_id wid, const UT_UTF8String &str);

	/** get a widget value float*/
	float getWidgetValueFloat(xap_widget_id wid);
	/** set a widget value float */
	void setWidgetValueFloat(xap_widget_id wid, float value);

	/** set the widget label */
	void setWidgetLabel(xap_widget_id wid, const UT_UTF8String &val);
	void setWidgetLabel(xap_widget_id wid, const std::string &val);

	// dialog framework

	/** set the data to the widgets in the dialog. Dialog specific, XP*/
	virtual void updateDialogData(void) {UT_ASSERT(UT_NOT_IMPLEMENTED);} //FIXME = 0

    virtual void maybeClosePopupPreviewBubbles();
    virtual void maybeReallowPopupPreviewBubbles();

protected:
	/** localize the widgets in the dialog. Dialog specific, XP */
	virtual void localizeDialog(void) {UT_ASSERT(UT_NOT_IMPLEMENTED);} //FIXME = 0
	/** construct the dialog. Dialog and platfom specific */
	virtual void constructDialog(void) {UT_ASSERT(UT_NOT_IMPLEMENTED);} //FIXME = 0

	/** convert widget ID to XAP_Widget. Must be implemented by each dialogs
	   on each platforms
	   \return a newly allocated XAP_Widget. Caller is responsible from
	   freeing it.
	*/
	virtual XAP_Widget *getWidget(xap_widget_id /*wid*/) { return NULL; };

	XAP_App *				m_pApp;
	XAP_DialogFactory *			m_pDlgFactory;
	XAP_Dialog_Id				m_id;

private:
	std::string m_helpUrl ;
};


class ABI_EXPORT XAP_Dialog_NonPersistent : public XAP_Dialog
{
public:
	XAP_Dialog_NonPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~XAP_Dialog_NonPersistent(void);

	virtual void			runModal(XAP_Frame * pFrame) = 0;

	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_NON_PERSISTENT; };

protected:
};


class ABI_EXPORT XAP_TabbedDialog_NonPersistent : public XAP_Dialog_NonPersistent
{
public:
	XAP_TabbedDialog_NonPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~XAP_TabbedDialog_NonPersistent(void);

	virtual void			setInitialPageNum 	(int which) { m_pageNum = which; } // support for dialogs with pages (tabs?)
	virtual int				getInitialPageNum 	() { return m_pageNum; }

protected:
	int		m_pageNum;
};


class ABI_EXPORT XAP_Dialog_Persistent : public XAP_Dialog
{
public:
	XAP_Dialog_Persistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~XAP_Dialog_Persistent(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

protected:
	bool						m_bInUse;
};

class ABI_EXPORT XAP_Dialog_FramePersistent : public XAP_Dialog_Persistent
{
public:
	XAP_Dialog_FramePersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~XAP_Dialog_FramePersistent(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_FRAME_PERSISTENT; };

protected:
};

class ABI_EXPORT XAP_Dialog_AppPersistent : public XAP_Dialog_Persistent
{
public:
	XAP_Dialog_AppPersistent(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~XAP_Dialog_AppPersistent(void);

	virtual void				useStart(void);
	virtual void				runModal(XAP_Frame * pFrame) = 0;
	virtual void				useEnd(void);

	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_APP_PERSISTENT; };

protected:
};


class ABI_EXPORT XAP_Dialog_Modeless : public XAP_Dialog_AppPersistent
{
public:
	XAP_Dialog_Modeless(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id, const char * helpUrl = NULL );
	virtual ~XAP_Dialog_Modeless(void);

	void						useStart(void);
	void						useEnd(void);

// runModal is not a virtual pure function.  It's here only to make happy the old
// dialogs that have been partially transformed to non modal dialogs (is it true?)
	virtual void				runModal(XAP_Frame * /*pFrame*/) {}

	virtual void				runModeless(XAP_Frame * pFrame) = 0;
	virtual void				setActiveFrame(XAP_Frame *pFrame);

// not pure functions.  Unix doesn't need to implement these functions
	virtual void				notifyActiveFrame(XAP_Frame * /*pFrame*/) {}
	virtual void				notifyCloseFrame(XAP_Frame * /*pFrame*/) {}

	virtual void				destroy(void) = 0;
	virtual void				activate(void) = 0;
	XAP_Frame *					getActiveFrame() const;
	virtual void                modeless_cleanup(void);
	bool						isRunning(void) const;
    std::string                 BuildWindowName( const char * pDialogName ) const;
	void						BuildWindowName(char * pWindowName, const char * pDialogName, UT_uint32 width ) const;
	static XAP_Dialog_Type		s_getPersistence(void) { return XAP_DLGT_APP_PERSISTENT; };

	// ugly hack necessary for Win32
	virtual void *				pGetWindowHandle(void) { return NULL; }

protected:
        XAP_Dialog_Modeless *                    m_pDialog;
};


/*!
 * Interface for a tabbed dialog to be extensible by plugins.
 */
class ABI_EXPORT XAP_NotebookDialog
{
public:

	class ABI_EXPORT Page {
	public:
		Page() {}
		Page(const gchar *_title, AbiNativeWidget * _widget);
		~Page();

		gchar 			* title;
		AbiNativeWidget * widget;
	};

	virtual ~XAP_NotebookDialog() {}
	virtual void addPage (const XAP_NotebookDialog::Page *page) = 0;
};

#endif /* XAP_DIALOG_H */
