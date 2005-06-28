/* LibAbi
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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


#include <glib-object.h>

#include "pd_Document.h"

#include "abi-doc.h"


enum {
	_PROP_0,
	NUM_PROPS
};


static void abi_doc_class_init   (AbiDocClass  *klass);
static void abi_doc_init         (AbiDoc       *self);
static void abi_doc_set_property (GObject      *obj,
								  guint		    param_id,
								  const GValue *val,
								  GParamSpec   *pspec);
static void abi_doc_get_property (GObject      *obj,
								  guint         param_id,
								  GValue       *val,
								  GParamSpec   *pspec);
static void abi_doc_dispose 	 (GObject 	   *obj);
static void abi_doc_finalize 	 (GObject 	   *obj);


typedef struct _AbiDocPrivate AbiDocPrivate;


struct _AbiDoc {
	GObject obj;
	AbiDocPrivate *priv;
};

struct _AbiDocPrivate {
	bool is_disposed;
};

struct _AbiDocClass {
	GObjectClass parent_class;
};


static GObjectClass *abi_doc_parent_class = NULL;


GType
abi_doc_get_type (void)
{
	static GType doc_type = 0;

	if (!doc_type) {

		static const GTypeInfo doc_info = {
			sizeof (AbiDocClass),
			NULL,		/* base_init */
			NULL,		/* base_finalize */
			(GClassInitFunc) abi_doc_class_init,
			NULL,		/* class_finalize */
			NULL,		/* class_data */
			sizeof (AbiDoc),
			0,		/* n_preallocs */
			(GInstanceInitFunc) abi_doc_init,
		};

		doc_type = g_type_register_static (G_TYPE_OBJECT, "AbiDoc", &doc_info, (GTypeFlags)0);
	}

	return doc_type;
}

static void 
abi_doc_class_init (AbiDocClass *klass)
{
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass*) klass;

	abi_doc_parent_class = (GObjectClass*) g_type_class_peek_parent (klass);

	gobject_class->set_property = abi_doc_set_property;
	gobject_class->get_property = abi_doc_get_property;
	gobject_class->dispose = abi_doc_dispose;
	gobject_class->finalize = abi_doc_finalize;

/*	
	g_object_class_install_property (gobject_class,
									 PROP_SHOW_RULERS,
									 g_param_spec_string ("show-rulers",
									 P_("show rulers"),	
									 P_("Show or hide rulers"),
									 FALSE,
									 (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
*/
}

static void
abi_doc_init (AbiDoc *self)
{
	self->priv = (AbiDocPrivate *) g_new0 (AbiDocPrivate, 1);
	self->priv->is_disposed = FALSE;
}

static void 
abi_doc_set_property (GObject      *obj,
					   guint         prop_id,
					   const GValue *val,
					   GParamSpec   *pspec)
{
	AbiDoc *self;

	self = ABI_DOC (obj);

/*
	switch (prop_id) {
		case PROP_SHOW_RULERS:
			self->priv->show_rulers = g_value_get_int (val);
		    break;
		default:      
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
		    break;
	}
*/
}

static void 
abi_doc_get_property (GObject    *obj,
					   guint       prop_id,
					   GValue     *val,
					   GParamSpec *pspec)
{
	AbiDoc *self;

	self = ABI_DOC (obj);

/*
	switch (prop_id) {
		case PROP_SHOW_RULERS:
			g_value_set_boolean (val, self->priv->show_rulers);
		    break;
		default:
		    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
		    break;
	}
*/
}

AbiDoc*
abi_doc_new (void)
{
	return ABI_DOC (g_object_new (ABI_TYPE_DOC, NULL));
}

static void 
abi_doc_dispose (GObject *obj)
{
	AbiDoc *self;

	self = ABI_DOC (obj);
	if (self->priv->is_disposed)
		return;

	self->priv->is_disposed = TRUE;

	G_OBJECT_CLASS (abi_doc_parent_class)->dispose (obj);
}

static void 
abi_doc_finalize (GObject *obj)
{
	AbiDoc *self;

	self = ABI_DOC (obj);

	g_free (self->priv);
	self->priv = NULL;

	G_OBJECT_CLASS (abi_doc_parent_class)->finalize (obj);
}
