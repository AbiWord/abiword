/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-image.c: Image formats
 *
 * Copyright (C) 2004, 2005 Jody Goldberg (jody@gnome.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <goffice/goffice-config.h>
#include <goffice/utils/go-color.h>
#include <goffice/utils/go-image.h>
#include <goffice/utils/go-cairo.h>
#include <glib/gi18n-lib.h>
#include <string.h>
#include <gsf/gsf-utils.h>
#include <gsf/gsf-impl-utils.h>
#include <glib/gi18n-lib.h>

static GOImageFormatInfo *pixbuf_image_format_infos = NULL;
static GHashTable *pixbuf_mimes = NULL;
static unsigned pixbuf_format_nbr = 0;
static gboolean pixbuf_format_done = FALSE;

#define PIXBUF_IMAGE_FORMAT_OFFSET (1+GO_IMAGE_FORMAT_UNKNOWN)

static void go_image_build_pixbuf_format_infos (void);

/**
 * go_mime_to_image_format:
 * @mime_type: a mime type string
 *
 * returns: file extension for the given mime type.
 **/

char *
go_mime_to_image_format (char const *mime_type)
{
 	guint i;
	const char* exceptions[] = {
		"image/svg", "svg",
		"image/svg+xml", "svg",
		"image/svg-xml", "svg",
		"image/vnd.adobe.svg+xml", "svg",
		"text/xml-svg", "svg",
		"image/x-wmf", "wmf",
		"image/x-emf", "emf",
		"application/pdf", "pdf",
		"application/postscript", "ps",
		"image/x-eps", "eps",
	};

	for (i = 0; i < G_N_ELEMENTS (exceptions); i += 2)
		if (strcmp (mime_type, exceptions[i]) == 0)
			return g_strdup (exceptions[i + 1]);

	go_image_build_pixbuf_format_infos ();

	return g_strdup (g_hash_table_lookup (pixbuf_mimes, mime_type));
}

/**
 * go_image_format_to_mime:
 * @format: a file extension string
 *
 * returns: corresponding mime type.
 **/

char *
go_image_format_to_mime (char const *format)
{
	char *ret = NULL;
 	guint i;
#ifdef GOFFICE_WITH_GTK
	GSList *ptr, *pixbuf_fmts;
#endif
	static const char* const formats[] = {
		"svg", "image/svg,image/svg+xml",
		"wmf", "image/x-wmf",
		"emf", "image/x-emf",
		"pdf", "application/pdf",
		"ps", "application/postscript",
		"eps", "image/x-eps",
	};

	if (format == NULL)
		return NULL;

	for (i = 0; i < G_N_ELEMENTS (formats); i += 2)
		if (strcmp (format, formats[i]) == 0)
			return g_strdup (formats[i + 1]);

#ifdef GOFFICE_WITH_GTK
	/* Not a format we have special knowledge about - ask gdk-pixbuf */
	pixbuf_fmts = gdk_pixbuf_get_formats ();
	for (ptr = pixbuf_fmts; ptr != NULL; ptr = ptr->next) {
		GdkPixbufFormat *pfmt = ptr->data;
		gchar *name = gdk_pixbuf_format_get_name (pfmt);
		int cmp = strcmp (format, name);
		g_free (name);
		if (cmp == 0) {
			gchar **mimes =
				gdk_pixbuf_format_get_mime_types (pfmt);
			ret = g_strjoinv (",", mimes);
			g_strfreev (mimes);
			break;
		}
	}
	g_slist_free (pixbuf_fmts);
#endif

	return ret;
}

static GOImageFormatInfo const image_format_infos[GO_IMAGE_FORMAT_UNKNOWN] = {
	{GO_IMAGE_FORMAT_SVG, (char *) "svg",  (char *) N_("SVG (vector graphics)"),
	 (char *) "svg", FALSE, FALSE, TRUE},
	{GO_IMAGE_FORMAT_PNG, (char *) "png",  (char *) N_("PNG (raster graphics)"),
	 (char *) "png", TRUE,  TRUE, TRUE},
	{GO_IMAGE_FORMAT_JPG, (char *) "jpeg", (char *) N_("JPEG (photograph)"),
	 (char *) "jpg", TRUE,  TRUE, FALSE},
	{GO_IMAGE_FORMAT_PDF, (char *) "pdf",  (char *) N_("PDF (portable document format)"),
	 (char *) "pdf", FALSE, FALSE, TRUE},
	{GO_IMAGE_FORMAT_PS,  (char *) "ps",   (char *) N_("PS (postscript)"),
	 (char *) "ps",  FALSE, TRUE, TRUE},
	{GO_IMAGE_FORMAT_EMF, (char *) "emf",  (char *) N_("EMF (extended metafile)"),
	 (char *) "emf", FALSE, FALSE, TRUE},
	{GO_IMAGE_FORMAT_WMF, (char *) "wmf",  (char *) N_("WMF (windows metafile)"),
	 (char *) "wmf", FALSE, FALSE, TRUE},
	{GO_IMAGE_FORMAT_EPS,  (char *) "eps",   (char *) N_("EPS (encapsulated postscript)"),
	 (char *) "eps",  FALSE, TRUE, TRUE},
};

static void
go_image_build_pixbuf_format_infos (void)
{
#ifdef GOFFICE_WITH_GTK
	GdkPixbufFormat *fmt;
	GSList *l, *pixbuf_fmts;
	GOImageFormatInfo *format_info;
	gchar **exts, **mimes;
	unsigned i, j;

	if (pixbuf_format_done)
		return;

	pixbuf_fmts = gdk_pixbuf_get_formats ();
	pixbuf_format_nbr = g_slist_length (pixbuf_fmts);

	if (pixbuf_format_nbr > 0) {
		pixbuf_image_format_infos = g_new (GOImageFormatInfo, pixbuf_format_nbr);
		pixbuf_mimes = g_hash_table_new_full
			(g_str_hash, g_str_equal, g_free, g_free);

		for (l = pixbuf_fmts, i = 1, format_info = pixbuf_image_format_infos;
		     l != NULL;
		     l = l->next, i++, format_info++) {
			fmt = (GdkPixbufFormat *)l->data;

			format_info->format = GO_IMAGE_FORMAT_UNKNOWN + i;
			format_info->name = gdk_pixbuf_format_get_name (fmt);
			format_info->desc = gdk_pixbuf_format_get_description (fmt);
			exts = gdk_pixbuf_format_get_extensions (fmt);
			format_info->ext = g_strdup (exts[0]);
			if (format_info->ext == NULL)
				format_info->ext = format_info->name;
			g_strfreev (exts);
			format_info->has_pixbuf_saver = gdk_pixbuf_format_is_writable (fmt);
			format_info->is_dpi_useful = FALSE;
			format_info->alpha_support = FALSE;

			mimes = gdk_pixbuf_format_get_mime_types (fmt);
			for (j = 0; mimes[j]; j++) {
				g_hash_table_insert
					(pixbuf_mimes,
					 g_strdup((gpointer) mimes[j]),
					 g_strdup((gpointer) format_info->name));
			}
			g_strfreev (mimes);
		}
	}

	g_slist_free (pixbuf_fmts);
#endif /* GOFFICE_WITH_GTK */
	pixbuf_format_done = TRUE;
}

/**
 * go_image_get_format_info:
 * @format: a #GOImageFormat
 *
 * Retrieves infromation associated to @format.
 *
 * returns: a #GOImageFormatInfo struct.
 **/

GOImageFormatInfo const *
go_image_get_format_info (GOImageFormat format)
{
	if (format > GO_IMAGE_FORMAT_UNKNOWN)
		go_image_build_pixbuf_format_infos ();

	g_return_val_if_fail (format != GO_IMAGE_FORMAT_UNKNOWN &&
			      format <= GO_IMAGE_FORMAT_UNKNOWN + pixbuf_format_nbr, NULL);
	if (format < GO_IMAGE_FORMAT_UNKNOWN)
		return &image_format_infos[format];

	return &pixbuf_image_format_infos[format - PIXBUF_IMAGE_FORMAT_OFFSET];
}

/**
 * go_image_get_format_from_name:
 * @name: a string
 *
 * returns: corresponding #GOImageFormat.
 **/

GOImageFormat
go_image_get_format_from_name (char const *name)
{
	unsigned i;

	go_image_build_pixbuf_format_infos ();

	for (i = 0; i < GO_IMAGE_FORMAT_UNKNOWN; i++) {
		if (strcmp (name, image_format_infos[i].name) == 0)
			return image_format_infos[i].format;
	}

	for (i = 0; i < pixbuf_format_nbr; i++) {
		if (strcmp (name, pixbuf_image_format_infos[i].name) == 0)
			return pixbuf_image_format_infos[i].format;
	}

	g_warning ("[GOImage::get_format_from_name] Unknown format name (%s)", name);
	return GO_IMAGE_FORMAT_UNKNOWN;
}

/**
 * go_image_get_formats_with_pixbuf_saver:
 *
 * returns: a list of #GOImageFormat that can be created from a pixbuf.
 **/

GSList *
go_image_get_formats_with_pixbuf_saver (void)
{
	GSList *list = NULL;
	unsigned i;

	for (i = 0; i < GO_IMAGE_FORMAT_UNKNOWN; i++)
		if (image_format_infos[i].has_pixbuf_saver)
			list = g_slist_prepend (list, GUINT_TO_POINTER (i));

	/* TODO: before enabling this code, we must remove duplicate in pixbuf_image_format_infos */
#if 0
	go_image_build_pixbuf_format_infos ();

	for (i = 0; i < pixbuf_format_nbr; i++) {
		if (pixbuf_image_format_infos[i].has_pixbuf_saver)
			list = g_slist_prepend (list, GUINT_TO_POINTER (i + PIXBUF_IMAGE_FORMAT_OFFSET));
	}
#endif

	return list;
}

/*********************************
 * GOImage object implementation *
 *********************************/

static GObjectClass *parent_klass;

struct _GOImage {
	GObject parent;
	guint8 *data;
	guint width, height, rowstride;
	gboolean target_cairo;
	cairo_t *cairo;
#ifdef GOFFICE_WITH_GTK
	GdkPixbuf *pixbuf, *thumbnail;
#else
	void *pixbuf;
#endif
	char *name;
};

enum {
	IMAGE_PROP_0,
	IMAGE_PROP_WIDTH,
	IMAGE_PROP_HEIGHT,
#ifdef GOFFICE_WITH_GTK
	IMAGE_PROP_PIXBUF,
#endif
};

static void
pixbuf_to_cairo (GOImage *image)
{
#ifdef GOFFICE_WITH_GTK
	unsigned char *src, *dst;

	g_return_if_fail (GO_IS_IMAGE (image) && image->data && image->pixbuf);

	src = gdk_pixbuf_get_pixels (image->pixbuf);
	dst = image->data;

	g_return_if_fail (gdk_pixbuf_get_rowstride (image->pixbuf) == (int) image->rowstride);

	go_cairo_convert_data_from_pixbuf (dst, src, image->width, image->height, image->rowstride);
#endif
}

#ifdef GOFFICE_WITH_GTK
static void
cairo_to_pixbuf (GOImage *image)
{
	unsigned char *src, *dst;

	g_return_if_fail (GO_IS_IMAGE (image) && image->data && image->pixbuf);

	dst = gdk_pixbuf_get_pixels (image->pixbuf);
	src = image->data;

	g_return_if_fail (gdk_pixbuf_get_rowstride (image->pixbuf) == (int) image->rowstride);

	go_cairo_convert_data_to_pixbuf (dst, src, image->width, image->height, image->rowstride);
}
#endif

static void
go_image_set_property (GObject *obj, guint param_id,
		       GValue const *value, GParamSpec *pspec)
{
	GOImage *image = GO_IMAGE (obj);
	gboolean size_changed = FALSE;
	guint n;

	switch (param_id) {
	case IMAGE_PROP_WIDTH:
		n = g_value_get_uint (value);
		if (n != image->width) {
			image->width = n;
			size_changed = TRUE;
		}
		break;
	case IMAGE_PROP_HEIGHT:
		n = g_value_get_uint (value);
		if (n != image->height) {
			image->height = n;
			size_changed = TRUE;
		}
		break;
#ifdef GOFFICE_WITH_GTK
	case IMAGE_PROP_PIXBUF: {
		GdkPixbuf *pixbuf = GDK_PIXBUF (g_value_get_object (value));
		if (!GDK_IS_PIXBUF (pixbuf))
			break;
		if (!gdk_pixbuf_get_has_alpha (pixbuf))
			pixbuf = gdk_pixbuf_add_alpha (pixbuf, FALSE, 0, 0, 0);
		else
			g_object_ref (pixbuf);
		if (image->pixbuf)
			g_object_unref (image->pixbuf);
		image->pixbuf = pixbuf;
		g_free (image->data);
		image->data = NULL;
		image->width = gdk_pixbuf_get_width (pixbuf);
		image->height = gdk_pixbuf_get_height (pixbuf);
		image->rowstride = gdk_pixbuf_get_rowstride (pixbuf);
		image->target_cairo = FALSE;
		if (image->thumbnail) {
			g_object_unref (image->thumbnail);
			image->thumbnail = NULL;
		}
	}
		break;
#endif

	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, param_id, pspec);
		return; /* NOTE : RETURN */
	}

	if (size_changed) {
		if (image->pixbuf) {
			g_object_unref (image->pixbuf);
			image->pixbuf = NULL;
		}
		g_free (image->data);
		/* GOImage only supports pixbuf with alpha values at the moment */
		image->rowstride = image->width * 4;
		image->data = g_new0 (guint8, image->height * image->rowstride);
		image->target_cairo = TRUE;
	}
}

static void
go_image_get_property (GObject *obj, guint param_id,
		       GValue *value, GParamSpec *pspec)
{
	GOImage *image = GO_IMAGE (obj);

	switch (param_id) {
	case IMAGE_PROP_WIDTH:
		g_value_set_uint (value, image->width);
		break;
	case IMAGE_PROP_HEIGHT:
		g_value_set_uint (value, image->height);
		break;
#ifdef GOFFICE_WITH_GTK
	case IMAGE_PROP_PIXBUF:
		if (image->target_cairo && image->pixbuf) {
			cairo_to_pixbuf (image);
			image->target_cairo = FALSE;
		}
		g_value_set_object (value, image->pixbuf);
		break;
#endif

	default: G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, param_id, pspec);
		return; /* NOTE : RETURN */
	}
}

static void
go_image_finalize (GObject *obj)
{
	GOImage *image = GO_IMAGE (obj);
	g_free (image->data);
#ifdef GOFFICE_WITH_GTK
	if (image->pixbuf)
		g_object_unref (image->pixbuf);
	if (image->thumbnail)
		g_object_unref (image->thumbnail);
#endif
	g_free (image->name);
	(parent_klass->finalize) (obj);
}

typedef GObjectClass GOImageClass;

static void
go_image_class_init (GOImageClass *klass)
{
	klass->finalize = go_image_finalize;
	klass->set_property = go_image_set_property;
	klass->get_property = go_image_get_property;
	parent_klass = g_type_class_peek_parent (klass);
	g_object_class_install_property (klass, IMAGE_PROP_WIDTH,
					 g_param_spec_uint ("width", _("Width"),
							    _("Image width in pixels"),
							    0, G_MAXUINT16, 0, G_PARAM_READWRITE));
	g_object_class_install_property (klass, IMAGE_PROP_HEIGHT,
					 g_param_spec_uint ("height", _("Height"),
							    _("Image height in pixels"),
							    0, G_MAXUINT16, 0, G_PARAM_READWRITE));
#ifdef GOFFICE_WITH_GTK
	g_object_class_install_property (klass, IMAGE_PROP_PIXBUF,
					 g_param_spec_object ("pixbuf", _("Pixbuf"),
							      _("GdkPixbuf object from which the GOImage is built"),
							      GDK_TYPE_PIXBUF, G_PARAM_READWRITE));
#endif
}

GSF_CLASS (GOImage, go_image,
	   go_image_class_init, NULL,
	   G_TYPE_OBJECT)

cairo_t *
go_image_get_cairo (GOImage *image)
{
	cairo_surface_t *surface ;
	cairo_t *cairo;

	g_return_val_if_fail (GO_IS_IMAGE (image), NULL);
	if (image->data == NULL && image->pixbuf == NULL)
		return NULL;
	if (image->data == NULL) {
		/* image built from a pixbuf */
		image->data = g_new0 (guint8, image->height * image->rowstride);
	}
	if (!image->target_cairo) {
		pixbuf_to_cairo (image);
		image->target_cairo = TRUE;
	}
	surface = cairo_image_surface_create_for_data (
						       image->data,
						       CAIRO_FORMAT_ARGB32,
						       image->width, image->height,
						       image->rowstride);
	cairo = cairo_create (surface);
	cairo_surface_destroy (surface);
	image->target_cairo = TRUE;
	return cairo;
}

static void
cb_surface_destroyed (void *data)
{
	GOImage *image = GO_IMAGE (data);
	/* We no longer need image->data.  */
	g_object_unref (image);
}

/**
 * go_image_create_cairo_pattern:
 * @image: a GOImage.
 *
 * returns: a cairo_pattern usable for cairo_set_source.
 *
 * Note: this function has lifespan issues.  The resulting pattern in only
 * valid until (a) a pixbuf is set for the, or (b) a pixbuf is _read_ from
 * the image.  In either of these cases, the pattern must have been
 * destroyed beforehand.  In particular, if the pattern has been attached
 * to a surface, that surface must either be finished itself, or have had
 * a new pattern attached.  See #632439.
 */
cairo_pattern_t *
go_image_create_cairo_pattern (GOImage *image)
{
	cairo_surface_t *surface ;
	cairo_pattern_t *pat;
	static const cairo_user_data_key_t key;

	g_return_val_if_fail (GO_IS_IMAGE (image), NULL);
	if (image->data == NULL && image->pixbuf == NULL)
		return NULL;
	if (image->data == NULL) {
		/* image built from a pixbuf */
		image->data = g_new0 (guint8, image->height * image->rowstride);
	}
	if (!image->target_cairo) {
		pixbuf_to_cairo (image);
		image->target_cairo = TRUE;
	}
	surface = cairo_image_surface_create_for_data
		(image->data,
		 CAIRO_FORMAT_ARGB32,
		 image->width, image->height,
		 image->rowstride);
	g_object_ref (image);
	cairo_surface_set_user_data (surface, &key,
				     image, cb_surface_destroyed);
	pat = cairo_pattern_create_for_surface (surface);
	cairo_surface_destroy (surface);
	return pat;
}

#ifdef GOFFICE_WITH_GTK
GOImage *
go_image_new_from_pixbuf (GdkPixbuf *pixbuf)
{
	return g_object_new (GO_TYPE_IMAGE, "pixbuf", pixbuf, NULL);
}

GdkPixbuf *
go_image_get_pixbuf (GOImage *image)
{
	g_return_val_if_fail (image != NULL, NULL);
	if (!image->pixbuf) {
		if (image->width == 0 || image->height == 0 || image->data == NULL)
			return NULL;
		image->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8,
								image->width, image->height);
	}
	if (image->target_cairo) {
		cairo_to_pixbuf (image);
		image->target_cairo = FALSE;
	}
	return image->pixbuf;
}

#define THUMBNAIL_SIZE 64
GdkPixbuf *
go_image_get_thumbnail (GOImage *image)
{
	g_return_val_if_fail (image != NULL, NULL);
	if (!image->pixbuf)
		go_image_get_pixbuf (image);
	if (!image->pixbuf)
		return NULL;
	if (!image->thumbnail) {
		int w, h;
		if (image->width <= THUMBNAIL_SIZE && image->height <= THUMBNAIL_SIZE)
			return image->pixbuf;
		if (image->width >= image->height) {
			w = THUMBNAIL_SIZE;
			h = THUMBNAIL_SIZE * image->height / image->width;
		} else {
			h = THUMBNAIL_SIZE;
			w = THUMBNAIL_SIZE * image->width / image->height;
		}
		image->thumbnail = gdk_pixbuf_scale_simple (image->pixbuf, w, h, GDK_INTERP_HYPER);
	}
	return image->thumbnail;
}
#endif

GOImage *
go_image_new_from_file (const char *filename, GError **error)
{
#ifdef GOFFICE_WITH_GTK
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, error);
	if (pixbuf) {
		GOImage *image = g_object_new (GO_TYPE_IMAGE,
					       "pixbuf", pixbuf,
					       NULL);
		g_object_unref (pixbuf);
		image->target_cairo = FALSE;
		return image;
	}
#else
	g_warning ("go_image_new_from_file not implemented!");
#endif
	return NULL;
}

guint8 *
go_image_get_pixels (GOImage *image)
{
	g_return_val_if_fail (image, NULL);
	return image->data;
}

int
go_image_get_rowstride (GOImage *image)
{
	g_return_val_if_fail (image, 0);
	return image->rowstride;
}

void
go_image_fill (GOImage *image, GOColor color)
{
	guint32 val;
	guint8 *dst;
	unsigned i, j;
	g_return_if_fail (image);

	dst = image->data;
	if (image->target_cairo)
		val = (GO_COLOR_UINT_R (color) << 8) + (GO_COLOR_UINT_G (color) << 16)
			+ (GO_COLOR_UINT_B (color) << 24) + GO_COLOR_UINT_A (color);
	else
		val = color;
	for (i = 0; i < image->height; i++) {
		for (j = 0; j < image->width; j++)
			*((guint32*) dst) = val;
		dst += image->rowstride - image->width * 4;
	}
}

void
go_image_set_name (GOImage *image, char const *name)
{
	g_free (image->name);
	image->name = (name)? g_strdup (name): NULL;
}

char const *
go_image_get_name (GOImage *image)
{
	g_return_val_if_fail (GO_IS_IMAGE (image), NULL);
	return image->name;
}

gboolean
go_image_same_pixbuf (GOImage *first, GOImage *second)
{
#ifdef GOFFICE_WITH_GTK
	void *pixels1, *pixels2;
	int size;
	g_return_val_if_fail (GO_IS_IMAGE (first), FALSE);
	g_return_val_if_fail (GO_IS_IMAGE (second), FALSE);
	if (!first->pixbuf)
		go_image_get_pixbuf (first);
	if (!second->pixbuf)
		go_image_get_pixbuf (second);
	if (!first->pixbuf || !second->pixbuf)
		return FALSE;
	if (gdk_pixbuf_get_n_channels (first->pixbuf) != gdk_pixbuf_get_n_channels (second->pixbuf))
		return FALSE;
	if (gdk_pixbuf_get_colorspace (first->pixbuf) != gdk_pixbuf_get_colorspace (second->pixbuf))
		return FALSE;
	if (gdk_pixbuf_get_bits_per_sample (first->pixbuf) != gdk_pixbuf_get_bits_per_sample (second->pixbuf))
		return FALSE;
	if (gdk_pixbuf_get_has_alpha (first->pixbuf) != gdk_pixbuf_get_has_alpha (second->pixbuf))
		return FALSE;
	if (gdk_pixbuf_get_width (first->pixbuf) != gdk_pixbuf_get_width (second->pixbuf))
		return FALSE;
	if (gdk_pixbuf_get_height (first->pixbuf) != gdk_pixbuf_get_height (second->pixbuf))
		return FALSE;
	if (gdk_pixbuf_get_rowstride (first->pixbuf) != gdk_pixbuf_get_rowstride (second->pixbuf))
		return FALSE;
	pixels1 = gdk_pixbuf_get_pixels (first->pixbuf);
	pixels2 = gdk_pixbuf_get_pixels (second->pixbuf);
	size = gdk_pixbuf_get_rowstride (first->pixbuf) * gdk_pixbuf_get_height (first->pixbuf);
	return !memcmp (pixels1, pixels2, size);
#else
	return FALSE;
#endif
}

void
go_image_save (GOImage *image, GsfXMLOut *output)
{
	g_return_if_fail (GO_IS_IMAGE (image) && image->name);
	gsf_xml_out_start_element (output, "GOImage");
	gsf_xml_out_add_cstr (output, "name", image->name);
	gsf_xml_out_add_int (output, "width", image->width);
	gsf_xml_out_add_int (output, "height", image->height);
	gsf_xml_out_add_int (output, "rowstride", image->rowstride);
	gsf_xml_out_add_base64 (output, NULL,
			go_image_get_pixels (image), image->height * image->rowstride);
	gsf_xml_out_end_element (output);
}

void
go_image_load_attrs (GOImage *image, G_GNUC_UNUSED GsfXMLIn *xin, xmlChar const **attrs)
{
	xmlChar const **attr;
	g_return_if_fail (image != NULL);
	for (attr = attrs; attr != NULL && attr[0] && attr[1] ; attr += 2)
		if (0 == strcmp ((const char*)attr[0], "width"))
			image->width = strtol ((const char*)attr[1], NULL, 10);
		else if (0 == strcmp ((const char*)attr[0], "height"))
			image->height= strtol ((const char*)attr[1], NULL, 10);
		else if (0 == strcmp ((const char*)attr[0], "rowstride"))
			image->rowstride = strtol ((const char*)attr[1], NULL, 10);
}

void
go_image_load_data (GOImage *image, GsfXMLIn *xin)
{
	int length;
	length = gsf_base64_decode_simple ((guint8*) xin->content->str, strlen(xin->content->str));
	image->data = g_memdup (xin->content->str, length);
	image->target_cairo = TRUE;
}

