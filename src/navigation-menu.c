/*
 * Copyright (C) 2010 - Jeff Johnston
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.remove_group_item
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gdk/gdkkeysyms.h>
#include <codeslayer/codeslayer.h>
#include "navigation-menu.h"

static void navigation_menu_class_init (NavigationMenuClass *klass);
static void navigation_menu_init       (NavigationMenu      *menu);
static void navigation_menu_finalize   (NavigationMenu      *menu);

static void previous_action            (NavigationMenu      *menu);
static void next_action                (NavigationMenu      *menu);
static void add_menu_items             (NavigationMenu      *menu,
                                        GtkWidget           *submenu,
                                        GtkAccelGroup       *accel_group);
                                        
enum
{
  PREVIOUS,
  NEXT,
  LAST_SIGNAL
};

static guint navigation_menu_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (NavigationMenu, navigation_menu, GTK_TYPE_MENU_ITEM)

static void
navigation_menu_class_init (NavigationMenuClass *klass)
{
  navigation_menu_signals[PREVIOUS] =
    g_signal_new ("previous", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (NavigationMenuClass, previous),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  navigation_menu_signals[NEXT] =
    g_signal_new ("next", 
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (NavigationMenuClass, next),
                  NULL, NULL, 
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  G_OBJECT_CLASS (klass)->finalize = (GObjectFinalizeFunc) navigation_menu_finalize;
}

static void
navigation_menu_init (NavigationMenu *menu)
{
  gtk_menu_item_set_label (GTK_MENU_ITEM (menu), "Navigate");
}

static void
navigation_menu_finalize (NavigationMenu *menu)
{
  G_OBJECT_CLASS (navigation_menu_parent_class)->finalize (G_OBJECT (menu));
}

GtkWidget*
navigation_menu_new (GtkAccelGroup *accel_group)
{
  GtkWidget *menu;
  GtkWidget *submenu;
  
  menu = g_object_new (navigation_menu_get_type (), NULL);
  
  submenu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), submenu);
  
  add_menu_items (NAVIGATION_MENU (menu), submenu, accel_group);

  return menu;
}

static void
add_menu_items (NavigationMenu *menu,
                GtkWidget        *submenu,
                GtkAccelGroup    *accel_group)
{
  GtkWidget *previous_item;
  GtkWidget *next_item;

  previous_item = codeslayer_menu_item_new_with_label (_("previous"));
  gtk_widget_add_accelerator (previous_item, "activate", accel_group, 
                              GDK_KEY_comma, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE); 
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), previous_item);

  next_item = codeslayer_menu_item_new_with_label (_("next"));
  gtk_widget_add_accelerator (next_item, "activate", accel_group, 
                              GDK_KEY_period, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), next_item);
  
  g_signal_connect_swapped (G_OBJECT (previous_item), "activate", 
                            G_CALLBACK (previous_action), menu);
   
  g_signal_connect_swapped (G_OBJECT (next_item), "activate", 
                            G_CALLBACK (next_action), menu);
}

static void 
previous_action (NavigationMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "previous");
}

static void 
next_action (NavigationMenu *menu) 
{
  g_signal_emit_by_name ((gpointer) menu, "next");
}
