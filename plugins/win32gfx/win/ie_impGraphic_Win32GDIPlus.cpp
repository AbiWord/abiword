/* AbiWord
 *
 * Copyright (C) 2004 Jordi Mas i Hern�ndez
 * Win32 native plugin based on win32 GDIPlus
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
#include <stdlib.h>
#include "ut_string.h"
#include "ut_bytebuf.h"
#include "ie_impGraphic.h"
#include "fg_GraphicRaster.h"
#include "ut_assert.h"
#include "xap_Module.h"

/*

		GDI Plus definitions

*/

typedef DWORD Status;
typedef DWORD ULONG_PTR;
typedef DWORD EncoderParameters;
class GpImage {};


struct GdiplusStartupInput
{
    UINT32 GdiplusVersion;  
    DWORD DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;  

};

struct GdiplusStartupOutput
{
    DWORD NotificationHook;
    DWORD NotificationUnhook;
};

/* PNG GUID's encoder */
CLSID gdip_png_guid = { 0x557cf406, 0x1a04, 0x11d3, { 0x9a, 0x73, 0x0, 0x0, 0xf8, 0x1e, 0xf3, 0x2e } };


/* Globals */
static ULONG_PTR gdiplusToken = NULL;
static HINSTANCE gdipluslib = NULL;


/* Callbacks */
typedef Status (CALLBACK* GDIPLUSSTARTUP) (ULONG_PTR *token,  const GdiplusStartupInput *input,
    OUT GdiplusStartupOutput *output);
typedef Status (CALLBACK* GDIPLOADIMAGEFROMSTREAM) (IStream* stream, GpImage **image);
typedef Status (CALLBACK* GDIPSAVEIMAGETOSTREAM) (GpImage *image, IStream* stream,
                      CLSID* clsidEncoder, EncoderParameters* encoderParams);

typedef Status (CALLBACK* GDIPDISPOSEIMAGE) (GpImage *image);

typedef void (CALLBACK* GDIPLUSSHUTDOWN) (ULONG_PTR token);


/*

		GDI Plus dynamic loded functions

*/

Status 
GdiplusStartup (ULONG_PTR *token,  const GdiplusStartupInput *input,
    GdiplusStartupOutput *output)
{

	GDIPLUSSTARTUP	proc = NULL;

	if (gdipluslib)			  
		proc = (GDIPLUSSTARTUP) GetProcAddress(gdipluslib, "GdiplusStartup");	

	if (!proc)
		return -1;

	return (*proc) (token, input, output);
}

void 
GdiplusShutdown (ULONG_PTR token)
{
	GDIPLUSSHUTDOWN	proc = NULL;

	if (gdipluslib)			  
		proc = (GDIPLUSSHUTDOWN) GetProcAddress(gdipluslib, "GdiplusShutdown");	

	if (!proc)
		return;

	(*proc) (token);
}

Status 
GdipLoadImageFromStream (IStream* stream, GpImage **image)
{

	GDIPLOADIMAGEFROMSTREAM	proc = NULL;

	if (gdipluslib)			  
		proc = (GDIPLOADIMAGEFROMSTREAM) GetProcAddress(gdipluslib, "GdipLoadImageFromStream");	

	if (!proc)
		return -1;

	return (*proc) (stream, image);
}



Status 
GdipSaveImageToStream (GpImage *image, IStream* stream,
                      CLSID* clsidEncoder, EncoderParameters* encoderParams)
{

	GDIPSAVEIMAGETOSTREAM	proc = NULL;

	if (gdipluslib)			  
		proc = (GDIPSAVEIMAGETOSTREAM) GetProcAddress(gdipluslib, "GdipSaveImageToStream");	

	if (!proc)
		return -1;

	return (*proc) (image, stream, clsidEncoder, encoderParams);

}

Status 
GdipDisposeImage (GpImage *image)
{
	GDIPDISPOSEIMAGE	proc = NULL;

	if (gdipluslib)			  
		proc = (GDIPDISPOSEIMAGE) GetProcAddress(gdipluslib, "GdipDisposeImage");	

	if (!proc)
		return -1;

	return (*proc) (image);
}


/*
	
	  Private functions

*/

// Initialize GDI+.
void 
initGDIPlus ()
{
	
	GdiplusStartupInput input;	

	if (!gdipluslib)
		gdipluslib = ::LoadLibrary (TEXT("gdiplus.dll"));

	if (!gdipluslib)
		return;

	input.GdiplusVersion = 1;
    input.DebugEventCallback = NULL;
    input.SuppressBackgroundThread = input.SuppressExternalCodecs = FALSE;

	GdiplusStartup (&gdiplusToken, &input, NULL);
	

}


/*
	
	  Public functions

*/

bool 
isGDIPlusAvailable()
{
	if (gdiplusToken==NULL)
		initGDIPlus();
			
	return (gdiplusToken == NULL ? false : true);
}

void 
shutDownGDIPlus()
{
	GdiplusShutdown(gdiplusToken);
}


/* Conversion function */
UT_Error 
GDIconvertGraphic(UT_ByteBuf * pBB, UT_ByteBuf* pBBOut)
{
	IStream *stream, *streamOut = NULL;	
	HGLOBAL hG;		
	GpImage *image = NULL;
	Status status;
	HRESULT hr;
	HGLOBAL phglobal = NULL;
	LARGE_INTEGER ulnSize;
	LARGE_INTEGER lnOffset;
	ULONG ulBytesRead;
	lnOffset.QuadPart = 0;
	
    // We need to store the incoming bytebuffer in a Windows global heap	
    size_t nBlockLen = pBB->getLength();   	
    hG = GlobalAlloc (GPTR, nBlockLen);

    if (!hG) 
      return UT_IE_NOMEMORY;	

	CopyMemory (hG, pBB->getPointer(0), nBlockLen);   	
	
    // Create the in stream from heap
    hr = CreateStreamOnHGlobal (hG, FALSE ,&stream);
    if (!SUCCEEDED (hr) || !stream)
	{
		GlobalFree (hG);
		return UT_IE_NOMEMORY;
	}

	 // Create the OUT stream and let GDI+ allocated it
    hr = CreateStreamOnHGlobal (NULL, TRUE, &streamOut);
	
	status =  GdipLoadImageFromStream (stream, &image);

	if (status != 0)
		return UT_ERROR;
	
	status =  GdipSaveImageToStream (image, streamOut, &gdip_png_guid, NULL);

	if (status != 0)
		return UT_ERROR;

	GdipDisposeImage (image);
	
	GetHGlobalFromStream (streamOut, &phglobal);

	// get the size of the stream	
	if(streamOut->Seek (lnOffset, STREAM_SEEK_END, (union _ULARGE_INTEGER *) &ulnSize) != S_OK)
		return UT_ERROR;    
	
	if(streamOut->Seek (lnOffset, STREAM_SEEK_SET, NULL) != S_OK)
		return UT_ERROR;

	DWORD sz = (DWORD)ulnSize.QuadPart;
	char *pBuff = new char [sz];

	// Read the stream directly into the buffer
    
    streamOut->Read (pBuff, (ULONG) ulnSize.QuadPart, (ULONG *) &ulBytesRead);
    
	pBBOut->append ((const unsigned char*)pBuff, ulBytesRead);

	delete pBuff;
	
	streamOut->Release ();
	stream->Release ();
    GlobalFree (hG);   
	
	
	return UT_OK;
}

