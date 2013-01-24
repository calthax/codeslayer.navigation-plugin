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

static void path_navigated_action         (NavigationEngine      *engine,
                                           gchar                 *from_file_path,
                                           gint                   from_line_number,
                                           gchar                 *to_file_path,
                                           gint                   to_line_number);
static void previous_action               (NavigationEngine      *engine);
static void next_action                   (NavigationEngine      *engine);
static void select_position_action        (NavigationEngine      *engine, 
                                           gint                   position);
static void clear_path                    (NavigationEngine      *engine);
static void toggle_dialog_action          (GtkToggleButton       *toggle_button,
                                           NavigationEngine      *engine);
static void add_pane                     (NavigationEngine      *engine);

#define NAVIGATION_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_ENGINE_TYPE, NavigationEnginePrivate))

#define MAIN "main"
#define SHOW_SIDE_PANE "show_side_pane"

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
  priv->pane = NULL;
}

static void
navigation_engine_finalize (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->pane != NULL)
    codeslayer_remove_from_side_pane (priv->codeslayer, priv->pane);

  g_signal_handler_disconnect (priv->codeslayer, priv->editor_navigated_id);

  g_list_foreach (priv->path, (GFunc) g_object_unref, NULL);
  
  if (priv->path != NULL)
    g_list_free (priv->path);
  
  G_OBJECT_CLASS (navigation_engine_parent_class)->finalize (G_OBJECT (engine));
}

NavigationEngine*
navigation_engine_new (CodeSlayer *codeslayer, 
                       GtkWidget  *menu)
{
  NavigationEnginePrivate *priv;
  NavigationEngine *engine;

  engine = NAVIGATION_ENGINE (g_object_new (navigation_engine_get_type (), NULL));
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  priv->codeslayer = codeslayer;
  
  add_pane (engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), engine);

  priv->editor_navigated_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "path-navigated", 
                                                        G_CALLBACK (path_navigated_action), engine);
                                                      
  return engine;
}

static NavigationNode*
create_node (gchar *file_path,
             gint   line_number)
{
  NavigationNode *node;
  node = navigation_node_new ();
  navigation_node_set_file_path (node, file_path); 
  navigation_node_set_line_number (node, line_number); 
  return node;
}

static void
clear_forward_positions (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  gint length;

  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  length = g_list_length (priv->path);
  
  while (priv->position < length - 1)
    {
      NavigationNode *node = g_list_nth_data (priv->path, length - 1);
      priv->path = g_list_remove (priv->path, node);
      g_object_unref (node);
      length = g_list_length (priv->path);
    }
}

/*static gboolean
node_equals (NavigationNode *from_node,
             NavigationNode *to_node)
{
  return g_strcmp0 (navigation_node_get_file_path (from_node), 
                    navigation_node_get_file_path (to_node)) == 0;
}*/

static void
path_navigated_action (NavigationEngine *engine,
                       gchar            *from_file_path,
                       gint              from_line_number,
                       gchar            *to_file_path,
                       gint              to_line_number)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  if (priv->path == NULL)
    {
      priv->path = g_list_append (priv->path, create_node (from_file_path, from_line_number));
      priv->position = 0;
    }
  else
    {
      NavigationNode *curr_node;
    
      clear_forward_positions (engine);
      
      curr_node = g_list_nth_data (priv->path, priv->position);
      
      if (g_strcmp0 (navigation_node_get_file_path (curr_node), from_file_path) != 0)
        {
          clear_path (engine);
          priv->path = g_list_append (priv->path, create_node (from_file_path, from_line_number));
          priv->position = 0;
        }
      else
        {
          priv->path = g_list_append (priv->path, create_node (from_file_path, from_line_number));
          priv->position = g_list_length (priv->path) - 1;
        }
    }
  
  priv->path = g_list_append (priv->path, create_node (to_file_path, to_line_number));
  priv->position = g_list_length (priv->path) - 1;

  if (priv->pane != NULL)
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

  if (priv->pane != NULL)
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
    {
      if (priv->pane != NULL)
        navigation_pane_refresh_path (NAVIGATION_PANE (priv->pane), priv->path, priv->position);
    }
  else
    {
      clear_path (engine);    
    }
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
    {
      if (priv->pane != NULL)
        navigation_pane_refresh_path (NAVIGATION_PANE (priv->pane), priv->path, priv->position);
    }
  else
    {
      clear_path (engine);    
    }
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

static gboolean
show_pane (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  GKeyFile *key_file;
  gchar *folder_path;
  gchar *file_path;
  gboolean result;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  folder_path = codeslayer_get_plugins_config_folder_path (priv->codeslayer);
  
  file_path = codeslayer_utils_get_file_path (folder_path, "navigation.conf");
  
  key_file = codeslayer_utils_get_key_file (file_path);
  
  if (g_key_file_has_key (key_file, MAIN, SHOW_SIDE_PANE, NULL))
    result = g_key_file_get_boolean (key_file, MAIN, SHOW_SIDE_PANE, NULL);

  g_free (folder_path);
  g_free (file_path);
  g_key_file_free (key_file);

  return result;
}

static void
add_pane (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);

  if (show_pane (engine))
    {
      priv->pane = navigation_pane_new (priv->codeslayer);
      codeslayer_add_to_side_pane (priv->codeslayer, priv->pane, _("Navigation"));
      g_signal_connect_swapped (G_OBJECT (priv->pane), "select-position", 
                                G_CALLBACK (select_position_action), engine);
    }                            
}

void
navigation_engine_open_dialog (NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  GtkWindow *window;
  GtkWidget *dialog;
  GtkWidget *content_area;
  GtkWidget *hbox;
  GtkWidget *toggle_button;

  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  window = codeslayer_get_toplevel_window (priv->codeslayer);

  dialog = gtk_dialog_new_with_buttons (_("Path Navigation"), 
                                        GTK_WINDOW (window),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                        NULL);

  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (dialog), TRUE);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (dialog), TRUE);
  
  content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  toggle_button = gtk_check_button_new_with_label (_("Show Navigation Pane?"));
  
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle_button),
                                show_pane (engine));
  
  g_signal_connect (G_OBJECT (toggle_button), "toggled", 
                    G_CALLBACK (toggle_dialog_action), engine);

  gtk_box_pack_start (GTK_BOX (hbox), toggle_button, TRUE, TRUE, 20);
  
  gtk_container_add (GTK_CONTAINER (content_area), hbox);
  
  gtk_widget_show_all (hbox);

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

static void 
toggle_dialog_action (GtkToggleButton  *toggle_button,
                      NavigationEngine *engine)
{
  NavigationEnginePrivate *priv;
  GKeyFile *key_file;
  gboolean active;
  gchar *folder_path;
  gchar *file_path;
  
  priv = NAVIGATION_ENGINE_GET_PRIVATE (engine);
  
  active = gtk_toggle_button_get_active (toggle_button);
  
  if (active && priv->pane == NULL)
    {
      priv->pane = navigation_pane_new (priv->codeslayer);
      codeslayer_add_to_side_pane (priv->codeslayer, priv->pane, _("Navigation"));
      g_signal_connect_swapped (G_OBJECT (priv->pane), "select-position", 
                                G_CALLBACK (select_position_action), engine);
      if (priv->path != NULL)
        navigation_pane_refresh_path (NAVIGATION_PANE (priv->pane), priv->path, priv->position);                                
    }
  else if (!active && priv->pane != NULL)
    {
      codeslayer_remove_from_side_pane (priv->codeslayer, priv->pane);
      priv->pane = NULL;
    }
    
  folder_path = codeslayer_get_plugins_config_folder_path (priv->codeslayer);
  file_path = codeslayer_utils_get_file_path (folder_path, "navigation.conf");
  
  key_file = codeslayer_utils_get_key_file (file_path);
  
  g_key_file_set_boolean (key_file, MAIN, SHOW_SIDE_PANE, active);
  
  codeslayer_utils_save_key_file (key_file, file_path);

  g_free (folder_path);
  g_free (file_path);
  g_key_file_free (key_file);
}
