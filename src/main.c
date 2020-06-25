////////////////////////////////////////////////////////////////////////////////
// TLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2019, 2020 BY ARMCODER - milton@armcoder.com.br
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <ncurses.h>

#include "board.h"
#include "tests.h"
#include "computer.h"

#include <gtk/gtk.h>

////////////////////////////////////////////////////////////////////////////////
static void
print_hello (GtkWidget *widget,
             gpointer   data) {

    g_print ("Hello World\n");
    computer_sim_run((computer_t*)data);
}

////////////////////////////////////////////////////////////////////////////////
static void
activate (GtkApplication* app,
          gpointer        user_data) {


    //computer_sim_run(comp);

  GtkWidget *window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  //gtk_window_set_default_size (GTK_WINDOW (window), 400, 200);

  GtkWidget *button_box = gtk_grid_new();//gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add (GTK_CONTAINER (window), button_box);

  GtkWidget *button = gtk_button_new_with_label ("Hello World");
  GtkWidget *button2 = gtk_button_new_with_label ("Exit");

  g_signal_connect (button, "clicked", G_CALLBACK (print_hello), user_data);
  g_signal_connect_swapped (button2, "clicked", G_CALLBACK (gtk_widget_destroy), window);
  gtk_grid_attach ((GtkGrid*)button_box, button, 1, 1, 1, 1);
  gtk_grid_attach ((GtkGrid*)button_box, button2, 2, 1, 1, 1);

  gtk_widget_show_all (window);

  //computer_sim_end(comp);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
  int status;

  computer_t *comp = computer_sim_begin();

  GtkApplication *app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), comp);

  status = g_application_run (G_APPLICATION (app), argc, argv);

  computer_sim_end(comp);

  g_object_unref (app);

  return status;
}
