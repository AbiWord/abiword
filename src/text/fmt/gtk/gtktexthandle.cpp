/* GTK - The GIMP Toolkit
 * Copyright © 2012 Carlos Garnacho <carlosg@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "gtktexthandleprivate.h"
#include <gtk/gtk.h>

typedef struct _HandleWindow HandleWindow;

enum {
  HANDLE_DRAGGED,
  DRAG_FINISHED,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_PARENT,
  PROP_RELATIVE_TO
};

struct _HandleWindow
{
  GdkWindow *window;
  GdkRectangle pointing_to;
  gint dx;
  gint dy;
  guint dragged : 1;
  guint mode_visible : 1;
  guint user_visible : 1;
  guint has_point : 1;
};

struct FvTextHandlePrivate
{
  HandleWindow windows[2];
  GtkWidget *parent;
  GdkWindow *relative_to;
  GtkStyleContext *style_context;

  gulong draw_signal_id;
  gulong event_signal_id;
  gulong style_updated_id;
  gulong composited_changed_id;
  guint realized : 1;
  guint mode : 2;
};

G_DEFINE_TYPE (FvTextHandle, _fv_text_handle, G_TYPE_OBJECT)

static guint signals[LAST_SIGNAL] = { 0 };

static void
_fv_text_handle_get_size (FvTextHandle *handle,
                          gint         *width,
                          gint         *height)
{
  FvTextHandlePrivate *priv;
  gint w, h;

  priv = handle->priv;

#if GTK_CHECK_VERSION (3, 6, 0)
  gtk_widget_style_get (priv->parent,
                        "text-handle-width", &w,
                        "text-handle-height", &h,
                        NULL);
#else
  /* Hardcode default values from GTK+ 3.6 */
  w = 16;
  h = 20;
#endif

  if (width)
    *width = w;

  if (height)
    *height = h;
}

static void
_fv_text_handle_draw (FvTextHandle         *handle,
                      cairo_t              *cr,
                      FvTextHandlePosition  pos)
{
  FvTextHandlePrivate *priv;
  gint width, height;

  priv = handle->priv;
  cairo_save (cr);

  cairo_save (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba (cr, 0, 0, 0, 0);
  cairo_paint (cr);
  cairo_restore (cr);

  gtk_style_context_save (priv->style_context);
  gtk_style_context_add_class (priv->style_context, "cursor-handle");

  if (pos == FV_TEXT_HANDLE_POSITION_SELECTION_END)
    {
      gtk_style_context_add_class (priv->style_context,
                                   GTK_STYLE_CLASS_BOTTOM);

      if (priv->mode == FV_TEXT_HANDLE_MODE_CURSOR)
        gtk_style_context_add_class (priv->style_context, "insertion-cursor");
    }
  else
    gtk_style_context_add_class (priv->style_context, "top");

  _fv_text_handle_get_size (handle, &width, &height);
  gtk_render_background (priv->style_context, cr, 0, 0, width, height);

  gtk_style_context_restore (priv->style_context);
  cairo_restore (cr);
}

static void
_fv_text_handle_update_shape (FvTextHandle         *handle,
                              GdkWindow            *window,
                              FvTextHandlePosition  pos)
{
  FvTextHandlePrivate *priv;
  cairo_surface_t *surface;
  cairo_region_t *region;
  cairo_t *cr;

  priv = handle->priv;

  surface =
    gdk_window_create_similar_surface (window,
                                       CAIRO_CONTENT_COLOR_ALPHA,
                                       gdk_window_get_width (window),
                                       gdk_window_get_height (window));

  cr = cairo_create (surface);
  _fv_text_handle_draw (handle, cr, pos);
  cairo_destroy (cr);

  region = gdk_cairo_region_create_from_surface (surface);

  if (gtk_widget_is_composited (priv->parent))
    gdk_window_shape_combine_region (window, NULL, 0, 0);
  else
    gdk_window_shape_combine_region (window, region, 0, 0);

  gdk_window_input_shape_combine_region (window, region, 0, 0);

  cairo_surface_destroy (surface);
  cairo_region_destroy (region);
}

static GdkWindow *
_fv_text_handle_create_window (FvTextHandle         *handle,
                               FvTextHandlePosition  pos)
{
  FvTextHandlePrivate *priv;
  GdkRGBA bg = { 0, 0, 0, 0 };
  GdkWindowAttr attributes;
  GdkWindow *window;
  GdkVisual *visual;
  gint mask;

  priv = handle->priv;

  attributes.x = 0;
  attributes.y = 0;
  _fv_text_handle_get_size (handle, &attributes.width, &attributes.height);
  attributes.window_type = GDK_WINDOW_TEMP;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.event_mask = (GDK_EXPOSURE_MASK |
                           GDK_BUTTON_PRESS_MASK |
                           GDK_BUTTON_RELEASE_MASK |
                           GDK_BUTTON1_MOTION_MASK);

  mask = GDK_WA_X | GDK_WA_Y;

  visual = gdk_screen_get_rgba_visual (gtk_widget_get_screen (priv->parent));

  if (visual)
    {
      attributes.visual = visual;
      mask |= GDK_WA_VISUAL;
    }

  window = gdk_window_new (NULL, &attributes, mask);
  gdk_window_set_user_data (window, priv->parent);
  gdk_window_set_background_rgba (window, &bg);

  _fv_text_handle_update_shape (handle, window, pos);

  return window;
}

static gboolean
fv_text_handle_widget_draw (GtkWidget    * /*widget*/,
                            cairo_t      *cr,
                            FvTextHandle *handle)
{
  FvTextHandlePrivate *priv;
  FvTextHandlePosition pos;

  priv = handle->priv;

  if (!priv->realized)
    return FALSE;

  if (gtk_cairo_should_draw_window (cr, priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].window))
    pos = FV_TEXT_HANDLE_POSITION_SELECTION_START;
  else if (gtk_cairo_should_draw_window (cr, priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].window))
    pos = FV_TEXT_HANDLE_POSITION_SELECTION_END;
  else
    return FALSE;

  _fv_text_handle_draw (handle, cr, pos);
  return TRUE;
}

static gboolean
fv_text_handle_widget_event (GtkWidget    * /*widget*/,
                             GdkEvent     *event,
                             FvTextHandle *handle)
{
  FvTextHandlePrivate *priv;
  FvTextHandlePosition pos;

  priv = handle->priv;

  if (event->any.window == priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].window)
    pos = FV_TEXT_HANDLE_POSITION_SELECTION_START;
  else if (event->any.window == priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].window)
    pos = FV_TEXT_HANDLE_POSITION_SELECTION_END;
  else
    return FALSE;

  if (event->type == GDK_BUTTON_PRESS)
    {
      priv->windows[pos].dx = event->button.x;
      priv->windows[pos].dy = event->button.y;
      priv->windows[pos].dragged = TRUE;
    }
  else if (event->type == GDK_BUTTON_RELEASE)
    {
      g_signal_emit (handle, signals[DRAG_FINISHED], 0, pos);
      priv->windows[pos].dx =  priv->windows[pos].dy = 0;
      priv->windows[pos].dragged = FALSE;
    }
  else if (event->type == GDK_MOTION_NOTIFY && priv->windows[pos].dragged)
    {
      gint x, y, width, height;

      _fv_text_handle_get_size (handle, &width, &height);
      gdk_window_get_origin (priv->relative_to, &x, &y);

      x = event->motion.x_root - priv->windows[pos].dx + (width / 2) - x;
      y = event->motion.y_root - priv->windows[pos].dy - y;

      if (pos == FV_TEXT_HANDLE_POSITION_SELECTION_START)
        y += height;

      g_signal_emit (handle, signals[HANDLE_DRAGGED], 0, pos, x, y);
    }

  return TRUE;
}

static void
_fv_text_handle_update_window_state (FvTextHandle         *handle,
                                     FvTextHandlePosition  pos)
{
  FvTextHandlePrivate *priv;
  HandleWindow *handle_window;

  priv = handle->priv;
  handle_window = &priv->windows[pos];

  if (!handle_window->window)
    return;

  if (handle_window->has_point &&
      handle_window->mode_visible && handle_window->user_visible)
    {
      gint x, y, width, height;

      x = handle_window->pointing_to.x;
      y = handle_window->pointing_to.y;
      _fv_text_handle_get_size (handle, &width, &height);

      if (pos == FV_TEXT_HANDLE_POSITION_CURSOR)
        y += handle_window->pointing_to.height;
      else
        y -= height;

      x -= width / 2;

      gdk_window_move_resize (handle_window->window, x, y, width, height);
      gdk_window_show (handle_window->window);
    }
  else
    gdk_window_hide (handle_window->window);
}

static void
_fv_text_handle_update_window (FvTextHandle         *handle,
                               FvTextHandlePosition  pos,
                               gboolean              recreate)
{
  FvTextHandlePrivate *priv;
  HandleWindow *handle_window;

  priv = handle->priv;
  handle_window = &priv->windows[pos];

  if (!handle_window->window)
    return;

  if (recreate)
    {
      gdk_window_destroy (handle_window->window);
      handle_window->window = _fv_text_handle_create_window (handle, pos);
    }

  _fv_text_handle_update_window_state (handle, pos);
}

static void
_fv_text_handle_update_windows (FvTextHandle *handle)
{
  _fv_text_handle_update_window (handle, FV_TEXT_HANDLE_POSITION_SELECTION_START, FALSE);
  _fv_text_handle_update_window (handle, FV_TEXT_HANDLE_POSITION_SELECTION_END, FALSE);
}

static void
_fv_text_handle_composited_changed (FvTextHandle *handle)
{
  _fv_text_handle_update_window (handle, FV_TEXT_HANDLE_POSITION_SELECTION_START, TRUE);
  _fv_text_handle_update_window (handle, FV_TEXT_HANDLE_POSITION_SELECTION_END, TRUE);
}

static void
fv_text_handle_constructed (GObject *object)
{
  FvTextHandlePrivate *priv;

  priv = FV_TEXT_HANDLE (object)->priv;
  g_assert (priv->parent != NULL);

  priv->draw_signal_id =
    g_signal_connect (priv->parent, "draw",
                      G_CALLBACK (fv_text_handle_widget_draw),
                      object);
  priv->event_signal_id =
    g_signal_connect (priv->parent, "event",
                      G_CALLBACK (fv_text_handle_widget_event),
                      object);
  priv->composited_changed_id =
    g_signal_connect_swapped (priv->parent, "composited-changed",
                              G_CALLBACK (_fv_text_handle_composited_changed),
                              object);
  priv->style_updated_id =
    g_signal_connect_swapped (priv->parent, "style-updated",
                              G_CALLBACK (_fv_text_handle_update_windows),
                              object);
}

static void
fv_text_handle_finalize (GObject *object)
{
  FvTextHandlePrivate *priv;

  priv = FV_TEXT_HANDLE (object)->priv;

  if (priv->relative_to)
    g_object_unref (priv->relative_to);

  if (priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].window)
    gdk_window_destroy (priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].window);

  if (priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].window)
    gdk_window_destroy (priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].window);

  if (g_signal_handler_is_connected (priv->parent, priv->draw_signal_id))
    g_signal_handler_disconnect (priv->parent, priv->draw_signal_id);

  if (g_signal_handler_is_connected (priv->parent, priv->event_signal_id))
    g_signal_handler_disconnect (priv->parent, priv->event_signal_id);

  if (g_signal_handler_is_connected (priv->parent, priv->composited_changed_id))
    g_signal_handler_disconnect (priv->parent, priv->composited_changed_id);

  if (g_signal_handler_is_connected (priv->parent, priv->style_updated_id))
    g_signal_handler_disconnect (priv->parent, priv->style_updated_id);

  g_object_unref (priv->style_context);
  g_object_unref (priv->parent);

  G_OBJECT_CLASS (_fv_text_handle_parent_class)->finalize (object);
}

static void
fv_text_handle_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  FvTextHandlePrivate *priv;
  FvTextHandle *handle;

  handle = FV_TEXT_HANDLE (object);
  priv = handle->priv;

  switch (prop_id)
    {
    case PROP_PARENT:
      priv->parent = GTK_WIDGET(g_value_dup_object (value));
      break;
    case PROP_RELATIVE_TO:
      _fv_text_handle_set_relative_to (handle,
                                       GDK_WINDOW(g_value_get_object (value)));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
fv_text_handle_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  FvTextHandlePrivate *priv;

  priv = FV_TEXT_HANDLE (object)->priv;

  switch (prop_id)
    {
    case PROP_PARENT:
      g_value_set_object (value, priv->parent);
      break;
    case PROP_RELATIVE_TO:
      g_value_set_object (value, priv->relative_to);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
_fv_text_handle_class_init (FvTextHandleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = fv_text_handle_constructed;
  object_class->finalize = fv_text_handle_finalize;
  object_class->set_property = fv_text_handle_set_property;
  object_class->get_property = fv_text_handle_get_property;

  signals[HANDLE_DRAGGED] =
    g_signal_new ("handle-dragged",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (FvTextHandleClass, handle_dragged),
		  NULL, NULL,
                  g_cclosure_marshal_generic,
		  G_TYPE_NONE, 3,
                  G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
  signals[DRAG_FINISHED] =
    g_signal_new ("drag-finished",
		  G_OBJECT_CLASS_TYPE (object_class),
		  G_SIGNAL_RUN_LAST, 0,
		  NULL, NULL,
                  g_cclosure_marshal_VOID__INT,
                  G_TYPE_NONE, 1, G_TYPE_INT);

  g_object_class_install_property (object_class,
                                   PROP_PARENT,
                                   g_param_spec_object ("parent",
                                                        "Parent widget",
                                                        "Parent widget",
                                                        GTK_TYPE_WIDGET,
                                                        (GParamFlags)(G_PARAM_READWRITE |
								      G_PARAM_CONSTRUCT_ONLY)));
  g_object_class_install_property (object_class,
                                   PROP_RELATIVE_TO,
                                   g_param_spec_object ("relative-to",
                                                        "Window",
                                                        "Window the coordinates are based upon",
                                                        GDK_TYPE_WINDOW,
                                                        (GParamFlags)G_PARAM_READWRITE));

  g_type_class_add_private (object_class, sizeof (FvTextHandlePrivate));
}

static void
_fv_text_handle_init (FvTextHandle *handle)
{
  FvTextHandlePrivate *priv;
  GtkWidgetPath *path;

  handle->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE (handle,
                                                     FV_TYPE_TEXT_HANDLE,
                                                     FvTextHandlePrivate);

  path = gtk_widget_path_new ();
  gtk_widget_path_append_type (path, FV_TYPE_TEXT_HANDLE);

  priv->style_context = gtk_style_context_new ();
  gtk_style_context_set_path (priv->style_context, path);
  gtk_widget_path_free (path);
}

FvTextHandle *
_fv_text_handle_new (GtkWidget *parent)
{
	return (FvTextHandle *)g_object_new (FV_TYPE_TEXT_HANDLE,
                       "parent", parent,
                       NULL);
}

void
_fv_text_handle_set_relative_to (FvTextHandle *handle,
                                 GdkWindow    *window)
{
  FvTextHandlePrivate *priv;

  g_return_if_fail (FV_IS_TEXT_HANDLE (handle));
  g_return_if_fail (!window || GDK_IS_WINDOW (window));

  priv = handle->priv;

  if (priv->relative_to)
    {
      gdk_window_destroy (priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].window);
      gdk_window_destroy (priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].window);
      g_object_unref (priv->relative_to);
    }

  if (window)
    {
      priv->relative_to = GDK_WINDOW(g_object_ref (window));
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].window =
        _fv_text_handle_create_window (handle, FV_TEXT_HANDLE_POSITION_SELECTION_START);
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].window =
        _fv_text_handle_create_window (handle, FV_TEXT_HANDLE_POSITION_SELECTION_END);
      priv->realized = TRUE;
    }
  else
    {
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].window = NULL;
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].window = NULL;
      priv->relative_to = NULL;
      priv->realized = FALSE;
    }

  g_object_notify (G_OBJECT (handle), "relative-to");
}

void
_fv_text_handle_set_mode (FvTextHandle     *handle,
                          FvTextHandleMode  mode)
{
  FvTextHandlePrivate *priv;

  g_return_if_fail (FV_IS_TEXT_HANDLE (handle));

  priv = handle->priv;

  if (priv->mode == mode)
    return;

  priv->mode = mode;

  switch (mode)
    {
    case FV_TEXT_HANDLE_MODE_CURSOR:
      priv->windows[FV_TEXT_HANDLE_POSITION_CURSOR].mode_visible = TRUE;
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].mode_visible = FALSE;
      break;
    case FV_TEXT_HANDLE_MODE_SELECTION:
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].mode_visible = TRUE;
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].mode_visible = TRUE;
      break;
    case FV_TEXT_HANDLE_MODE_NONE:
    default:
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_START].mode_visible = FALSE;
      priv->windows[FV_TEXT_HANDLE_POSITION_SELECTION_END].mode_visible = FALSE;
      break;
    }

  if (mode != FV_TEXT_HANDLE_MODE_NONE)
    _fv_text_handle_update_shape (handle,
                                  priv->windows[FV_TEXT_HANDLE_POSITION_CURSOR].window,
                                  FV_TEXT_HANDLE_POSITION_CURSOR);

  _fv_text_handle_update_window_state (handle, FV_TEXT_HANDLE_POSITION_SELECTION_START);
  _fv_text_handle_update_window_state (handle, FV_TEXT_HANDLE_POSITION_SELECTION_END);
}

FvTextHandleMode
_fv_text_handle_get_mode (FvTextHandle *handle)
{
  g_return_val_if_fail (FV_IS_TEXT_HANDLE (handle), FV_TEXT_HANDLE_MODE_NONE);

  return (FvTextHandleMode)handle->priv->mode;
}

void
_fv_text_handle_set_position (FvTextHandle         *handle,
                              FvTextHandlePosition  pos,
                              GdkRectangle         *rect)
{
  FvTextHandlePrivate *priv;
  HandleWindow *handle_window;

  g_return_if_fail (FV_IS_TEXT_HANDLE (handle));

  priv = handle->priv;
  pos = CLAMP (pos, FV_TEXT_HANDLE_POSITION_CURSOR,
               FV_TEXT_HANDLE_POSITION_SELECTION_START);
  handle_window = &priv->windows[pos];

  if (!priv->realized)
    return;

  if (priv->mode == FV_TEXT_HANDLE_MODE_NONE ||
      (priv->mode == FV_TEXT_HANDLE_MODE_CURSOR &&
       pos != FV_TEXT_HANDLE_POSITION_CURSOR))
    return;

  handle_window->pointing_to = *rect;
  handle_window->has_point = TRUE;
  gdk_window_get_root_coords (priv->relative_to,
                              rect->x, rect->y,
                              &handle_window->pointing_to.x,
                              &handle_window->pointing_to.y);

  _fv_text_handle_update_window_state (handle, pos);
}

void
_fv_text_handle_set_visible (FvTextHandle         *handle,
                             FvTextHandlePosition  pos,
                             gboolean              visible)
{
  FvTextHandlePrivate *priv;
  GdkWindow *window;

  g_return_if_fail (FV_IS_TEXT_HANDLE (handle));

  priv = handle->priv;
  pos = CLAMP (pos, FV_TEXT_HANDLE_POSITION_CURSOR,
               FV_TEXT_HANDLE_POSITION_SELECTION_START);

  if (!priv->realized)
    return;

  window = priv->windows[pos].window;

  if (!window)
    return;

  if (priv->windows[pos].dragged)
    return;

  priv->windows[pos].user_visible = visible;
  _fv_text_handle_update_window_state (handle, pos);
}

gboolean
_fv_text_handle_get_is_dragged (FvTextHandle         *handle,
                                FvTextHandlePosition  pos)
{
  FvTextHandlePrivate *priv;

  g_return_val_if_fail (FV_IS_TEXT_HANDLE (handle), FALSE);

  priv = handle->priv;
  pos = CLAMP (pos, FV_TEXT_HANDLE_POSITION_CURSOR,
               FV_TEXT_HANDLE_POSITION_SELECTION_START);

  return priv->windows[pos].dragged;
}
