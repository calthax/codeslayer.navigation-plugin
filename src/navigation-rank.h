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

#ifndef __NAVIGATION_RANK_H__
#define	__NAVIGATION_RANK_H__

#include <gtk/gtk.h>
#include <codeslayer/codeslayer.h>

G_BEGIN_DECLS

#define NAVIGATION_RANK_TYPE            (navigation_rank_get_type ())
#define NAVIGATION_RANK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), NAVIGATION_RANK_TYPE, NavigationRank))
#define NAVIGATION_RANK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), NAVIGATION_RANK_TYPE, NavigationRankClass))
#define IS_NAVIGATION_RANK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), NAVIGATION_RANK_TYPE))
#define IS_NAVIGATION_RANK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), NAVIGATION_RANK_TYPE))

typedef struct _NavigationRank NavigationRank;
typedef struct _NavigationRankClass NavigationRankClass;

struct _NavigationRank
{
  GObject parent_instance;
};

struct _NavigationRankClass
{
  GObjectClass parent_class;
};

GType navigation_rank_get_type (void) G_GNUC_CONST;

NavigationRank*    navigation_rank_new                      (CodeSlayer     *codeslayer, 
                                                             GtkWidget      *menu);
                                       
CodeSlayerEditor*  navigation_rank_get_prev_editor          (NavigationRank *rank);

G_END_DECLS

#endif /* _NAVIGATION_RANK_H */
