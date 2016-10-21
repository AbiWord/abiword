/*
 * AbiPaint plugin
 *
 * AbiWord plugin to ease editing embedded images via an
 * External Image Editing program.  The image editing program
 * used and optional image format conversion may be specified,
 * though sensible (platform specific) defaults are defined.
 *
 */

/* The default image editor and exported image type depends
 * on the platform.
 *
 * For Windows:
 *   The PNG images by default are converted to BMP files
 *   and the default image editor is Microsoft's (R) Paint (MSPAINT.EXE)
 *     PNG to BMP conversion done using:
 *       PNGDIB - a mini DIB-PNG conversion library for Win32
 *       By Jason Summers  <jason1@pobox.com>
 *
 * For Unix Systems (and similar)
 *   The images are exported as PNG files
 *   and the default image editor is the GIMP (gimp)
 *   GNU Image Manipulation Program - see http://www.gimp.org/
 */

/*
 * Based on AbiGimp copyright Martin Sevior which in turn
 *   is based on AiksaurusABI - Abiword plugin for Aiksaurus
 *   Copyright (C) 2001 by Jared Davis
 * Also tidbits taken from ImageMagick plugin, copyright 2002
 *   by Dom Lachowicz
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

#ifndef ABIPAINT_H
#define ABIPAINT_H

#include <string>

#include "AbiGeneric.h"

#ifdef _WIN32
#define ENABLE_BMP
const char *szProgramsDesc = "Image Editing Programs (*.exe,*.com)";
const char *szProgramSuffix= "*.exe; *.com";
#else /* UNIX */
const char *szProgramsDesc = "Image Editing Programs (*)";
const char *szProgramSuffix= "*";
#endif

#ifndef MAX_PATH
#define MAX_PATH 2048
#endif

// various helper functions
static void getDefaultApp(std::string &imageApp, bool &bLeaveImageAsPNG);


#ifdef ENABLE_BMP
// TODO: FIX bmp support, as the BMP loader is now an optional plugin
//       so we now convert to BMP and back to PNG (using pngdib)
//       instead of allowing AbiWord's BMP importer to do the conversion
#include "ie_types.h"	// for BMP file type

// #include "pngdib.h"
// returns 0 on success, nonzero of failure
int convertPNG2BMP(const char *pngfn, const char *bmpfn);
int convertBMP2PNG(const char *bmpfn, const char *pngfn);
#endif


#endif /* ABIPAINT_H */
