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

static void navigation_engine_class_init  (NavigationEngineClass *klass);
static void navigation_engine_init        (NavigationEngine      *engine);
static void navigation_engine_finalize    (NavigationEngine      *engine);

static void previous_action           (NavigationEngine      *engine);
static void next_action               (NavigationEngine      *engine);
                            
#define NAVIGATION_ENGINE_GET_PRIVATE(obj) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((obj), NAVIGATION_ENGINE_TYPE, NavigationEnginePrivate))

typedef struct _NavigationEnginePrivate NavigationEnginePrivate;

struct _NavigationEnginePrivate
{
  CodeSlayer *codeslayer;
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
}

static void
navigation_engine_finalize (NavigationEngine *engine)
{
  G_OBJECT_CLASS (navigation_engine_parent_class)->finalize (G_OBJECT(engine));
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
  
  g_signal_connect_swapped (G_OBJECT (menu), "previous", 
                            G_CALLBACK (previous_action), engine);
  
  g_signal_connect_swapped (G_OBJECT (menu), "next", 
                            G_CALLBACK (next_action), engine);

  return engine;
}

static void
previous_action (NavigationEngine *engine)
{
  g_print ("previous_action\n");
}

static void
next_action (NavigationEngine *engine)
{
  g_print ("next_action\n");
}
