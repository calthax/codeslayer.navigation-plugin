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

typedef struct
{
  CodeSlayer *codeslayer;
  gint        id;  
  gchar      *text;
} Process;

static void navigation_engine_class_init         (NavigationEngineClass *klass);
static void navigation_engine_init               (NavigationEngine      *engine);
static void navigation_engine_finalize           (NavigationEngine      *engine);

static void editor_switched_action               (NavigationEngine      *engine,
                                                  CodeSlayerEditor      *editor);
static void editor_added_action                  (NavigationEngine      *engine,
                                                  CodeSlayerEditor      *editor);
static void editor_removed_action                (NavigationEngine      *engine,
                                                  CodeSlayerEditor      *editor);
static void previous_action                      (NavigationEngine      *engine);
static void next_action                          (NavigationEngine      *engine);
static void remove_duplicates                    (NavigationEngine      *engine);
static void add_editor                           (NavigationEngine      *engine,
                                                  CodeSlayerEditor      *editor);
static gint get_editor_count_within_position     (NavigationEngine      *engine, 
                                                  CodeSlayerEditor      *editor);
static gint get_duplicate_count_within_position  (NavigationEngine      *engine, 
                                                  CodeSlayerEditor      *editor);
static void print_list                           (NavigationEngine      *engine);
                            
#define NAVIGATION_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_ENGINE_TYPE, NavigationEnginePrivate))

typedef struct _NavigationEnginePrivate NavigationEnginePrivate;

struct _NavigationEnginePrivate
{
  CodeSlayer *codeslayer;
  gulong      editor_switched_id;
  gulong      editor_added_id;
  gulong      editor_removed_id;
  GList      *list;
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
  priv->list = NULL;
}

static void
navigation_engine_finalize (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_switched_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_added_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_removed_id);
  
  if (priv->list != NULL)
    g_list_free (priv->list);
  
  G_OBJECT_CLASS (navigation_engine_parent_class)->finalize (G_OBJECT(engine));
}

NavigationEngine*
navigation_engine_new (CodeSlayer *codeslayer, 
                       GtkWidget  *menu)
{
  NavigationEnginePrivate *priv;
  NavigationEngine *engine;
  GList *editors;
  GList *tmp;

  engine = NAVIGATION_ENGINE (g_object_new (navigation_engine_get_type (), NULL));
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  
  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), engine);
                            
  priv->editor_switched_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-switched", 
                                                       G_CALLBACK (editor_switched_action), engine);

  priv->editor_added_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-added", 
                                                    G_CALLBACK (editor_added_action), engine);

  priv->editor_removed_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-removed", 
                                                      G_CALLBACK (editor_removed_action), engine);
                                                      
  editors = codeslayer_get_all_editors (codeslayer);
  tmp = editors;
  
  while (tmp != NULL)
    {
      CodeSlayerEditor *editor = tmp->data;
      add_editor (NAVIGATION_ENGINE (engine), editor);
      tmp = g_list_next (tmp);
    }
  
  g_list_free (editors);      
    
  print_list (engine);

  return engine;
}

static void
editor_switched_action (NavigationEngine *engine,
                        CodeSlayerEditor *editor)
{
  g_print ("editor_switched_action\n");

  add_editor (engine, editor);
  remove_duplicates (engine);

  print_list (engine);
}                                                        

static void
editor_added_action (NavigationEngine *engine,
                     CodeSlayerEditor *editor)
{
  g_print ("editor_added_action\n");

  add_editor (engine, editor);
  remove_duplicates (engine);

  print_list (engine);
}                                                        

static void
editor_removed_action (NavigationEngine *engine,
                       CodeSlayerEditor *editor)
{
  NavigationEnginePrivate *priv;
  gint count;

  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  g_print ("editor_removed_action\n");

  count = get_editor_count_within_position (engine, editor);
  priv->position = priv->position - count;
  g_print ("editor_count_within_position - %d\n", count);
  
  priv->list = g_list_remove_all (priv->list, editor);

  print_list (engine);

  count = get_duplicate_count_within_position (engine, editor);
  g_print ("duplicate_count_within_position - %d\n", count);
  priv->position = priv->position - count;

  remove_duplicates (engine);

  print_list (engine);
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
  
  editor = g_list_nth_data (priv->list, priv->position);
  
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  
  g_signal_handler_block (priv->codeslayer, priv->editor_switched_id);
  codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, 0);
  g_signal_handler_unblock (priv->codeslayer, priv->editor_switched_id);

  g_object_unref (document);

  print_list (engine);
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
  
  length = g_list_length (priv->list);
  
  if (priv->position >= length - 1)
    return;
  
  g_print ("next_action\n");

  priv->position = priv->position + 1;
  
  editor = g_list_nth_data (priv->list, priv->position);
  
  document = codeslayer_editor_get_document (editor);
  file_path = codeslayer_document_get_file_path (document);
  
  g_signal_handler_block (priv->codeslayer, priv->editor_switched_id);
  codeslayer_select_editor_by_file_path (priv->codeslayer, file_path, 0);
  g_signal_handler_unblock (priv->codeslayer, priv->editor_switched_id);
  
  g_object_unref (document);

  print_list (engine);
}

static void
add_editor (NavigationEngine *engine,
            CodeSlayerEditor *editor)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  priv->list = g_list_append (priv->list, editor);
  priv->position = g_list_length (priv->list) - 1;
}                                                        

static gint
get_editor_count_within_position (NavigationEngine *engine, 
                                  CodeSlayerEditor *editor)
{
  NavigationEnginePrivate *priv;
  gint result = 0;
  gint i = 0;

  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  for (; i <= priv->position; i++)
    {
      CodeSlayerEditor *current = g_list_nth_data (priv->list, i);
      if (current == editor)
        result++;
    }    

  return result;    
}

static gint
get_duplicate_count_within_position (NavigationEngine *engine, 
                                     CodeSlayerEditor *editor)
{
  NavigationEnginePrivate *priv;
  gint result = 0;
  gint i = 0;

  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  for (; i <= priv->position - 1; i++)
    {
      CodeSlayerEditor *current = g_list_nth_data (priv->list, i);
      CodeSlayerEditor *next = g_list_nth_data (priv->list, i+1);
      if (current == next)
        result++;
    }

  return result;    
}

static void
remove_duplicates (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  gint length;
  guint i = 0;
  GList *clean_list = NULL;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  length = g_list_length (priv->list);
  
  if (length < 2)
    return;
  
  g_print ("remove_duplicates\n");

  for (; i < length-1; ++i)
    {
      CodeSlayerEditor *current = g_list_nth_data (priv->list, i);
      CodeSlayerEditor *next = g_list_nth_data (priv->list, i+1);

      if (current != next)
        clean_list = g_list_append (clean_list, current);
        
      if (length-1 == i+1)
        clean_list = g_list_append (clean_list, next);
    }

  g_list_free (priv->list);
  priv->list = clean_list;
}

static void
print_list (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  gint length;
  guint i = 0;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  g_print ("****** list ********\n");
  
  length = g_list_length (priv->list);
  
  for (; i < length; ++i)
    {
      CodeSlayerEditor *editor = g_list_nth_data (priv->list, i);
      const gchar *file_path;
      file_path = codeslayer_editor_get_file_path (editor);
      if (priv->position == i)
        g_print ("%s *\n", file_path);
      else
        g_print ("%s\n", file_path);
    }
    
  g_print ("**************************\n");
}
