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

#include <codeslayer/codeslayer.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gmodule.h>
#include <glib.h>
#include "navigation-engine.h"
#include "navigation-menu.h"

G_MODULE_EXPORT void activate    (CodeSlayer *codeslayer);
G_MODULE_EXPORT void deactivate  (CodeSlayer *codeslayer);
G_MODULE_EXPORT void configure   (CodeSlayer *codeslayer);

NavigationEngine *engine;
GtkWidget *menu;

G_MODULE_EXPORT void
activate (CodeSlayer *codeslayer)
{
  GtkAccelGroup *accel_group;
  accel_group = codeslayer_get_menu_bar_accel_group (codeslayer);
  menu = navigation_menu_new (accel_group);
  codeslayer_add_to_menu_bar (codeslayer, GTK_MENU_ITEM (menu));  

  engine = navigation_engine_new (codeslayer, menu);
}

G_MODULE_EXPORT void 
deactivate (CodeSlayer *codeslayer)
{
  codeslayer_remove_from_menu_bar (codeslayer, GTK_MENU_ITEM (menu));
  g_object_unref (engine);
}

G_MODULE_EXPORT void 
configure  (CodeSlayer *codeslayer)
{
  navigation_engine_open_dialog (engine);
}
