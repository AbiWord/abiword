// pngdib.h

#ifndef __PNGDIB_H__
#define __PNGDIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PNGDIB_HEADER_VERSION          20200
#define PNGDIB_HEADER_VERSION_STRING   "2.2.0"


// error codes returned by read_png_to_dib() and/or
//   write_dib_to_png()

#define PNGD_E_SUCCESS   0
#define PNGD_E_ERROR     1   // unspecified error
#define PNGD_E_VERSION   2   // struct size problem
#define PNGD_E_NOMEM     3   // could not alloc memory
#define PNGD_E_UNSUPP    4   // unsupported image type
#define PNGD_E_LIBPNG    5   // libpng error (corrupt PNG?)
#define PNGD_E_BADBMP    6   // corrupt or unsupported DIB
#define PNGD_E_BADPNG    7   // corrupt or unsupported PNG
#define PNGD_E_READ      8   // couldn't read PNG file
#define PNGD_E_WRITE     9   // couldn't write PNG file

struct PNGD_COLOR_struct {
	unsigned char red, green, blue, reserved;
};

typedef struct PNGD_D2PINFO_struct {
	DWORD           structsize;      // sizeof(PNGD_D2PINFO)
	DWORD           flags;
#define PNGD_INTERLACED        0x00000001
#define PNGD_NO_GAMMA_LABEL    0x00000002

	const char*     pngfn;          // PNG filename to write

	LPBITMAPINFOHEADER    lpdib;
	int             dibsize;        // can be 0

	VOID*           lpbits;         // can be NULL
	int             bitssize;       // can be 0

	char*           software;       // (NULL==don't include)
// added in v2.0
	char*           errmsg;          // user can set to null or 100-char buffer
} PNGD_D2PINFO;


typedef struct PNGD_IMAGEINFO_struct {
	DWORD           structsize;    // sizeof(PNGD_IMAGEINFO)
	DWORD           flags;

} PNGD_IMAGEINFO;

typedef struct PNGD_P2DINFO_struct {
	DWORD           structsize;      // sizeof(PNGD_P2DINFO)

	DWORD           flags;           // combination of below:
#define PNGD_USE_BKGD          0x00000001
#define PNGD_USE_CUSTOM_BG     0x00000002
#define PNGD_GAMMA_CORRECTION  0x00000004
#define PNGD_USE_HEAPALLOC     0x00000008

#define PNGD_BG_RETURNED     0x00010000 // return value only
#define PNGD_RES_RETURNED    0x00020000 // set if xres,yres,res_units are valid
#define PNGD_GAMMA_RETURNED  0x00040000 // set if file_gamma is valid

	const char*     pngfn;           // PNG filename to read

	LPBITMAPINFOHEADER    lpdib;     // return value only
	int             dibsize;         // return value only
	int             palette_offs;    // return value only
	int             bits_offs;       // return value only
	RGBQUAD*        palette;         // return value only
	int             palette_colors;  // return value only
	VOID*           lpbits;          // return value only
// added in v2.0  (size=48)
	struct PNGD_COLOR_struct bgcolor; // IN OUT
	char*           errmsg;          // user can set to null or 100-char buffer
// added in v2.1  (size=88)
	int             color_type;
	int             bits_per_sample;
	int             bits_per_pixel;
	int             interlace;
	int             res_x,res_y;
	int             res_units;
	int             reserved1;
	double          file_gamma;
// added in v2.2   (size=96)
	HANDLE          heap;
	int             reserved2;

} PNGD_P2DINFO;



int read_png_to_dib(PNGD_P2DINFO *p2dinfo);

int write_dib_to_png(PNGD_D2PINFO *d2pinfo);

char* pngdib_get_version_string(void);
int pngdib_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* __PNGDIB_H__ */
