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

#ifndef __NAVIGATION_MENU_H__
#define	__NAVIGATION_MENU_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define NAVIGATION_MENU_TYPE            (navigation_menu_get_type ())
#define NAVIGATION_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NAVIGATION_MENU_TYPE, NavigationMenu))
#define NAVIGATION_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NAVIGATION_MENU_TYPE, NavigationMenuClass))
#define IS_NAVIGATION_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NAVIGATION_MENU_TYPE))
#define IS_NAVIGATION_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NAVIGATION_MENU_TYPE))

typedef struct _NavigationMenu NavigationMenu;
typedef struct _NavigationMenuClass NavigationMenuClass;

struct _NavigationMenu
{
  GtkMenuItem parent_instance;
};

struct _NavigationMenuClass
{
  GtkMenuItemClass parent_class;

  void (*previous) (NavigationMenu *menu);
  void (*next) (NavigationMenu *menu);
  void (*rank) (NavigationMenu *menu);
};

GType navigation_menu_get_type (void) G_GNUC_CONST;
  
GtkWidget*  navigation_menu_new  (GtkAccelGroup *accel_group);
                                            
G_END_DECLS

#endif /* __NAVIGATION_MENU_H__ */
