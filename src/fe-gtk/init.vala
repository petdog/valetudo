using XchatFrontend;
using Posix;
using GLib;

void vala_redraw_trans_xtexts () {
    var done_main = false;
    print("fu fu fu\n");
    sess_list.foreach((sess) => {
        var s = (Session*)sess;
        if (s->gui->xtext.transparent) {
            if (!s->gui->is_tab || !done_main)
                (s->gui->xtext).refresh(1);
            if (s->gui->is_tab)
                done_main = true;
        }
    });
}

void vala_fe_new_window (Session* s, int focus) {
    int tab = 0;
    if (s->type == 3 /*SESS_DIALOG*/) {
        if(prefs.privmsgtab) tab = 1;
    } else {
        if(prefs.tabchannels) tab = 1;
    }
    mg_changui_new(s, null, tab, focus);
}

void vala_fe_init () {
	palette_load ();
	key_init ();
	pixmaps_init ();

	channelwin_pix = pixmap_load_from_file (prefs.background);
	input_style = create_input_style (new Gtk.Style());
}

uint vala_fe_timeout_add (int interval, SourceFunc callback) {
    // magic from the other side. vala automatically adds a new parameter.
    return Timeout.add(interval, callback);
}

void vala_fe_timeout_remove (int tag) {
    Source.remove(tag);
}

void vala_fe_print_text (Session* s, string text, time_t time) {
	PrintTextRaw(s->res->buffer, text, prefs.indent_nicks, time);
	if (!s->new_data && s != current_tab &&
        s->gui->is_tab && !s->nick_said && time == 0) {
		s->new_data = true;
		if (s->msg_said) fe_set_tab_color(s, 2);
		else fe_set_tab_color(s, 1);
	}
}

bool try_browser (string browser, string url) {
    Pid pid;
    var path = Environment.find_program_in_path(browser);
    if (null == path) return false;
    string[] argv = {path, url, null};
    xchat_execv(argv);
    return true;
}

void vala_fe_open_url (string url) {
    Func test;
    test = (url) => {
        string u = (string)url;
        if (try_browser("xdg-open", u))
            return;
        if (try_browser("firefox", u))
            return;
    };
    if (url[0] != '/' && !url.contains(":")) {
        test("http://%s".printf(url));
    } else test(url);
}



////////

void vala_log(string ss) {
    GLib.stdout.printf("%s\n", ss);
}
