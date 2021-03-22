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

static void gtk_web_view_filter_resource(WebKitURISchemeRequest *request, gpointer user_data)
{
    std::string path;
    gsize size;
    g_autoptr (GInputStream) stream = NULL;
    g_autoptr (GError) error = NULL;

    path = webkit_uri_scheme_request_get_path(request);
    path = "/org" + path;

    if (!g_resources_get_info (path.c_str(), (GResourceLookupFlags) 0, &size, NULL, &error)) {
        webkit_uri_scheme_request_finish_error(request, error);
        return;
    }

    stream = g_resources_open_stream (path.c_str(), (GResourceLookupFlags)0, &error);
    if (stream)
        webkit_uri_scheme_request_finish(request, stream, size, NULL);
    else
        webkit_uri_scheme_request_finish_error(request, error);
}

GtkWebView::GtkWebView() : Gtk::Widget(webkit_web_view_new())
{
    WebKitWebContext *context = webkit_web_context_get_default();
    g_signal_connect(*this, "load-changed", G_CALLBACK(gtk_on_load_changed), this);

    /* register to load gresource files */
    webkit_web_context_register_uri_scheme(context, "alexa-org",
		                           gtk_web_view_filter_resource, NULL, NULL);

    webkit_security_manager_register_uri_scheme_as_secure(webkit_web_context_get_security_manager(context), "alexa-org");
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
