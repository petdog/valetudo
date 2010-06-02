/* X-Chat
 * Copyright (C) 1998 Peter Zelezny.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "fe-gtk.h"

#include <gtk/gtkmain.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkprogressbar.h>
#include <gtk/gtkbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkversion.h>

#include "../common/xchat.h"
#include "../common/fe.h"
#include "../common/util.h"
#include "../common/text.h"
#include "../common/cfgfiles.h"
#include "../common/xchatc.h"
#include "../common/plugin.h"
#include "gtkutil.h"
#include "maingui.h"
#include "pixmaps.h"
#include "joind.h"
#include "xtext.h"
#include "palette.h"
#include "menu.h"
#include "notifygui.h"
#include "textgui.h"
#include "fkeys.h"
#include "plugin-tray.h"
#include "urlgrab.h"

#ifdef USE_XLIB
#include <gdk/gdkx.h>
#include <gtk/gtkinvisible.h>
#endif

#ifdef USE_GTKSPELL
#include <gtk/gtktextview.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

GdkPixmap *channelwin_pix;


/* === command-line parameter parsing : requires glib 2.6 === */

static char *arg_cfgdir = NULL;
static gint arg_show_autoload = 0;
static gint arg_show_config = 0;
static gint arg_show_version = 0;

static const GOptionEntry gopt_entries[] = 
{
 {"no-auto",	'a', 0, G_OPTION_ARG_NONE,	&arg_dont_autoconnect, N_("Don't auto connect to servers"), NULL},
 {"cfgdir",	'd', 0, G_OPTION_ARG_STRING,	&arg_cfgdir, N_("Use a different config directory"), "PATH"},
 {"no-plugins",	'n', 0, G_OPTION_ARG_NONE,	&arg_skip_plugins, N_("Don't auto load any plugins"), NULL},
 {"plugindir",	'p', 0, G_OPTION_ARG_NONE,	&arg_show_autoload, N_("Show plugin auto-load directory"), NULL},
 {"configdir",	'u', 0, G_OPTION_ARG_NONE,	&arg_show_config, N_("Show user config directory"), NULL},
 {"url",	 0,  0, G_OPTION_ARG_STRING,	&arg_url, N_("Open an irc://server:port/channel URL"), "URL"},
#ifndef WIN32	/* uses DBUS */
 {"command",	'c', 0, G_OPTION_ARG_STRING,	&arg_command, N_("Execute command:"), "COMMAND"},
 {"existing",	'e', 0, G_OPTION_ARG_NONE,	&arg_existing, N_("Open URL or execute command in an existing XChat"), NULL},
#endif
 {"version",	'v', 0, G_OPTION_ARG_NONE,	&arg_show_version, N_("Show version information"), NULL},
 {NULL}
};

int
fe_args (int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new (NULL);
	g_option_context_add_main_entries (context, gopt_entries, GETTEXT_PACKAGE);
	g_option_context_add_group (context, gtk_get_option_group (FALSE));
	g_option_context_parse (context, &argc, &argv, &error);

	if (error)
	{
		if (error->message)
			printf ("%s\n", error->message);
		return 1;
	}

	g_option_context_free (context);

	if (arg_show_version)
	{
		printf (PACKAGE_TARNAME" "PACKAGE_VERSION"\n");
		return 0;
	}

	if (arg_show_autoload)
	{
#ifdef WIN32
		/* see the chdir() below */
		char *sl, *exe = strdup (argv[0]);
		sl = strrchr (exe, '\\');
		if (sl)
		{
			*sl = 0;
			printf ("%s\\plugins\n", exe);
		}
#else
		printf ("%s\n", XCHATLIBDIR"/plugins");
#endif
		return 0;
	}

	if (arg_show_config)
	{
		printf ("%s\n", get_xdir_fs ());
		return 0;
	}

#ifdef WIN32
	/* this is mainly for irc:// URL handling. When windows calls us from */
	/* I.E, it doesn't give an option of "Start in" directory, like short */
	/* cuts can. So we have to set the current dir manually, to the path  */
	/* of the exe. */
	{
		char *tmp = strdup (argv[0]);
		char *sl;

		sl = strrchr (tmp, '\\');
		if (sl)
		{
			*sl = 0;
			chdir (tmp);
		}
		free (tmp);
	}
#endif

	if (arg_cfgdir)	/* we want filesystem encoding */
	{
		xdir_fs = strdup (arg_cfgdir);
		if (xdir_fs[strlen (xdir_fs) - 1] == '/')
			xdir_fs[strlen (xdir_fs) - 1] = 0;
		g_free (arg_cfgdir);
	}

	gtk_init (&argc, &argv);

#ifdef USE_XLIB
	gdk_window_set_events (gdk_get_default_root_window (), GDK_PROPERTY_CHANGE_MASK);
	//gdk_window_add_filter (gdk_get_default_root_window (), (GdkFilterFunc)root_event_cb, NULL);
#endif

	return -1;
}

char *cursor_color_rc =
	"style \"xc-ib-st\""
	"{"
#ifdef USE_GTKSPELL
		"GtkTextView::cursor-color=\"#%02x%02x%02x\""
#else
		"GtkEntry::cursor-color=\"#%02x%02x%02x\""
#endif
	"}"
	"widget \"*.xchat-inputbox\" style : application \"xc-ib-st\"";

void fe_main(void){gtk_main();}
void fe_cleanup (void){}
void fe_exit (void){gtk_main_quit();}
int fe_timeout_add (int interval, void *callback, void *userdata)
{
	// for some obscure reason it works. but you should investigate into 
	// not dropping userdata.
	return vala_fe_timeout_add(interval, callback, userdata);
}
int fe_input_add (int sok, int flags, void *func, void *data) {
	// same stuff as above
	vala_fe_input_add(sok, flags, func, data);
}

void
fe_new_server (struct server *serv)
{
	// this stuff is difficult to port in vala
	serv->gui = malloc (sizeof (struct server_gui));
	memset (serv->gui, 0, sizeof (struct server_gui));
}

void fe_idle_add (void *func, void *data){g_idle_add (func, data);}

#ifndef WIN32
static int
lastlog_regex_cmp (char *a, regex_t *reg)
{
	return !regexec (reg, a, 1, NULL, REG_NOTBOL);
}
#endif

void
fe_lastlog (session *sess, session *lastlog_sess, char *sstr, gboolean regexp)
{
#ifndef WIN32
	regex_t reg;
#endif

	if (gtk_xtext_is_empty (sess->res->buffer))
	{
		PrintText (lastlog_sess, _("Search buffer is empty.\n"));
		return;
	}

	if (!regexp)
	{
		gtk_xtext_lastlog (lastlog_sess->res->buffer, sess->res->buffer,
								 (void *) nocasestrstr, sstr);
		return;
	}

#ifndef WIN32
	if (regcomp (&reg, sstr, REG_ICASE | REG_EXTENDED | REG_NOSUB) == 0)
	{
		gtk_xtext_lastlog (lastlog_sess->res->buffer, sess->res->buffer,
								 (void *) lastlog_regex_cmp, &reg);
		regfree (&reg);
	}
#endif
}

void
fe_set_lag (server *serv, int lag)
{
	GSList *list = sess_list;
	session *sess;
	gdouble per;
	char lagtext[64];
	char lagtip[128];
	unsigned long nowtim;

	if (lag == -1)
	{
		if (!serv->lag_sent)
			return;
		nowtim = make_ping_time ();
		lag = (nowtim - serv->lag_sent) / 100000;
	}

	per = (double)((double)lag / (double)10);
	if (per > 1.0)
		per = 1.0;

	snprintf (lagtext, sizeof (lagtext) - 1, "%s%d.%ds",
				 serv->lag_sent ? "+" : "", lag / 10, lag % 10);
	snprintf (lagtip, sizeof (lagtip) - 1, "Lag: %s%d.%d seconds",
				 serv->lag_sent ? "+" : "", lag / 10, lag % 10);

	while (list)
	{
		sess = list->data;
		if (sess->server == serv)
		{
			if (sess->res->lag_tip)
				free (sess->res->lag_tip);
			sess->res->lag_tip = strdup (lagtip);

			if (!sess->gui->is_tab || current_tab == sess)
			{
				if (sess->gui->lagometer)
				{
					gtk_progress_bar_set_fraction ((GtkProgressBar *) sess->gui->lagometer, per);
					add_tip (sess->gui->lagometer->parent, lagtip);
				}
				if (sess->gui->laginfo)
					gtk_label_set_text ((GtkLabel *) sess->gui->laginfo, lagtext);
			} else
			{
				sess->res->lag_value = per;
				if (sess->res->lag_text)
					free (sess->res->lag_text);
				sess->res->lag_text = strdup (lagtext);
			}
		}
		list = list->next;
	}
}

void
fe_set_throttle (server *serv)
{
	GSList *list = sess_list;
	struct session *sess;
	float per;
	char tbuf[96];
	char tip[160];

	per = (float) serv->sendq_len / 1024.0;
	if (per > 1.0)
		per = 1.0;

	while (list)
	{
		sess = list->data;
		if (sess->server == serv)
		{
			snprintf (tbuf, sizeof (tbuf) - 1, _("%d bytes"), serv->sendq_len);
			snprintf (tip, sizeof (tip) - 1, _("Network send queue: %d bytes"), serv->sendq_len);

			if (sess->res->queue_tip)
				free (sess->res->queue_tip);
			sess->res->queue_tip = strdup (tip);

			if (!sess->gui->is_tab || current_tab == sess)
			{
				if (sess->gui->throttlemeter)
				{
					gtk_progress_bar_set_fraction ((GtkProgressBar *) sess->gui->throttlemeter, per);
					add_tip (sess->gui->throttlemeter->parent, tip);
				}
				if (sess->gui->throttleinfo)
					gtk_label_set_text ((GtkLabel *) sess->gui->throttleinfo, tbuf);
			} else
			{
				sess->res->queue_value = per;
				if (sess->res->queue_text)
					free (sess->res->queue_text);
				sess->res->queue_text = strdup (tbuf);
			}
		}
		list = list->next;
	}
}

void
fe_ctrl_gui (session *sess, fe_gui_action action, int arg)
{
	switch (action)
	{
	case FE_GUI_HIDE:
		gtk_widget_hide (sess->gui->window); break;
	case FE_GUI_SHOW:
		gtk_widget_show (sess->gui->window);
		gtk_window_present (GTK_WINDOW (sess->gui->window));
		break;
	case FE_GUI_FOCUS:
		mg_bring_tofront_sess (sess); break;
	case FE_GUI_FLASH:
		fe_flash_window (sess); break;
	case FE_GUI_COLOR:
		fe_set_tab_color (sess, arg); break;
	case FE_GUI_ICONIFY:
		gtk_window_iconify (GTK_WINDOW (sess->gui->window)); break;
	case FE_GUI_MENU:
		menu_bar_toggle ();	/* toggle menubar on/off */
		break;
	case FE_GUI_ATTACH:
		mg_detach (sess, arg);	/* arg: 0=toggle 1=detach 2=attach */
		break;
	case FE_GUI_APPLY:
		setup_apply_real (TRUE, TRUE);
	}
}

static void
dcc_saveas_cb (struct DCC *dcc, char *file)
{
	if (is_dcc (dcc))
	{
		if (dcc->dccstat == STAT_QUEUED)
		{
			if (file)
				dcc_get_with_destfile (dcc, file);
			else if (dcc->resume_sent == 0)
				dcc_abort (dcc->serv->front_session, dcc);
		}
	}
}

void
fe_confirm (const char *message, void (*yesproc)(void *), void (*noproc)(void *), void *ud)
{
	/* warning, assuming fe_confirm is used by DCC only! */
	struct DCC *dcc = ud;

	if (dcc->file)
		gtkutil_file_req (message, dcc_saveas_cb, ud, dcc->file,
								FRF_WRITE|FRF_FILTERISINITIAL|FRF_NOASKOVERWRITE);
}

int
fe_gui_info (session *sess, int info_type)
{
	switch (info_type)
	{
	case 0:	/* window status */
#if GTK_CHECK_VERSION(2,20,0)
		if (!gtk_widget_get_visible (GTK_WINDOW (sess->gui->window)))
#else
		if (!GTK_WIDGET_VISIBLE (GTK_WINDOW (sess->gui->window)))
#endif
			return 2;	/* hidden (iconified or systray) */
#if GTK_CHECK_VERSION(2,4,0)
		if (gtk_window_is_active (GTK_WINDOW (sess->gui->window)))
#else
#if GTK_CHECK_VERSION(2,2,0)
		if (GTK_WINDOW (sess->gui->window)->is_active)
#endif
#endif
			return 1;	/* active/focused */

		return 0;		/* normal (no keyboard focus or behind a window) */
	}

	return -1;
}

void *
fe_gui_info_ptr (session *sess, int info_type)
{
	switch (info_type)
	{
	case 0:	/* native window pointer (for plugins) */
#ifdef WIN32
		return GDK_WINDOW_HWND (sess->gui->window->window);
#else
		return sess->gui->window;
#endif
	}
	return NULL;
}

void
fe_set_inputbox_cursor (session *sess, int delta, int pos)
{
	if (!sess->gui->is_tab || sess == current_tab)
	{
		if (delta)
			pos += SPELL_ENTRY_GET_POS (sess->gui->input_box);
		SPELL_ENTRY_SET_POS (sess->gui->input_box, pos);
	} else
	{
		/* we don't support changing non-front tabs yet */
	}
}

void
fe_get_file (const char *title, char *initial,
				 void (*callback) (void *userdata, char *file), void *userdata,
				 int flags)
				
{
	/* OK: Call callback once per file, then once more with file=NULL. */
	/* CANCEL: Call callback once with file=NULL. */
	gtkutil_file_req (title, callback, userdata, initial, flags | FRF_FILTERISINITIAL);
}
