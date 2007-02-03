/*
 * termbook.c
 * This file is part of Stjerm
 *
 * Copyright (C) 2007 - Stjepan Glavina
 *
 * Stjerm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Stjerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stjerm; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */


#include <gtk/gtk.h>
#include <vte/vte.h>
#include <string.h>
#include "stjerm.h"


GtkWidget *popupmenu;
int open_count;

void build_termbook(void);
static void build_popupmenu(void);
static void open_tab(void);
static void close_tab(void);
void term_grab_focus(void);
static gboolean term_button_press(GtkWidget*, GdkEventButton*, gpointer);
static void popupmenu_activate(gchar*);


void build_termbook(void)
{
	build_popupmenu();
	termbook = gtk_notebook_new();
	
	open_count = 0;
	open_tab();
}


void build_popupmenu(void)
{
	popupmenu = gtk_menu_new();
	
	GtkWidget *menuitem;
	GtkWidget *img;
	
	gchar *labels[] = { "Open Tab", "Close Tab", "Copy", "Paste", "Preferences",
	                    "About", "Quit" };
	gchar *stocks[] = { GTK_STOCK_ADD, GTK_STOCK_CLOSE, GTK_STOCK_COPY,
	                    GTK_STOCK_PASTE, GTK_STOCK_PREFERENCES, GTK_STOCK_ABOUT,
	                    GTK_STOCK_QUIT };
	
	int i;
	for (i = 0; i < 7; i++)
	{
		if (i == 2 || i == 4 || i == 5)
		{
			menuitem = gtk_separator_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(popupmenu), menuitem);
			gtk_widget_show(GTK_WIDGET(menuitem));
		}
		
		menuitem = gtk_image_menu_item_new_with_label(labels[i]);
		img = gtk_image_new_from_stock(stocks[i], GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuitem), GTK_WIDGET(img));
		g_signal_connect_swapped(G_OBJECT(menuitem), "activate",
		                         G_CALLBACK(popupmenu_activate),
		                         (gpointer)labels[i]);
		gtk_menu_shell_append(GTK_MENU_SHELL(popupmenu), menuitem);
		gtk_widget_show(GTK_WIDGET(menuitem));
	}
}


static void open_tab(void)
{
	open_count++;
	
	GtkHBox *box;
	VteTerminal *term;
	GtkVScrollbar *sbar;
	
	box = GTK_HBOX(gtk_hbox_new(FALSE, 0));
	term = VTE_TERMINAL(vte_terminal_new());
	sbar = GTK_VSCROLLBAR(gtk_vscrollbar_new(
	                          vte_terminal_get_adjustment(VTE_TERMINAL(term))));
	
	vte_terminal_fork_command(term, "/bin/sh", NULL, NULL,
	                          currdir, FALSE, TRUE, TRUE);
	
	GdkColor fore, back, tint, highlight, cursor, black;
	black.red = black.green = black.blue = 0x0000;
	
	back.red   = 0x0000;
	back.green = 0x0000;
	back.blue  = 0x0000;

    fore.red   = 0xffff;
    fore.green = 0xffff;
    fore.blue  = 0xffff;

    highlight.red = highlight.green = highlight.blue = 0xc000;
    cursor.red = 0xffff;
    cursor.green = cursor.blue = 0x8000;
    tint.red = tint.green = tint.blue = 0;
    tint = black;
    
    vte_terminal_set_background_tint_color (VTE_TERMINAL(term), &tint);
    vte_terminal_set_colors (VTE_TERMINAL(term), &fore, &back, NULL, 0);
    
    
	vte_terminal_set_color_foreground(VTE_TERMINAL(term), &fore);
	
	gtk_widget_show(GTK_WIDGET(box));
	gtk_widget_show(GTK_WIDGET(term));
	gtk_widget_show(GTK_WIDGET(sbar));
	
	g_signal_connect_swapped(G_OBJECT(term), "button-press-event",
	                         G_CALLBACK(term_button_press), NULL);
	
	gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(term), TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(box), GTK_WIDGET(sbar), FALSE, FALSE, 0);
	
	GtkWidget *label;
	char str[20];
	sprintf(str, "Terminal %i", open_count);
	label = gtk_label_new(str);
	
	gtk_notebook_append_page(GTK_NOTEBOOK(termbook), GTK_WIDGET(box), label);
	gtk_notebook_set_tab_label_packing(GTK_NOTEBOOK(termbook), GTK_WIDGET(box),
	                                   TRUE, FALSE, GTK_PACK_START);
}


static void close_tab(void)
{
	gint currpage = gtk_notebook_get_current_page(GTK_NOTEBOOK(termbook));
	gtk_notebook_remove_page(GTK_NOTEBOOK(termbook), currpage);
}


void term_grab_focus(void)
{
	gtk_window_present(GTK_WINDOW(mainwindow));
	
	GList *children;
	GtkWidget *box;
	gint currPage;
	
	currPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(termbook));
	box = gtk_notebook_get_nth_page(GTK_NOTEBOOK(termbook), currPage);
	children = gtk_container_get_children(GTK_CONTAINER(box));
	gtk_widget_grab_focus(GTK_WIDGET(g_list_nth_data(children, 0)));

	g_list_free(children);
}


static gboolean term_button_press(GtkWidget *widget, GdkEventButton *event,
                                  gpointer user_data)
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		gtk_menu_popup(GTK_MENU(popupmenu), NULL, NULL, NULL, NULL, event->button,
		               event->time);
	}
	
	return FALSE;
}


static void popupmenu_activate(gchar *label)
{
	if (!strcmp(label, "Open Tab"))
	{
		open_tab();
	}
	else if (!strcmp(label, "Close Tab"))
	{
		close_tab();
	}
}
