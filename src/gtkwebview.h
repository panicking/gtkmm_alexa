#pragma once

#include <gtkmm/widget.h>
#include <webkit2/webkit2.h>

class GtkWebView : public Gtk::Widget
{
public:
    GtkWebView();
    ~GtkWebView() override = default;

    operator WebKitWebView*();

public:
    void load_uri(const Glib::ustring& uri);
    void reload();

public:
    sigc::signal<void, bool> signalLoadingChanged();

private:
    sigc::signal<void, bool> LoadingChanged;
};
