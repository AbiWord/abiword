/* This is directly taken from gtk+/demos/gtk-demo/foreigndrawing.c
 * Gtk+ is license under LGPLv2+
 */

#include <string.h>

#include "ut_assert.h"

#include "xap_GtkStyle.h"

static void
append_element (GtkWidgetPath *path,
                const char    *selector)
{
#if 0
  static const struct {
    const char    *name;
    GtkStateFlags  state_flag;
  } pseudo_classes[] = {
    { "active",        GTK_STATE_FLAG_ACTIVE },
    { "hover",         GTK_STATE_FLAG_PRELIGHT },
    { "selected",      GTK_STATE_FLAG_SELECTED },
    { "disabled",      GTK_STATE_FLAG_INSENSITIVE },
    { "indeterminate", GTK_STATE_FLAG_INCONSISTENT },
    { "focus",         GTK_STATE_FLAG_FOCUSED },
    { "backdrop",      GTK_STATE_FLAG_BACKDROP },
    { "dir(ltr)",      GTK_STATE_FLAG_DIR_LTR },
    { "dir(rtl)",      GTK_STATE_FLAG_DIR_RTL },
    { "link",          GTK_STATE_FLAG_LINK },
    { "visited",       GTK_STATE_FLAG_VISITED },
    { "checked",       GTK_STATE_FLAG_CHECKED },
    { "drop(active)",  GTK_STATE_FLAG_DROP_ACTIVE }
  };
  guint i;
#endif
  const char *next;
  char *name;
  char type;

  next = strpbrk (selector, "#.:");
  if (next == NULL)
    next = selector + strlen (selector);

  name = g_strndup (selector, next - selector);
  if (g_ascii_isupper (selector[0]))
    {
      GType gtype;
      gtype = g_type_from_name (name);
      if (gtype == G_TYPE_INVALID)
        {
          g_critical ("Unknown type name `%s'", name);
          g_free (name);
          return;
        }
      gtk_widget_path_append_type (path, gtype);
    }
  else
    {
#if 0
      /* Omit type, we're using name */
      gtk_widget_path_append_type (path, G_TYPE_NONE);
      gtk_widget_path_iter_set_object_name (path, -1, name);
#else
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
#endif
    }
  g_free (name);

  while (*next != '\0')
    {
      type = *next;
      selector = next + 1;
      next = strpbrk (selector, "#.:");
      if (next == NULL)
        next = selector + strlen (selector);
      name = g_strndup (selector, next - selector);

      switch (type)
        {
        case '#':
          gtk_widget_path_iter_set_name (path, -1, name);
          break;

        case '.':
          gtk_widget_path_iter_add_class (path, -1, name);
          break;

        case ':':
#if 0
          for (i = 0; i < G_N_ELEMENTS (pseudo_classes); i++)
            {
              if (g_str_equal (pseudo_classes[i].name, name))
                {
                  gtk_widget_path_iter_set_state (
                    path,
                    -1,
                    (GtkStateFlags)(gtk_widget_path_iter_get_state (path, -1)
                                    | pseudo_classes[i].state_flag));
                  break;
                }
            }
          if (i == G_N_ELEMENTS (pseudo_classes))
            g_critical ("Unknown pseudo-class :%s", name);
#else
          // you reached it, check what you passed to XAP_GtkStyle_get_style()
          // this require a version of Gtk that is too recent.
          // XXX eventually fix when we bump the requirements.
          UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
#endif
          break;

        default:
          g_assert_not_reached ();
          break;
        }

      g_free (name);
    }
}

static GtkStyleContext *
create_context_for_path (GtkWidgetPath   *path,
                         GtkStyleContext *parent)
{
  GtkStyleContext *context;

  context = gtk_style_context_new ();
  gtk_style_context_set_path (context, path);
  gtk_style_context_set_parent (context, parent);
  /* Unfortunately, we have to explicitly set the state again here
   * for it to take effect
   */
#if 0 // we haven't set a state anyway.
  gtk_style_context_set_state (context, gtk_widget_path_iter_get_state (path, -1));
#endif
  gtk_widget_path_unref (path);

  return context;
}

GtkStyleContext *
XAP_GtkStyle_get_style (GtkStyleContext *parent,
                        const char      *selector)
{
  GtkWidgetPath *path;

  if (parent)
    path = gtk_widget_path_copy (gtk_style_context_get_path (parent));
  else
    path = gtk_widget_path_new ();

  append_element (path, selector);

  return create_context_for_path (path, parent);
}

