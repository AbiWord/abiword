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
#include <string.h>
#include <stdlib.h>
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_BeOSDlg_FontChooser.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include "gr_BeOSGraphics.h"
#include "ut_Rehydrate.h"

/*****************************************************************/
class FontWin:public BWindow {
	public:
		FontWin(BMessage *data);
		void SetDlg(XAP_BeOSDialog_FontChooser *font);
		void MessageReceived(BMessage *msg);
		virtual bool QuitRequested(void);
		void UpdateFontList();
		void UpdateTypeList();
		void UpdatePreview();
		void FillInSizeList();
	private:
		int 			spin;
		XAP_BeOSDialog_FontChooser *m_FontChooser;
		BFont m_CurrentFont;
		rgb_color m_TextColor;
};
void FontWin::UpdateTypeList()
{
	Lock();
	BStringItem *currItem;
	BListView *typeList=(BListView *)FindView("typelist");

	while ((currItem=dynamic_cast<BStringItem *>(typeList->RemoveItem(0L))))
	{
		delete currItem;
	}
	font_family currFamily;
	font_style currStyle;
	m_CurrentFont.GetFamilyAndStyle(&currFamily,&currStyle);
	UT_uint32 numStyles=count_font_styles(currFamily);
	UT_uint32 i;
	UT_uint32 flags;
	for (i=0;i<numStyles;i++)
	{
		if (get_font_style(currFamily,(long int)i,&currStyle,(uint32 *) &flags)==B_OK)
			typeList->AddItem(new BStringItem(currStyle));
	}
	typeList->Select(0);
	Unlock();

}
void FontWin::FillInSizeList()
{
	BListView *sizeList=(BListView *)FindView("sizelist");
	UT_uint32 i;
	char TempBuffer[3];
	Lock();
	for (i=8;i<12;i++)
	{
		sprintf(TempBuffer,"%d",i);
		TempBuffer[3]=0;
		sizeList->AddItem(new BStringItem(TempBuffer));
	}
	for (i=12;i<28;i+=2)
	{
		sprintf(TempBuffer,"%d",i);
		TempBuffer[3]=0;
		sizeList->AddItem(new BStringItem(TempBuffer));
	}
	for (i=28;i<72;i+=8)
	{
		sprintf(TempBuffer,"%d",i);
		TempBuffer[3]=0;
		sizeList->AddItem(new BStringItem(TempBuffer));
	}
	Unlock();
}
void FontWin::UpdateFontList()
{
	font_family family;
	UT_uint32 i;
	UT_uint32 numFamilies;
	UT_uint32 flags;
	numFamilies=count_font_families();
	Lock();
	BListView *fontList=(BListView *)FindView("fontlist");
	BStringItem *currItem;
	while ((currItem=dynamic_cast<BStringItem *>(fontList->RemoveItem(0L))))
	{
		delete currItem;
	}
	for (i=0;i<numFamilies;i++)
	{
		if(get_font_family(i,&family,(uint32 *)&flags) == B_OK)
		{
			fontList->AddItem(new BStringItem(family));
		}
	}
	fontList->Select(0);
	Unlock();
}
void FontWin::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case 'fsel':
		{
			UT_sint32 index=0;
			BListView *theView;
			msg->FindInt32("index",(int32 *)&index);
			msg->FindPointer("source",(void **)&theView);

			if (index > -1)
			{
				BStringItem *theItem=dynamic_cast<BStringItem *>(theView->ItemAt(index));

				m_CurrentFont.SetFamilyAndStyle(theItem->Text(),NULL);
				UpdateTypeList();
				UpdatePreview();
			}
		}
		break;
		case 'tsel':
		{
			UT_sint32 index=0;
			BListView *theView;
			msg->FindInt32("index",(int32 *)&index);
			msg->FindPointer("source",(void **)&theView);
			if (index > -1)
			{
				BStringItem *theItem=dynamic_cast<BStringItem *>(theView->ItemAt(index));
				m_CurrentFont.SetFamilyAndStyle(NULL,theItem->Text());
				UpdatePreview();
			}
		}
		break;
		case 'undl':
		{
			UT_sint32 value=0;
			msg->FindInt32("be:value",(int32 *)&value);
			if (value)
			{
			}
			else
			{
			}
			UpdatePreview();
		}
		break;
		case 'strk':
		{
			UT_sint32 value=0;
			msg->FindInt32("be:value",(int32 *) &value);
			if (value)
			{
			}
			else
			{
			}
			UpdatePreview();
		}
		break;
		case 'ssel':
		{
		UT_sint32 index=0;
			BListView *theView;
			msg->FindInt32("index",(int32 *)&index);
			msg->FindPointer("source",(void **)&theView);
			if (index > -1)
			{
				BStringItem *theItem=dynamic_cast<BStringItem *>(theView->ItemAt(index));
				m_CurrentFont.SetSize(atoi(theItem->Text()));
				UpdatePreview();
			}
		}
		break;
		case 'csel':
		{
			BColorControl *theColorControl;
			msg->FindPointer("source",(void **)&theColorControl);

			m_TextColor=theColorControl->ValueAsColor();
			UpdatePreview();
		}
		break;

		default:
			BWindow::MessageReceived(msg);
			break;
	}
}
void FontWin::UpdatePreview()
{
	BView *previewView=(BView *)FindView("previewview");
	previewView->Window()->Lock();
	previewView->SetFont(&m_CurrentFont);
	rgb_color white={255,255,255};
	rgb_color black={0,0,0};
	previewView->SetHighColor(white);
	previewView->FillRect(Bounds());
	previewView->SetHighColor(m_TextColor);
	previewView->DrawString("Lorem ipsum dolor sit amet, consectetaur adipisicing...",BPoint(0,previewView->Bounds().Height()/2));
	previewView->Window()->Unlock();
}

FontWin::FontWin(BMessage *data) 
	  :BWindow(data) {
	spin = 1;	
}

void FontWin::SetDlg(XAP_BeOSDialog_FontChooser *font) {
	m_FontChooser = font;
	
//	We need to tie up the caller thread for a while ...
	Show();
	UpdateFontList();
	UpdateTypeList();
	FillInSizeList();
	UpdatePreview();
	while (spin) { snooze(1000); }
	Hide();
}
//Behave like a good citizen
bool FontWin::QuitRequested() {
/*
	UT_ASSERT(m_DlgFont);
	m_DlgFont->setAnswer(AP_Dialog_Font::a_CANCEL);
*/
	BListView *FontList=(BListView *)FindView("fontlist");
	
	BStringItem *TheItem;
	
	while ((TheItem=dynamic_cast<BStringItem *>(FontList->RemoveItem(0L))))
	{
		delete(TheItem);
	}
	FontList=(BListView *)FindView("typelist");
	while ((TheItem=dynamic_cast<BStringItem *>(FontList->RemoveItem(0L))))
	{
		delete(TheItem);
	}
	FontList=(BListView *)FindView("sizelist");
	while ((TheItem=dynamic_cast<BStringItem *>(FontList->RemoveItem(0L))))
	{
		delete(TheItem);
	}
	spin = 0;
	return(true);
}

/*****************************************************************/

XAP_Dialog * XAP_BeOSDialog_FontChooser::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_BeOSDialog_FontChooser * p = new XAP_BeOSDialog_FontChooser(pFactory,id);
	return p;
}

XAP_BeOSDialog_FontChooser::XAP_BeOSDialog_FontChooser(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: XAP_Dialog_FontChooser(pDlgFactory,id)
{
	bAbusingTheFontSize = UT_FALSE;
}

XAP_BeOSDialog_FontChooser::~XAP_BeOSDialog_FontChooser(void)
{
}

void XAP_BeOSDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	BMessage msg;
	if (RehydrateWindow("FontWindow", &msg)) {
                FontWin *newwin = new FontWin(&msg);
		newwin->SetDlg(this);
		//Take the information here ...
		newwin->Close();
        }                                                
}
