/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */

/* 
 * Copyright (C) 2000 Eazel, Inc
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au> 
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Maciej Stachowiak <mjs@eazel.com> and Martin Sevior.
 * This is based on the nautilus-sample-content-view.h code of Maciej
 * Stachowiak.
 */

/* This
 * component displays a wordprocessing document in the abiword control.
 */


#ifndef NAUTILUS_ABIWORD_CONTENT_VIEW_H
#define NAUTILUS_ABIWORD_CONTENT_VIEW_H

#include <libnautilus/nautilus-view.h>
#include "xap_UnixDialogHelper.h"

#define NAUTILUS_TYPE_ABIWORD_CONTENT_VIEW	     (nautilus_abiword_content_view_get_type ())
#define NAUTILUS_ABIWORD_CONTENT_VIEW(obj)	     (GTK_CHECK_CAST ((obj), NAUTILUS_TYPE_ABIWORD_CONTENT_VIEW, NautilusAbiWordContentView))
#define NAUTILUS_ABIWORD_CONTENT_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), NAUTILUS_TYPE_ABIWORD_CONTENT_VIEW, NautilusAbiWordContentViewClass))
#define NAUTILUS_IS_ABIWORD_CONTENT_VIEW(obj)	     (GTK_CHECK_TYPE ((obj), NAUTILUS_TYPE_ABIWORD_CONTENT_VIEW))
#define NAUTILUS_IS_ABIWORD_CONTENT_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), NAUTILUS_TYPE_ABIWORD_CONTENT_VIEW))

typedef struct NautilusAbiWordContentViewDetails NautilusAbiWordContentViewDetails;

typedef struct {
	NautilusView parent;
	NautilusAbiWordContentViewDetails *details;
} NautilusAbiWordContentView;

typedef struct {
	NautilusViewClass parent;
} NautilusAbiWordContentViewClass;

GtkType nautilus_abiword_content_view_get_type (void);

#endif /* NAUTILUS_ABIWORD_CONTENT_VIEW_H */






