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

#ifndef __NAVIGATION_PATH_H__
#define	__NAVIGATION_PATH_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define NAVIGATION_PATH_TYPE            (navigation_path_get_type ())
#define NAVIGATION_PATH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NAVIGATION_PATH_TYPE, NavigationPath))
#define NAVIGATION_PATH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NAVIGATION_PATH_TYPE, NavigationPathClass))
#define IS_NAVIGATION_PATH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NAVIGATION_PATH_TYPE))
#define IS_NAVIGATION_PATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NAVIGATION_PATH_TYPE))

typedef struct _NavigationPath NavigationPath;
typedef struct _NavigationPathClass NavigationPathClass;

struct _NavigationPath
{
  GObject parent_instance;
};

struct _NavigationPathClass
{
  GObjectClass parent_class;
};

GType navigation_path_get_type (void) G_GNUC_CONST;

NavigationPath*  navigation_path_new  (CodeSlayer *codeslayer, 
                                       GtkWidget  *menu);

G_END_DECLS

#endif /* _NAVIGATION_PATH_H */
