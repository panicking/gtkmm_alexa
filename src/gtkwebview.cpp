#include "gtkwebview.h"

static void gtk_on_load_changed(WebKitWebView* webView, WebKitLoadEvent loadEvent, gpointer userData)
{
    auto webViewWidget = reinterpret_cast<GtkWebView*>(userData);

    if (!webViewWidget)
        return;

    switch (loadEvent) {
    case WEBKIT_LOAD_STARTED:
        webViewWidget->signalLoadingChanged().emit(true);
        break;
    case WEBKIT_LOAD_COMMITTED:
        break;
    case WEBKIT_LOAD_FINISHED:
        webViewWidget->signalLoadingChanged().emit(false);
        break;
    default:
	break;
    }
}

GtkWebView::GtkWebView() : Gtk::Widget(webkit_web_view_new())
{
    g_signal_connect(*this, "load-changed", G_CALLBACK(gtk_on_load_changed), this);
}

GtkWebView::operator WebKitWebView*()
{
    return WEBKIT_WEB_VIEW(gobj());
}

void GtkWebView::load_uri(const Glib::ustring &uri)
{
    webkit_web_view_load_uri(*this, uri.c_str());
}

sigc::signal<void, bool>  GtkWebView::signalLoadingChanged()
{
    return LoadingChanged;
}
