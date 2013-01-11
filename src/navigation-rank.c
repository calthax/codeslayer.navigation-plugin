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

#include "navigation-rank.h"

static void navigation_rank_class_init  (NavigationRankClass *klass);
static void navigation_rank_init        (NavigationRank      *rank);
static void navigation_rank_finalize    (NavigationRank      *rank);

static void editor_switched_action        (NavigationRank      *rank,
                                           CodeSlayerEditor      *editor);
static void editor_removed_action         (NavigationRank      *rank,
                                           CodeSlayerEditor      *editor);
static void rank_action                   (NavigationRank      *rank);
static void print_rank                    (NavigationRank      *rank);
static void add_rank_editor               (CodeSlayerEditor      *editor, 
                                           NavigationRank      *rank);
                            
#define NAVIGATION_RANK_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_RANK_TYPE, NavigationRankPrivate))

typedef struct _NavigationRankPrivate NavigationRankPrivate;

struct _NavigationRankPrivate
{
  CodeSlayer *codeslayer;
  gulong      editor_removed_id;
  gulong      editor_switched_id;
  GList      *rank;
};

G_DEFINE_TYPE (NavigationRank, navigation_rank, G_TYPE_OBJECT)

static void
navigation_rank_class_init (NavigationRankClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = (GObjectFinalizeFunc) navigation_rank_finalize;
  g_type_class_add_private (klass, sizeof (NavigationRankPrivate));
}

static void
navigation_rank_init (NavigationRank *rank) 
{
  NavigationRankPrivate *priv;
  priv = NAVIGATION_RANK_GET_PRIVATE (rank);
  priv->rank = NULL;
}

static void
navigation_rank_finalize (NavigationRank *rank)
{
  NavigationRankPrivate *priv;
  priv = NAVIGATION_RANK_GET_PRIVATE (rank);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_switched_id);
  g_signal_handler_disconnect (priv->codeslayer, priv->editor_removed_id);
  
  if (priv->rank != NULL)
    g_list_free (priv->rank);
  
  G_OBJECT_CLASS (navigation_rank_parent_class)->finalize (G_OBJECT (rank));
}

NavigationRank*
navigation_rank_new (CodeSlayer *codeslayer, 
                     GtkWidget  *menu)
{
  NavigationRankPrivate *priv;
  NavigationRank *rank;
  GList *editors;

  rank = NAVIGATION_RANK (g_object_new (navigation_rank_get_type (), NULL));
  priv = NAVIGATION_RANK_GET_PRIVATE (rank);

  priv->codeslayer = codeslayer;
  
  g_signal_connect_swapped (G_OBJECT (menu), "rank", 
                            G_CALLBACK (rank_action), rank);
                            
  priv->editor_switched_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-switched", 
                                                       G_CALLBACK (editor_switched_action), rank);

  priv->editor_removed_id = g_signal_connect_swapped (G_OBJECT (codeslayer), "editor-removed", 
                                                       G_CALLBACK (editor_removed_action), rank);

  editors = codeslayer_get_all_editors (codeslayer);
  if (editors != NULL)
    g_list_foreach (editors, (GFunc) add_rank_editor, NAVIGATION_RANK (rank));
                                                      
  return rank;
}

CodeSlayerEditor*  
navigation_rank_get_prev_editor (NavigationRank *rank)
{
  NavigationRankPrivate *priv;
  GList *list;

  priv = NAVIGATION_RANK_GET_PRIVATE (rank);
  
  list = g_list_first (priv->rank);
  
  if (list != NULL)
    list = g_list_next (priv->rank);
  
  if (list == NULL)
    return NULL;
    
  return list->data;
}

static void
add_rank_editor (CodeSlayerEditor *editor, 
                 NavigationRank   *rank)
{
  NavigationRankPrivate *priv;
  priv = NAVIGATION_RANK_GET_PRIVATE (rank);
  priv->rank = g_list_prepend (priv->rank, editor);
  
  print_rank (rank);
}

static void
editor_switched_action (NavigationRank   *rank,
                        CodeSlayerEditor *editor)
{
  NavigationRankPrivate *priv;  
  priv = NAVIGATION_RANK_GET_PRIVATE (rank);
  
  if (priv->rank == NULL)
    {
      priv->rank = g_list_append (priv->rank, editor);
    }
  else
    {
      priv->rank = g_list_remove (priv->rank, editor);
      priv->rank = g_list_prepend (priv->rank, editor);        
    }

  print_rank (rank);
}

static void
editor_removed_action (NavigationRank   *rank,
                       CodeSlayerEditor *editor)
{
  NavigationRankPrivate *priv;  
  priv = NAVIGATION_RANK_GET_PRIVATE (rank);
  
  if (priv->rank != NULL)
    priv->rank = g_list_remove (priv->rank, editor);

  print_rank (rank);
}

static void
rank_action (NavigationRank *rank)
{
  print_rank (rank);
}

static void
print_rank (NavigationRank *rank)
{
  NavigationRankPrivate *priv;
  gint length;
  guint i = 0;
  
  priv = NAVIGATION_RANK_GET_PRIVATE (rank);
  
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
