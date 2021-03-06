extern GtkStyle *input_style;
extern GtkWidget *parent_window;

struct session_gui *mg_gui;

void mg_changui_new (struct session *sess, struct restore_gui *res, int tab, int focus);
void mg_update_xtext (GtkWidget *wid);
void mg_open_quit_dialog (gboolean minimize_button);
void mg_switch_page (int relative, int num);
void mg_move_tab (struct session *, int delta);
void mg_move_tab_family (struct session *, int delta);
void mg_bring_tofront (GtkWidget *vbox);
void mg_bring_tofront_sess (struct session *sess);
void mg_decide_userlist (struct session *sess, gboolean switch_to_current);
void mg_set_topic_tip (struct session *sess);
GtkWidget *mg_create_generic_tab (char *name, char *title, int force_toplevel, int link_buttons, void *close_callback, void *userdata, int width, int height, GtkWidget **vbox_ret, void *family);
void mg_set_title (GtkWidget *button, char *title);
void mg_set_access_icon (struct session_gui *gui, GdkPixbuf *pix, gboolean away);
void mg_apply_setup (void);
void mg_close_sess (struct session *);
void mg_tab_close (struct session *sess);
void mg_detach (struct session *sess, int mode);
void mg_progressbar_create (struct session_gui *gui);
void mg_progressbar_destroy (struct session_gui *gui);
void mg_dnd_drop_file (struct session *sess, char *target, char *uri);
void mg_change_layout (int type);
void mg_update_meters (struct session_gui *);
void mg_inputbox_cb (GtkWidget *igad, struct session_gui *gui);
void mg_create_icon_item (char *label, char *stock, GtkWidget *menu, void *callback, void *userdata);
GtkWidget *mg_submenu (GtkWidget *menu, char *text);
/* DND */
gboolean mg_drag_begin_cb (GtkWidget *widget, GdkDragContext *context, gpointer userdata);
void mg_drag_end_cb (GtkWidget *widget, GdkDragContext *context, gpointer userdata);
gboolean mg_drag_drop_cb (GtkWidget *widget, GdkDragContext *context, int x, int y, guint time, gpointer user_data);
gboolean mg_drag_motion_cb (GtkWidget *widget, GdkDragContext *context, int x, int y, guint time, gpointer user_data); 
