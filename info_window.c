/* $Id: info_window.c,v 1.4 2003/02/22 23:17:09 d3august Exp $
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

#include "xt.h"
#include <stdlib.h>
#include <string.h>

extern GtkWidget *clist; /* To change list when prev and next are pressed */

static gint text_has_changed = FALSE;
static gint guard = FALSE;
static GtkWidget *dialog;

static struct
{
  gint nr;
  
  GtkWidget *title;

  GtkWidget *name_label;
  GtkWidget *ip_label;
  GtkWidget *time_label;
  GtkWidget *dist_label;
  GtkWidget *acc_label;
  GtkWidget *lat_entry;
  GtkWidget *lon_entry;
  GtkWidget *info_entry;
} current_site;

const char *accs[]=
{
    N_("There is no info on this site."),
    N_("This info is guessed from the site's suffix."),
    N_("This info came from xtraceroutes' internal database."),
    N_("This info came from the \"networks.cache\" file from NDG software."),
    N_("This info came from the \"hosts.cache\" file from NDG software."),
    N_("This info came from the GPOS field in the DNS server."),
    N_("This info came from the LOC field in the DNS server."),
    N_("This info came from your site-wide database."),
    N_("This info came from your personal database.")
};

static gint destroy_widget_callback(GtkWidget *wi, gpointer target)
{
  if(target)
    gtk_widget_destroy(GTK_WIDGET(target));
  guard = FALSE;

  return TRUE;
}

void infowin_change_site(int site)
{
  char string[200];

  if(guard == FALSE)
    return;

  /* Discard any changes made. For now at least. */
  text_has_changed = FALSE;
  
  /* Fill in the struct */
  
  current_site.nr = site;

  sprintf(string, _("Database record for host nr %d"), current_site.nr);
  gtk_label_set_text(GTK_LABEL(current_site.title), string);

  gtk_label_set_text(GTK_LABEL(current_site.name_label), sites[site].name);

  gtk_label_set_text(GTK_LABEL(current_site.ip_label), sites[site].ip);

  sprintf(string, _("%d ms"), sites[site].time);
  gtk_label_set_text(GTK_LABEL(current_site.time_label), string);

  sprintf(string, _("%lu km"), distance(site)/1000);
  gtk_label_set_text(GTK_LABEL(current_site.dist_label), string);

  gtk_label_set_text(GTK_LABEL(current_site.acc_label), _(accs[sites[site].accuracy]));

  sprintf(string, "%f",sites[site].lat);
  gtk_entry_set_text(GTK_ENTRY(current_site.lat_entry), string);

  sprintf(string, "%f",sites[site].lon);
  gtk_entry_set_text(GTK_ENTRY(current_site.lon_entry), string);

  gtk_entry_set_text(GTK_ENTRY(current_site.info_entry),sites[site].info);

  text_has_changed = FALSE;

  // FIXME! Should somehow "repack" the window here, so it
  // can shrink as well as grow.
}

static gint prevbutton_callback (GtkWidget *wi, gpointer *data)
{
  if (current_site.nr > 0)
    {
      printf("prev\n");
      GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(clist));
      GtkTreeModel *model;
      GtkTreeIter iter;

      gtk_tree_selection_get_selected(selection, &model, &iter);
      GtkTreePath *path = gtk_tree_model_get_path (model, &iter);
      gtk_tree_path_prev(path);
      gtk_tree_selection_select_path(selection, path);
      // gtk_clist_moveto (GTK_CLIST(clist), current_site.nr - 1, 1, 0.5, 0.5);
      gtk_tree_path_free(path);
    }
  return TRUE;
}

static gint nextbutton_callback (GtkWidget *wi, gpointer *data)
{
  if (sites[current_site.nr+1].draw)
    {
      printf("next\n");
      GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(clist));
      GtkTreeModel *model;
      GtkTreeIter iter;

      gtk_tree_selection_get_selected(selection, &model, &iter);
      if (gtk_tree_model_iter_next(model, &iter))
          gtk_tree_selection_select_iter(selection, &iter);

      // gtk_clist_moveto (GTK_CLIST(clist), current_site.nr + 1, 1, 0.5, 0.5);
    }
  return TRUE;
}

static gint yesbutton_callback(GtkWidget *wi, gpointer *data)
{
  if(text_has_changed)
    {
      dbentry ent;
      //  tell_user(_("Info has changed!\n Updating the database is not yet supported!"));
      text_has_changed = FALSE;
      
      ent.lat = atof(gtk_entry_get_text(GTK_ENTRY(current_site.lat_entry)));
      ent.lon = atof(gtk_entry_get_text(GTK_ENTRY(current_site.lon_entry)));
      strcpy(ent.info, gtk_entry_get_text(GTK_ENTRY(current_site.info_entry)));
      strcpy(ent.name, gtk_label_get_text(GTK_LABEL(current_site.name_label)));
      strcpy(ent.ip, gtk_label_get_text(GTK_LABEL( current_site.ip_label )));
      
      addToHostDB(local_user_hosts, &ent);
    }
  return TRUE;
}

static gint helpbutton_callback(GtkWidget *wi, gpointer *data)
{
  tell_user(_("No help yet!"));

  return TRUE;
}

static gint
text_change_callback(GtkWidget *wi, gpointer *data)
{
    text_has_changed = TRUE;
    return TRUE;
}


static void 
add_widget_to_table(GtkWidget* wi, GtkTable* table, gint row, gint column)
{
  if(GTK_IS_MISC(wi))
    gtk_misc_set_alignment(GTK_MISC(wi), column ? 0.0 : 1.0, 0.5);
  gtk_table_attach_defaults(table, GTK_WIDGET(wi), column, column+1, row, row+1);
  gtk_widget_show(GTK_WIDGET(wi));
}

static void 
add_string_to_table(const char* string, GtkTable* table, gint row, gint column)
{
  GtkWidget *wi;

  wi = gtk_label_new(string);
  add_widget_to_table(GTK_WIDGET(wi), table, row, column);
}

gint info_window(gint site)
{
  GtkWidget *table;
  GtkWidget *header_hbox;
  GtkWidget *prevbutton;
  GtkWidget *nextbutton;
  GtkWidget *yesbutton;
  GtkWidget *nobutton;
  GtkWidget *helpbutton;
  GtkWidget *frame;
  char string[100];

  if(guard == TRUE)
  {
    //tell_user(_("You can only have one\ninfo window at at time!"));
    destroy_widget_callback(NULL, dialog);
    return TRUE;
  }
  guard = TRUE;

  current_site.nr = site;

  dialog = gtk_dialog_new();
  g_signal_connect (G_OBJECT (dialog), "destroy",
		      G_CALLBACK(destroy_widget_callback), NULL);
  sprintf(string, _("Database record for host nr %d"), current_site.nr);
  current_site.title = gtk_label_new(string);
  gtk_widget_show(GTK_WIDGET(current_site.title));
  
  gtk_window_set_title (GTK_WINDOW(dialog),_("Record info"));
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
  
  table  = gtk_table_new(8, 2, FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(table), 5);
  gtk_table_set_col_spacings(GTK_TABLE(table), 10);

  add_string_to_table(_("Hostname:"),          GTK_TABLE(table), 0, 0);
  add_string_to_table(_("IP address:"),        GTK_TABLE(table), 1, 0);
  add_string_to_table(_("Ping time:"),         GTK_TABLE(table), 2, 0);
  add_string_to_table(_("Minimum distance:"),  GTK_TABLE(table), 3, 0);
  add_string_to_table(_("Accuracy:"),          GTK_TABLE(table), 4, 0);
  add_string_to_table(_("Latitude:"),          GTK_TABLE(table), 5, 0);
  add_string_to_table(_("Longitude:"),         GTK_TABLE(table), 6, 0);
  add_string_to_table(_("Info:"),              GTK_TABLE(table), 7, 0);


  current_site.name_label = gtk_label_new(sites[site].name);

  current_site.ip_label   = gtk_label_new(sites[site].ip);

  sprintf(string, _("%d ms"), sites[site].time);
  current_site.time_label = gtk_label_new(string);

  sprintf(string, _("%lu km"), distance(site)/1000);
  current_site.dist_label = gtk_label_new(string);
  current_site.acc_label  = gtk_label_new(_(accs[sites[site].accuracy]));
  
  current_site.lat_entry =  gtk_entry_new();
  sprintf(string, "%f",sites[site].lat);
  gtk_entry_set_text(GTK_ENTRY(current_site.lat_entry),string);
  g_signal_connect(G_OBJECT(current_site.lat_entry),"changed",
		     G_CALLBACK(text_change_callback), NULL);

  current_site.lon_entry =  gtk_entry_new();
  sprintf(string, "%f",sites[site].lon);
  gtk_entry_set_text(GTK_ENTRY(current_site.lon_entry),string);
  g_signal_connect(G_OBJECT(current_site.lon_entry),"changed",
		     G_CALLBACK(text_change_callback), NULL);

  current_site.info_entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(current_site.info_entry),sites[site].info);
  g_signal_connect(G_OBJECT(current_site.info_entry),"changed",
		     G_CALLBACK(text_change_callback), NULL);


  add_widget_to_table(GTK_WIDGET(current_site.name_label), GTK_TABLE(table), 0, 1);
  add_widget_to_table(GTK_WIDGET(current_site.ip_label),   GTK_TABLE(table), 1, 1);
  add_widget_to_table(GTK_WIDGET(current_site.time_label), GTK_TABLE(table), 2, 1);
  add_widget_to_table(GTK_WIDGET(current_site.dist_label), GTK_TABLE(table), 3, 1);
  add_widget_to_table(GTK_WIDGET(current_site.acc_label),  GTK_TABLE(table), 4, 1);
  add_widget_to_table(GTK_WIDGET(current_site.lat_entry),  GTK_TABLE(table), 5, 1);
  add_widget_to_table(GTK_WIDGET(current_site.lon_entry),  GTK_TABLE(table), 6, 1);
  add_widget_to_table(GTK_WIDGET(current_site.info_entry), GTK_TABLE(table), 7, 1);
  
  gtk_widget_show(GTK_WIDGET(table));

  /* Pack everything */

  frame = gtk_frame_new (_("Info"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_add(GTK_CONTAINER(frame), table);
  gtk_widget_show(GTK_WIDGET(frame));
  
  header_hbox = gtk_hbox_new (FALSE, FALSE);
  prevbutton = gtk_button_new_with_label(_("Prev"));
  gtk_box_pack_start (GTK_BOX(header_hbox), prevbutton, 
		      TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT(prevbutton), "clicked",
		      G_CALLBACK (prevbutton_callback), NULL);
  gtk_widget_show (prevbutton);
  gtk_box_pack_start (GTK_BOX(header_hbox), current_site.title, TRUE,
		      TRUE, 0);
  nextbutton = gtk_button_new_with_label(_("Next"));
  gtk_box_pack_start (GTK_BOX(header_hbox), nextbutton, 
		      TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT(nextbutton), "clicked",
		      G_CALLBACK (nextbutton_callback), NULL);
  gtk_widget_show (nextbutton);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), header_hbox, TRUE,
		     TRUE, 0);
  gtk_widget_show (header_hbox);
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

  return TRUE;
}
