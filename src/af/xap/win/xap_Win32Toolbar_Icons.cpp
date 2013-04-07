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

#include <windows.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_color.h"
#include "ut_Xpm2Bmp.h"
#include "xap_Win32Toolbar_Icons.h"
#include "xap_Win32App.h"
#include "ap_Win32Resources.rc2"
#include "ap_Features.h"
#include "xap_Features.h" 

#define TRANSPARENT_R	0xF0
#define TRANSPARENT_G	0
#define TRANSPARENT_B	0

struct _map_name_id
{
	const char*		name;
	DWORD		 	id;
};

// Maps icon name into win32 resource id
static struct _map_name_id s_nametoIdTable[] =
{
	// Keep this list alphabetically sorted
	{"1COLUMN", AP_RID_TI_1COLUMN},
	{"2COLUMN", AP_RID_TI_2COLUMN},
	{"3COLUMN", AP_RID_TI_3COLUMN},
	{"ADD_COLUMN", AP_RID_TI_ADD_COLUMN},
	{"ADD_ROW", AP_RID_TI_ADD_ROW},
	{"ALIGN_CENTER", AP_RID_TI_ALIGN_CENTER},
	{"ALIGN_JUSTIFY", AP_RID_TI_ALIGN_JUSTIFY},
	{"ALIGN_LEFT", AP_RID_TI_ALIGN_LEFT},
	{"ALIGN_RIGHT", AP_RID_TI_ALIGN_RIGHT},
	{"BORDER_STYLE_DASHED", AP_RID_TI_BORDER_STYLE_DASHED},
	{"BORDER_STYLE_DOTTED", AP_RID_TI_BORDER_STYLE_DOTTED},
	{"BORDER_STYLE_NONE", AP_RID_TI_BORDER_STYLE_NONE},
	{"BORDER_STYLE_SOLID", AP_RID_TI_BORDER_STYLE_SOLID},
	{"COLOR_BACK", AP_RID_TI_COLOR_BACK},
	{"COLOR_FORE", AP_RID_TI_COLOR_FORE},
	{"DELETE_COLUMN", AP_RID_TI_DELETE_COLUMN},
	{"DELETE_ROW", AP_RID_TI_DELETE_ROW},
	{"DOUBLE_SPACE", AP_RID_TI_DOUBLE_SPACE},
	{"EDIT_COPY", AP_RID_TI_EDIT_COPY},
	{"EDIT_CUT", AP_RID_TI_EDIT_CUT},
	{"EDIT_FOOTER", AP_RID_TI_EDIT_FOOTER},
	{"EDIT_HEADER", AP_RID_TI_EDIT_HEADER},
	{"EDIT_PASTE", AP_RID_TI_EDIT_PASTE},
	{"EDIT_REDO", AP_RID_TI_EDIT_REDO},
	{"EDIT_REMOVEFOOTER", AP_RID_TI_EDIT_REMOVEFOOTER},
	{"EDIT_REMOVEHEADER", AP_RID_TI_EDIT_REMOVEHEADER},
	{"EDIT_UNDO", AP_RID_TI_EDIT_UNDO},
	{"FILE_NEW", AP_RID_TI_FILE_NEW},
	{"FILE_OPEN", AP_RID_TI_FILE_OPEN},
	{"FILE_PRINT", AP_RID_TI_FILE_PRINT},
	{"FILE_PRINT_PREVIEW", AP_RID_TI_FILE_PRINT_PREVIEW},
	{"FILE_SAVE", AP_RID_TI_FILE_SAVE},
	{"FILE_SAVEAS", AP_RID_TI_FILE_SAVEAS},
	{"FMT_BOLD", AP_RID_TI_FMT_BOLD},
	{"FMT_BOOKMARK", AP_RID_TI_FMT_BOOKMARK},
	{"FMT_BOTTOMLINE", AP_RID_TI_FMT_BOTTOMLINE},
	{"FMT_DIR_OVERRIDE_LTR", AP_RID_TI_FMT_DIR_OVERRIDE_LTR},
	{"FMT_DIR_OVERRIDE_RTL", AP_RID_TI_FMT_DIR_OVERRIDE_RTL},
	{"FMT_DOM_DIRECTION", AP_RID_TI_FMT_DOM_DIRECTION},
	{"FMT_FONT", AP_RID_TI_FMT_FONT},
	{"FMT_HYPERLINK", AP_RID_TI_FMT_HYPERLINK},
	{"FMT_ITALIC", AP_RID_TI_FMT_ITALIC},
	{"FMT_OVERLINE", AP_RID_TI_FMT_OVERLINE},
	{"FMT_SIZE", AP_RID_TI_FMT_SIZE},
	{"FMT_STRIKE", AP_RID_TI_FMT_STRIKE},
	{"FMT_STYLE", AP_RID_TI_FMT_STYLE},
	{"FMT_SUBSCRIPT", AP_RID_TI_FMT_SUBSCRIPT},
	{"FMT_SUPERSCRIPT", AP_RID_TI_FMT_SUPERSCRIPT},
	{"FMT_TOPLINE", AP_RID_TI_FMT_TOPLINE},
	{"FMT_UNDERLINE", AP_RID_TI_FMT_UNDERLINE},
	{"FMTPAINTER", AP_RID_TI_FMTPAINTER},
	{"FT_LINEBOTTOM", AP_RID_TI_FT_LINEBOTTOM},
	{"FT_LINELEFT", AP_RID_TI_FT_LINELEFT},
	{"FT_LINERIGHT", AP_RID_TI_FT_LINERIGHT},
	{"FT_LINETOP", AP_RID_TI_FT_LINETOP},
	{"HELP", AP_RID_TI_HELP},
	{"IMG", AP_RID_TI_IMG},
	{"INDENT", AP_RID_TI_INDENT},
	{"INSERT_SYMBOL", AP_RID_TI_INSERT_SYMBOL},
	{"INSERT_TABLE", AP_RID_TI_INSERT_TABLE},
	{"LISTS_BULLETS", AP_RID_TI_LISTS_BULLETS},
	{"LISTS_NUMBERS", AP_RID_TI_LISTS_NUMBERS},	
	{"Menu_AbiWord_About", AP_RID_TI_Menu_AbiWord_About},
	{"Menu_AbiWord_Add_Column", AP_RID_TI_Menu_AbiWord_Add_Column},
	{"Menu_AbiWord_Add_Row", AP_RID_TI_Menu_AbiWord_Add_Row},
	{"Menu_AbiWord_Align_Center", AP_RID_TI_Menu_AbiWord_Align_Center},
	{"Menu_AbiWord_Align_Justify", AP_RID_TI_Menu_AbiWord_Align_Justify},
	{"Menu_AbiWord_Align_Left", AP_RID_TI_Menu_AbiWord_Align_Left},
	{"Menu_AbiWord_Align_Right", AP_RID_TI_Menu_AbiWord_Align_Right},
	{"Menu_AbiWord_Bold", AP_RID_TI_Menu_AbiWord_Bold},
	{"Menu_AbiWord_Book", AP_RID_TI_Menu_AbiWord_Book},
	{"Menu_AbiWord_Bookmark", AP_RID_TI_Menu_AbiWord_Bookmark},
	{"Menu_AbiWord_Bottomline", AP_RID_TI_Menu_AbiWord_Bottomline},
	{"Menu_AbiWord_Clear", AP_RID_TI_Menu_AbiWord_Clear},
	{"Menu_AbiWord_Close", AP_RID_TI_Menu_AbiWord_Close},
	{"Menu_AbiWord_Copy", AP_RID_TI_Menu_AbiWord_Copy},
	{"Menu_AbiWord_Credits", AP_RID_TI_Menu_AbiWord_Credits},
	{"Menu_AbiWord_Cut", AP_RID_TI_Menu_AbiWord_Cut},
	{"Menu_AbiWord_Delete_Column", AP_RID_TI_Menu_AbiWord_Delete_Column},
	{"Menu_AbiWord_Delete_Row", AP_RID_TI_Menu_AbiWord_Delete_Row},
	{"Menu_AbiWord_Delete_Table", AP_RID_TI_Menu_AbiWord_Delete_Table},
	{"Menu_AbiWord_Execute", AP_RID_TI_Menu_AbiWord_Execute},
	{"Menu_AbiWord_Exit", AP_RID_TI_Menu_AbiWord_Exit},
	{"Menu_AbiWord_Export", AP_RID_TI_Menu_AbiWord_Export},
	{"Menu_AbiWord_Font", AP_RID_TI_Menu_AbiWord_Font},
	{"Menu_AbiWord_Goto", AP_RID_TI_Menu_AbiWord_Goto},
	{"Menu_AbiWord_Help", AP_RID_TI_Menu_AbiWord_Help},
	{"Menu_AbiWord_Hyperlink", AP_RID_TI_Menu_AbiWord_Hyperlink},
	{"Menu_AbiWord_Img", AP_RID_TI_Menu_AbiWord_Img},
	{"Menu_AbiWord_Import", AP_RID_TI_Menu_AbiWord_Import},
	{"Menu_AbiWord_Insert_Symbol", AP_RID_TI_Menu_AbiWord_Insert_Symbol},
	{"Menu_AbiWord_Insert_Table", AP_RID_TI_Menu_AbiWord_Insert_Table},
	{"Menu_AbiWord_Italic", AP_RID_TI_Menu_AbiWord_Italic},
	{"Menu_AbiWord_Merge_Cells", AP_RID_TI_Menu_AbiWord_Merge_Cells},
	{"Menu_AbiWord_New", AP_RID_TI_Menu_AbiWord_New},
	{"Menu_AbiWord_Open", AP_RID_TI_Menu_AbiWord_Open},
	{"Menu_AbiWord_Overline", AP_RID_TI_Menu_AbiWord_Overline},
	{"Menu_AbiWord_Paste", AP_RID_TI_Menu_AbiWord_Paste},
	{"Menu_AbiWord_Preferences", AP_RID_TI_Menu_AbiWord_Preferences},
	{"Menu_AbiWord_Print", AP_RID_TI_Menu_AbiWord_Print},
	{"Menu_AbiWord_Print_Preview", AP_RID_TI_Menu_AbiWord_Print_Preview},
	{"Menu_AbiWord_Print_Setup", AP_RID_TI_Menu_AbiWord_Print_Setup},
	{"Menu_AbiWord_Properties", AP_RID_TI_Menu_AbiWord_Properties},
	{"Menu_AbiWord_Redo", AP_RID_TI_Menu_AbiWord_Redo},
	{"Menu_AbiWord_Replace", AP_RID_TI_Menu_AbiWord_Replace},
	{"Menu_AbiWord_Revert", AP_RID_TI_Menu_AbiWord_Revert},
	{"Menu_AbiWord_Save", AP_RID_TI_Menu_AbiWord_Save},
	{"Menu_AbiWord_SaveAs", AP_RID_TI_Menu_AbiWord_SaveAs},
	{"Menu_AbiWord_Search", AP_RID_TI_Menu_AbiWord_Search},
	{"Menu_AbiWord_Spellcheck", AP_RID_TI_Menu_AbiWord_Spellcheck},
	{"Menu_AbiWord_Split_Cells", AP_RID_TI_Menu_AbiWord_Split_Cells},
	{"Menu_AbiWord_Strike", AP_RID_TI_Menu_AbiWord_Strike},
	{"Menu_AbiWord_Subscript", AP_RID_TI_Menu_AbiWord_Subscript},
	{"Menu_AbiWord_Superscript", AP_RID_TI_Menu_AbiWord_Superscript},
	{"Menu_AbiWord_Topline", AP_RID_TI_Menu_AbiWord_Topline},
	{"Menu_AbiWord_Underline", AP_RID_TI_Menu_AbiWord_Underline},
	{"Menu_AbiWord_Undo", AP_RID_TI_Menu_AbiWord_Undo},
	{"MERGE_CELLS", AP_RID_TI_MERGE_CELLS},
	{"MERGEABOVE", AP_RID_TI_MERGEABOVE},
	{"MERGEBELOW", AP_RID_TI_MERGEBELOW},
	{"MERGELEFT", AP_RID_TI_MERGELEFT},
	{"MERGERIGHT", AP_RID_TI_MERGERIGHT},
	{"MIDDLE_SPACE", AP_RID_TI_MIDDLE_SPACE},
	{"OPTIONSDLG", AP_RID_TI_OPTIONSDLG},
	{"PARA_0BEFORE", AP_RID_TI_PARA_0BEFORE},
	{"PARA_12BEFORE", AP_RID_TI_PARA_12BEFORE},
	{"SCRIPT_PLAY", AP_RID_TI_SCRIPT_PLAY},
	{"SINGLE_SPACE", AP_RID_TI_SINGLE_SPACE},
	{"SPELLCHECK", AP_RID_TI_SPELLCHECK},
	{"SPLIT_CELLS", AP_RID_TI_SPLIT_CELLS},
	{"SPLITABOVE", AP_RID_TI_SPLITABOVE},
	{"SPLITBELOW", AP_RID_TI_SPLITBELOW},
	{"SPLITHORIMID", AP_RID_TI_SPLITHORIMID},
	{"SPLITLEFT", AP_RID_TI_SPLITLEFT},
	{"SPLITRIGHT", AP_RID_TI_SPLITRIGHT},
	{"SPLITVERTMID", AP_RID_TI_SPLITVERTMID},
	{"TB_ADD_COLUMN", AP_RID_TI_TB_ADD_COLUMN},
	{"TB_ADD_ROW", AP_RID_TI_TB_ADD_ROW},
	{"TB_DELETE_COLUMN", AP_RID_TI_TB_DELETE_COLUMN},
	{"TB_DELETE_ROW", AP_RID_TI_TB_DELETE_ROW},
	{"TB_MERGE_CELLS", AP_RID_TI_TB_MERGE_CELLS},
	{"TB_SPLIT_CELLS", AP_RID_TI_TB_SPLIT_CELLS},
	{"TRANSPARENTLANG", AP_RID_TI_TRANSPARENTLANG},
	{"UNINDENT", AP_RID_TI_UNINDENT},
	{"VIEW_SHOWPARA", AP_RID_TI_VIEW_SHOWPARA}
};


#ifdef DEBUG
XAP_Win32Toolbar_Icons iconswin32debug;
#endif

XAP_Win32Toolbar_Icons::XAP_Win32Toolbar_Icons(void)
{
	#if defined(DEBUG)
	// Check that the lists are in alphabetically order
	UT_uint32 range = G_N_ELEMENTS(s_nametoIdTable), i;
	UT_sint32 cmp;	

	for (i = 1; i < range; i++)
	{
		cmp = g_ascii_strcasecmp(s_nametoIdTable[i].name, s_nametoIdTable[i-1].name);
		UT_ASSERT(cmp > 0);
	}	
#endif
}

XAP_Win32Toolbar_Icons::~XAP_Win32Toolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}

#if defined(EXPORT_XPM_TO_BMP)
#include <stdio.h>

bool XAP_Win32Toolbar_Icons::saveBitmap (const char *szFilename)
{
	HBITMAP hBitmap;
	UT_RGBColor color (TRANSPARENT_R, TRANSPARENT_G, TRANSPARENT_B);
	char szName [1024];

	if (!XAP_Win32Toolbar_Icons::getBitmapForIconFromXPM (GetDesktopWindow(),
		255, 255, &color, szFilename, &hBitmap))
		return false;

	strcpy (szName, szFilename);
	strlwr (szName);
	strcat (szName, ".bmp");

	HDC        hdc=NULL;
	FILE*      fp=NULL;
	LPVOID     pBuf=NULL;
	BITMAPINFO bmpInfo;
	BITMAPFILEHEADER  bmpFileHeader;
	bool rslt = true;

	hdc=GetDC(NULL);
	ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	GetDIBits(hdc,hBitmap,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);

	if(bmpInfo.bmiHeader.biSizeImage<=0)
		bmpInfo.bmiHeader.biSizeImage=bmpInfo.bmiHeader.biWidth*abs(bmpInfo.bmiHeader.biHeight)*(bmpInfo.bmiHeader.biBitCount+7)/8;

	if((pBuf = malloc(bmpInfo.bmiHeader.biSizeImage))==NULL) {
		return false;
	}

	bmpInfo.bmiHeader.biCompression=BI_RGB;
	int scan_lines = GetDIBits(hdc,hBitmap,0,bmpInfo.bmiHeader.biHeight,pBuf, &bmpInfo, DIB_RGB_COLORS);
	if ((fp = fopen(szName,"wb"))==NULL) {
		return false;
	}

	bmpFileHeader.bfReserved1 = 0;
	bmpFileHeader.bfReserved2 = 0;
	bmpFileHeader.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+bmpInfo.bmiHeader.biSizeImage;
	bmpFileHeader.bfType='MB';
	bmpFileHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

	fwrite(&bmpFileHeader,sizeof(BITMAPFILEHEADER),1,fp);
	fwrite(&bmpInfo.bmiHeader,sizeof(BITMAPINFOHEADER),1,fp);
	fwrite(pBuf,bmpInfo.bmiHeader.biSizeImage,1,fp);

	if (hdc)
		ReleaseDC(NULL,hdc);

	if (pBuf)
		free(pBuf);

	if (fp)
		fclose(fp);

	DeleteObject (hBitmap);

	return true;

}

#endif


bool XAP_Win32Toolbar_Icons::getBitmapForIcon(HWND /*hwnd*/,
												UT_uint32 maxWidth,
												UT_uint32 maxHeight,
												const UT_RGBColor * pColor,
												const char * szIconName,
												HBITMAP * pBitmap)
{
	*pBitmap = NULL;

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
	HINSTANCE hInst = pWin32App->getInstance();		
	UT_uint32 range = G_N_ELEMENTS(s_nametoIdTable);
	UT_sint32 middle, right = range - 1, left = 0, cmp;
	HBITMAP dibBitmap = NULL;

	// load in our image as a DIB

	while (left <= right)
	{
		middle = (left + right) >> 1;
		cmp = g_ascii_strcasecmp(szIconName, s_nametoIdTable[middle].name);

		if (cmp == 0) {
			dibBitmap = (HBITMAP) LoadImage (hInst, MAKEINTRESOURCE (s_nametoIdTable[middle].id),
				IMAGE_BITMAP, maxWidth, maxHeight, LR_CREATEDIBSECTION);
			break;
		}

		if (cmp > 0)
			left = middle + 1;
		else
			right = middle - 1;
	}
	
	// Search the toolbariconmap for ID to iconname
	if (dibBitmap==NULL) 
	{
		//	Format: ICONNAME_LANGCODE where LANGCODE code can be _XX (_yi) or _XXXA (_caES)
		char szBaseID[300];
		strcpy(szBaseID, szIconName);
		char *pLast = strrchr(szBaseID, '_');

		if (pLast)
			*pLast = '\0';
			
		right = range - 1;
		left = 0;
		
		while (left <= right)
		{
			middle = (left + right) >> 1;
			cmp = g_ascii_strcasecmp(szBaseID, s_nametoIdTable[middle].name);

			if (cmp == 0) {
				dibBitmap = (HBITMAP) LoadImage (hInst, MAKEINTRESOURCE (s_nametoIdTable[middle].id), 
					IMAGE_BITMAP, maxWidth, maxHeight, LR_CREATEDIBSECTION | LR_LOADTRANSPARENT);
				break;
			}

			if (cmp > 0)
				left = middle + 1;
			else
				right = middle - 1;		
		}
	}	
	
	if (dibBitmap == NULL) 
		return false;
		
	/* Applies transparency to the DIB */
	
	HDC        	hdc=NULL;
	LPVOID     	pBuf=NULL;
	BITMAPINFO 	bmpInfo;
	BYTE		R, G, B;
	
	R = (BYTE) pColor->m_red;
  	G = (BYTE) pColor->m_grn;
  	B = (BYTE) pColor->m_blu;

	hdc=GetDC(NULL);
	ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);

	GetDIBits(hdc, dibBitmap, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);
	pBuf = malloc(bmpInfo.bmiHeader.biSizeImage);		
	if (pBuf == NULL) 
		return false;	
	
	bmpInfo.bmiHeader.biCompression=BI_RGB;
	GetDIBits(hdc, dibBitmap, 0, bmpInfo.bmiHeader.biHeight, pBuf, &bmpInfo, DIB_RGB_COLORS);
	DeleteObject(dibBitmap);

	BYTE* buff = (BYTE *)pBuf;

	for (UT_uint32 pixel = 0; pixel < bmpInfo.bmiHeader.biSizeImage; pixel += 4)
	{
		if (!(buff[pixel] == TRANSPARENT_B && buff[pixel + 1] == TRANSPARENT_G && buff[pixel + 2] == TRANSPARENT_R))
			continue;

		buff[pixel] = B;
		buff[pixel + 1] = G;
		buff[pixel + 2] = R;
		buff[pixel + 3] = 0;
	}

	// convert the DIB into a DDB for display purposes
	*pBitmap = CreateCompatibleBitmap(hdc, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight);
	SetDIBits(hdc, *pBitmap, 0, bmpInfo.bmiHeader.biHeight, pBuf, &bmpInfo, DIB_RGB_COLORS);
	free (pBuf);	
	return true;
}

// Returns PARGB32 DIB bitmap for use in Vista+ menus

bool XAP_Win32Toolbar_Icons::getAlphaBitmapForIcon(HWND /*hwnd*/,
												UT_uint32 maxWidth,
												UT_uint32 maxHeight,
												const char * szIconName,
												HBITMAP * pBitmap)
{
	*pBitmap = NULL;

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
	HINSTANCE hInst = pWin32App->getInstance();		
	UT_uint32 range = G_N_ELEMENTS(s_nametoIdTable);
	UT_sint32 middle, right = range - 1, left = 0, cmp;
	HBITMAP dibBitmap = NULL;

	// load in our image as a DIB

	while (left <= right)
	{
		middle = (left + right) >> 1;
		cmp = g_ascii_strcasecmp(szIconName, s_nametoIdTable[middle].name);

		if (cmp == 0) {
			dibBitmap = (HBITMAP) LoadImage (hInst, MAKEINTRESOURCE (s_nametoIdTable[middle].id),
				IMAGE_BITMAP, maxWidth, maxHeight, LR_CREATEDIBSECTION);
			break;
		}

		if (cmp > 0)
			left = middle + 1;
		else
			right = middle - 1;
	}
	
	// Search the toolbariconmap for ID to iconname
	if (dibBitmap==NULL) 
	{
		//	Format: ICONNAME_LANGCODE where LANGCODE code can be _XX (_yi) or _XXXA (_caES)
		char szBaseID[300];
		strcpy(szBaseID, szIconName);
		char *pLast = strrchr(szBaseID, '_');

		if (pLast)
			*pLast = '\0';
			
		right = range - 1;
		left = 0;
		
		while (left <= right)
		{
			middle = (left + right) >> 1;
			cmp = g_ascii_strcasecmp(szBaseID, s_nametoIdTable[middle].name);

			if (cmp == 0) {
				dibBitmap = (HBITMAP) LoadImage (hInst, MAKEINTRESOURCE (s_nametoIdTable[middle].id), 
					IMAGE_BITMAP, maxWidth, maxHeight, LR_CREATEDIBSECTION | LR_LOADTRANSPARENT);
				break;
			}

			if (cmp > 0)
				left = middle + 1;
			else
				right = middle - 1;		
		}
	}	
	
	if (dibBitmap == NULL) 
		return false;
		
	/* Applies transparency to the DIB */
	
	HDC        	hdc=NULL;
	LPVOID     	pBuf=NULL;
	BITMAPINFO 	bmpInfo;

	hdc=GetDC(NULL);
	ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);

	GetDIBits(hdc, dibBitmap, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);
	pBuf = malloc(bmpInfo.bmiHeader.biSizeImage);		
	if (pBuf == NULL) 
		return false;	
	
	bmpInfo.bmiHeader.biCompression=BI_RGB;
	GetDIBits(hdc, dibBitmap, 0, bmpInfo.bmiHeader.biHeight, pBuf, &bmpInfo, DIB_RGB_COLORS);
	DeleteObject(dibBitmap);

	BYTE* buff = (BYTE *)pBuf;

	UT_ASSERT((bmpInfo.bmiHeader.biSizeImage == bmpInfo.bmiHeader.biWidth *
		bmpInfo.bmiHeader.biHeight * 4 ))

	for (UT_uint32 pixel = 0; pixel < bmpInfo.bmiHeader.biSizeImage; pixel += 4)
	{
		if (!(buff[pixel] == TRANSPARENT_B && buff[pixel + 1] == TRANSPARENT_G && buff[pixel + 2] == TRANSPARENT_R))
		{
			buff[pixel + 3] = 255;
			continue;
		}

		*(LPDWORD)&buff[pixel] = 0;
	}

	LPBYTE bmbits;
	HBITMAP r = CreateDIBSection(hdc,&bmpInfo,DIB_RGB_COLORS,(void**)&bmbits,NULL,0);
	if (r && bmbits) {
		memcpy(bmbits,pBuf,bmpInfo.bmiHeader.biSizeImage);
		GdiFlush();
		*pBitmap = r;
	}
	free (pBuf);	
	return false;
}

#if defined(EXPORT_XPM_TO_BMP)

bool XAP_Win32Toolbar_Icons::getBitmapForIconFromXPM(HWND hwnd,
												UT_uint32 maxWidth,
												UT_uint32 maxHeight,
												UT_RGBColor * pColor,
												const char * szIconName,
												HBITMAP * pBitmap)
{
	UT_ASSERT(hwnd);
	UT_ASSERT(szIconName && *szIconName);
	UT_ASSERT(pBitmap);

	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array

	bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
		return false;

	HDC hdc = GetDC(hwnd);
	bool bCreated = UT_Xpm2Bmp(maxWidth,maxHeight,pIconData,sizeofIconData,hdc,pColor,pBitmap);
	ReleaseDC(hwnd,hdc);

	return bCreated;
}
#endif

