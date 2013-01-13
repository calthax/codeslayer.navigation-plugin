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

#ifndef __NAVIGATION_PANE_H__
#define	__NAVIGATION_PANE_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define NAVIGATION_PANE_TYPE            (navigation_pane_get_type ())
#define NAVIGATION_PANE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NAVIGATION_PANE_TYPE, NavigationPane))
#define NAVIGATION_PANE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NAVIGATION_PANE_TYPE, NavigationPaneClass))
#define IS_NAVIGATION_PANE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NAVIGATION_PANE_TYPE))
#define IS_NAVIGATION_PANE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NAVIGATION_PANE_TYPE))

typedef struct _NavigationPane NavigationPane;
typedef struct _NavigationPaneClass NavigationPaneClass;

struct _NavigationPane
{
  GtkVBox parent_instance;
};

struct _NavigationPaneClass
{
  GtkVBoxClass parent_class;

  void (*select_position) (NavigationPane *pane);
};

GType navigation_pane_get_type (void) G_GNUC_CONST;
     
GtkWidget*  navigation_pane_new           (CodeSlayer     *codeslayer);

void        navigation_pane_refresh_path  (NavigationPane *pane, 
                                           GList          *path, 
                                           gint            position);

G_END_DECLS

#endif /* __NAVIGATION_PANE_H__ */
