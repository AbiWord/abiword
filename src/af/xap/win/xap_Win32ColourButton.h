/* AbiWord
 * Copyright (C) 2003 Jordi Mas i Hern�ndez
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

#ifndef xap_Win32ColourButton_H
#define xap_Win32ColourButton_H

#include <windows.h>

#define COLOUR_SMPLENGTH  20
#define COLOUR_SMPHIGH   10

class ABI_EXPORT XAP_Win32ColourButton
{
public:
	
	XAP_Win32ColourButton()
	{		
		m_hBrush = NULL;
	}
	
	~XAP_Win32ColourButton()
	{
		if (m_hBrush) DeleteObject (m_hBrush);
	}	
		
	void setColour(COLORREF  color)
	{	
		if (m_hBrush) DeleteObject (m_hBrush);	
		m_hBrush = CreateSolidBrush(color);	
	}	
	
	void draw(DRAWITEMSTRUCT* dis)
	{				
		RECT colourArea;		

		DrawFrameControl(dis->hDC, &dis->rcItem, DFC_BUTTON, DFCS_BUTTONPUSH);
		
		/* Draw colour example*/
		colourArea.top = ((dis->rcItem.bottom-dis->rcItem.top)-COLOUR_SMPHIGH)/2;
		colourArea.left = ((dis->rcItem.right-dis->rcItem.left)-COLOUR_SMPLENGTH)/2;
		colourArea.bottom = colourArea.top+COLOUR_SMPHIGH;
		colourArea.right = colourArea.left+COLOUR_SMPLENGTH;
		
		if (m_hBrush)			
			FillRect(dis->hDC, &colourArea, m_hBrush);							
		else
			FillRect(dis->hDC, &colourArea, GetSysColorBrush(COLOR_BTNFACE));											

		if ((dis->itemState &  ODS_SELECTED) == ODS_SELECTED)
			DrawEdge(dis->hDC, &dis->rcItem, EDGE_RAISED, BF_RECT |BF_FLAT);		
	}

private:

	HBRUSH m_hBrush;
};



#endif /* xap_Win32ColourButton_H */

