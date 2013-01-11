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

#include "navigation-path.h"

static void navigation_path_class_init  (NavigationPathClass *klass);
static void navigation_path_init        (NavigationPath      *path);
static void navigation_path_finalize    (NavigationPath      *path);

static void editor_removed_action         (NavigationPath      *path,
                                           CodeSlayerEditor      *editor);
static void editor_navigated_action       (NavigationPath      *path,
                                           CodeSlayerEditor      *editor);
static void previous_action               (NavigationPath      *path);
static void next_action                   (NavigationPath      *path);
static void print_path                    (NavigationPath      *path);
                            
#define NAVIGATION_PATH_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_PATH_TYPE, NavigationPathPrivate))

typedef struct _NavigationPathPrivate NavigationPathPrivate;

struct _NavigationPathPrivate
{
  CodeSlayer     *codeslayer;
  NavigationRank *rank;
  gulong          editor_removed_id;
  gulong          editor_navigated_id;
  GList          *path;
  gint            position;
};

G_DEFINE_TYPE (NavigationPath, navigation_path, G_TYPE_OBJECT)

static void
navigation_path_class_init (NavigationPathClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) navigation_path_finalize;
  g_type_class_add_private (klass, sizeof (NavigationPathPrivate));
}

static void
navigation_path_init (NavigationPath *path) 
{
  NavigationPathPrivate *priv;
  priv = NAVIGATION_PATH_GET_PRIVATE (path);
  priv->path = NULL;
}

static void
navigation_path_finalize (NavigationPath *path)
{
  NavigationPathPrivate *priv;
  priv = NAVIGATION_PATH_GET_PRIVATE (path);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_removed_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_navigated_id);
  
  if (priv->path != NULL)
    g_list_free (priv->path);
  
  G_OBJECT_CLASS (navigation_path_parent_class)->finalize (G_OBJECT (path));
}

NavigationPath*
navigation_path_new (CodeSlayer     *codeslayer, 
                     GtkWidget      *menu, 
                     NavigationRank *rank)
{
  NavigationPathPrivate *priv;
  NavigationPath *path;

  path = NAVIGATION_PATH (g_object_new (navigation_path_get_type (), NULL));
  priv = NAVIGATION_PATH_GET_PRIVATE (path);

  priv->codeslayer = codeslayer;
  priv->rank = rank;
  
  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), path);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), path);
                            
  priv->editor_removed_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-removed", 
                                                       G_CALLBACK (editor_removed_action), path);

  priv->editor_navigated_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-navigated", 
                                                      G_CALLBACK (editor_navigated_action), path);
                                                      
  return path;
}

static void
editor_removed_action (NavigationPath *path,
                       CodeSlayerEditor *editor)
{
}

static void
editor_navigated_action (NavigationPath   *path,
                         CodeSlayerEditor *editor)
{
  NavigationPathPrivate *priv;
  priv = NAVIGATION_PATH_GET_PRIVATE (path);
  
  if (priv->path == NULL)
    {
      CodeSlayerEditor *prev_editor;
      prev_editor = navigation_rank_get_prev_editor (priv->rank);
      if (prev_editor != NULL)
        {
          priv->path = g_list_append (priv->path, prev_editor);
          priv->position = g_list_length (priv->path) - 1;        
        }
    }
  
  priv->path = g_list_append (priv->path, editor);
  priv->position = g_list_length (priv->path) - 1;

  print_path (path);
}

static void
previous_action (NavigationPath *path)
{
  NavigationPathPrivate *priv;
  CodeSlayerEditor *editor;
  CodeSlayerDocument *document;
  const gchar *file_path;
  
  priv = NAVIGATION_PATH_GET_PRIVATE (path);
  
  if (priv->position <= 0)
    return;
  
  g_print ("previous_action\n");

  priv->position = priv->position - 1;
  
  editor = g_list_nth_data (priv->path, priv->position);
  
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  
  codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, 0);

  print_path (path);
}

static void
next_action (NavigationPath *path)
{
  NavigationPathPrivate *priv;
  CodeSlayerEditor *editor;
  CodeSlayerDocument *document;
  const gchar *file_path;
  gint length;
  
  priv = NAVIGATION_PATH_GET_PRIVATE (path);
  
  length = g_list_length (priv->path);
  
  if (priv->position >= length - 1)
    return;
  
  g_print ("next_action\n");

  priv->position = priv->position + 1;
  
  editor = g_list_nth_data (priv->path, priv->position);
  
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  
  codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, 0);
  
  print_path (path);
}

static void
print_path (NavigationPath *path)
{
  NavigationPathPrivate *priv;
  gint length;
  guint i = 0;
  
  priv = NAVIGATION_PATH_GET_PRIVATE (path);
  
  g_print ("****** path ********\n");
  
  length = g_list_length (priv->path);
  
  for (; i < length; ++i)
    {
      CodeSlayerEditor *editor = g_list_nth_data (priv->path, i);
      const gchar *file_path;
      file_path = codeslayer_editor_get_file_path (editor);
      if (priv->position == i)
        g_print ("%s *\n", file_path);
      else
        g_print ("%s\n", file_path);
    }
    
  g_print ("**************************\n");
}
