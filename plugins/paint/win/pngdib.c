// pngdib.c
//
// PNGDIB - a mini imaging library for Win32
// By Jason Summers
// This software may be used without restriction.
//
// read_png_to_dib()
//     Read a PNG image file and convert it to a Windows "Device Independent
//     Bitmap" (DIB)
//
// write_dib_to_png()
//     Write a DIB to a PNG image file
//
// pngdib_get_version_string(void)
//
// pngdib_get_version(void)
//

#define PNGDIB_SRC_VERSION           20200
#define PNGDIB_SRC_VERSION_STRING   "2.2.0"

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include "png.h"
#include "pngdib.h"

#if PNGDIB_SRC_VERSION != PNGDIB_HEADER_VERSION
#error Wrong PNGDIB header file version
#endif

#if (PNG_LIBPNG_VER<10202) || \
    (PNG_LIBPNG_VER==10202 && PNG_LIBPNG_BUILD_TYPE<2) || \
    (PNG_LIBPNG_VER==10202 && PNG_LIBPNG_BUILD_TYPE==2 && PNG_LIBPNG_VER_BUILD<5)
#error libpng 1.2.2b5 or higher is recommended
/* You can comment out the previous line if you aren't using gamma
 * correction, or don't care about a few obscure gamma correction
 * problems that exist in earlier versions of libpng. */
#endif


// This is basically a Windows-only utility with a simple-as-possible
// interface, so I'm not too concerned about allowing a
// user-configurable screen gamma.
static const double screen_gamma = 2.2;

#define MAX_ERRMSGLEN 100

struct errstruct {
	jmp_buf *jbufp;
	char *errmsg;
};

static void pngd_get_error_message(int rv,char *e)
{
	switch(rv) {
	case PNGD_E_ERROR: strcpy(e,"Unknown error"); break;
	case PNGD_E_VERSION: strcpy(e,"Incompatible library version"); break;
	case PNGD_E_NOMEM: strcpy(e,"Unable to allocate memory"); break;
	case PNGD_E_UNSUPP: strcpy(e,"Invalid or unsupported image"); break;
	case PNGD_E_LIBPNG: strcpy(e,"libpng reported an error"); break;
	case PNGD_E_BADBMP: strcpy(e,"Invalid BMP image"); break;
	case PNGD_E_BADPNG: strcpy(e,"Invalid PNG image"); break;
	case PNGD_E_READ: strcpy(e,"Unable to read file"); break;
	case PNGD_E_WRITE: strcpy(e,"Unable to write file"); break;
	}
}

static unsigned char* uncompress_dib(LPBITMAPINFO lpbmi1, int infosize, void *lpbits1)
{
	LPBITMAPINFOHEADER lpdib2;
	unsigned char *lpbits2;
	void *whatever;
	int linesize, bitssize;
	HBITMAP hb;
	HDC hdc;
	HGDIOBJ rvgdi;
	int rvi;
	int width,height;
	LPBITMAPINFOHEADER lpdib1;

	lpdib1=(LPBITMAPINFOHEADER)lpbmi1;
	width=lpdib1->biWidth;
	height=lpdib1->biHeight;

	linesize= (((width * lpdib1->biBitCount)+31)/32)*4;
	bitssize= linesize*height;

	lpdib2= (LPBITMAPINFOHEADER)malloc(infosize);
	if(!lpdib2) return NULL;

	//  create a header for the new uncompressed DIB
	CopyMemory((void*)lpdib2,(void*)lpdib1,infosize);
	lpdib2->biCompression=BI_RGB;
	lpdib2->biSizeImage=0;

	lpbits2= (unsigned char*)malloc(bitssize);
	if(!lpbits2) { free((void*)lpdib2); return NULL; }


	// Windows bitmap handling functions are not exactly convenient,
	// especially when trying to deal with DIBs. Every function wants
	// to convert them into DDBs. We have to play stupid games and
	// convert back and forth. This probably uses too much memory,
	// and I'm not 100% sure it is exactly correct, but it seems to
	// work for me.

	hb=CreateDIBSection(NULL,(LPBITMAPINFO)lpdib2,DIB_RGB_COLORS,&whatever,NULL,0);

	hdc=CreateCompatibleDC(NULL);
	rvgdi=SelectObject(hdc,hb);
	//SetStretchBltMode(hdc,COLORONCOLOR);
	rvi=StretchDIBits(hdc,
		0,0,width,height,
		0,0,width,height,
		lpbits1, (LPBITMAPINFO)lpdib1,
		DIB_RGB_COLORS,SRCCOPY);
	rvi=GetDIBits(hdc,hb,0,height, (LPVOID)lpbits2,
		(LPBITMAPINFO)lpdib2,DIB_RGB_COLORS);

	DeleteDC(hdc);
	DeleteObject(hb);
	free((void*)lpdib2);

	return lpbits2;
}


static void my_error_fn(png_structp png_ptr, const char *err_msg)
{
	struct errstruct *errinfop;
	jmp_buf *j;

	errinfop = (struct errstruct *)png_get_error_ptr(png_ptr);
	j = errinfop->jbufp;
	_snprintf(errinfop->errmsg,MAX_ERRMSGLEN,"[libpng] %s",err_msg);
	errinfop->errmsg[MAX_ERRMSGLEN-1]='\0';
	longjmp(*j, -1);
}


static void my_warning_fn(png_structp png_ptr, const char *warn_msg)
{
	return;
}


// This function should perform identically to libpng's gamma correction.
// I'd prefer to have libpng do all gamma correction itself,
// but I can't figure out how to do that efficiently.
static void gamma_correct(double screen_gamma,double file_gamma,
	 unsigned char *red, unsigned char *green, unsigned char *blue)
{
	double g;

#ifndef PNG_GAMMA_THRESHOLD
#  define PNG_GAMMA_THRESHOLD 0.05
#endif

	if(fabs(screen_gamma*file_gamma-1.0)<=PNG_GAMMA_THRESHOLD) return;

	if (screen_gamma>0.000001)
		g=1.0/(file_gamma*screen_gamma);
	else
		g=1.0;

	(*red)   = (unsigned char)(pow((double)(*red  )/255.0,g)*255.0+0.5);
	(*green) = (unsigned char)(pow((double)(*green)/255.0,g)*255.0+0.5);
	(*blue)  = (unsigned char)(pow((double)(*blue )/255.0,g)*255.0+0.5);
}

int read_png_to_dib(PNGD_P2DINFO *p2dp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	jmp_buf jbuf;
	struct errstruct errinfo;
	png_uint_32 width, height;
	int png_bit_depth, color_type, interlace_type;
	png_colorp png_palette;
	png_uint_32 res_x, res_y;
	int has_phys, has_gama;
	int res_unit_type;
	FILE *fp;
	int palette_entries;
	unsigned char **row_pointers;
	unsigned char *lpdib;
	unsigned char *dib_palette;
	unsigned char *dib_bits;
	unsigned char *tmprow;
	int dib_bpp, dib_bytesperrow;
	int i,j;
	int rv;
	png_color_16 bkgd; // used with png_set_background
	int has_trns, trns_color;
	int has_bkgd;  // ==1 if there a bkgd chunk, and USE_BKGD flag
	png_color_16p temp_colorp;
	png_color_16p bg_colorp;  // background color (if has_bkgd)
	png_bytep trns_trans;
	int manual_trns;
	int manual_gamma;
	struct PNGD_COLOR_struct bkgd_color;
	int is_grayscale,has_alpha_channel;
	double file_gamma;
	char *errmsg;
	char dummy_errmsg[MAX_ERRMSGLEN];
	int imginfo; // flag for v2.1 fields
	int use_heapalloc;
	HANDLE heap;

	imginfo=0;
	use_heapalloc=0;

	// fields through errmsg must exist
	if(p2dp->structsize<48) return PNGD_E_VERSION;

	// try to be somewhat backward-compatible
	if(p2dp->structsize>=88) {
		//if(p2dp->reserved1 != 0) return PNGD_E_VERSION;
		imginfo=1;
	}

	if(p2dp->structsize>=96) {
		//if(p2dp->reserved2 != 0) return PNGD_E_VERSION;
		if(p2dp->flags & PNGD_USE_HEAPALLOC) {
			use_heapalloc=1;
			heap = p2dp->heap;
			if(!heap) heap = GetProcessHeap();
		}
	}

	manual_trns=0;
	has_trns=has_bkgd=0;
	rv=PNGD_E_ERROR;
	png_ptr=NULL;
	info_ptr=NULL;
	fp=NULL;
	row_pointers=NULL;
	lpdib=NULL;

	// make sure errmsg is not NULL even if user doesn't want it
	if(p2dp->errmsg)
		errmsg=p2dp->errmsg;
	else
		errmsg=dummy_errmsg;
	errmsg[0]='\0';

	if(p2dp->flags&PNGD_USE_CUSTOM_BG) {
		bkgd_color.red=   p2dp->bgcolor.red;
		bkgd_color.green= p2dp->bgcolor.green;
		bkgd_color.blue=  p2dp->bgcolor.blue;
	}
	else {
		bkgd_color.red=   255; // Should never get used. If the
		bkgd_color.green= 128; // background turns orange, it's a bug.
		bkgd_color.blue=  0;
	}

	// Set the user-defined pointer to point to our jmp_buf. This will
	// hopefully protect against potentially different sized jmp_buf's in
	// libpng, while still allowing this library to be threadsafe.
	errinfo.jbufp = &jbuf;
	errinfo.errmsg = errmsg;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,(void*)(&errinfo),
		my_error_fn, my_warning_fn);
	if(!png_ptr) { rv=PNGD_E_NOMEM; goto abort; }


	info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr) {
		//png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		rv=PNGD_E_NOMEM; goto abort;
	}

	if(setjmp(jbuf)) {
		// we'll get here if an error occurred in any of the following
		// png_ functions

		rv=PNGD_E_LIBPNG;
		goto abort;
	}

	if((fp = fopen(p2dp->pngfn, "rb")) == NULL) {
		rv=PNGD_E_READ;
		goto abort;
	}

	png_init_io(png_ptr, fp);

	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &png_bit_depth, &color_type,
		&interlace_type, NULL, NULL);

	if(imginfo) {
		// return some info to the caller
		p2dp->color_type=color_type;
		p2dp->bits_per_sample=png_bit_depth;
		p2dp->interlace=interlace_type;
		switch(color_type) {
		case PNG_COLOR_TYPE_RGB:        p2dp->bits_per_pixel=png_bit_depth*3; break;
		case PNG_COLOR_TYPE_RGB_ALPHA:  p2dp->bits_per_pixel=png_bit_depth*4; break;
		case PNG_COLOR_TYPE_GRAY_ALPHA: p2dp->bits_per_pixel=png_bit_depth*2; break;
		default: p2dp->bits_per_pixel=png_bit_depth;
		}
	}

	is_grayscale = !(color_type&PNG_COLOR_MASK_COLOR);
	has_alpha_channel = (color_type&PNG_COLOR_MASK_ALPHA)?1:0;

	has_trns = png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS);


	// look for bKGD chunk, and process if applicable
	if(p2dp->flags & PNGD_USE_BKGD) {
		if(png_get_bKGD(png_ptr, info_ptr, &bg_colorp)) {
			// process the background, store 8-bit RGB in bkgd_color
			has_bkgd=1;

			if(is_grayscale && png_bit_depth<8) {
				bkgd_color.red  =
				bkgd_color.green=
				bkgd_color.blue =
					(unsigned char) ( (bg_colorp->gray*255)/( (1<<png_bit_depth)-1 ) );
			}
			else if(png_bit_depth<=8) {
				bkgd_color.red=(unsigned char)(bg_colorp->red);
				bkgd_color.green=(unsigned char)(bg_colorp->green);
				bkgd_color.blue =(unsigned char)(bg_colorp->blue);
			}
			else {
				bkgd_color.red=(unsigned char)(bg_colorp->red>>8);
				bkgd_color.green=(unsigned char)(bg_colorp->green>>8);
				bkgd_color.blue =(unsigned char)(bg_colorp->blue>>8);
			}
		}
	}

	if( !(color_type & PNG_COLOR_MASK_ALPHA) && !has_trns) {
		// If no transparency, we can skip this whole background-color mess.
		goto notrans;
	}

	if(has_bkgd && (png_bit_depth>8 || !is_grayscale || has_alpha_channel)) {
		png_set_background(png_ptr, bg_colorp,
			   PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	}
	else if(is_grayscale && has_trns && png_bit_depth<=8
		&& (has_bkgd || (p2dp->flags & PNGD_USE_CUSTOM_BG)) )
	{
		// grayscale binarytrans,<=8bpp: transparency is handle manually
		// by modifying a palette entry (later)
		png_get_tRNS(png_ptr,info_ptr,&trns_trans, &i, &temp_colorp);
		if(i>=1) {
			trns_color= temp_colorp->gray; // corresponds to a palette entry
			manual_trns=1;
		}
	}
	else if(!has_bkgd && (has_trns || has_alpha_channel) &&
		(p2dp->flags & PNGD_USE_CUSTOM_BG) )
	{      // process most CUSTOM background colors
		bkgd.index = 0; // unused
		bkgd.red   = p2dp->bgcolor.red;
		bkgd.green = p2dp->bgcolor.green;
		bkgd.blue  = p2dp->bgcolor.blue;

		// libpng may use bkgd.gray if bkgd.red==bkgd.green==bkgd.blue.
		// Not sure if that's a libpng bug or not.
		bkgd.gray  = p2dp->bgcolor.red;

		if(png_bit_depth>8) {
			bkgd.red  = (bkgd.red  <<8)|bkgd.red;
			bkgd.green= (bkgd.green<<8)|bkgd.green;
			bkgd.blue = (bkgd.blue <<8)|bkgd.blue;
			bkgd.gray = (bkgd.gray <<8)|bkgd.gray;
		}

		if(is_grayscale) {
			/* assert(png_bit_depth>8); */

			/* Need to expand to full RGB if unless background is pure gray */
			if(bkgd.red!=bkgd.green || bkgd.red!=bkgd.blue) {
				png_set_gray_to_rgb(png_ptr);

				// png_set_tRNS_to_alpha() is called here because otherwise
				// binary transparency for 16-bps grayscale images doesn't
				// work. Libpng will think black pixels are transparent.
				// I don't know exactly why it works. It does *not* add an
				// alpha channel, as you might think (adding an alpha
				// channnel makes no sense if you are using
				// png_set_background).
				//
				// Here's an alternate hack that also seems to work, but
				// uses direct structure access:
				//
				// png_ptr->trans_values.red   =
				//  png_ptr->trans_values.green =
				//	png_ptr->trans_values.blue  = png_ptr->trans_values.gray;
				if(has_trns)
					png_set_tRNS_to_alpha(png_ptr);

				png_set_background(png_ptr, &bkgd,
					  PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

			}
			else {  // gray custom background
				png_set_background(png_ptr, &bkgd,
					  PNG_BACKGROUND_GAMMA_SCREEN, 1, 1.0);
			}

		}
		else {
			png_set_background(png_ptr, &bkgd,
				  PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
		}
	}

notrans:

	// If we don't have any background color at all that we can use,
	// strip the alpha channel.
	if(has_alpha_channel && !has_bkgd &&
		!(p2dp->flags & PNGD_USE_CUSTOM_BG) )
	{
		png_set_strip_alpha(png_ptr);
	}

	if(png_bit_depth>8)
		png_set_strip_16(png_ptr);

	if (png_get_sRGB(png_ptr, info_ptr, &i)) {
		has_gama=1;
		file_gamma = 0.45455;
	}
	else if(png_get_gAMA(png_ptr, info_ptr, &file_gamma)) {
		has_gama=1;
	}
	else {
		has_gama=0;
		file_gamma = 0.45455;
	}

	if(imginfo && has_gama) {
		p2dp->file_gamma=file_gamma;
		p2dp->flags |= PNGD_GAMMA_RETURNED;
	}

	manual_gamma=0;
	if(p2dp->flags&PNGD_GAMMA_CORRECTION) {

		if(!is_grayscale || png_bit_depth>8 || has_alpha_channel) {
			png_set_gamma(png_ptr, screen_gamma, file_gamma);
			//png_ptr->transformations |= 0x2000; // hack for old libpng versions
		}
		else manual_gamma=1;

		if(has_bkgd) {
			// Gamma correct the background color (if we got it from the file)
			// before returning it to the app.
			gamma_correct(screen_gamma,file_gamma,&bkgd_color.red,&bkgd_color.green,&bkgd_color.blue);
		}
	}

	png_read_update_info(png_ptr, info_ptr);

	// color type may have changed, due to our transformations
	color_type = png_get_color_type(png_ptr,info_ptr);


	switch(color_type) {
	case PNG_COLOR_TYPE_RGB:
	case PNG_COLOR_TYPE_RGB_ALPHA:
		dib_bpp=24;
		palette_entries=0;
		png_set_bgr(png_ptr);
		break;
	case PNG_COLOR_TYPE_PALETTE:
		dib_bpp=png_bit_depth;
		png_get_PLTE(png_ptr,info_ptr,&png_palette,&palette_entries);
		break;
	case PNG_COLOR_TYPE_GRAY:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		dib_bpp=png_bit_depth;
		if(png_bit_depth>8) dib_bpp=8;
		palette_entries= 1<<dib_bpp;
		// we'll construct a grayscale palette later
		break;
	default:
		rv=PNGD_E_BADPNG;
		goto abort;
	}

	if(dib_bpp==2) dib_bpp=4;

	// DIBs *do* support physical resolution
	has_phys=png_get_valid(png_ptr,info_ptr,PNG_INFO_pHYs);
	if(has_phys) {
		png_get_pHYs(png_ptr,info_ptr,&res_x,&res_y,&res_unit_type);
		if(imginfo && (res_x>0) && (res_y>0)) {
			p2dp->res_x=res_x;
			p2dp->res_y=res_y;
			p2dp->res_units=res_unit_type;
			p2dp->flags |= PNGD_RES_RETURNED;
		}
	}


	// DIB scanlines are padded to 4-byte boundaries.
	dib_bytesperrow= (((width * dib_bpp)+31)/32)*4;

	p2dp->dibsize=sizeof(BITMAPINFOHEADER)+4*palette_entries+height*dib_bytesperrow;
	if(use_heapalloc)
		lpdib = (unsigned char*)HeapAlloc(heap,HEAP_ZERO_MEMORY,p2dp->dibsize);
	else
		lpdib = (unsigned char*)GlobalAlloc(GPTR,p2dp->dibsize);

	if(!lpdib) { rv=PNGD_E_NOMEM; goto abort; }
	p2dp->lpdib = (LPBITMAPINFOHEADER)lpdib;

	row_pointers=(unsigned char**)malloc(height*sizeof(unsigned char*));
	if(!row_pointers) { rv=PNGD_E_NOMEM; goto abort; }

	// yes, there is some redundancy here
	p2dp->palette_offs=sizeof(BITMAPINFOHEADER);
	p2dp->bits_offs   =sizeof(BITMAPINFOHEADER) + 4*palette_entries;
	dib_palette= &lpdib[p2dp->palette_offs];
	p2dp->palette= (RGBQUAD*)dib_palette;
	dib_bits   = &lpdib[p2dp->bits_offs];
	p2dp->lpbits = (VOID*)dib_bits;
	p2dp->palette_colors = palette_entries;

	// set up the DIB palette, if needed
	switch(color_type) {
	case PNG_COLOR_TYPE_PALETTE:
		for(i=0;i<palette_entries;i++) {
			p2dp->palette[i].rgbRed   = png_palette[i].red;
			p2dp->palette[i].rgbGreen = png_palette[i].green;
			p2dp->palette[i].rgbBlue  = png_palette[i].blue;
		}
		break;
	case PNG_COLOR_TYPE_GRAY:
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		for(i=0;i<palette_entries;i++) {
			p2dp->palette[i].rgbRed   =
			p2dp->palette[i].rgbGreen =
			p2dp->palette[i].rgbBlue  = (i*255)/(palette_entries-1);
			//if(dib_bpp<=8) {
			//if(png_bit_depth<16) {
			if(manual_gamma) {
				gamma_correct(screen_gamma,file_gamma,
					  &(p2dp->palette[i].rgbRed),
					  &(p2dp->palette[i].rgbGreen),
					  &(p2dp->palette[i].rgbBlue));
			}
		}
		if(manual_trns) {
			p2dp->palette[trns_color].rgbRed   = bkgd_color.red;
			p2dp->palette[trns_color].rgbGreen = bkgd_color.green;
			p2dp->palette[trns_color].rgbBlue  = bkgd_color.blue;
		}
		break;
	}

	for(j=0;j<(int)height;j++) {
		row_pointers[height-1-j]= &dib_bits[j*dib_bytesperrow];
	}

	png_read_image(png_ptr, row_pointers);

	// special handling for this bit depth, since it doesn't exist in DIBs
	// expand 2bpp to 4bpp
	if(png_bit_depth==2) {
		tmprow = (unsigned char*)malloc((width+3)/4 );
		if(!tmprow) { rv=PNGD_E_NOMEM; goto abort; }

		for(j=0;j<(int)height;j++) {
			CopyMemory(tmprow, row_pointers[j], (width+3)/4 );
			ZeroMemory(row_pointers[j], (width+1)/2 );

			for(i=0;i<(int)width;i++) {
				row_pointers[j][i/2] |=
					( ((tmprow[i/4] >> (2*(3-i%4)) ) & 0x03)<< (4*(1-i%2)) );
			}
		}
		free((void*)tmprow);
	}

	free((void*)row_pointers);
	row_pointers=NULL;

	png_read_end(png_ptr, info_ptr);

	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	png_ptr=NULL;

	fclose(fp);
	fp=NULL;

	// fill in the DIB header fields
	p2dp->lpdib->biSize=          sizeof(BITMAPINFOHEADER);
	p2dp->lpdib->biWidth=         width;
	p2dp->lpdib->biHeight=        height;
	p2dp->lpdib->biPlanes=        1;
	p2dp->lpdib->biBitCount=      dib_bpp;
	p2dp->lpdib->biCompression=   BI_RGB;  // uncompressed
	// biSizeImage can also be 0 in uncompressed bitmaps
	p2dp->lpdib->biSizeImage=     height*dib_bytesperrow;

	if(has_phys) {
		if(res_unit_type==1) {
			p2dp->lpdib->biXPelsPerMeter= res_x;
			p2dp->lpdib->biYPelsPerMeter= res_y;
		}
	}
	p2dp->lpdib->biClrUsed=       palette_entries;
	p2dp->lpdib->biClrImportant=  0;

	if(has_bkgd || (p2dp->flags&PNGD_USE_CUSTOM_BG)) {
		// return the background color if one was used
		p2dp->bgcolor.red   = bkgd_color.red;
		p2dp->bgcolor.green = bkgd_color.green;
		p2dp->bgcolor.blue  = bkgd_color.blue;
		p2dp->flags |= PNGD_BG_RETURNED;
	}

	return PNGD_E_SUCCESS;

abort:

	if(png_ptr) png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	if(fp) fclose(fp);
	if(row_pointers) free((void*)row_pointers);
	if(lpdib) {
		if(use_heapalloc)
			HeapFree(heap,0,(LPVOID)lpdib);
		else
			GlobalFree((HGLOBAL)lpdib);
	}

	// If we don't have an error message yet, use a
	// default one based on the code
	if(!strlen(errmsg)) {
		pngd_get_error_message(rv,errmsg);
	}

	return rv;
}


//--------------------------------------------------------------------

int write_dib_to_png(PNGD_D2PINFO *d2pp)
{
	png_structp png_ptr;
	png_infop info_ptr;
	jmp_buf jbuf;
	struct errstruct errinfo;
	png_text text_ptr[1];
	unsigned char *bits;
	unsigned char *newimage;
	RGBQUAD* dib_palette;
	int headersize, dib_bpp;
	png_uint_32 res_x, res_y;
	png_color_8 pngc8;
	int height,width;
	int palette_entries;
	int topdown;
	int dib_bytesperrow;
	int compression;
	FILE *fp;
	png_color png_palette[256];
	unsigned char **row_pointers;
	int i,x,y,size;
	DWORD *bitfields;
	unsigned int v;
	int bf_format;   // bitfields format identifier
	int iscompressed;
	int bfsize; // bytes in "bitfields" section
	int palsize;  // bytes in palette

	int palentsize;  // bytes in a palette entry: 3 or 4;
	BITMAPCOREHEADER *lpolddib;
	int rv;  // return code
	char *errmsg;
	char dummy_errmsg[MAX_ERRMSGLEN];

	if(d2pp->structsize != sizeof(PNGD_D2PINFO)) return PNGD_E_VERSION;

	rv=PNGD_E_ERROR;  // this should always get changed before returning
	png_ptr=NULL;
	info_ptr=NULL;
	fp=NULL;
	row_pointers=NULL;
	newimage=NULL;

	if(d2pp->errmsg)
		errmsg=d2pp->errmsg;
	else
		errmsg=dummy_errmsg;
	errmsg[0]='\0';

	headersize= d2pp->lpdib->biSize;

	if(headersize<40 && headersize!=12) {
		sprintf(errmsg,"Unexpected BMP header size (%d)",headersize);
		rv=PNGD_E_BADBMP; goto abort;
	}

	if(headersize==12) {
		// This is to support an old kind of DIBs (OS/2) that aren't really
		// used anymore.
		palentsize= 3;
		lpolddib= (BITMAPCOREHEADER*) d2pp->lpdib;
		width= lpolddib->bcWidth;
		height= lpolddib->bcHeight;
		dib_bpp= lpolddib->bcBitCount;
		compression = BI_RGB;
		res_x = res_y = 0;

		// This will get adjusted later if there is a palette.
		// Not sure it's right, though. Do old DIBs always have a
		// full-sized palette?
		palette_entries=0;
	}
	else {
		palentsize=4;
		width= d2pp->lpdib->biWidth;
		height= d2pp->lpdib->biHeight;
		dib_bpp= d2pp->lpdib->biBitCount;
		compression= d2pp->lpdib->biCompression;
		res_x = d2pp->lpdib->biXPelsPerMeter;
		res_y = d2pp->lpdib->biYPelsPerMeter;
		palette_entries = d2pp->lpdib->biClrUsed;
	}

	// supposedly, if the height is negative, the top scanline is stored first
	topdown=0;
	if(height<0) {
		height= -height;
		topdown=1;
	}

	// sanity check
	if(height<1 || height>1000000 || width<1 || width>1000000) {
		sprintf(errmsg,"Unreasonable image dimensions (%dx%d)",width,height);
		rv=PNGD_E_BADBMP; goto abort;
	}

	// only certain combinations of compression and bpp are allowed
	switch(compression) {
	case BI_RGB:
		if(dib_bpp!=1 && dib_bpp!=4 && dib_bpp!=8 && dib_bpp!=16
			&& dib_bpp!=24 && dib_bpp!=32)
		{
			sprintf(errmsg,"Unsupported bit depth (%d)",dib_bpp);
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	case BI_RLE4:
		if(dib_bpp!=4) {
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	case BI_RLE8:
		if(dib_bpp!=8) {
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	case BI_BITFIELDS:
		if(dib_bpp!=16 && dib_bpp!=32) {
			rv=PNGD_E_UNSUPP; goto abort;
		}
		break;
	default:
		sprintf(errmsg,"Unsupported compression scheme");
		return PNGD_E_UNSUPP;
	}

	iscompressed= (compression==BI_RLE4 || compression==BI_RLE8);

	// uncompressed dibs are padded to 4-byte bondaries
	dib_bytesperrow= (((width * dib_bpp)+31)/32)*4;

	if(dib_bpp<16) {
		if(palette_entries==0) palette_entries = 1<<dib_bpp;
	}

	//if(dib_bpp>8) {
	//	palette_entries=0;
	//}
	//else {
	//	// 0 means it has a full palette
	//	if(palette_entries==0) palette_entries = 1<<dib_bpp;
	//}

	bfsize = (compression==BI_BITFIELDS)?12:0;

	//if(compression==BI_BITFIELDS) palsize=12;
	//else palsize=palentsize*palette_entries;
	palsize=palentsize*palette_entries;

	// bounds check
	size= headersize + bfsize + palsize;
	if(d2pp->dibsize) {
		if(size>d2pp->dibsize) {
			rv=PNGD_E_BADBMP; goto abort;
		}
	}

	if(d2pp->lpbits) {
		if(d2pp->bitssize && !iscompressed) {   // bounds check
			size=dib_bytesperrow*height;
			if(size>d2pp->bitssize) { rv=PNGD_E_BADBMP; goto abort; }
		}

		bits=(unsigned char*)d2pp->lpbits;
	}
	else {
		// If not provided by user, assume the bits immediately
		// follow the palette.

		if(d2pp->dibsize && !iscompressed) {  // bounds check
			size= headersize+bfsize+palsize+dib_bytesperrow*height;
			if(size>d2pp->dibsize) { rv=PNGD_E_BADBMP; goto abort; }
		}

		bits= &((unsigned char*)(d2pp->lpdib))[headersize+bfsize+palsize];
	}

	bitfields   =  (DWORD*)  ( &((unsigned char*)(d2pp->lpdib))[headersize] );
	dib_palette = (RGBQUAD*) ( &((unsigned char*)(d2pp->lpdib))[headersize+bfsize] );

	//bitfields=(DWORD*)dib_palette;

	bf_format=0;
	if(compression==BI_BITFIELDS) {
		if(dib_bpp==16) {
			if     (bitfields[0]==0x00007c00 && bitfields[1]==0x000003e0 &&
			        bitfields[2]==0x0000001f) bf_format=11;  // 555
			else if(bitfields[0]==0x0000f800 && bitfields[1]==0x000007e0 &&
			        bitfields[2]==0x0000001f) bf_format=12;  // 565
			else { rv=PNGD_E_UNSUPP; goto abort; }
		}
		if(dib_bpp==32) {
			if     (bitfields[0]==0x00ff0000 && bitfields[1]==0x0000ff00 &&
			        bitfields[2]==0x000000ff) bf_format=21;
			else { rv=PNGD_E_UNSUPP; goto abort; }
		}
	}

	if(bf_format==0 && dib_bpp==16) bf_format=10;
	if(bf_format==0 && dib_bpp==32) bf_format=20;


	// Done analyzing the DIB, now time to convert it to PNG


	// jbuf: see comments in read_png_to_dib()
	errinfo.jbufp = &jbuf;
	errinfo.errmsg = errmsg;
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (void*)(&errinfo),
	    my_error_fn, my_warning_fn);
	if (!png_ptr) { rv=PNGD_E_NOMEM; goto abort; }


	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		//png_destroy_write_struct(&png_ptr,(png_infopp)NULL);
		rv=PNGD_E_NOMEM; goto abort;
	}

	if(setjmp(jbuf)) {
		// we'll get here if an error occurred in any of the following
		// png_ functions
		rv=PNGD_E_LIBPNG;
		goto abort;
	}

	fp=fopen(d2pp->pngfn,"wb");
	if(!fp) {
		rv=PNGD_E_WRITE;
		goto abort;
	}

	png_init_io(png_ptr, fp);

	png_set_IHDR(png_ptr, info_ptr, width, height, (dib_bpp>8)?8:dib_bpp,
		(dib_bpp>8)?PNG_COLOR_TYPE_RGB:PNG_COLOR_TYPE_PALETTE,
		(d2pp->flags&PNGD_INTERLACED)?PNG_INTERLACE_ADAM7:PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	// write sRGB and gAMA chunks
	if(!(d2pp->flags&PNGD_NO_GAMMA_LABEL)) {
		png_set_sRGB(png_ptr, info_ptr, PNG_sRGB_INTENT_RELATIVE);
		png_set_gAMA(png_ptr, info_ptr, 0.45455);
	}

	// For 16-bit DIBs, we get to write an sBIT chunk.
	if(bf_format==10 || bf_format==11) {
		pngc8.red= 5;  pngc8.green= 5;  pngc8.blue= 5;
		png_set_sBIT(png_ptr, info_ptr, &pngc8);
	}
	if(bf_format==12) {
		pngc8.red= 5;  pngc8.green= 6;  pngc8.blue= 5;
		png_set_sBIT(png_ptr, info_ptr, &pngc8);
	}

	// pHYs
	if(res_x>0 && res_y>0)
		png_set_pHYs(png_ptr, info_ptr, res_x, res_y, 1);


	if(palette_entries>0) {
		for(i=0;i<palette_entries;i++) {
			if(palentsize==3) {
				png_palette[i].red   = ((RGBTRIPLE*)dib_palette)[i].rgbtRed;
				png_palette[i].green = ((RGBTRIPLE*)dib_palette)[i].rgbtGreen;
				png_palette[i].blue  = ((RGBTRIPLE*)dib_palette)[i].rgbtBlue;
			}
			else {
				png_palette[i].red   = dib_palette[i].rgbRed;
				png_palette[i].green = dib_palette[i].rgbGreen;
				png_palette[i].blue  = dib_palette[i].rgbBlue;
			}
		}
		png_set_PLTE(png_ptr, info_ptr, png_palette, palette_entries);
	}

	if(dib_bpp>8)
		png_set_bgr(png_ptr);

	png_write_info(png_ptr, info_ptr);

	row_pointers=(unsigned char**)malloc(height*sizeof(unsigned char*));
	if(!row_pointers) { rv=PNGD_E_NOMEM; goto abort; }


	if(dib_bpp==16 || dib_bpp==32) {

		// Special handling for these bit depths.
		// This uses a lot of memory, and could be improved by processing
		// one line at a time (but that makes it tricky to write interlaced
		// images).

		newimage=(unsigned char*)malloc(height*width*3);
		if(!newimage) { rv=PNGD_E_NOMEM; goto abort; }

		for(y=0;y<height;y++) {
			for(x=0;x<width;x++) {
				switch(bf_format) {
				case 10:  case 11:  // 16-bit, format 555 (xRRRRRGG GGGBBBBB)
					v= bits[y*dib_bytesperrow+x*2+0] | (bits[y*dib_bytesperrow+x*2+1]<<8);
					newimage[(y*width+x)*3+0]= (v & 0x0000001f)<<3 | (v & 0x0000001f)>>2;  // blue
					newimage[(y*width+x)*3+1]= (v & 0x000003e0)>>2 | (v & 0x000003e0)>>7;  // green
					newimage[(y*width+x)*3+2]= (v & 0x00007c00)>>7 | (v & 0x00007c00)>>12; // red
					break;
				case 12:            // 16-bit, format 565 (RRRRRGGG GGGBBBBB)
					v= bits[y*dib_bytesperrow+x*2+0] | (bits[y*dib_bytesperrow+x*2+1]<<8);
					newimage[(y*width+x)*3+0]= (v & 0x0000001f)<<3 | (v & 0x0000001f)>>2;  // blue
					newimage[(y*width+x)*3+1]= (v & 0x000007e0)>>3 | (v & 0x000007e0)>>9;  // green
					newimage[(y*width+x)*3+2]= (v & 0x0000f800)>>8 | (v & 0x0000f800)>>13; // red
					break;
				case 20:  case 21:  // 32-bit, every 4th byte wasted (b g r x)
					newimage[(y*width+x)*3+0]= bits[y*dib_bytesperrow+x*4+0]; // blue
					newimage[(y*width+x)*3+1]= bits[y*dib_bytesperrow+x*4+1]; // green
					newimage[(y*width+x)*3+2]= bits[y*dib_bytesperrow+x*4+2]; // red
					break;
				}
			}
		}

		for(i=0;i<height;i++) {
			if(topdown)
				row_pointers[i]= &newimage[i*width*3];
			else
				row_pointers[height-1-i]= &newimage[i*width*3];
		}
		png_write_image(png_ptr, row_pointers);

		GlobalFree(newimage);
		newimage=NULL;
	}
	else if(iscompressed) {
		newimage= uncompress_dib((LPBITMAPINFO)d2pp->lpdib, headersize+bfsize+palsize, bits);
		if(!newimage) { rv=PNGD_E_NOMEM; goto abort; }
		for(i=0;i<height;i++) {
			if(topdown)
				row_pointers[i]= &newimage[i*dib_bytesperrow];
			else
				row_pointers[height-1-i]= &newimage[i*dib_bytesperrow];
		}
		png_write_image(png_ptr, row_pointers);

		GlobalFree(newimage);
		newimage=NULL;
	}
	else {
		for(i=0;i<height;i++) {
			if(topdown)
				row_pointers[i]= &bits[i*dib_bytesperrow];
			else
				row_pointers[height-1-i]= &bits[i*dib_bytesperrow];
		}
		png_write_image(png_ptr, row_pointers);
	}

	GlobalFree((VOID*)row_pointers);
	row_pointers=NULL;

	if(d2pp->software) {
		text_ptr[0].key = "Software";
		text_ptr[0].text = d2pp->software;
		text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
		png_set_text(png_ptr, info_ptr, text_ptr, 1);
	}

	png_write_end(png_ptr, info_ptr);

	rv=PNGD_E_SUCCESS;

abort:
	if(png_ptr) png_destroy_write_struct(&png_ptr, &info_ptr);
	if(fp) fclose(fp);
	if(row_pointers) GlobalFree(row_pointers);
	if(newimage) GlobalFree(newimage);

	// If we don't have an error message yet, use a
	// default one based on the code
	if(!strlen(errmsg)) {
		pngd_get_error_message(rv,errmsg);
	}
	return rv;
}

char* pngdib_get_version_string(void)
{
	return PNGDIB_SRC_VERSION_STRING;
}

int pngdib_get_version(void)
{
	return PNGDIB_SRC_VERSION;
}
