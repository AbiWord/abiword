/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#ifndef XAP_COCOADIALOG_PLUGIN_MANAGER_H
#define XAP_COCOADIALOG_PLUGIN_MANAGER_H

#include <Cocoa/Cocoa.h>

#include "xap_App.h"
#include "xap_Dlg_PluginManager.h"
#include "xap_Module.h"

#import "xap_Cocoa_NSTableUtils.h"

class XAP_Frame;
class XAP_CocoaDialog_PluginManager;

@interface XAP_CocoaDlg_PluginManagerController : NSWindowController <XAP_CocoaDialogProtocol>
{
    IBOutlet NSTextField *_authorData;
    IBOutlet NSTextField *_authorLabel;
    IBOutlet NSButton *_closeBtn;
    IBOutlet NSButton *_deactivateAllBtn;
    IBOutlet NSButton *_deactivateBtn;
    IBOutlet NSTextField *_descriptionData;
    IBOutlet NSTextField *_descriptionLabel;
    IBOutlet NSBox *_detailsBox;
    IBOutlet NSButton *_installBtn;
    IBOutlet NSTextField *_nameData;
    IBOutlet NSTextField *_nameLabel;
    IBOutlet NSTableView *_pluginList;
    IBOutlet NSTextField *_versionData;
    IBOutlet NSTextField *_versionLabel;
	XAP_CocoaDialog_PluginManager*	_xap;
}
- (IBAction)closeAction:(id)sender;
- (IBAction)deactivateAction:(id)sender;
- (IBAction)deactivateAllAction:(id)sender;
- (IBAction)installAction:(id)sender;
- (IBAction)selectAction:(id)sender;

- (int)selectedPlugin;
- (void)setSelectedPlugin:(int)idx;
- (void)setModuleInfo:(const XAP_ModuleInfo*)info;
- (void)setDataSource:(XAP_StringListDataSource*)source;
@end

class XAP_CocoaDialog_PluginManager : public XAP_Dialog_PluginManager
{
public:
	XAP_CocoaDialog_PluginManager(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~XAP_CocoaDialog_PluginManager(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	void event_DeactivateAll ();
	void event_Deactivate ();
	void event_Load ();
	void event_Select1 ();
	void event_Close();

protected:
#if 0
	GtkWidget * m_windowMain;

	void _constructWindowContents (GtkWidget * container);
	virtual GtkWidget * _constructWindow ();
#endif
private:
	void _refreshAll ();
	void _refreshTab1 ();
	void _refreshTab2 ();
#if 0
	GtkWidget * m_clist;
	GtkWidget * m_name;
	GtkWidget * m_author;
	GtkWidget * m_version;
	GtkWidget * m_desc;
#endif
	XAP_Frame * m_pFrame;
	XAP_CocoaDlg_PluginManagerController* m_dlg;
	XAP_StringListDataSource*	m_dataSource;
};

#endif /* PLUGIN_MANAGER_H */
