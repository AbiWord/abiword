/* AbiWord
 * Copyright (C) 2002-2003 Hubert Figuiere
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


/* XAP_GenericListChooser_Controller */

#import <Cocoa/Cocoa.h>

class XAP_GenericListChooser_Proxy
{
public:
	virtual ~XAP_GenericListChooser_Proxy() {}
	virtual void okAction () = 0;
	virtual void cancelAction () = 0;
	virtual void selectAction () = 0;
};


@interface XAP_GenericListChooser_Controller : NSWindowController
{
    IBOutlet NSButton *m_cancelBtn;
    IBOutlet NSBox *m_enclosingBox;
    IBOutlet NSTableView *m_listTable;
    IBOutlet NSButton *m_okBtn;
	XAP_GenericListChooser_Proxy*	_proxy;
}
+ (XAP_GenericListChooser_Controller *)loadFromNib;
- (IBAction)cancelClicked:(id)sender;
- (IBAction)listClicked:(id)sender;
- (IBAction)okClicked:(id)sender;

/* GUI customization */
- (void)setTitle:(NSString*)title;
- (void)setLabel:(NSString*)label;

/* other */
- (void)setXAPProxy:(XAP_GenericListChooser_Proxy*)proxy;
- (void)setDataSource:(id)obj;
- (void)setSelected:(int)idx;
- (int)selected;
@end


