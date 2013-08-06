#include <gtk/gtk.h>


void insert_into_doc(GtkWidget *widget, gpointer view)
{
  
}

void insert_symbol(GtkWidget *widget, gpointer view)
{
  gtk_entry_append_text(GTK_ENTRY(view),"\\alpha");
}

void close_window(GtkWidget *widget)
{
  gtk_main_quit();
}

int main(int argc, char** argv) 
{

  GtkWidget *view;
  GtkWidget *window;
  GtkWidget *frame;
  GtkWidget *alpha;
  GtkWidget *insert;
  GtkWidget *close;

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 360);
  gtk_window_set_title(GTK_WINDOW(window), "Advanced Editor");

  frame = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), frame);


  alpha = gtk_button_new_with_label("a");
  gtk_widget_set_size_request(alpha, 40, 40);
  gtk_fixed_put(GTK_FIXED(frame), alpha, 60, 80);

  insert = gtk_button_new_with_label("Insert");
  gtk_widget_set_size_request(insert, 80, 35);
  gtk_fixed_put(GTK_FIXED(frame), insert, 50, 300);

  close = gtk_button_new_with_label("Close");
  gtk_widget_set_size_request(close, 80, 35);
  gtk_fixed_put(GTK_FIXED(frame), close, 240, 300);

  view = gtk_entry_new ();
  gtk_fixed_put(GTK_FIXED(frame), view, 50, 50); 

  gtk_widget_show_all(window);

  g_signal_connect(window, "destroy",
      G_CALLBACK (gtk_main_quit), NULL);

  g_signal_connect(alpha, "clicked", 
      G_CALLBACK(insert_symbol), view);

  g_signal_connect(insert, "clicked", 
      G_CALLBACK(insert_into_doc), view);


  g_signal_connect(close, "clicked", 
      G_CALLBACK(close_window), view);

  gtk_main();

  return 0;
}
