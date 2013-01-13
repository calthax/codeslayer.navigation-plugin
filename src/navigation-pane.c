/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <gtksourceview/gtksourceview.h>
#include "navigation-pane.h"
#include "navigation-node.h"

typedef struct
{
  gchar *file_path;
  gint   line_number;
  gint   start_offset;
  gint   end_offset;
} Link;

static void navigation_pane_class_init  (NavigationPaneClass *klass);
static void navigation_pane_init        (NavigationPane      *pane);
static void navigation_pane_finalize    (NavigationPane      *pane);

#define NAVIGATION_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_PANE_TYPE, NavigationPanePrivate))

typedef struct _NavigationPanePrivate NavigationPanePrivate;

struct _NavigationPanePrivate
{
  CodeSlayer    *codeslayer;
  GtkWidget     *text_view;
  GtkTextBuffer *buffer;
};

G_DEFINE_TYPE (NavigationPane, navigation_pane, GTK_TYPE_VBOX)

static void
navigation_pane_class_init (NavigationPaneClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) navigation_pane_finalize;
  g_type_class_add_private (klass, sizeof (NavigationPanePrivate));
}

static void
navigation_pane_init (NavigationPane *pane) 
{
  NavigationPanePrivate *priv;
  GtkWidget *scrolled_window;
  
  priv = NAVIGATION_PANE_GET_PRIVATE (pane);
  
  priv->text_view = gtk_source_view_new ();
  priv->buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->text_view));
  gtk_text_buffer_create_tag (priv->buffer, "header", "weight", PANGO_WEIGHT_BOLD, NULL);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), priv->text_view);

  gtk_box_pack_start (GTK_BOX (pane), scrolled_window, TRUE, TRUE, 0);
}

static void
navigation_pane_finalize (NavigationPane *pane)
{
  G_OBJECT_CLASS (navigation_pane_parent_class)->finalize (G_OBJECT(pane));
}

GtkWidget*
navigation_pane_new (CodeSlayer *codeslayer)
{
  NavigationPanePrivate *priv;
  GtkWidget *pane;

  pane = g_object_new (navigation_pane_get_type (), NULL);
  priv = NAVIGATION_PANE_GET_PRIVATE (pane);
  priv->codeslayer = codeslayer;
  
  return pane;
}                                 

void 
navigation_pane_refresh_path (NavigationPane *pane, 
                              GList          *path, 
                              gint            position)
{
  gint length;
  guint i = 0;
  
  g_print ("****** path ********\n");
  
  length = g_list_length (path);
  
  for (; i < length; ++i)
    {
      NavigationNode *node = g_list_nth_data (path, i);
      const gchar *file_path;
      gint line_number;
      file_path = navigation_node_get_file_path (node);
      line_number = navigation_node_get_line_number (node);
      if (position == i)
        g_print ("%s:%d *\n", file_path, line_number);
      else
        g_print ("%s:%d\n", file_path, line_number);
    }
    
  g_print ("**************************\n");
}
