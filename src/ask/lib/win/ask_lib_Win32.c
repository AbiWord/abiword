/*
 * AbiSource Setup Kit
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

#include <windows.h>
#include <shlobj.h>
#include <objidl.h>
#include <regstr.h>

#include "png.h"

#include "ask.h"

HINSTANCE	g_hInstance;

HWND		g_hwndMain;
WNDCLASS	g_wndclassMain;

HWND		g_hwndPane;
WNDCLASS	g_wndclassPane;

HWND		g_hwndGraphic;
WNDCLASS	g_wndclassGraphic;
BITMAPINFO* g_pDIB;

HWND		g_hwndButtonNext;
HWND		g_hwndButtonCancel;
int 		g_iWhichButton;

ASK_FileSet* g_pSet_BrowseDir;
HWND		g_hwndStatic_BrowseDir;
HWND		g_hwndStatic_DiskSpace;
char		g_szBrowseDir[1024];

HFONT		g_hfontMain;

static int	s_bInstallationSuccessfull = 0;

#define		ID_PANE			1000
#define		ID_BTN_NEXT		1001
#define		ID_BTN_CANCEL	1002
#define		ID_BTN_BROWSE	1003
#define		ID_GRAPHIC		1004

void QuitEventLoop(void);

char		g_szRemoveFileName[ASK_MAX_PATH+1];
char		g_szRemoveKey[1024];
char		g_szRemoveName[256];
FILE*		g_fpRemove;

void ASK_createRemoveFile(char* pszName)
{
	char szName[ASK_MAX_PATH+1];
	char szWinDir[ASK_MAX_PATH+1];
	int i = 0;

	strcpy(g_szRemoveName, pszName);
	sprintf(g_szRemoveKey, "%s\\%s", REGSTR_PATH_UNINSTALL, pszName);
	
	GetWindowsDirectory(szWinDir, ASK_MAX_PATH);
	
	for (i=0; ; i++)
	{
		sprintf(szName, "%s\\askrm%03d.txt", szWinDir, i);
		if (!ASK_fileExists(szName))
		{
			break;
		}
	}

	strcpy(g_szRemoveFileName, szName);
	g_fpRemove = fopen(szName, "w");
	if (g_fpRemove)
	{
		fprintf(g_fpRemove, "%s\n", g_szRemoveKey);
	}
}

void ASK_addToRemoveFile(const char* pszFileName)
{
	if (g_fpRemove)
	{
		fprintf(g_fpRemove, "%s\n", pszFileName);
	}
}

void ASK_closeRemoveFile(void)
{
	if (g_fpRemove)
	{
		fclose(g_fpRemove);
	}
}

void ASK_registerForRemove(void)
{
	char szCmdLine[1024];
	HKEY hKey = NULL;
	DWORD iDisposition;
	char szWinDir[ASK_MAX_PATH+1];

	GetWindowsDirectory(szWinDir, ASK_MAX_PATH);

	RegCreateKeyEx(HKEY_LOCAL_MACHINE,
				   g_szRemoveKey,
				   0,
				   NULL,
				   REG_OPTION_NON_VOLATILE,
				   KEY_ALL_ACCESS,
				   NULL,
				   &hKey,
				   &iDisposition
				   );

	RegSetValueEx(hKey,
				  REGSTR_VAL_UNINSTALLER_DISPLAYNAME,
				  0,
				  REG_SZ,
				  g_szRemoveName,
				  strlen(g_szRemoveName)+1
				  );

	sprintf(szCmdLine, "%s\\askrm.exe %s", szWinDir, g_szRemoveFileName);
	
	RegSetValueEx(hKey,
				  REGSTR_VAL_UNINSTALLER_COMMANDLINE,
				  0,
				  REG_SZ,
				  szCmdLine,
				  strlen(szCmdLine)+1
				  );

	RegCloseKey(hKey);
}

struct _bb
{
	unsigned char* 	pBytes;
	long			iCurPos;
};

void _updateDirAndSpace(char* pszPath)
{
	char szBuf[256];
	char* p;
	long iBytesLow, iBytesHigh;
	int iResult;

	if (g_pSet_BrowseDir->pszDirName &&g_pSet_BrowseDir->pszDirName[0])
	{
		sprintf(g_szBrowseDir, "%s/%s", pszPath, g_pSet_BrowseDir->pszDirName);
	}
	else
	{
		sprintf(g_szBrowseDir, "%s", pszPath);
	}
	
	// first set the name of the dir
	ASK_fixSlashes(g_szBrowseDir);
	SetWindowText(g_hwndStatic_BrowseDir, g_szBrowseDir);

	// now find out how much free disk space we've got
	strcpy(szBuf, g_szBrowseDir);
	ASK_fixSlashes(szBuf);
	p = strchr(szBuf, '\\');
	p++;
	*p = 0;
	
	iResult = ASK_getDiskFreeSpace(szBuf, &iBytesHigh, &iBytesLow);

	// and set it into the static text display
	ASK_convert64BitsToString(iBytesHigh, iBytesLow, szBuf);
	SetWindowText(g_hwndStatic_DiskSpace, szBuf);

	if(iResult == 0)
	{
		EnableWindow(g_hwndButtonNext, FALSE);
		MessageBox(g_hwndMain,
			"The selected disk is not available.", "Error",
			MB_ICONEXCLAMATION | MB_OK);
	}
	else
	{
		EnableWindow(g_hwndButtonNext, TRUE);
	}
}

static void _png_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct _bb* p = (struct _bb*) png_get_io_ptr(png_ptr);

	memcpy(data, p->pBytes + p->iCurPos, length);
	p->iCurPos += length;
}

int initGraphic(void)
{
	if (g_pGraphicFile)
	{
		unsigned char* pBytes = ASK_decompressFile(g_pGraphicFile);

		png_structp png_ptr;
		png_infop info_ptr;
		png_uint_32 width, height;
		int bit_depth, color_type, interlace_type;
		struct _bb myBB;
		long iBytesInRow;
		unsigned char* pBits;
		unsigned long iRow;

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void*) NULL,
										 NULL, NULL);

		if (png_ptr == NULL)
		{
			return 0;
		}

		/* Allocate/initialize the memory for image information.  REQUIRED. */
		info_ptr = png_create_info_struct(png_ptr);
		if (info_ptr == NULL)
		{
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			return 0;
		}

		/* Set error handling if you are using the setjmp/longjmp method (this is
		 * the normal method of doing things with libpng).  REQUIRED unless you
		 * set up your own error handlers in the png_create_read_struct() earlier.
		 */
		if (setjmp(png_ptr->jmpbuf))
		{
			/* Free all of the memory associated with the png_ptr and info_ptr */
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	  
			/* If we get here, we had a problem reading the file */
			return 0;
		}

		myBB.pBytes = pBytes;
		myBB.iCurPos = 0;
	
		png_set_read_fn(png_ptr, (void *)&myBB, _png_read);

		/* The call to png_read_info() gives us all of the information from the
		 * PNG file before the first IDAT (image data chunk).  REQUIRED
		 */
		png_read_info(png_ptr, info_ptr);

		png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
					 &interlace_type, NULL, NULL);

		/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
		 * byte into separate bytes (useful for paletted and grayscale images).
		 */
		png_set_packing(png_ptr);

		/* Expand paletted colors into true RGB triplets */
		if (color_type == PNG_COLOR_TYPE_PALETTE)
		{
			png_set_expand(png_ptr);
		}

		/* flip the RGB pixels to BGR (or RGBA to BGRA) */
		png_set_bgr(png_ptr);

		iBytesInRow = width * 3;
		if (iBytesInRow % 4)
		{
			iBytesInRow += (4 - (iBytesInRow % 4));
		}
	
		g_pDIB = (BITMAPINFO*) malloc(sizeof(BITMAPINFOHEADER) + height * iBytesInRow);
		if (!g_pDIB)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return 0;
		}

		/*
		  Note that we do NOT create a DIB of iDisplayWidth,iDisplayHeight, since
		  DIBs can be stretched automatically by the Win32 API.  So we simply remember
		  the display size for drawing later.
		*/

		g_pDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		g_pDIB->bmiHeader.biWidth = width;
		g_pDIB->bmiHeader.biHeight = height;
		g_pDIB->bmiHeader.biPlanes = 1;
		g_pDIB->bmiHeader.biBitCount = 24;
		g_pDIB->bmiHeader.biCompression = BI_RGB;
		g_pDIB->bmiHeader.biSizeImage = 0;
		g_pDIB->bmiHeader.biXPelsPerMeter = 0;
		g_pDIB->bmiHeader.biYPelsPerMeter = 0;
		g_pDIB->bmiHeader.biClrUsed = 0;
		g_pDIB->bmiHeader.biClrImportant = 0;
	
		pBits = ((unsigned char*) g_pDIB) + g_pDIB->bmiHeader.biSize;
	
		/* Now it's time to read the image.  One of these methods is REQUIRED */
		for (iRow = 0; iRow < height; iRow++)
		{
			unsigned char* pRow = pBits + (height - iRow - 1) * iBytesInRow;
		
			png_read_rows(png_ptr, &pRow, NULL, 1);
		}

		/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
		png_read_end(png_ptr, info_ptr);

		/* clean up after the read, and free any memory allocated - REQUIRED */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

		return 1;
		
	}

	return 0;
}

int try_cancel(void)
{
	if (IDYES == MessageBox(g_hwndMain, "Are you sure you want to cancel the installation?", "AbiSetup", MB_YESNO))
	{
		g_iWhichButton = ID_BTN_CANCEL;
		QuitEventLoop();
		return 1;
	}
	else
	{
		return 0;
	}
}

// Main_OnBrowse - browses for a program folder. 
// hwnd - handle of the application's main window
// returns 0 on cancel
int BrowseDir(HWND hwnd, char* pszDest) 
{ 
	LPMALLOC      pMalloc;
    BROWSEINFO bi; 
    LPSTR lpBuffer; 
    LPITEMIDLIST pidlPrograms;  // PIDL for Programs folder 
    LPITEMIDLIST pidlBrowse;    // PIDL selected by user 
	int result = 0;
	
	// We are going to create a pidl, and it will need to be
	// freed by the shell mallocator. Get the shell mallocator
	// object using API SHGetMalloc function. Return if failure.
	if(FAILED(SHGetMalloc(&pMalloc)))
	{
		return 0;
	}

    // Allocate a buffer to receive browse information. 
    if ((lpBuffer = (LPSTR) pMalloc->lpVtbl->Alloc(pMalloc, MAX_PATH)) == NULL)
	{
        return 0;
	}
 
    // Get the PIDL for the Programs folder. 
    if (!SUCCEEDED(SHGetSpecialFolderLocation(hwnd, CSIDL_DRIVES, &pidlPrograms)))
	{ 
        pMalloc->lpVtbl->Free(pMalloc, lpBuffer); 
        return 0; 
    } 
 
    // Fill in the BROWSEINFO structure. 
    bi.hwndOwner = hwnd; 
    bi.pidlRoot = pidlPrograms; 
    bi.pszDisplayName = lpBuffer; 
    bi.lpszTitle = "Please choose a folder"; 
    bi.ulFlags = BIF_RETURNONLYFSDIRS; 
    bi.lpfn = NULL; 
    bi.lParam = 0; 

    // Browse for a folder and return its PIDL. 
    pidlBrowse = SHBrowseForFolder(&bi); 
    if (pidlBrowse != NULL)
	{ 
        if (SHGetPathFromIDList(pidlBrowse, lpBuffer))
		{
			strcpy(pszDest, lpBuffer);

			if( pszDest[ strlen( pszDest ) - 1 ] == '\\' )
				pszDest[ strlen( pszDest ) - 1 ] = '\0';

			result = 1;
		}
 
        // Free the PIDL returned by SHBrowseForFolder. 
        pMalloc->lpVtbl->Free(pMalloc, pidlBrowse); 
    } 
 
    // Clean up. 
    pMalloc->lpVtbl->Free(pMalloc, pidlPrograms); 
    pMalloc->lpVtbl->Free(pMalloc, lpBuffer);

	return result;
} 

LRESULT CALLBACK WndProc_Main(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (msg)
	{
	case WM_CHAR:
	{
		if (wParam == 13)
		{
			g_iWhichButton = ID_BTN_NEXT;
			QuitEventLoop();
			return 0;
		}
		else if (wParam == 27)
		{
			try_cancel();
			return 0;
		}
		else
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}	
	case WM_COMMAND:
	{
		int wNotifyCode = HIWORD(wParam);
		int wID = LOWORD(wParam);

		switch (wNotifyCode)
		{
		case BN_CLICKED:
			if (wID == ID_BTN_CANCEL)
			{
				try_cancel();
				return 0;
			}
			else if (wID == ID_BTN_NEXT)
			{
				g_iWhichButton = ID_BTN_NEXT;
				QuitEventLoop();
				return 0;
			}
			
			return 0;
		}
		
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
		
	case WM_CREATE:
	{
		return 0;
	}

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);
		
		return 0;
	}

	case WM_CLOSE:
	{
		if(s_bInstallationSuccessfull)
		{
			QuitEventLoop();
			PostQuitMessage(0);
			return 0;
		}
		else
		{
			return ! try_cancel();
		}
	}

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		
		return 0;
	}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Pane(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (msg)
	{
	case WM_COMMAND:
	{
		int wNotifyCode = HIWORD(wParam);
		int wID = LOWORD(wParam);

		switch (wNotifyCode)
		{
		case BN_CLICKED:
			if (wID == ID_BTN_BROWSE)
			{
				char szPath[1024];
				
				if (BrowseDir(g_hwndMain, szPath))
				{
					_updateDirAndSpace(szPath);
				}
				
				return 0;
			}
			
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
		
	case WM_CTLCOLOREDIT:
		// TODO why the heck doesn't this work?
		
		SetBkColor((HDC) wParam, RGB(255,255,255));
		
		return (LRESULT) GetStockObject(WHITE_BRUSH);
	
	case WM_CREATE:
		return 0;

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);
		
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc_Graphic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (msg)
	{
	case WM_CREATE:
		return 0;

	case WM_PAINT:
	{
		RECT r;
		
		hdc = BeginPaint(hwnd, &ps);

		GetClientRect(g_hwndGraphic, &r);

		if (g_pDIB)
		{
			long iSizeOfColorData = g_pDIB->bmiHeader.biClrUsed * sizeof(RGBQUAD);
			unsigned char* pBits = ((unsigned char*) g_pDIB) + g_pDIB->bmiHeader.biSize + iSizeOfColorData;
			int xDest = (r.right - g_pDIB->bmiHeader.biWidth) / 2;
			int yDest = (r.bottom - g_pDIB->bmiHeader.biHeight) / 2;
	
			int iRes = StretchDIBits(hdc,
									 xDest,
									 yDest,
									 g_pDIB->bmiHeader.biWidth,
									 g_pDIB->bmiHeader.biHeight,
									 0,
									 0,
									 g_pDIB->bmiHeader.biWidth,
									 g_pDIB->bmiHeader.biHeight,
									 pBits,
									 g_pDIB,
									 DIB_RGB_COLORS,
									 SRCCOPY
				);
	
			if (iRes == GDI_ERROR)
			{
				DWORD err = GetLastError();

				// TODO
			}
		}

		EndPaint(hwnd, &ps);
		
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

#define LABEL_WIDTH			120
#define BUTTON_WIDTH		64
#define BUTTON_HEIGHT		24
#define BUTTON_GAP			10

#define MAINWIN_WIDTH		640
#define	MAINWIN_HEIGHT		480
#define BUTTONAREA_HEIGHT	(BUTTON_HEIGHT + 2 * BUTTON_GAP)

#define GRAPHIC_WIDTH		200

int ASK_Win32_Init(HINSTANCE hInstance, long iIconId)
{
	int 		iScreenWidth;
	int 		iScreenHeight;
	int 		iWindowWidth;
	int 		iWindowHeight;
	LOGFONT		lf;
	
	{
		HWND hwndDesktop = GetDesktopWindow();
		RECT r;

		GetClientRect(hwndDesktop, &r);
		iScreenWidth = r.right;
		iScreenHeight = r.bottom;
	}
	
	g_hInstance = hInstance;
	
	g_wndclassMain.style = CS_HREDRAW | CS_VREDRAW;
	g_wndclassMain.lpfnWndProc = WndProc_Main;
	g_wndclassMain.cbClsExtra = 0;
	g_wndclassMain.cbWndExtra = 0;
	g_wndclassMain.hInstance = hInstance;
	g_wndclassMain.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(iIconId));
	g_wndclassMain.hCursor = LoadCursor(NULL, IDC_ARROW);
	g_wndclassMain.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	g_wndclassMain.lpszMenuName = NULL;
	g_wndclassMain.lpszClassName = "AbiSetup_MainWindow";

	if (!RegisterClass(&g_wndclassMain))
	{
		// TODO fail

		return -1;
	}

	g_wndclassPane.style = CS_HREDRAW | CS_VREDRAW;
	g_wndclassPane.lpfnWndProc = WndProc_Pane;
	g_wndclassPane.cbClsExtra = 0;
	g_wndclassPane.cbWndExtra = 0;
	g_wndclassPane.hInstance = hInstance;
	g_wndclassPane.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	g_wndclassPane.hCursor = LoadCursor(NULL, IDC_ARROW);
	g_wndclassPane.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	g_wndclassPane.lpszMenuName = NULL;
	g_wndclassPane.lpszClassName = "AbiSetup_Pane";

	if (!RegisterClass(&g_wndclassPane))
	{
		// TODO fail

		return -1;
	}

	g_wndclassGraphic.style = CS_HREDRAW | CS_VREDRAW;
	g_wndclassGraphic.lpfnWndProc = WndProc_Graphic;
	g_wndclassGraphic.cbClsExtra = 0;
	g_wndclassGraphic.cbWndExtra = 0;
	g_wndclassGraphic.hInstance = hInstance;
	g_wndclassGraphic.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	g_wndclassGraphic.hCursor = LoadCursor(NULL, IDC_ARROW);
	g_wndclassGraphic.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	g_wndclassGraphic.lpszMenuName = NULL;
	g_wndclassGraphic.lpszClassName = "AbiSetup_Graphic";

	if (!RegisterClass(&g_wndclassGraphic))
	{
		// TODO fail

		return -1;
	}

	lf.lfWidth = 0;
	lf.lfEscapement = 0; 
	lf.lfOrientation = 0; 
	lf.lfItalic = 0; 
	lf.lfUnderline = 0; 
	lf.lfStrikeOut = 0; 
	lf.lfCharSet = 0; 
	lf.lfOutPrecision = 0; 
	lf.lfClipPrecision = 0; 
	lf.lfQuality = 0; 
	lf.lfPitchAndFamily = 0; 
	strcpy(lf.lfFaceName, "MS Sans Serif");
	
	lf.lfHeight = 12;
	lf.lfWeight = 0;
	g_hfontMain = CreateFontIndirect(&lf);

	g_hwndMain = CreateWindow("AbiSetup_MainWindow",
						  "AbiSetup",
						  WS_BORDER | WS_VISIBLE | WS_CLIPCHILDREN | WS_SYSMENU,
						  (iScreenWidth - MAINWIN_WIDTH) / 2,
						  (iScreenHeight - MAINWIN_HEIGHT) /2,
						  MAINWIN_WIDTH,
						  MAINWIN_HEIGHT,
						  NULL,
						  NULL,
						  hInstance,
						  NULL);

	{
		RECT r;

		GetClientRect(g_hwndMain, &r);
		iWindowWidth = r.right;
		iWindowHeight = r.bottom;
	}
	
	g_hwndGraphic = CreateWindow("AbiSetup_Graphic",
							  "",
							  WS_CHILD | WS_VISIBLE,
							  0,
							  0,
							  GRAPHIC_WIDTH,
							  iWindowHeight,
							  g_hwndMain,
							  (HMENU) ID_GRAPHIC,
							  hInstance,
							  NULL);

	g_hwndPane = CreateWindow("AbiSetup_Pane",
							  "",
							  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
							  GRAPHIC_WIDTH + 1,
							  0,
							  iWindowWidth - GRAPHIC_WIDTH - 1,
							  iWindowHeight - BUTTONAREA_HEIGHT,
							  g_hwndMain,
							  (HMENU) ID_PANE,
							  hInstance,
							  NULL);

	g_hwndButtonNext = CreateWindow("BUTTON",
									"Next",
									WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
									iWindowWidth - (2 * BUTTON_WIDTH) - (2 * BUTTON_GAP),
									iWindowHeight - BUTTON_HEIGHT - BUTTON_GAP,
									BUTTON_WIDTH,
									BUTTON_HEIGHT,
									g_hwndMain,
									(HMENU) ID_BTN_NEXT,
									hInstance,
									NULL);
	
	g_hwndButtonCancel = CreateWindow("BUTTON",
									"Cancel",
									WS_CHILD | WS_VISIBLE,
									iWindowWidth - (1 * BUTTON_WIDTH) - (1 * BUTTON_GAP),
									iWindowHeight - BUTTON_HEIGHT - BUTTON_GAP,
									BUTTON_WIDTH,
									BUTTON_HEIGHT,
									g_hwndMain,
									(HMENU) ID_BTN_CANCEL,
									hInstance,
									NULL);
	SendMessage(g_hwndButtonNext, WM_SETFONT, (WPARAM) g_hfontMain, (LPARAM) 0);
	SendMessage(g_hwndButtonCancel, WM_SETFONT, (WPARAM) g_hfontMain, (LPARAM) 0);

	initGraphic();
	
	CoInitialize(NULL);

	InitCommonControls();
	
	return 0;
}

int g_iEventLoopDepth;

int DoEventLoop(void)
{
	MSG msg;
	int iMyDepth;

	SetFocus(g_hwndMain);
	
	g_iWhichButton = 0;
	g_iEventLoopDepth++;
	iMyDepth = g_iEventLoopDepth;
	
	while (g_iEventLoopDepth == iMyDepth)
	{
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			break;
		}
	}

	return (g_iWhichButton == ID_BTN_NEXT);
}

void QuitEventLoop(void)
{
	g_iEventLoopDepth--;
}

int ASK_YesNo(char* pszTitle, char* pszMessage)
{
	return (IDYES == MessageBox(g_hwndMain, pszMessage, pszTitle, MB_YESNO));
}

#define WELCOME_STATIC_MARGIN		16

#define WELCOME_TEXT1_PERCENT		10
#define WELCOME_TEXT2_PERCENT		55
#define WELCOME_TEXT3_PERCENT		35

#define WELCOME_FONT1_SIZE			36
#define WELCOME_FONT2_SIZE			20
#define	WELCOME_FONT3_SIZE			14

int ASK_DoScreen_welcome(char* pszHeading, char* pszIntro, char* pszFinePrint)
{
	int 	result;
	
	HWND	hwndStatic1;
	HFONT	hFont1;
	HWND	hwndStatic2;
	HFONT	hFont2;
	HWND	hwndStatic3;
	HFONT	hFont3;

	int 	iHeight1;
	int 	iHeight2;
	int		iHeight3;
	
	LOGFONT	lf;
	RECT 	r;

	SetWindowText(g_hwndMain, pszHeading);
	
	GetClientRect(g_hwndPane, &r);

	iHeight1 = (r.bottom - WELCOME_STATIC_MARGIN*2) * WELCOME_TEXT1_PERCENT / 100;
	iHeight2 = (r.bottom - WELCOME_STATIC_MARGIN*2) * WELCOME_TEXT2_PERCENT / 100;
	iHeight3 = (r.bottom - WELCOME_STATIC_MARGIN*2) * WELCOME_TEXT3_PERCENT / 100;
	
	hwndStatic1 = CreateWindow("STATIC",
							  "",
							  WS_CHILD | WS_VISIBLE | SS_CENTER,
							  WELCOME_STATIC_MARGIN,
							  WELCOME_STATIC_MARGIN,
							  r.right - WELCOME_STATIC_MARGIN*2,
							  iHeight1,
							  g_hwndPane,
							  (HMENU) 2000,
							  g_hInstance,
							  NULL);
	hwndStatic2 = CreateWindow("STATIC",
							  "",
							  WS_CHILD | WS_VISIBLE | SS_LEFT,
							  WELCOME_STATIC_MARGIN,
							  WELCOME_STATIC_MARGIN + iHeight1,
							  r.right - WELCOME_STATIC_MARGIN*2,
							  iHeight2,
							  g_hwndPane,
							  (HMENU) 2001,
							  g_hInstance,
							  NULL);
	hwndStatic3 = CreateWindow("STATIC",
							  "",
							  WS_CHILD | WS_VISIBLE | SS_LEFT,
							  WELCOME_STATIC_MARGIN,
							  WELCOME_STATIC_MARGIN + iHeight1 + iHeight2,
							  r.right - WELCOME_STATIC_MARGIN*2,
							  iHeight3,
							  g_hwndPane,
							  (HMENU) 2002,
							  g_hInstance,
							  NULL);

	lf.lfWidth = 0;
	lf.lfEscapement = 0; 
	lf.lfOrientation = 0; 
	lf.lfItalic = 0; 
	lf.lfUnderline = 0; 
	lf.lfStrikeOut = 0; 
	lf.lfCharSet = 0; 
	lf.lfOutPrecision = 0; 
	lf.lfClipPrecision = 0; 
	lf.lfQuality = 0; 
	lf.lfPitchAndFamily = 0; 
	strcpy(lf.lfFaceName, "Arial");
	
	lf.lfHeight = WELCOME_FONT1_SIZE; 
	lf.lfWeight = FW_BOLD; 
	hFont1 = CreateFontIndirect(&lf);

	strcpy(lf.lfFaceName, "MS Sans Serif");
	
	lf.lfHeight = WELCOME_FONT2_SIZE; 
	lf.lfWeight = 0; 
	hFont2 = CreateFontIndirect(&lf);

	lf.lfHeight = WELCOME_FONT3_SIZE; 
	lf.lfWeight = 0; 
	hFont3 = CreateFontIndirect(&lf);
	
	SendMessage(hwndStatic1, WM_SETFONT, (WPARAM) hFont1, 0);
	SendMessage(hwndStatic2, WM_SETFONT, (WPARAM) hFont2, 0);
	SendMessage(hwndStatic3, WM_SETFONT, (WPARAM) hFont3, 0);

	SetWindowText(hwndStatic1, pszHeading);

	SetWindowText(hwndStatic2, pszIntro);

	SetWindowText(hwndStatic3, pszFinePrint);
	
	result = DoEventLoop();

	SendMessage(hwndStatic1, WM_SETFONT, (WPARAM) 0, 0);
	SendMessage(hwndStatic2, WM_SETFONT, (WPARAM) 0, 0);
	SendMessage(hwndStatic3, WM_SETFONT, (WPARAM) 0, 0);

	DeleteObject(hFont1);
	DeleteObject(hFont2);
	DeleteObject(hFont3);
	
	DestroyWindow(hwndStatic1);
	DestroyWindow(hwndStatic2);
	DestroyWindow(hwndStatic3);
	
	return result;
}

int ASK_DoScreen_chooseDirForFileSet(ASK_FileSet* pSet)
{
	char 	buf[1024];
	char	buf2[64];
	int 	result;
	
	HWND	hwndStatic_Top;
	HWND	hwndStatic_DirLabel;
	HWND	hwndStatic_SpaceLabel;
	HWND	hwndStatic_Dir;
	HWND	hwndStatic_Space;
	HWND	hwndButton;
	int		iWidth;
	int		iHeight;
	RECT	r;

	HCURSOR hcursorHourGlass;
	HCURSOR	hcursorPrev;

	if (pSet->bFixedPath)
	{
		// just do the fixed default path, but check for special names

		if (0 == strcmp("WINDIR", pSet->pszDefaultPath))
		{
			GetWindowsDirectory(pSet->szInstallPath, ASK_MAX_PATH);
		}
		else
		{
			strcpy(pSet->szInstallPath, pSet->pszDefaultPath);
		}

		return 1;
	}

	hcursorHourGlass = LoadCursor(NULL, IDC_WAIT);
	hcursorPrev = SetCursor(hcursorHourGlass);

	sprintf(buf, "Choose directory for \"%s\"", pSet->pszName);
	SetWindowText(g_hwndMain, buf);

	GetClientRect(g_hwndPane, &r);
	iWidth = r.right;
	iHeight = r.bottom;
		
	hwndButton = CreateWindow("BUTTON",
							  "Browse...",
							  WS_CHILD | WS_VISIBLE,
							  iWidth - (BUTTON_WIDTH) - (BUTTON_GAP),
							  iHeight - 2*BUTTON_HEIGHT - BUTTON_GAP,
							  BUTTON_WIDTH,
							  BUTTON_HEIGHT,
							  g_hwndPane,
							  (HMENU) ID_BTN_BROWSE,
							  g_hInstance,
							  NULL);
	
	hwndStatic_DirLabel = CreateWindow("STATIC",
									   "Directory:",
									   WS_CHILD | WS_VISIBLE | SS_RIGHT,
									   BUTTON_GAP,
									   iHeight - BUTTON_GAP/2 - BUTTON_HEIGHT - BUTTON_GAP/2 - BUTTON_HEIGHT,
									   LABEL_WIDTH,
									   BUTTON_HEIGHT,
									   g_hwndPane,
									   (HMENU) 2001,
									   g_hInstance,
									   NULL);
	hwndStatic_SpaceLabel = CreateWindow("STATIC",
										 "Disk Space Available:",
										 WS_CHILD | WS_VISIBLE | SS_RIGHT,
										 BUTTON_GAP,
										 iHeight - BUTTON_GAP/2 - BUTTON_HEIGHT,
										 LABEL_WIDTH,
										 BUTTON_HEIGHT,
										 g_hwndPane,
										 (HMENU) 2002,
										 g_hInstance,
										 NULL);
	hwndStatic_Dir = CreateWindow("STATIC",
								  "TODO the dir",
								  WS_CHILD | WS_VISIBLE | SS_LEFT,
								  BUTTON_GAP + LABEL_WIDTH + BUTTON_GAP,
								  iHeight - BUTTON_GAP/2 - BUTTON_HEIGHT - BUTTON_GAP/2 - BUTTON_HEIGHT,
								  iWidth - (BUTTON_GAP + LABEL_WIDTH + BUTTON_GAP) - (BUTTON_WIDTH + 2*BUTTON_GAP),
								  BUTTON_HEIGHT,
								  g_hwndPane,
								  (HMENU) 2003,
								  g_hInstance,
								  NULL);
	hwndStatic_Space = CreateWindow("STATIC",
									"TODO the space",
									WS_CHILD | WS_VISIBLE | SS_LEFT,
									BUTTON_GAP + LABEL_WIDTH + BUTTON_GAP,
									iHeight - BUTTON_GAP/2 - BUTTON_HEIGHT,
									iWidth - (BUTTON_GAP + LABEL_WIDTH + BUTTON_GAP) - (BUTTON_WIDTH + 2*BUTTON_GAP),
									BUTTON_HEIGHT,
									g_hwndPane,
									(HMENU) 2004,
									g_hInstance,
									NULL);
	
	hwndStatic_Top = CreateWindow("STATIC",
								  "",
								  WS_CHILD | WS_VISIBLE | SS_LEFT,
								  BUTTON_GAP,
								  BUTTON_GAP,
								  iWidth - (2*BUTTON_GAP),
								  iHeight - BUTTON_GAP/2 - BUTTON_HEIGHT - BUTTON_GAP/2 - BUTTON_HEIGHT - 2*BUTTON_GAP,
								  g_hwndPane,
								  (HMENU) 2004,
								  g_hInstance,
								  NULL);

	SendMessage(hwndStatic_Top, WM_SETFONT, (WPARAM) g_hfontMain, 0);

	SendMessage(hwndButton, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	SendMessage(hwndStatic_DirLabel, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	SendMessage(hwndStatic_SpaceLabel, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	SendMessage(hwndStatic_Dir, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	SendMessage(hwndStatic_Space, WM_SETFONT, (WPARAM) g_hfontMain, 0);

	ASK_convertBytesToString(ASK_getFileSetTotalSizeInBytesToCopy(pSet), buf2);
	
	sprintf(buf,
			"\r\nPlease select a directory where '%s' will be installed.\r\n\r\nThe set '%s' contains %d files,\r\na total of %s.\r\n\r\n",
			pSet->pszName,
			pSet->pszName,
			ASK_getFileSetTotalFilesToCopy(pSet),
			buf2);
	SetWindowText(hwndStatic_Top, buf);

	g_pSet_BrowseDir = pSet;
	g_hwndStatic_BrowseDir = hwndStatic_Dir;
	g_hwndStatic_DiskSpace = hwndStatic_Space;
	
	SetCursor(hcursorPrev);

	_updateDirAndSpace(g_pSet_BrowseDir->pszDefaultPath);

	result = DoEventLoop();

	g_pSet_BrowseDir = NULL;
	g_hwndStatic_BrowseDir = NULL;
	g_hwndStatic_DiskSpace = NULL;
	
	DestroyWindow(hwndStatic_Top);
	DestroyWindow(hwndStatic_DirLabel);
	DestroyWindow(hwndStatic_SpaceLabel);
	DestroyWindow(hwndStatic_Dir);
	DestroyWindow(hwndStatic_Space);
	DestroyWindow(hwndButton);

	strcpy(pSet->szInstallPath, g_szBrowseDir);
	
	return result;
}

#define		TEXT_MARGIN			1

int ASK_DoScreen_readyToCopy(int iNumSets, ASK_FileSet** ppSets)
{
	int 	iTotalBytes = 0;
	int 	iTotalNumFiles = 0;
	char	buf[4096];
	int		ndxSet;
	char	buf2[256];

	for (ndxSet=0; ndxSet<iNumSets; ndxSet++)
	{
		ASK_FileSet* pSet = ppSets[ndxSet];

		iTotalBytes += ASK_getFileSetTotalSizeInBytesToCopy(pSet);
		iTotalNumFiles += ASK_getFileSetTotalFilesToCopy(pSet);
	}

	ASK_convertBytesToString(iTotalBytes, buf2);
	
	sprintf(buf, "\r\nFinal Confirmation\r\n\r\nSo far, nothing has been installed on your computer.\r\n\r\n\
You will be installing %d files, a total of\r\n\
%s.\r\n\r\n\
Please click the Next button to confirm that\r\n\
you would like to proceed with the installation.\r\n\
", iTotalNumFiles, buf2);

	return ASK_DoScreen_readme("Ready to Install", buf);
}

int ASK_DoScreen_copyComplete(int iNumSets, ASK_FileSet** ppSets)
{
	char buf[4096];

	ASK_closeRemoveFile();
	
	sprintf(buf, "\r\nSetup Complete\r\n\r\nThe software has been successfully installed\r\n\
on your computer.\r\n");

	EnableWindow(g_hwndButtonCancel, FALSE);
	SetWindowText(g_hwndButtonNext, "OK");

	return ASK_DoScreen_readme("All Done", buf);
}

int ASK_DoScreen_copy(int iNumSets, ASK_FileSet** ppSets)
{
	/*
	  Create:
	    a progress meter
		a static text which says "Copying file set 'name'"
		a static text which says "Copying file n of m (filename)"
		a static text which says "Copied n bytes of m (23%)"

	  for each file, check to see if it already exists.  If so,
	  compare the two.  copy to a bak directory if appropriate.
	  overwrite if appropriate.  prompt for overwrite if appropriate.
	  etc.
	*/

	int 	bCopyCancelled = 0;
	int		bErrorOccured = 0;
	int 	ndxSet;
	int 	ndxFile;

	int 	iTotalBytes = 0;
	int 	iTotalNumFiles = 0;
	int		iCurFile = 0;
	int		iCurBytes = 0;

	char 	buf[1024];
	
	HWND	hwndStatic_Top;
	HWND	hwndStatic_CurSet;
	HWND	hwndStatic_CurFile;
	HWND	hwndStatic_CurBytes;
	HWND	hwndProgress;
	int		iWidth;
	int		iHeight;
	RECT	r;

	SetWindowText(g_hwndMain, "Copying Files");

	for (ndxSet=0; ndxSet<iNumSets; ndxSet++)
	{
		ASK_FileSet* pSet = ppSets[ndxSet];

		iTotalBytes += ASK_getFileSetTotalSizeInBytesToCopy(pSet);
		iTotalNumFiles += ASK_getFileSetTotalFilesToCopy(pSet);
	}

	GetClientRect(g_hwndPane, &r);
	iWidth = r.right;
	iHeight = r.bottom;
		
	hwndStatic_CurSet = CreateWindow("STATIC",
								  "",
								  WS_CHILD | WS_VISIBLE | SS_LEFT,
								  4*BUTTON_GAP,
								  iHeight - BUTTON_GAP - BUTTON_HEIGHT*4,
								  iWidth - 4*BUTTON_GAP - BUTTON_GAP,
								  BUTTON_HEIGHT,
								  g_hwndPane,
								  (HMENU) 2000,
								  g_hInstance,
								  NULL);
	
	hwndStatic_CurFile = CreateWindow("STATIC",
								  "",
								  WS_CHILD | WS_VISIBLE | SS_LEFT,
								  4*BUTTON_GAP,
								  iHeight - BUTTON_GAP - BUTTON_HEIGHT*3,
								  iWidth - 4*BUTTON_GAP - BUTTON_GAP,
								  BUTTON_HEIGHT,
								  g_hwndPane,
								  (HMENU) 2001,
								  g_hInstance,
								  NULL);
	
	hwndStatic_CurBytes = CreateWindow("STATIC",
								  "",
								  WS_CHILD | WS_VISIBLE | SS_LEFT,
								  4*BUTTON_GAP,
								  iHeight - BUTTON_GAP - BUTTON_HEIGHT*2,
								  iWidth - 4*BUTTON_GAP - BUTTON_GAP,
								  BUTTON_HEIGHT,
								  g_hwndPane,
								  (HMENU) 2002,
								  g_hInstance,
								  NULL);

	SendMessage(hwndStatic_CurSet, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	SendMessage(hwndStatic_CurFile, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	SendMessage(hwndStatic_CurBytes, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	
	hwndProgress = CreateWindowEx(0,
								  PROGRESS_CLASS,
								  NULL,
								  WS_CHILD | WS_VISIBLE | WS_BORDER,
								  4*BUTTON_GAP,
								  iHeight - BUTTON_GAP - BUTTON_HEIGHT*1,
								  iWidth - 4*BUTTON_GAP - BUTTON_GAP,
								  BUTTON_HEIGHT,
								  g_hwndPane,
								  (HMENU) 2003,
								  g_hInstance,
								  NULL);
	SendMessage(hwndProgress, PBM_SETRANGE, (WPARAM) 0, MAKELPARAM(0,100));
	
	hwndStatic_Top = CreateWindow("STATIC",
								  "",
								  WS_CHILD | WS_VISIBLE | SS_LEFT,
								  BUTTON_GAP,
								  BUTTON_GAP,
								  iWidth - (2*BUTTON_GAP),
								  iHeight - BUTTON_GAP - BUTTON_HEIGHT*4 - BUTTON_GAP,
								  g_hwndPane,
								  (HMENU) 2004,
								  g_hInstance,
								  NULL);

	SendMessage(hwndStatic_Top, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	
	sprintf(buf,
			"\r\nPlease wait while files are currently being installed on your computer."
			);
	SetWindowText(hwndStatic_Top, buf);

	for (ndxSet=0; ndxSet<iNumSets && bCopyCancelled == 0; ndxSet++)
	{
		ASK_FileSet* pSet = ppSets[ndxSet];

		// TODO specify directory for this set?
		sprintf(buf, "Copying '%s'", pSet->pszName);
		SetWindowText(hwndStatic_CurSet, buf);

		ASK_fixSlashes(pSet->szInstallPath);
		ASK_verifyDirExists(pSet->szInstallPath);

		for (ndxFile=0; ndxFile<pSet->iNumFilesInSet && bCopyCancelled == 0; ndxFile++)
		{
			if(pSet->aFiles[ndxFile]->bNoCopy == 0)
			{
				ASK_DataFile* pFile = pSet->aFiles[ndxFile];
				
				iCurFile++;
				
				sprintf(buf, "Copying file %d of %d (%s)",
						iCurFile,
						iTotalNumFiles,
						pFile->pszFileName
						);
				SetWindowText(hwndStatic_CurFile, buf);

				{
					int err;
					int bKeepExistingFile = 0;

					if (pFile->pszRelPath)
					{
						char szRelDir[ASK_MAX_PATH + 1];

						sprintf(szRelDir, "%s/%s", pSet->szInstallPath, pFile->pszRelPath);
						ASK_fixSlashes(szRelDir);
						ASK_verifyDirExists(szRelDir);

						sprintf(pFile->szInstallPath, "%s/%s/%s", pSet->szInstallPath, pFile->pszRelPath, pFile->pszFileName);
					}
					else
					{
						sprintf(pFile->szInstallPath, "%s/%s", pSet->szInstallPath, pFile->pszFileName);
					}

					ASK_fixSlashes(pFile->szInstallPath);

					if(ASK_isFileNewer(pFile->szInstallPath, pFile->iModTime))
					{
						sprintf(buf, "A file on your system is newer than the one supplied with this\r\n " \
									"installation. It is recommended that you keep the existing file.\r\n\r\n " \
									"File name:\t'%s'\r\n\r\n" \
									"Do you want to keep this file?",
							pFile->szInstallPath);

						if(IDYES == MessageBox(g_hwndMain, buf, "Version Conflict",
							MB_ICONEXCLAMATION | MB_YESNO))
						{
							bKeepExistingFile = 1;
							iCurBytes += pFile->iOriginalLength;
						}
					}

					if(bKeepExistingFile == 0)
					{
						err = ASK_decompressAndWriteFile(pFile);
						if (err < 0)
						{
							sprintf(buf, "Attempt to write file '%s' failed", pFile->szInstallPath);

							bErrorOccured = 1;
							if(IDCANCEL == MessageBox(g_hwndMain, buf, "Error",
								MB_ICONEXCLAMATION | MB_OKCANCEL))
							{
								if(bCopyCancelled = try_cancel())
								{
									break;
								}
							}
						}
						else
						{
							iCurBytes += pFile->iOriginalLength;
						}
					}

					sprintf(buf, "Processed %d of %d bytes (%d%%)",iCurBytes, iTotalBytes, (iCurBytes * 100 / iTotalBytes));
					SetWindowText(hwndStatic_CurBytes, buf);

					SendMessage(hwndProgress, PBM_SETPOS, (WPARAM) (iCurBytes * 100 / iTotalBytes), (LPARAM) 0);

					if (
						!(pFile->bNoCopy)
						&& !(pFile->bNoRemove))
					{
						ASK_addToRemoveFile(pFile->szInstallPath);
					}
				}
			}
		}
	}

	if(bErrorOccured == 0)
	{
		sprintf(buf,
				"\r\nCopying is now complete.  Click the 'Next' button below to continue."
				);
		SetWindowText(hwndStatic_Top, buf);

		SetWindowText(g_hwndMain, "Done Copying Files");
		EnableWindow(g_hwndButtonCancel, FALSE);

		ShowWindow(hwndProgress, SW_HIDE);

		s_bInstallationSuccessfull = 1;
		DoEventLoop();
	}
	else
	{
		EnableWindow(g_hwndButtonNext, FALSE);
		EnableWindow(g_hwndButtonCancel, FALSE);
	}
	
	DestroyWindow(hwndStatic_Top);
	DestroyWindow(hwndStatic_CurSet);
	DestroyWindow(hwndStatic_CurFile);
	DestroyWindow(hwndStatic_CurBytes);
	DestroyWindow(hwndProgress);
	
	return bErrorOccured;
}

int ASK_DoScreen_license(char* pszText)
{
	int		result;
	HWND	hwndText;
	HWND	hwndStatic;
	RECT 	r;

	SetWindowText(g_hwndMain, "GNU General Public License.");
	
	GetClientRect(g_hwndPane, &r);
	
	hwndText = CreateWindow("EDIT",
							  "",
							  WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_LEFT | ES_READONLY | WS_HSCROLL | WS_VSCROLL,
							  TEXT_MARGIN,
							  TEXT_MARGIN,
							  r.right - 2*TEXT_MARGIN,
							  r.bottom - 30 - TEXT_MARGIN,
							  g_hwndPane,
							  (HMENU) 2000,
							  g_hInstance,
							  NULL);
	
	hwndStatic = CreateWindow("STATIC",
							  "",
							  WS_CHILD | WS_VISIBLE | SS_LEFT,
							  TEXT_MARGIN,
							  r.bottom - 24,
							  r.right - 2*TEXT_MARGIN,
							  24,
							  g_hwndPane,
							  (HMENU) 2000,
							  g_hInstance,
							  NULL);

	SendMessage(hwndStatic, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	SendMessage(hwndText, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	
	SetWindowText(hwndStatic, "These are the terms of the GNU General Public License.");
	
	SetWindowText(hwndText, pszText);

	result = DoEventLoop();

	SetWindowText(g_hwndButtonNext, "Next");
	SetWindowText(g_hwndButtonCancel, "Cancel");

	DestroyWindow(hwndText);
	DestroyWindow(hwndStatic);
	
	hwndText = NULL;
	
	return result;
}

int ASK_DoScreen_readme(char* pszTitle, char* pszText)
{
	int 	result;
	RECT 	r;
	HWND 	hwndText;

	SetWindowText(g_hwndMain, pszTitle);
	
	GetClientRect(g_hwndPane, &r);
	
	hwndText = CreateWindow("EDIT",
							  "",
							  WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_LEFT | ES_READONLY | WS_HSCROLL | WS_VSCROLL,
							  TEXT_MARGIN,
							  TEXT_MARGIN,
							  r.right - 2*TEXT_MARGIN,
							  r.bottom - TEXT_MARGIN,
							  g_hwndPane,
							  (HMENU) 2000,
							  g_hInstance,
							  NULL);
	SendMessage(hwndText, WM_SETFONT, (WPARAM) g_hfontMain, 0);
	
	SetWindowText(hwndText, pszText);

	result = DoEventLoop();

	DestroyWindow(hwndText);
	hwndText = NULL;
	
	return result;
}

// CreateLink - uses the shell's IShellLink and IPersistFile interfaces 
//   to create and store a shortcut to the specified object. 
// Returns the result of calling the member functions of the interfaces. 
// lpszPathObj - address of a buffer containing the path of the object 
// lpszPathLink - address of a buffer containing the path where the 
//   shell link is to be stored 
// lpszDesc - address of a buffer containing the description of the 
//   shell link 
 
HRESULT CreateLink(LPCSTR lpszPathObj, LPSTR lpszPathLink, LPSTR lpszDesc) 
{ 
    HRESULT hres; 
    IShellLink* psl; 
 
    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, 
        CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl); 
    if (SUCCEEDED(hres)) { 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target, and add the 
        // description. 
        psl->lpVtbl->SetPath(psl, lpszPathObj); 
        psl->lpVtbl->SetDescription(psl, lpszDesc); 
 
       // Query IShellLink for the IPersistFile interface for saving the 
       // shortcut in persistent storage. 
        hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, 
            &ppf); 
 
        if (SUCCEEDED(hres)) { 
            WORD wsz[MAX_PATH]; 
 
            // Ensure that the string is ANSI. 
            MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, 
                wsz, MAX_PATH); 
 
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->lpVtbl->Save(ppf, wsz, TRUE); 
            ppf->lpVtbl->Release(ppf); 
        } 
        psl->lpVtbl->Release(psl); 
    }

	ASK_addToRemoveFile(lpszPathLink);
	
    return hres; 
} 

void ASK_CreateDesktopShortcut(ASK_DataFile* pFile)
{
	LPMALLOC      ShellMalloc;
	LPITEMIDLIST  DesktopPidl;
	
	char szDesktopDir[MAX_PATH];

	// We are going to create a pidl, and it will need to be
	// freed by the shell mallocator. Get the shell mallocator
	// object using API SHGetMalloc function. Return if failure.
	if(FAILED(SHGetMalloc(&ShellMalloc)))
		return;

	// use the API to get a pidl for the desktop directory
	// if function fails, return without proceeding
	if(FAILED(SHGetSpecialFolderLocation(NULL,
										 CSIDL_DESKTOPDIRECTORY,
										 &DesktopPidl)))
		return;

	// Now convert the pidl to a character string
	// return if function fails
	if(!SHGetPathFromIDList(DesktopPidl, szDesktopDir))
	{
		ShellMalloc->lpVtbl->Free(ShellMalloc, DesktopPidl);
		ShellMalloc->lpVtbl->Release(ShellMalloc);
		return;
	}

	// At this point, we are done with the pidl and the
	// mallocator, so free them up
	ShellMalloc->lpVtbl->Free(ShellMalloc, DesktopPidl);
	ShellMalloc->lpVtbl->Release(ShellMalloc);

	{
		char szLinkName[512];

		sprintf(szLinkName, "\\%s.lnk", pFile->pszDesktopShortcut);
		strcat(szDesktopDir, szLinkName);
	
		CreateLink(pFile->szInstallPath, szDesktopDir, pFile->pszDesktopShortcut);
	}
}

void ASK_PopulateStartMenu(char* pszGroupName, int iNumSets, ASK_FileSet** ppSets)
{
	LPMALLOC      ShellMalloc;
	LPITEMIDLIST  DesktopPidl;
	
	char szProgramsDir[MAX_PATH];

	// We are going to create a pidl, and it will need to be
	// freed by the shell mallocator. Get the shell mallocator
	// object using API SHGetMalloc function. Return if failure.
	if(FAILED(SHGetMalloc(&ShellMalloc)))
		return;

	// use the API to get a pidl for the desktop directory
	// if function fails, return without proceeding
	if(FAILED(SHGetSpecialFolderLocation(NULL,
										 CSIDL_PROGRAMS,
										 &DesktopPidl)))
		return;

	// Now convert the pidl to a character string
	// return if function fails
	if(!SHGetPathFromIDList(DesktopPidl, szProgramsDir))
	{
		ShellMalloc->lpVtbl->Free(ShellMalloc, DesktopPidl);
		ShellMalloc->lpVtbl->Release(ShellMalloc);
		return;
	}

	// At this point, we are done with the pidl and the
	// mallocator, so free them up
	ShellMalloc->lpVtbl->Free(ShellMalloc, DesktopPidl);
	ShellMalloc->lpVtbl->Release(ShellMalloc);

	strcat(szProgramsDir, "\\");
	strcat(szProgramsDir, pszGroupName);
	CreateDirectory(szProgramsDir, NULL);
	ASK_addToRemoveFile(szProgramsDir);

	{
		int ndxSet;
		int ndxFile;
	
		for (ndxSet=0; ndxSet<iNumSets; ndxSet++)
		{
			ASK_FileSet* pSet = ppSets[ndxSet];

			for (ndxFile=0; ndxFile<pSet->iNumFilesInSet; ndxFile++)
			{
				ASK_DataFile* pFile = pSet->aFiles[ndxFile];

				if (pFile->pszProgramsShortcut)
				{
					char szLinkName[ASK_MAX_PATH+1];

					sprintf(szLinkName, "%s\\%s.lnk", szProgramsDir, pFile->pszProgramsShortcut);
	
					CreateLink(pFile->szInstallPath, szLinkName, pFile->pszDesktopShortcut);
				}
			}
		}
	}
}

void ASK_CreateDesktopShortcuts(int iNumSets, ASK_FileSet** ppSets)
{
	int ndxSet;
	int ndxFile;
	
	for (ndxSet=0; ndxSet<iNumSets; ndxSet++)
	{
		ASK_FileSet* pSet = ppSets[ndxSet];

		for (ndxFile=0; ndxFile<pSet->iNumFilesInSet; ndxFile++)
		{
			ASK_DataFile* pFile = pSet->aFiles[ndxFile];

			if (pFile->pszDesktopShortcut)
			{
				ASK_CreateDesktopShortcut(pFile);
			}
		}
	}
}

int ASK_verifyDirExists(char* szDir)
{
	char szDir2[ASK_MAX_PATH+1];
	char* p;

	p = szDir;
	while (*p)
	{
		if (*p == '\\')
		{
			int len = p - szDir;
			
			strncpy(szDir2, szDir, p - szDir);
			szDir2[len] = 0;

			if (!strchr(szDir2, '\\'))
			{
				strcat(szDir2, "\\");
			}

			if (!ASK_isDirectory(szDir2))
			{
				CreateDirectory(szDir2, NULL);
				ASK_addToRemoveFile(szDir2);
			}
		}

		p++;
	}

	if (!ASK_isDirectory(szDir))
	{
		CreateDirectory(szDir, NULL);
		ASK_addToRemoveFile(szDir);
	}
			
	return 0;
}

