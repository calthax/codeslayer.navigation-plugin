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

#include "navigation-engine.h"

static void navigation_engine_class_init  (NavigationEngineClass *klass);
static void navigation_engine_init        (NavigationEngine      *engine);
static void navigation_engine_finalize    (NavigationEngine      *engine);

static void editor_switched_action        (NavigationEngine      *engine,
                                           CodeSlayerEditor      *editor);
static void editor_removed_action         (NavigationEngine      *engine,
                                           CodeSlayerEditor      *editor);
static void editor_navigated_action       (NavigationEngine      *engine,
                                           CodeSlayerEditor      *editor);
static void previous_action               (NavigationEngine      *engine);
static void next_action                   (NavigationEngine      *engine);
static void rank_action                   (NavigationEngine      *engine);
static void print_path                    (NavigationEngine      *engine);
static void print_rank                    (NavigationEngine      *engine);
static void add_rank_editor               (CodeSlayerEditor      *editor, 
                                           NavigationEngine      *engine);
                            
#define NAVIGATION_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_ENGINE_TYPE, NavigationEnginePrivate))

typedef struct _NavigationEnginePrivate NavigationEnginePrivate;

struct _NavigationEnginePrivate
{
  CodeSlayer *codeslayer;
  gulong      editor_removed_id;
  gulong      editor_switched_id;
  gulong      editor_navigated_id;
  GList      *path;
  GList      *rank;
  gint        position;
};

G_DEFINE_TYPE (NavigationEngine, navigation_engine, G_TYPE_OBJECT)

static void
navigation_engine_class_init (NavigationEngineClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) navigation_engine_finalize;
  g_type_class_add_private (klass, sizeof (NavigationEnginePrivate));
}

static void
navigation_engine_init (NavigationEngine *engine) 
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  priv->path = NULL;
  priv->rank = NULL;
}

static void
navigation_engine_finalize (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_switched_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_removed_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_navigated_id);
  
  if (priv->path != NULL)
    g_list_free (priv->path);
  
  if (priv->rank != NULL)
    g_list_free (priv->rank);
  
  G_OBJECT_CLASS (navigation_engine_parent_class)->finalize (G_OBJECT (engine));
}

NavigationEngine*
navigation_engine_new (CodeSlayer *codeslayer, 
                       GtkWidget  *menu)
{
  NavigationEnginePrivate *priv;
  NavigationEngine *engine;
  GList *editors;

  engine = NAVIGATION_ENGINE (g_object_new (navigation_engine_get_type (), NULL));
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  
  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), engine);
                            
  g_signal_connect_swapped (G_OBJECT (menu), "rank", 
                            G_CALLBACK (rank_action), engine);
                            
  priv->editor_switched_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-switched", 
                                                       G_CALLBACK (editor_switched_action), engine);

  priv->editor_removed_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-removed", 
                                                       G_CALLBACK (editor_removed_action), engine);

  priv->editor_navigated_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-navigated", 
                                                      G_CALLBACK (editor_navigated_action), engine);

  editors = codeslayer_get_all_editors (codeslayer);
  if (editors != NULL)
    g_list_foreach (editors, (GFunc) add_rank_editor, NAVIGATION_ENGINE (engine));
                                                      
  return engine;
}

static void
add_rank_editor (CodeSlayerEditor *editor, 
                 NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  priv->rank = g_list_append (priv->rank, editor);
}

static void
editor_switched_action (NavigationEngine *engine,
                        CodeSlayerEditor *editor)
{
  NavigationEnginePrivate *priv;  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->rank == NULL)
    {
      priv->rank = g_list_append (priv->rank, editor);
    }
  else
    {
      priv->rank = g_list_remove (priv->rank, editor);
      priv->rank = g_list_prepend (priv->rank, editor);        
    }

  print_rank (engine);
}

static void
editor_removed_action (NavigationEngine *engine,
                       CodeSlayerEditor *editor)
{
  NavigationEnginePrivate *priv;  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->rank != NULL)
    priv->rank = g_list_remove (priv->rank, editor);

  print_rank (engine);
}

static void
editor_navigated_action (NavigationEngine *engine,
                         CodeSlayerEditor *editor)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  priv->path = g_list_append (priv->path, editor);
  priv->position = g_list_length (priv->path) - 1;

  print_path (engine);
}

static void
previous_action (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  CodeSlayerEditor *editor;
  CodeSlayerDocument *document;
  const gchar *file_path;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->position <= 0)
    return;
  
  g_print ("previous_action\n");

  priv->position = priv->position - 1;
  
  editor = g_list_nth_data (priv->path, priv->position);
  
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  
  g_signal_handler_block (priv->codeslayer, priv->editor_switched_id);
  codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, 0);
  g_signal_handler_unblock (priv->codeslayer, priv->editor_switched_id);

  print_path (engine);
}

static void
next_action (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  CodeSlayerEditor *editor;
  CodeSlayerDocument *document;
  const gchar *file_path;
  gint length;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  length = g_list_length (priv->path);
  
  if (priv->position >= length - 1)
    return;
  
  g_print ("next_action\n");

  priv->position = priv->position + 1;
  
  editor = g_list_nth_data (priv->path, priv->position);
  
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  
  g_signal_handler_block (priv->codeslayer, priv->editor_switched_id);
  codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, 0);
  g_signal_handler_unblock (priv->codeslayer, priv->editor_switched_id);
  
  print_path (engine);
}

static void
rank_action (NavigationEngine *engine)
{
  print_rank (engine);
}

static void
print_path (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  gint length;
  guint i = 0;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
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

static void
print_rank (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  gint length;
  guint i = 0;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  g_print ("****** rank ********\n");
  
  length = g_list_length (priv->rank);
  
  for (; i < length; ++i)
    {
      CodeSlayerEditor *editor = g_list_nth_data (priv->rank, i);
      const gchar *file_path;
      file_path = codeslayer_editor_get_file_path (editor);
      g_print ("%s\n", file_path);
    }
    
  g_print ("**************************\n");
}
