/* $Id: db_gen_gui.c,v 1.2 2003/02/22 23:17:09 d3august Exp $
*/
/*  xtraceroute - graphically show traceroute information.
 *  Copyright (C) 1996-1998  Bj�rn Augustsson 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*  File created by Edouard Lafargue 07/98
*/

#include "xt.h"
#include <stdlib.h>
#include <string.h>

/* TODO FIXME Allow updating the generic database. */

static gint text_has_changed = FALSE;
static gint guard = FALSE;

static GtkWidget *name_entry;
static GtkWidget *lat_entry;
static GtkWidget *lon_entry;
static GtkWidget *info_entry;

static gint destroy_widget_callback(GtkWidget *wi, gpointer *target)
{
  gtk_widget_destroy(GTK_WIDGET(target));
  guard = FALSE;
  return TRUE;
}

static gint yesbutton_callback(GtkWidget *wi, gpointer *data)
{
  dbentry *ent;
  
  if(text_has_changed)
    {

      text_has_changed = FALSE;
      
      ent = (dbentry *)malloc(sizeof(dbentry));

      /* This is a good place to do some sanity/bounds checking! */
      
      ent->lat = atof(gtk_entry_get_text(GTK_ENTRY(lat_entry)));
      ent->lon = atof(gtk_entry_get_text(GTK_ENTRY(lon_entry)));
      strcpy(ent->name,   gtk_entry_get_text(GTK_ENTRY(name_entry)));
      strcpy(ent->info, gtk_entry_get_text(GTK_ENTRY(info_entry)));
      
      /* Always add new entries to the private database. */
        addToGenDB(local_user_generic, ent);
      
      free(ent); 
    }

  return TRUE;
}

static gint helpbutton_callback(GtkWidget *wi, gpointer *data)
{
  tell_user(_("No help yet!"));

  return TRUE;
}

static gint text_change_callback(GtkWidget *wi, gpointer *data)
{
    text_has_changed = TRUE;
    return TRUE;
}

/***
 * These two from db_host_gui.c, common to db_*_gui.c
 */

extern void
add_widget_to_table(GtkWidget*, GtkTable*, gint, gint);

extern void
add_string_to_table(const char*, GtkTable*, gint, gint);

void addGen(GtkWidget* wi)
{
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *yesbutton;
  GtkWidget *nobutton;
  GtkWidget *helpbutton;
  GtkWidget *title;
  GtkWidget *frame;
  char string[100];
  
  if(guard == TRUE)
    {
      tell_user(_("You can only have one\ndatabase window at at time!"));
      return;
    }
  guard = TRUE;
  
  dialog = gtk_dialog_new();
  
  sprintf(string, _("New generic record"));
  title  = gtk_label_new(string);
  gtk_widget_show(GTK_WIDGET(title));
  
  gtk_window_set_title (GTK_WINDOW(dialog),_("Record info"));
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
    
  table  = gtk_table_new(4,2,FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(table), 10);
  
  
  add_string_to_table(_("Match string:"), GTK_TABLE(table), 0, 0);
  add_string_to_table(_("Latitude:"),     GTK_TABLE(table), 1, 0);
  add_string_to_table(_("Longitude:"),    GTK_TABLE(table), 2, 0);
  add_string_to_table(_("Info:"),         GTK_TABLE(table), 3, 0);

  name_entry = gtk_entry_new();
  lat_entry  = gtk_entry_new();
  lon_entry  = gtk_entry_new();
  info_entry = gtk_entry_new();

  g_signal_connect(G_OBJECT(name_entry), "changed",
		     G_CALLBACK(text_change_callback), NULL);
  g_signal_connect(G_OBJECT(lat_entry),  "changed",
		     G_CALLBACK(text_change_callback), NULL);
  g_signal_connect(G_OBJECT(lon_entry),  "changed",
		     G_CALLBACK(text_change_callback), NULL);
  g_signal_connect(G_OBJECT(info_entry), "changed",
		     G_CALLBACK(text_change_callback), NULL);

  add_widget_to_table(GTK_WIDGET(name_entry), GTK_TABLE(table), 0, 1);
  add_widget_to_table(GTK_WIDGET(lat_entry),  GTK_TABLE(table), 1, 1);
  add_widget_to_table(GTK_WIDGET(lon_entry),  GTK_TABLE(table), 2, 1);
  add_widget_to_table(GTK_WIDGET(info_entry), GTK_TABLE(table), 3, 1);

  gtk_widget_show(GTK_WIDGET(table));

  /* Pack everything */

  frame = gtk_frame_new (_("String matching"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_add(GTK_CONTAINER(frame), table);
  gtk_widget_show(GTK_WIDGET(frame));

  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), title, TRUE,
		     TRUE, 0);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), frame, TRUE,
		     TRUE, 0);
  
  
  yesbutton = gtk_button_new_with_label(_("OK"));
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), yesbutton,
		     TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT (yesbutton), "clicked",
		     G_CALLBACK (yesbutton_callback), dialog);
  g_signal_connect(G_OBJECT (yesbutton), "clicked",
		     G_CALLBACK (destroy_widget_callback), dialog);
  gtk_widget_set_can_default(yesbutton, TRUE);
  gtk_widget_grab_default(yesbutton);
  gtk_widget_show(yesbutton);

  nobutton = gtk_button_new_with_label(_("Cancel"));
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), nobutton,
		     TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT (nobutton), "clicked",
		      G_CALLBACK(destroy_widget_callback), dialog);
  gtk_widget_show(nobutton);

  helpbutton = gtk_button_new_with_label(_("Help"));
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_action_area(GTK_DIALOG(dialog))), helpbutton,
		     TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT (helpbutton), "clicked",
		      G_CALLBACK(helpbutton_callback), dialog);
  gtk_widget_show(helpbutton);

  gtk_widget_show (dialog);
}
