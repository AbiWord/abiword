/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Goto.h"
#include "ap_MacDialog_Goto.h"

#include "fv_View.h"

#if 0
/*****************************************************************/
class GotoWin:public BWindow {
	public:
		GotoWin(BMessage *data);
		void SetDlg(AP_MacDialog_Goto *brk);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		// Called to save pointers to the dialogs controls.. avoids multiple FindView calls.
		void _savePointers();
		BListView* gotoWhat;
		BTextControl* numberBox;
		BStringView* infoBox;
				
		AP_MacDialog_Goto 	*m_DlgGoto;
};

GotoWin::GotoWin(BMessage *data) 
	  :BWindow(data) 
{

} //BreakWin::BreakWin

void GotoWin::SetDlg(AP_MacDialog_Goto *brk)
{
//	const XAP_StringSet * pSS = brk->m_pApp->getStringSet();
	int iTarget;
	char **ppszTargets;
	
	m_DlgGoto = brk;
	 
	_savePointers();
	
	BButton* gotoButton;
	gotoButton = (BButton *)FindView("Goto");
	gotoButton->SetEnabled(false);
			
	numberBox->MakeFocus(true);
	 
 	m_DlgGoto->m_iRow = 0;

	ppszTargets = m_DlgGoto->getJumpTargets();
	for ( iTarget = 0; ppszTargets[ iTarget ] != NULL; iTarget++ )
		gotoWhat->AddItem( new BStringItem( ppszTargets[ iTarget ]));

	gotoWhat->Select(0);

	Show();
}

void GotoWin::DispatchMessage(BMessage *msg, BHandler *handler)
{
	BListView* pSource = NULL;
	int32 dwCounter , dwStart , dwTextLength;
	bool bValueOK = TRUE;
	char* pBuf = NULL;

	switch(msg->what) 
	{
		case 'next':
			m_DlgGoto->GoTo("+1");
			break;
		
		case 'prev':
			m_DlgGoto->GoTo("-1");
			break;
			
		case 'goto':
			m_DlgGoto->GoTo( m_DlgGoto->m_pszOldValue );
			break;
			
		case 'lbch':
			m_DlgGoto->m_iRow = gotoWhat->CurrentSelection();
			break;
			
		case 'nued':
			
			BButton* gotoButton;
			gotoButton = (BButton *)FindView("Goto");
			
			dwTextLength = strlen(numberBox->Text());
			
			if( dwTextLength )
			{
				pBuf = new char [ dwTextLength + 1 ];
				if( !pBuf )
					return ;
				
				strcpy(pBuf, numberBox->Text());//GetWindowText( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_EDIT_NUMBER), pBuf, dwTextLength + 1 );

				// If the first character is + or -, skip over it in the
				// check loop below
				if( *pBuf == '-' || *pBuf == '+' )
					dwStart = 1;
				else
					dwStart = 0;

				// Make sure everything we have is numeric
				for( dwCounter = dwStart; dwCounter < dwTextLength; dwCounter++ )
				{
					if( !UT_UCS_isdigit( pBuf[ dwCounter ] ) )
					{
						if( m_DlgGoto->m_pszOldValue == NULL )
						{
							m_DlgGoto->m_pszOldValue = new char[ 1 ];
							*m_DlgGoto->m_pszOldValue = '\0';
						}
						
						numberBox->SetText(m_DlgGoto->m_pszOldValue);//SetWindowText( GetDlgItem(hWnd,AP_RID_DIALOG_GOTO_EDIT_NUMBER), m_pszOldValue );
						
						bValueOK = FALSE;

						break;
					}
				}

				if( bValueOK )
				{
					if( m_DlgGoto->m_pszOldValue != NULL )
						DELETEP( m_DlgGoto->m_pszOldValue );

					m_DlgGoto->m_pszOldValue = pBuf;

					// Only SetEnabled the goto button if what we have actually contains a number
					gotoButton->SetEnabled( !(((pBuf[ 0 ] == '-') || (pBuf[ 0 ] == '+')) && (pBuf[ 1 ] == '\0')) );
				}
				else
				{
					FREEP( pBuf );

					gotoButton->SetEnabled( FALSE );
				}
			}
			else
			{
				if( m_DlgGoto->m_pszOldValue != NULL )
					DELETEP( m_DlgGoto->m_pszOldValue );

				m_DlgGoto->m_pszOldValue = NULL;

				gotoButton->SetEnabled( FALSE );
			}

			break;
					
		default:
			BWindow::DispatchMessage(msg, handler);
	}
} 


bool GotoWin::QuitRequested()
{		
	m_DlgGoto->destroy();
			
	return(true);
}

void GotoWin::_savePointers()
{
	gotoWhat = (BListView *)FindView("gotoWhat");
	numberBox = (BTextControl *)FindView("Number");
	infoBox = (BStringView *)FindView("infoBox");
}

/*****************************************************************/
#endif

XAP_Dialog * AP_MacDialog_Goto::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	AP_MacDialog_Goto * p = new AP_MacDialog_Goto(pFactory,id);
	return p;
}

AP_MacDialog_Goto::AP_MacDialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : AP_Dialog_Goto(pDlgFactory,id)
{
	m_pszOldValue = NULL;
	m_iRow = 0;
}

AP_MacDialog_Goto::~AP_MacDialog_Goto(void)
{
}


void AP_MacDialog_Goto::activate(void)
{

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}


void AP_MacDialog_Goto::destroy(void)
{

//	UT_ASSERT(UT_NOT_IMPLEMENTED);
}


void AP_MacDialog_Goto::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	UT_ASSERT(pFrame);

#if 0		
	BMessage msg;
	GotoWin* newwin;
	
	if (RehydrateWindow("GotoWindow", &msg))
		{
        newwin = new GotoWin(&msg);
		newwin->SetDlg(this);
        } 
#endif
}

void AP_MacDialog_Goto::GoTo (const char *number)
{
	UT_UCSChar *ucsnumber = (UT_UCSChar *) malloc (sizeof (UT_UCSChar) * (strlen(number) + 1));
	UT_UCS_strcpy_char (ucsnumber, number);
	int target = m_iRow;
	this->getView()->gotoTarget ((AP_JumpTarget) target, ucsnumber);
	free (ucsnumber);
}





