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

#include <string.h>
#include <stdlib.h>
#include <gtksourceview/gtksourceview.h>
#include "navigation-pane.h"
#include "navigation-node.h"

static void navigation_pane_class_init  (NavigationPaneClass *klass);
static void navigation_pane_init        (NavigationPane      *pane);
static void navigation_pane_finalize    (NavigationPane      *pane);

static gchar* get_text_name             (CodeSlayer          *codeslayer, 
                                         const gchar         *file_path,
                                         gint                 line_number);
static gboolean select_path             (NavigationPane      *pane, 
                                         GtkTreeIter         *treeiter, 
                                         GtkTreeViewColumn   *column);

#define NAVIGATION_PANE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_PANE_TYPE, NavigationPanePrivate))

typedef struct _NavigationPanePrivate NavigationPanePrivate;

struct _NavigationPanePrivate
{
  CodeSlayer   *codeslayer;
  GtkWidget    *tree;
  GtkListStore *store;
};

enum
{
  TEXT,
  FILE_PATH,
  LINE_NUMBER,
  POSITION,
  COLUMNS
};

G_DEFINE_TYPE (NavigationPane, navigation_pane, GTK_TYPE_VBOX)

enum
{
  SELECT_POSITION,
  LAST_SIGNAL
};

static guint navigation_pane_signals[LAST_SIGNAL] = { 0 };

static void
navigation_pane_class_init (NavigationPaneClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  
  navigation_pane_signals[SELECT_POSITION] =
    g_signal_new ("select-position", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (NavigationPaneClass, select_position), 
                  NULL, NULL,
                  g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
  
  gobject_class->finalize = (GObjectFinalizeFunc) navigation_pane_finalize;
  g_type_class_add_private (klass, sizeof (NavigationPanePrivate));
}

static void
navigation_pane_init (NavigationPane *pane) 
{
  NavigationPanePrivate *priv;

  GtkWidget *tree;
  GtkListStore *store;
  GtkTreeViewColumn *column;
  GtkCellRenderer *renderer;
  GtkTreeSelection *selection;
  GtkWidget *scrolled_window;
  
  priv = NAVIGATION_PANE_GET_PRIVATE (pane);
  
  tree = gtk_tree_view_new ();
  priv->tree = tree;
  store = gtk_list_store_new (COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
  priv->store = store;
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree), GTK_TREE_MODEL (store));
  g_object_unref (store);
                           
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer, "text", TEXT, NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (tree));

  gtk_box_pack_start (GTK_BOX (pane), scrolled_window, TRUE, TRUE, 0);

  g_signal_connect_swapped (G_OBJECT (tree), "row_activated",
                            G_CALLBACK (select_path), pane);
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
  NavigationPanePrivate *priv;
  GtkTreeSelection *selection;
  GtkTreeIter iter;
  gint length;
  guint i = 0;
  
  priv = NAVIGATION_PANE_GET_PRIVATE (pane);

  if (priv->store != NULL)
    gtk_list_store_clear (priv->store);
  
  length = g_list_length (path);
  
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  
  for (; i < length; ++i)
    {
      NavigationNode *node = g_list_nth_data (path, i);
      const gchar *file_path;
      gchar *text_name;
      gint line_number;
      
      file_path = navigation_node_get_file_path (node);
      line_number = navigation_node_get_line_number (node);
      text_name = get_text_name (priv->codeslayer, file_path, line_number);

      gtk_list_store_append (priv->store, &iter);

      if (position == i)
        {
          gtk_list_store_set (priv->store, &iter,
                              TEXT, text_name, 
                              FILE_PATH, file_path, 
                              LINE_NUMBER, line_number,
                              POSITION, i, -1);

          gtk_tree_selection_select_iter (selection, &iter);
        }
      else
        {
          gtk_list_store_set (priv->store, &iter,
                              TEXT, text_name, 
                              FILE_PATH, file_path, 
                              LINE_NUMBER, line_number,
                              POSITION, i, -1);
        }
        
      g_free (text_name);
    }
}

static gchar*
get_text_name (CodeSlayer  *codeslayer, 
               const gchar *file_path,
               gint         line_number)
{
  CodeSlayerProject *project;
  const gchar *folder_path;
  const gchar *project_name;
  gchar *substr;
  gchar *result;
  
  project = codeslayer_get_project_by_file_path (codeslayer, file_path);
  project_name = codeslayer_project_get_name (project);

  folder_path = codeslayer_project_get_folder_path (project);
  
  substr = codeslayer_utils_substr (file_path, strlen(folder_path) + 1, strlen(file_path));

  result = g_strdup_printf ("%s - %s:%d", project_name, substr, line_number);
    
  g_free (substr);
  
  return result;
}

static gboolean
select_path (NavigationPane    *pane, 
             GtkTreeIter       *treeiter, 
             GtkTreeViewColumn *column)
{
  NavigationPanePrivate *priv;
  GtkTreeModel *model;
  GtkTreeIter iter;
  GtkTreeSelection *treeselection;
  
  priv = NAVIGATION_PANE_GET_PRIVATE (pane);

  treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->tree));
  if (gtk_tree_selection_get_selected (treeselection, &model, &iter))
    {
      gchar *file_path = NULL;
      gint line_number;
      gint position;

      gtk_tree_model_get (GTK_TREE_MODEL (priv->store), &iter,
                          FILE_PATH, &file_path,
                          LINE_NUMBER, &line_number,
                          POSITION, &position, -1);

      codeslayer_select_document_by_file_path (priv->codeslayer, file_path, line_number);
      
      g_signal_emit_by_name ((gpointer) pane, "select-position", position);

      g_free (file_path);
    }

  return FALSE;
}
