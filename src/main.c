////////////////////////////////////////////////////////////////////////////////
// GTLSIM
// A TINY LOGIC CIRCUIT SIMULATOR
// (C) 2020 BY ARMCODER - milton@armcoder.com.br
//
// https://github.com/ARMCoderBR/gtlsim
//
// THIS PROGRAM IS FREE SOFTWARE
// SEE LICENSE AT https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <ncurses.h>

#include "board.h"
#include "tests.h"
#include "computer.h"
#include "boardres.h"

#include <gtk/gtk.h>
#include <pthread.h>


pthread_t simthread;
int running = 0;

////////////////////////////////////////////////////////////////////////////////
void *run_sim(void *args){

    computer_t *comp = args;
    running = 1;

    for (;running;){

        computer_sim_run((computer_t*)comp);
        usleep(SIM_STEP_TIME_US);    //100 US
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
gint timeout_callback (gpointer data){

    board_refresh(((computer_t*)data)->pctx, ((computer_t*)data)->mainboard);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
static void
activate (GtkApplication* app,
          gpointer        comp) {

    GtkWidget *window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");

    //gtk_window_set_default_size (GTK_WINDOW (window), 400, 200);

    GtkWidget *main_grid = gtk_grid_new();//gtk_button_box_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add (GTK_CONTAINER (window), main_grid);

    boardres_init_streams();

    GdkPixbuf *icon = gdk_pixbuf_new_from_stream(nand_s, NULL, NULL);
    gtk_window_set_icon ((GtkWindow*)window, icon);

    char name[64];
    sprintf(name,"GTLSIM V%d.%d.%d",SW_VERSION, SW_REVISION, SW_MINOR);
    gtk_window_set_title ((GtkWindow*)window, name);

    GtkWidget *button2 = gtk_button_new_with_label ("Exit");

    g_signal_connect_swapped (button2, "clicked", G_CALLBACK (gtk_widget_destroy), window);

    gtk_grid_attach ((GtkGrid*)main_grid, button2, 4, 3, 1, 1);

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

    /*gint*/ g_timeout_add (32,//guint32     interval,
            timeout_callback,//GtkFunction function,
                        comp//gpointer    data
                        );

    status = g_application_run (G_APPLICATION (app), argc, argv);

    running = 0;
    pthread_join(simthread, NULL);

    computer_sim_end(comp);

    g_object_unref (app);

    return status;
}
