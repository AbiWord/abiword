#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_BeOSDialog_Columns.h"

#include "ut_Rehydrate.h"
#include "xap_BeOSToolbar_Icons.h"
#include "gr_BeOSGraphics.h"

/*****************************************************************/

#define BUTTONPIC_OFFSET_X 3
#define BUTTONPIC_OFFSET_Y 3

class ColumnButton : public BButton
{
public:
	ColumnButton(BRect frameRect , const char* Label, const char* name, BMessage * message);
	virtual ~ColumnButton();
	
	virtual void Draw(BRect updateRect);
	void SetBitmap(BBitmap* pBitmap);
	
	virtual	void		MouseDown(BPoint where);
	virtual	void		MouseUp(BPoint pt);
	virtual	void		MouseMoved(BPoint pt, uint32 code, const BMessage *msg);

private:
	BBitmap* buttonBitmap;
	BPoint drawPoint;
	
	// Mouse stuff.
	bool mouseDown;
	BPoint downPoint;
};

ColumnButton::ColumnButton(BRect frameRect , const char* label, const char *name , BMessage * message) : BButton(frameRect , name , label , message)
{
	buttonBitmap = NULL;
	drawPoint.x = BUTTONPIC_OFFSET_X;
	drawPoint.y = BUTTONPIC_OFFSET_Y;
	mouseDown = false;
}

ColumnButton::~ColumnButton()
{
	if(buttonBitmap)
		delete buttonBitmap;
}

void ColumnButton::SetBitmap(BBitmap* pBitmap)
{
	if(buttonBitmap)
		delete buttonBitmap;
		
		buttonBitmap = pBitmap;
}

void ColumnButton::Draw(BRect updateRect)
{
	BButton::Draw(updateRect);
	
	if(buttonBitmap)
		DrawBitmap(buttonBitmap, drawPoint);
		
}


void ColumnButton::MouseDown(BPoint where)
{
#if 0
	if(Value() == 1)
	{
		SetValue(0);//BButton::MouseUp(where);
		BControl::Invoke(Message());
		return;
	}
#endif
		
	SetValue(1);
	// Me thinks that this starts a thread to poll the mouse or something.
	// BButton::MouseDown(where);
	
	mouseDown = true;
	downPoint = where;
}

void ColumnButton::MouseUp(BPoint pt)
{
	// Invocation, post the message to the handler.
	if(Value() == 1)
	{
		BControl::Invoke(Message());
		//Invoke(Message());	
	}
	
	mouseDown = false;
}

void ColumnButton::MouseMoved(BPoint pt, uint32 code, const BMessage *msg)
{
	BPoint point;
	uint32 buttons;
	
	if(mouseDown)
	{
		// Ensure the mouse is still down..
		GetMouse(&point , &buttons);
		if(!(buttons & B_PRIMARY_MOUSE_BUTTON))
		{
			SetValue(0);
			return;
		}
		
		if(B_EXITED_VIEW == code)
			SetValue(0);
	
		if(B_ENTERED_VIEW == code)
			SetValue(1);
	}
	
}

/*****************************************************************/

class ColumnWin:public BWindow {
	public:
		ColumnWin(BMessage *data);
		virtual ~ColumnWin();
		
		void SetDlg(AP_BeOSDialog_Columns *brk);
		virtual void DispatchMessage(BMessage *msg, BHandler *handler);
		virtual bool QuitRequested(void);
		
	private:
		AP_BeOSDialog_Columns 	*m_DlgColumn;
		GR_BeOSGraphics			*m_BeOSGraphics;
		
		sem_id modalSem;
		status_t WaitForDelete(sem_id deleteSem);
		
		void _setPictureButtons();
		
		// Button pointers..
		ColumnButton* buttonCol1;
		ColumnButton* buttonCol2;
		ColumnButton* buttonCol3;
};

ColumnWin::ColumnWin(BMessage *data) 
	  :BWindow(data) 
{

}

ColumnWin::~ColumnWin()
{
	DELETEP(m_BeOSGraphics);
}

void ColumnWin::SetDlg(AP_BeOSDialog_Columns *brk) 
{
	BRadioButton *on = NULL;

	m_DlgColumn = brk;
	m_DlgColumn->m_answer = AP_BeOSDialog_Columns::a_CANCEL;

	Show();

	_setPictureButtons();
	
	BView* preview = FindView("preview");
	
	//Create our preview window graphics
	m_BeOSGraphics  = new GR_BeOSGraphics(preview, m_DlgColumn->m_pApp);

	if (preview->Window()->Lock())
	{
		m_DlgColumn->_createPreviewFromGC(m_BeOSGraphics,preview->Frame().Width(),preview->Frame().Height());
		
		preview->Window()->Unlock();
	}
	
	modalSem = create_sem(0,"ColumnWindowSem");
	WaitForDelete(modalSem);
	
	Hide();
}

void ColumnWin::_setPictureButtons()
{
	BView* pParent;
	ColumnButton* pButton;
	BBitmap* testBitmap;
	
	AP_BeOSToolbar_Icons Icons;
	
	Lock();
	
	BButton* col1 = (BButton *) FindView("column1");
	BButton* col2 = (BButton *) FindView("column2");
	BButton* col3 = (BButton *) FindView("column3");
		
	if(col1)
	{
		pButton = new ColumnButton(col1->Frame() , col1->Label() , col1->Name() , new BMessage(*col1->Message()));
		testBitmap = Icons.GetIconBitmap("tb_1column_xpm");
		pButton->SetBitmap(testBitmap);
		
		pParent = col1->Parent();
		if(pParent)
		{
			pParent->RemoveChild(col1);
			pParent->AddChild(pButton);
			delete col1;
		}
		buttonCol1 = pButton;
	}
	
	if(col2)
	{
		pButton = new ColumnButton(col2->Frame() , col2->Label() , col2->Name() , new BMessage(*col2->Message()));
		testBitmap = Icons.GetIconBitmap("tb_2column_xpm");
		pButton->SetBitmap(testBitmap);
		
		pParent = col2->Parent();
		if(pParent)
		{
			pParent->RemoveChild(col2);
			pParent->AddChild(pButton);
			delete col2;
		}
		buttonCol2 = pButton;
	}
	
	if(col3)
	{
		pButton = new ColumnButton(col3->Frame() , col3->Label() , col3->Name() , new BMessage(*col3->Message()));
		testBitmap = Icons.GetIconBitmap("tb_3column_xpm");
		pButton->SetBitmap(testBitmap);
		
		pParent = col3->Parent();
		if(pParent)
		{
			pParent->RemoveChild(col3);
			pParent->AddChild(pButton);
			delete col3;
		}
		buttonCol3 = pButton;
	}

	Unlock();
}

status_t ColumnWin::WaitForDelete(sem_id blocker)
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
			// update the window periodically			
			pWin->UpdateIfNeeded();
			result = acquire_sem_etc(blocker, 1, B_TIMEOUT, 10000);
		} while (result != B_BAD_SEM_ID);
	} else {
		do {
			// just wait for exit
			result = acquire_sem(blocker);
		} while (result != B_BAD_SEM_ID);
	}
	return result;
}


void ColumnWin::DispatchMessage(BMessage *msg, BHandler *handler) 
{
	switch(msg->what)
	{
		// Set one column..
		case 'colo':
			m_DlgColumn->setColumns(1);
			buttonCol3->SetValue(0);
			buttonCol2->SetValue(0);
			break;
		
		// Set two columns..
		case 'colt':
			buttonCol3->SetValue(0);
			buttonCol1->SetValue(0);
			m_DlgColumn->setColumns(2);
			break;
	
		// Set three columns..
		case 'colh':
			buttonCol1->SetValue(0);
			buttonCol2->SetValue(0);
			m_DlgColumn->setColumns(3);
			break;
			
		case 'line':
			BCheckBox* lineBox;
			lineBox = (BCheckBox *)FindView("chkLineBetween");
			m_DlgColumn->setLineBetween(lineBox->Value());
			break;
			
		case 'okay':
			m_DlgColumn->m_answer = AP_BeOSDialog_Columns::a_OK;
			PostMessage(B_QUIT_REQUESTED);
			
			break;
			
		default:
			BWindow::DispatchMessage(msg, handler);
	}
} 

//Behave like a good citizen
bool ColumnWin::QuitRequested() 
{
	delete_sem(modalSem);
	
	return(true);
}

/*****************************************************************/

XAP_Dialog * AP_BeOSDialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_BeOSDialog_Columns * p = new AP_BeOSDialog_Columns(pFactory,id);
	return p;
}

AP_BeOSDialog_Columns::AP_BeOSDialog_Columns(XAP_DialogFactory * pDlgFactory,
					 XAP_Dialog_Id id) 
		   : AP_Dialog_Columns(pDlgFactory,id) 
{		    
	newwin = NULL;
	enableLineBetween = true;
} 

AP_BeOSDialog_Columns::~AP_BeOSDialog_Columns(void) {
}

/*****************************************************************/

void AP_BeOSDialog_Columns::runModal(XAP_Frame * pFrame)
{
	BMessage msg;

	if (RehydrateWindow("ColumnsWindow", &msg)) {
                newwin = new ColumnWin(&msg);
               	enableLineBetweenControl(enableLineBetween);
		newwin->SetDlg(this);
		//Take the information here ...
        }                                                
}

void AP_BeOSDialog_Columns::enableLineBetweenControl(UT_Bool bState)
{
	enableLineBetween = (bState == UT_TRUE);
	
	if(newwin)
	{
		BCheckBox* pBox = (BCheckBox *)newwin->FindView("chkLineBetween");
		pBox->SetEnabled(enableLineBetween);
	}
}


