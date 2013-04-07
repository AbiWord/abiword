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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Image.h"
#include "xap_Win32Dlg_Image.h"

#include "xap_Win32Resources.rc2"
#include "ap_Win32App.h"

#define BUFSIZE 1024
/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Image::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Image * p = new XAP_Win32Dialog_Image(pFactory,id);
	return p;
}

#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

XAP_Win32Dialog_Image::XAP_Win32Dialog_Image(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Image(pDlgFactory,id)
{
}

XAP_Win32Dialog_Image::~XAP_Win32Dialog_Image(void)
{
}

void XAP_Win32Dialog_Image::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_IMAGE);

	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_IMAGE));
}

#define _DS(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_Image::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet* pSS = m_pApp->getStringSet();

	// Update the caption
	setDialogTitle(pSS->getValue(XAP_STRING_ID_DLG_Image_Title));
 
	/* Localise controls*/
	_DSX(IMAGE_BTN_OK,			DLG_OK);
	_DSX(IMAGE_BTN_CANCEL,			DLG_Cancel);
	_DS(IMAGE_LBL_HEIGHT,			DLG_Image_Height);
	_DS(IMAGE_LBL_WIDTH,			DLG_Image_Width);
	_DS(IMAGE_CHK_ASPECT,			DLG_Image_Aspect);
	_DS(IMAGE_LBL_TITLE,			DLG_Image_LblTitle);
	_DS(IMAGE_LBL_DESCRIPTION,			DLG_Image_LblDescription);
	_DS(IMAGE_LBL_SIZE,			DLG_Image_ImageSize);
	_DS(IMAGE_LBL_NAME,			DLG_Image_ImageDesc);
	_DS(IMAGE_LBL_WRAPPING,			DLG_Image_TextWrapping);
	_DS(IMAGE_LBL_PLACEMENT,			DLG_Image_Placement);
	_DS(IMAGE_RADIO_INLINE,			DLG_Image_InLine);
	_DS(IMAGE_RADIO_FLOAT,			DLG_Image_WrappedNone);
	_DS(IMAGE_RADIO_RIGHT,			DLG_Image_WrappedRight);	
	_DS(IMAGE_RADIO_LEFT,			DLG_Image_WrappedLeft);
	_DS(IMAGE_RADIO_BOTHSIDES,			DLG_Image_WrappedBoth);
	_DS(IMAGE_RADIO_PARAGRAPH,			DLG_Image_PlaceParagraph);
	_DS(IMAGE_RADIO_COLUMN,			DLG_Image_PlaceColumn);
	_DS(IMAGE_RADIO_PAGE,			DLG_Image_PlacePage);
	_DS(IMAGE_LBL_TYPE,			DLG_Image_WrapType);
	_DS(IMAGE_RADIO_SQUARE,			DLG_Image_SquareWrap);
	_DS(IMAGE_RADIO_TIGHT,			DLG_Image_TightWrap);

	// Initialize controls
	setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
	setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
	setControlText(XAP_RID_DIALOG_IMAGE_EBX_TITLE, getTitle().utf8_str());
	setControlText(XAP_RID_DIALOG_IMAGE_EBX_DESCRIPTION, getDescription().utf8_str());
	checkButton( XAP_RID_DIALOG_IMAGE_CHK_ASPECT, getPreserveAspect());

	// Initialize text wrapping radio buttons
	if(getWrapping() == WRAP_INLINE)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_INLINE, XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES,
		XAP_RID_DIALOG_IMAGE_RADIO_INLINE);
	}
	else if(getWrapping() == WRAP_NONE)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_INLINE, XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES,
		XAP_RID_DIALOG_IMAGE_RADIO_FLOAT);
	}
	else if(getWrapping() == WRAP_TEXTRIGHT)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_INLINE, XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES,
		XAP_RID_DIALOG_IMAGE_RADIO_RIGHT);
	}
	else if(getWrapping() == WRAP_TEXTLEFT)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_INLINE, XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES,
		XAP_RID_DIALOG_IMAGE_RADIO_LEFT);
	}
	else if(getWrapping() == WRAP_TEXTBOTH)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_INLINE, XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES,
		XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES);
	}

	// Initialize image placement radio buttons
	if(getPositionTo() == POSITION_TO_PARAGRAPH)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_PARAGRAPH, XAP_RID_DIALOG_IMAGE_RADIO_PAGE,
		XAP_RID_DIALOG_IMAGE_RADIO_PARAGRAPH);
	}
	else if(getPositionTo() == POSITION_TO_COLUMN)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_PARAGRAPH, XAP_RID_DIALOG_IMAGE_RADIO_PAGE,
		XAP_RID_DIALOG_IMAGE_RADIO_COLUMN);
	}
	else if(getPositionTo() == POSITION_TO_PAGE)
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_PARAGRAPH, XAP_RID_DIALOG_IMAGE_RADIO_PAGE,
		XAP_RID_DIALOG_IMAGE_RADIO_PAGE);
	}

	//Initialize type radio buttons
	if(isTightWrap())
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_SQUARE, XAP_RID_DIALOG_IMAGE_RADIO_TIGHT,
		XAP_RID_DIALOG_IMAGE_RADIO_TIGHT);
	}
	else
	{
        CheckRadioButton(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_SQUARE, XAP_RID_DIALOG_IMAGE_RADIO_TIGHT,
		XAP_RID_DIALOG_IMAGE_RADIO_SQUARE);
	}

	wrappingChanged();
	return TRUE;
}

BOOL XAP_Win32Dialog_Image::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_COLUMN_BTN_CANCEL
		setAnswer( a_Cancel );
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also AP_RID_DIALOG_COLUMN_BTN_OK
		// change wrapping
		if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_INLINE))
		{
			setWrapping(WRAP_INLINE);
		}
		else if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_FLOAT))
		{
			setWrapping(WRAP_NONE);
		}
		else if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_RIGHT))
		{
			setWrapping(WRAP_TEXTRIGHT);
		}
		else if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_LEFT))
		{
			setWrapping(WRAP_TEXTLEFT);
		}
		else if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES))
		{
			setWrapping(WRAP_TEXTBOTH);
		}

		if(!IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_INLINE))
		{
			// change placement if the inline option isn't selected
			if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_PARAGRAPH))
			{
				setPositionTo(POSITION_TO_PARAGRAPH);
			}
			else if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_COLUMN))
			{
				setPositionTo(POSITION_TO_COLUMN);
			}
			else if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_PAGE))
			{
				setPositionTo(POSITION_TO_PAGE);
			}

			//change type if the inline option isn't selected
			if(IsDlgButtonChecked(hWnd, XAP_RID_DIALOG_IMAGE_RADIO_SQUARE))
			{
				setTightWrap(false);
			}
			else
			{
				setTightWrap(true);
			}
		}

		char buf[BUFSIZE];
		getControlText(XAP_RID_DIALOG_IMAGE_EBX_TITLE,buf,BUFSIZE);
		setTitle(buf);
		getControlText(XAP_RID_DIALOG_IMAGE_EBX_DESCRIPTION,buf,BUFSIZE);
		setDescription(buf);

		setAnswer( a_OK );
		EndDialog(hWnd,0);
		return 1;

	case XAP_RID_DIALOG_IMAGE_EBX_HEIGHT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			//char bufHeight[BUFSIZE];
			//GetDlgItemText( hWnd, wId, bufHeight, BUFSIZE );
			UT_Win32LocaleString str, units;
			getDlgItemText (wId, str);
			units.fromASCII (UT_dimensionName(getPreferedUnits()));

			//let the user manually change dimensions too
			if(!wcsstr(str.c_str(), units.c_str()))
				str.appendLocale (units.c_str());

			setHeight( str.utf8_str().utf8_str() );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		}
		return 1;

	case XAP_RID_DIALOG_IMAGE_EBX_WIDTH:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			UT_Win32LocaleString str, units;
			getDlgItemText (wId, str);

			//let the user manually change dimensions too
			if(!wcsstr(str.c_str(), units.c_str()))
				str.appendLocale (units.c_str());

			setWidth( str.utf8_str().utf8_str() );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		}
		return 1;

	case XAP_RID_DIALOG_IMAGE_CHK_ASPECT:
		setPreserveAspect( isChecked(wId)!=0 );
		return 1;

	case XAP_RID_DIALOG_IMAGE_RADIO_INLINE: case XAP_RID_DIALOG_IMAGE_RADIO_FLOAT:
	case XAP_RID_DIALOG_IMAGE_RADIO_RIGHT: case XAP_RID_DIALOG_IMAGE_RADIO_LEFT: case XAP_RID_DIALOG_IMAGE_RADIO_BOTHSIDES:
		wrappingChanged();
		return 1;

	 default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_Image::_onDeltaPos(NM_UPDOWN * pnmud)
{
	switch( pnmud->hdr.idFrom )
	{
	case XAP_RID_DIALOG_IMAGE_SPN_WIDTH:
		if( pnmud->iDelta < 0 )
		{
			incrementWidth( true );
		}
		else
		{
			incrementWidth( false );
		}
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		return 1;

	case XAP_RID_DIALOG_IMAGE_SPN_HEIGHT:
		if( pnmud->iDelta < 0 )
		{
			incrementHeight( true );
		}
		else
		{
			incrementHeight( false );
		}
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		return 1;

	default:
		return 0;
	}
}

void XAP_Win32Dialog_Image::wrappingChanged(void)
{
	if(isChecked(XAP_RID_DIALOG_IMAGE_RADIO_INLINE))
	{
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_PARAGRAPH,FALSE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_COLUMN,FALSE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_PAGE,FALSE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_SQUARE,FALSE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_TIGHT,FALSE);
	}
	else
	{
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_PARAGRAPH, TRUE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_COLUMN,TRUE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_PAGE,TRUE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_SQUARE,TRUE);
		enableControl(XAP_RID_DIALOG_IMAGE_RADIO_TIGHT,TRUE);
	}
}
