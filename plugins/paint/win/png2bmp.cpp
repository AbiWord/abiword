/**
PNGDIB - a mini DIB-PNG conversion library for Win32
By Jason Summers  <jason1@pobox.com>
Version 2.1.0/2.2.0, copyright 2002
Web site: http://pobox.com/~jason1/imaging/pngdib/


This software is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.

Permission is hereby granted to use, copy, modify, and distribute this
source code for any purpose, without fee.
**/

// png2dib.c
// sample using PNGDIB library
// convert a PNG image file to a BMP image file

#include <windows.h>
//#include "png.h"
#include <stdio.h>
#include "pngdib.h"


int write_dib_to_bmp(const char *bmpfn, LPBITMAPINFOHEADER lpdib, 
					 int dibsize, int bitsoffset)
{
	HANDLE hfile;
	BITMAPFILEHEADER h;
	DWORD written, err;

	ZeroMemory((void*)&h,sizeof(h));
	h.bfType= MAKEWORD('B','M');
	h.bfSize=    sizeof(BITMAPFILEHEADER)+dibsize;
	h.bfOffBits= sizeof(BITMAPFILEHEADER)+bitsoffset;

	hfile=CreateFile(bmpfn,GENERIC_WRITE,FILE_SHARE_READ,NULL,
		CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile==INVALID_HANDLE_VALUE) return 1;

	if(!WriteFile(hfile,(void*)&h,sizeof(BITMAPFILEHEADER),&written,NULL)) {
		err=GetLastError();
		CloseHandle(hfile);
		return 1;
	}
	if(!WriteFile(hfile,(void*)lpdib,dibsize,&written,NULL)) {
		err=GetLastError();
		CloseHandle(hfile);
		return 1;
	}
	CloseHandle(hfile);
	return 0;
}


// returns 0 on success, nonzero of failure
int convertPNG2BMP(const char *pngfn, const char *bmpfn)
{
	PNGD_P2DINFO p2d;
	int ret;
	char errmsg[100];

	ZeroMemory((void*)&p2d,sizeof(PNGD_P2DINFO));

	p2d.structsize=sizeof(PNGD_P2DINFO);
	p2d.flags=0;
	p2d.pngfn=pngfn;      // name of the file to read
	p2d.errmsg=errmsg; strcpy(errmsg,"");

	if ((ret=read_png_to_dib(&p2d))) return ret;
	// returns 0 on success
	// see pngdib.h for a list of error codes
	//	printf("Error: %s (%d)\n",errmsg,ret);

	if(write_dib_to_bmp(bmpfn, p2d.lpdib, p2d.dibsize, p2d.bits_offs)) 
	{
		//	printf("Can't write BMP file\n");
		ret = PNGD_E_WRITE;  // yeah I know, BMP != PNG, but write error is better than generic error
	}

	// The DIB will have been allocated with GlobalAlloc(GMEM_FIXED...).
	// You should free it with GlobalFree when you're done with it.
	GlobalFree((void*)p2d.lpdib);

	return ret;
}
