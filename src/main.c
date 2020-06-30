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
#include <pthread.h>

int state = 1;

GtkImage * image1;
GtkWidget *main_grid;

pthread_t simthread;
int running = 0;

////////////////////////////////////////////////////////////////////////////////
static void
print_hello (GtkWidget *widget,
             gpointer   comp) {

    g_print ("Hello World\n");
    //computer_sim_run((computer_t*)comp);

    state ^= 1;

    if (state)
        gtk_image_set_from_file (image1,"../led-red-on.png");
    else
        gtk_image_set_from_file (image1,"../led-red-off.png");
}


////////////////////////////////////////////////////////////////////////////////
void *run_sim(void *args){

    computer_t *comp = args;
    running = 1;

    for (;running;){

        computer_sim_run((computer_t*)comp);
        usleep(100);
    }

    return NULL;
}


////////////////////////////////////////////////////////////////////////////////
static void
activate (GtkApplication* app,
          gpointer        comp) {

    GtkWidget *window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    //gtk_window_set_default_size (GTK_WINDOW (window), 400, 200);

    main_grid = gtk_grid_new();//gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add (GTK_CONTAINER (window), main_grid);

    GtkWidget *button = gtk_button_new_with_label ("Hello World");
    GtkWidget *button2 = gtk_button_new_with_label ("Exit");

    g_signal_connect (button, "clicked", G_CALLBACK (print_hello), comp);
    g_signal_connect_swapped (button2, "clicked", G_CALLBACK (gtk_widget_destroy), window);

    gtk_grid_attach ((GtkGrid*)main_grid, button, 1, 1, 1, 1);
    gtk_grid_attach ((GtkGrid*)main_grid, button2, 2, 1, 1, 1);

    image1 = gtk_image_new_from_file ("../led-red-on.png");

    gtk_grid_attach ((GtkGrid*)main_grid, image1, 3, 1, 1, 1);

    computer_sim_begin(comp, (GtkGrid*)main_grid);

    pthread_create(&simthread, NULL, run_sim, comp);

    gtk_widget_show_all (window);
}

////////////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv) {

    int status;

    computer_t *comp = malloc(sizeof(computer_t));
    memset(comp, 0, sizeof(computer_t));

    GtkApplication *app = gtk_application_new ("cpstecnologia.com.br", G_APPLICATION_FLAGS_NONE);

    g_signal_connect (app, "activate", G_CALLBACK (activate), comp);

    status = g_application_run (G_APPLICATION (app), argc, argv);

    running = 0;
    pthread_join(simthread, NULL);

    computer_sim_end(comp);

    g_object_unref (app);

    return status;
}
