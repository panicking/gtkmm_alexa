#pragma once

#include <giomm/settings.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/label.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <webkit2/webkit2.h>
#include "alexaclient.h"
#include "gtkwebview.h"

class Window : public Gtk::ApplicationWindow {
    public:
    Window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~Window();

    static Window* create();

    private:
    Glib::RefPtr<Gtk::Builder>  builder;
    Glib::RefPtr<Gio::Settings> settings;
    Gtk::HeaderBar*             headerBar;
    Gtk::ScrolledWindow*        scrolledView;
    Gtk::Label*                 alexaStatus;
    GtkWebView*                 webview;

    AlexaClient                 alexa;

    void setHeaderBar();
    void on_status_changed(std::string status);
};
