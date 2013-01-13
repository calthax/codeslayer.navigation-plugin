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
#include "navigation-pane.h"
#include "navigation-node.h"

static void navigation_engine_class_init  (NavigationEngineClass *klass);
static void navigation_engine_init        (NavigationEngine      *engine);
static void navigation_engine_finalize    (NavigationEngine      *engine);

static void editor_navigated_action       (NavigationEngine      *engine,
                                           CodeSlayerEditor      *from_editor,
                                           CodeSlayerEditor      *to_editor);
static void previous_action               (NavigationEngine      *engine);
static void next_action                   (NavigationEngine      *engine);
static void select_position_action        (NavigationEngine      *engine, 
                                           gint                   position);
static void clear_path                    (NavigationEngine      *engine);

#define NAVIGATION_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_ENGINE_TYPE, NavigationEnginePrivate))

typedef struct _NavigationEnginePrivate NavigationEnginePrivate;

struct _NavigationEnginePrivate
{
  CodeSlayer *codeslayer;
  GtkWidget  *pane;
  gulong      editor_switched_id;
  gulong      editor_navigated_id;
  GList      *path;
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
}

static void
navigation_engine_finalize (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_navigated_id);

  g_list_foreach (priv->path, (GFunc) g_object_unref, NULL);
  
  if (priv->path != NULL)
    g_list_free (priv->path);
  
  G_OBJECT_CLASS (navigation_engine_parent_class)->finalize (G_OBJECT (engine));
}

NavigationEngine*
navigation_engine_new (CodeSlayer *codeslayer, 
                       GtkWidget  *menu, 
                       GtkWidget  *pane)
{
  NavigationEnginePrivate *priv;
  NavigationEngine *engine;

  engine = NAVIGATION_ENGINE (g_object_new (navigation_engine_get_type (), NULL));
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  priv->pane = pane;
  
  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), engine);
                            
  g_signal_connect_swapped (G_OBJECT (pane), "select-position", 
                            G_CALLBACK (select_position_action), engine);
                            
  priv->editor_navigated_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-navigated", 
                                                        G_CALLBACK (editor_navigated_action), engine);
                                                      
  return engine;
}

static NavigationNode*
create_node (CodeSlayerEditor *editor)
{
  NavigationNode *node;
  CodeSlayerDocument *document;

  node = navigation_node_new ();
  
  document = codeslayer_editor_get_document (editor);
  
  navigation_node_set_file_path (node, codeslayer_editor_get_file_path (editor)); 
  navigation_node_set_line_number (node, codeslayer_document_get_line_number (document)); 

  return node;
}

static void
clear_forward_positions (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  gint i;
  gint length;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  i = priv->position + 1;
  length = g_list_length (priv->path);
  
  for (; i < length; i++)
    {
      NavigationNode *node = g_list_nth_data (priv->path, i);
      priv->path = g_list_remove (priv->path, node);
      g_object_unref (node);
    }
}

static gboolean
editors_equal (CodeSlayerEditor *from_editor,
               CodeSlayerEditor *to_editor)
{
  return g_strcmp0 (codeslayer_editor_get_file_path (from_editor), 
                    codeslayer_editor_get_file_path (to_editor)) == 0;
}

static gboolean
nodes_equal (NavigationNode *from_node,
             NavigationNode *to_node)
{
  return g_strcmp0 (navigation_node_get_file_path (from_node), 
                    navigation_node_get_file_path (to_node)) == 0;
}

static void
editor_navigated_action (NavigationEngine *engine,
                         CodeSlayerEditor *from_editor,
                         CodeSlayerEditor *to_editor)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->path == NULL)
    {
      if (!editors_equal (from_editor, to_editor))
        {
          priv->path = g_list_append (priv->path, create_node (from_editor));
          priv->position = 0;
        }
    }
  else
    {
      NavigationNode *curr_node;
      NavigationNode *from_node;
    
      clear_forward_positions (engine);
      
      curr_node = g_list_nth_data (priv->path, priv->position);
      from_node = create_node (from_editor);
      
      if (!nodes_equal (curr_node, from_node))
        {
          clear_path (engine);

          if (!editors_equal (from_editor, to_editor))
            {
              priv->path = g_list_append (priv->path, create_node (from_editor));
              priv->position = 0;
            }
        }
      
      g_object_unref (from_node);      
    }
  
  priv->path = g_list_append (priv->path, create_node (to_editor));
  priv->position = g_list_length (priv->path) - 1;

  navigation_pane_refresh_path (NAVIGATION_PANE (priv->pane), priv->path, priv->position);
}

static void
previous_action (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  NavigationNode *node;
  const gchar *file_path;
  gint line_number;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->position <= 0)
    return;
  
  priv->position = priv->position - 1;
  
  node = g_list_nth_data (priv->path, priv->position);
  
  file_path = navigation_node_get_file_path (node);
  line_number = navigation_node_get_line_number (node);
  
  codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, line_number);

  navigation_pane_refresh_path (NAVIGATION_PANE (priv->pane), priv->path, priv->position);
}

static void
next_action (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  NavigationNode *node;
  const gchar *file_path;
  gint line_number;
  gint length;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  length = g_list_length (priv->path);
  
  if (priv->position >= length - 1)
    return;
  
  priv->position = priv->position + 1;
  
  node = g_list_nth_data (priv->path, priv->position);
  
  file_path = navigation_node_get_file_path (node);
  line_number = navigation_node_get_line_number (node);
  
  if (codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, line_number))
    navigation_pane_refresh_path (NAVIGATION_PANE (priv->pane), priv->path, priv->position);
  else
    clear_path (engine);
}

static void
select_position_action (NavigationEngine *engine, 
                        gint              position)
{
  NavigationEnginePrivate *priv;
  NavigationNode *node;
  const gchar *file_path;
  gint line_number;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  priv->position = position;
  
  node = g_list_nth_data (priv->path, priv->position);
  
  file_path = navigation_node_get_file_path (node);
  line_number = navigation_node_get_line_number (node);
  
  if (codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, line_number))
    navigation_pane_refresh_path (NAVIGATION_PANE (priv->pane), priv->path, priv->position);
  else
    clear_path (engine);
}

static void
clear_path (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  g_list_foreach (priv->path, (GFunc) g_object_unref, NULL);
  g_list_free (priv->path);
  priv->path = NULL;
  priv->position = 0;
}
