/* AbiSource
 *
 * Copyright (C) 2012 Tanya Guza <tanya.guza@gmail.com>
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



#ifndef _ODE_THUMBNAILSWRITER_H_
#define _ODE_THUMBNAILSWRITER_H_

#include <gsf/gsf.h>

class PD_Document;

class ODe_ThumbnailsWriter
{
public:

    static bool writeThumbnails(PD_Document* pDoc, GsfOutfile* oo);

private:

    ODe_ThumbnailsWriter ();
};

#endif //_ODE_THUMBNAILSWRITER_H_
