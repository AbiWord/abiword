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
		void SelectFontPrefs();
		
	private:
		XAP_BeOSDialog_FontChooser *m_FontChooser;
		BFont m_CurrentFont;
		rgb_color m_TextColor;
		
		status_t WaitForDelete(sem_id blocker);
		sem_id modalSem;
};

status_t FontWin::WaitForDelete(sem_id blocker)
{
	status_t	result;
	thread_id	this_tid = find_thread(NULL);
	BLooper		*pLoop;
	BWindow		*pWin = 0;

	pLoop = BLooper::LooperForThread(this_tid);
	if (pLoop)
		pWin = dynamic_cast<BWindow*>(pLoop);

	// block until semaphore is deleted (modal is finished)
	if (pWin) {
		do {
			pWin->Unlock(); // Who will know?=)
			snooze(100);
			pWin->Lock();
			
			// update the window periodically			
			pWin->UpdateIfNeeded();
			result = acquire_sem_etc(blocker, 1, B_TIMEOUT, 1000);
		} while (result != B_BAD_SEM_ID);
	} else {
		do {
			// just wait for exit
			result = acquire_sem(blocker);
		} while (result != B_BAD_SEM_ID);
	}
	return result;
}

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
		{
			typeList->AddItem(new BStringItem(currStyle));
		}
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
	bool underline, overline, strikeout;
	uint16 fontFace;
	
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
				m_FontChooser->setFontFamily(theItem->Text());
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
				
				m_FontChooser->setFontWeight("normal"); //m_FontChooser->m_bChangedFontWeight = true;
				m_FontChooser->setFontStyle("normal"); // m_FontChooser->m_bChangedFontStyle = true;
						
				if(*theItem->Text() == 'B')//m_FontChooser->setFontStyle(theItem->Text());
					m_FontChooser->setFontWeight("bold");
				if(strstr(theItem->Text() , "Italic") != NULL)
					m_FontChooser->setFontStyle("italic");
					
				UpdatePreview();
			}
		}
		break;
		case 'undl':
		{
			UT_sint32 value=0;
			msg->FindInt32("be:value",(int32 *)&value);
			fontFace = m_CurrentFont.Face();
			fontFace = B_UNDERSCORE_FACE;
			m_CurrentFont.SetFace(fontFace);
			
			m_FontChooser->getChangedUnderline(&underline);
			m_FontChooser->getChangedOverline(&overline);
			m_FontChooser->getChangedStrikeOut(&strikeout);

			m_FontChooser->setFontDecoration(!underline,overline,strikeout,false,false);

			UpdatePreview();
		}
		break;
		case 'strk':
		{
			UT_sint32 value=0;
			msg->FindInt32("be:value",(int32 *) &value);
			
			fontFace = m_CurrentFont.Face();
			fontFace = B_STRIKEOUT_FACE;
			m_CurrentFont.SetFace(fontFace);
			
			m_FontChooser->getChangedUnderline(&underline);
			m_FontChooser->getChangedOverline(&overline);
			m_FontChooser->getChangedStrikeOut(&strikeout);
			
			m_FontChooser->setFontDecoration(underline,overline,!strikeout,false,false);
			
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
	
				char bufSize[10];
				if (atoi(theItem->Text()) > 0)
					sprintf(bufSize,"%dpt",atoi(theItem->Text()));
				else
					bufSize[0] = 0;
					
				m_FontChooser->setFontSize(bufSize);
				UpdatePreview();
			}
		}
		break;
		case 'csel':
		{
			char bufColor[10];
			BColorControl *theColorControl;
			msg->FindPointer("source",(void **)&theColorControl);

			m_TextColor=theColorControl->ValueAsColor();
			
			sprintf(bufColor,"%02x%02x%02x",m_TextColor.red,m_TextColor.green,m_TextColor.blue);
			m_FontChooser->setColor(bufColor);

			UpdatePreview();
		}
		break;
	
		case 'okck':
			m_FontChooser->setAnswer(m_FontChooser->a_OK);
			PostMessage(B_QUIT_REQUESTED);
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
	  :BWindow(data) 
{

}

void FontWin::SelectFontPrefs()
{
	int i;
	const XML_Char *fontFamily, *fontSize, *fontWeight , *fontStyled , *fontColor;
	bool fontUnderline, fontOverline, fontStrikeOut;
	
	m_FontChooser->getChangedFontFamily(&fontFamily);
	m_FontChooser->getChangedFontSize(&fontSize);
	m_FontChooser->getChangedFontWeight(&fontWeight);
	m_FontChooser->getChangedFontStyle(&fontStyled);
	m_FontChooser->getChangedColor(&fontColor);
	m_FontChooser->getChangedUnderline(&fontUnderline);
	m_FontChooser->getChangedOverline(&fontOverline);
	m_FontChooser->getChangedStrikeOut(&fontStrikeOut);

	Lock();

	// Select the family..

	BListView *fontList=(BListView *)FindView("fontlist");
	BStringItem *currItem;
	int32 numItems = fontList->CountItems();
	
	for(i = 0; i < numItems; i ++)
	{
		currItem = (BStringItem *)fontList->ItemAt(i);
		if( currItem)
		{
			const char* itemText = currItem->Text();
			if(strcmp(itemText , fontFamily) == 0)
				break;
		}
	}	
	
	if(i != numItems)
		fontList->Select(i);
		
	BListView* sizeList=(BListView *)FindView("sizelist");
	numItems = sizeList->CountItems();
	
	for(i = 0; i < numItems; i ++)
	{
		currItem = (BStringItem *)sizeList->ItemAt(i);
		if(strncmp(currItem->Text() , fontSize , strlen(fontSize) - 2) == 0)
			break;
	}
	
	if(i != numItems)
		sizeList->Select(i);
		
	// Set the underline / strikeout status, then our color.
	BCheckBox* strikeOut = (BCheckBox *)FindView("undercheck");
	BCheckBox* underLine = (BCheckBox *)FindView("strikecheck");
	
	strikeOut->SetValue((fontStrikeOut == true));
	underLine->SetValue((fontUnderline == true));
	
	rgb_color textColor;
	sscanf(fontColor,"%02x%02x%02x",&textColor.red,&textColor.green,&textColor.blue);
	
	m_TextColor = textColor;

	BColorControl* colorControl= (BColorControl *)FindView("colourview");
	if(!colorControl)
	{
		BTabView* pTabView = (BTabView *)FindView("fonttab");
		if(pTabView)
			colorControl = (BColorControl *)pTabView->TabAt(1)->View()->FindView("colourview");
	}
	
	colorControl->SetValue(textColor);
	
	Unlock();
}

void FontWin::SetDlg(XAP_BeOSDialog_FontChooser *font) 
{
	m_FontChooser = font;
	
//	We need to tie up the caller thread for a while ...
	Show();
	UpdateFontList();
	UpdateTypeList();
	FillInSizeList();
	
	// Select the items in the dialog box they have set now.
	SelectFontPrefs();
	
	UpdatePreview();
	
	// Default answer, modified if the user hits OK.
	m_FontChooser->setAnswer(m_FontChooser->a_CANCEL);
	
	modalSem = create_sem(0,"ParagraphSem");
	WaitForDelete(modalSem);
	
	Hide();
}

bool FontWin::QuitRequested() 
{

	UT_ASSERT(m_FontChooser);

	// We need to activate the first tab so we can find the view children.
	BTabView* pTabView = (BTabView *)FindView("fonttab");
	pTabView->Select(0);
	
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

	delete_sem(modalSem);
	return(false);
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
	bAbusingTheFontSize = false;
}

XAP_BeOSDialog_FontChooser::~XAP_BeOSDialog_FontChooser(void)
{
}

void XAP_BeOSDialog_FontChooser::setAnswer(XAP_Dialog_FontChooser::tAnswer answer)
{
	m_answer = answer;
}

void XAP_BeOSDialog_FontChooser::runModal(XAP_Frame * pFrame)
{
	BMessage msg;
	if (RehydrateWindow("FontWindow", &msg)) {
                FontWin *newwin = new FontWin(&msg);
		newwin->SetDlg(this);			
	
	m_bChangedFontFamily	= true;
	m_bChangedFontSize		= true;
	m_bChangedFontWeight	= true;
	m_bChangedFontStyle		= true;
	m_bChangedColor			= true;
	m_bChangedUnderline		= true;
	m_bChangedOverline		= true;
	m_bChangedStrikeOut		= true;
	
		//Take the information here ...
		newwin->Lock();
		newwin->Quit();
        }                                                
}
