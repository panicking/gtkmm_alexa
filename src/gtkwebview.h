class GtkWebView : public Gtk::Widget
{
public:

    GtkWebView () : Gtk::Widget (webkit_web_view_new()) {}
    virtual ~GtkWebView () {};

    operator WebKitWebView * () { return WEBKIT_WEB_VIEW(gobj()); }

    //  Wrap any functions you want to use like this:
    void load_uri (const gchar *strUri) { webkit_web_view_load_uri(*this, strUri); }  
};
