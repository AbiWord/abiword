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

// bmp2png.c
// A sample program that uses the PNGDIB library
// converts a BMP image file to a PNG image file

#include <windows.h>
#include <stdio.h>
#include "pngdib.h"


// This is just a generic function to read a file into a memory block.
int read_bmp_to_mem(const char *bmpfn,unsigned char **bmppp, DWORD *fsizep)
{
	HANDLE hfile;
	DWORD fsize;
	unsigned char *fbuf;
	DWORD bytesread;

	hfile=CreateFile(bmpfn,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile==INVALID_HANDLE_VALUE) return 1;

	fsize=GetFileSize(hfile,NULL);
	if(fsize>0) {
		fbuf=(unsigned char*)GlobalAlloc(GPTR,fsize);
		if(fbuf) {
			if(ReadFile(hfile,(void*)fbuf,fsize,&bytesread,NULL)) {
				if(bytesread==fsize) { 
					(*bmppp)  = fbuf;
					(*fsizep) = fsize;
					CloseHandle(hfile);
					return 0;   // success
				}
			}
			GlobalFree((void*)fbuf);
		}
	}
	CloseHandle(hfile);
	return 1;  // error
}


// returns 0 on success, nonzero of failure
int convertBMP2PNG(const char *bmpfn, const char *pngfn)
{
	PNGD_D2PINFO d2p;
	int ret;
	unsigned char *bmpp;
	DWORD bmpsize;
	LPBITMAPFILEHEADER lpbmfh;
	char errmsg[100];

	if(read_bmp_to_mem(bmpfn,&bmpp, &bmpsize)) {
		printf("can't read BMP from file\n");
		return 1;
	}

	ZeroMemory((void*)&d2p,sizeof(PNGD_D2PINFO));

	d2p.structsize=sizeof(PNGD_D2PINFO);

	// The only flag currently defined is PNGD_INTERLACE, which
	// does what you'd expect.
	d2p.flags=0;

	d2p.pngfn=pngfn;

	// The first 14 bytes of a BMP file is a file header, the
	// rest is a DIB.
	d2p.lpdib = (LPBITMAPINFOHEADER)&bmpp[14];

	// This can be left at 0 if you know your DIB is valid.
	d2p.dibsize = bmpsize-14;

	lpbmfh= (LPBITMAPFILEHEADER)bmpp;
	if(lpbmfh->bfOffBits >= bmpsize) {
		printf("Corrupt BMP\n");
		return 1;
	}

	// It's usually okay to leave lpbits set to NULL.
	d2p.lpbits = &bmpp[lpbmfh->bfOffBits];

	// This can be left at 0 if you know your DIB is valid.
	d2p.bitssize = bmpsize-lpbmfh->bfOffBits;

	d2p.software = "PNGDIB bmp2png";
	d2p.errmsg = errmsg; strcpy(errmsg,"");

	ret=write_dib_to_png(&d2p);
	// returns 0 on success
	if(ret) {
		printf("Error: %s (%d)\n",d2p.errmsg,ret);
		return 1;
	}

	// 
	GlobalFree(bmpp);

	return 0;
}

