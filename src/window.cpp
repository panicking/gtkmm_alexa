#include "window.h"

#include <gtkmm/settings.h>
#include <webkit2/webkit2.h>
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include <iostream>

#include "projectdefinitions.h"
#include "gtkwebview.h"

void Window::on_status_changed(std::string status) {
    std::cout << status << std::endl;
    AlexaClient::AlexaState state;
    alexaStatus->set_text(status.c_str());
    state = alexa.alexaState();
    if (state == AlexaClient::UNKNOWN) {
         webview->reload();
    }
}

Window::Window(Gtk::ApplicationWindow::BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject),
      builder(builder),
      settings(nullptr),
      headerBar(nullptr),
      scrolledView(nullptr) {

    builder->get_widget("scrolled_webview", scrolledView);
    if (!scrolledView) {
        throw std::runtime_error("No \"scrolledview\" object in window.ui");
    }

    builder->get_widget("alexa_status", alexaStatus);

    webview = new GtkWebView;
    scrolledView->add(*webview);

    alexa.onNewWebConnectionChanged.connect(boost::bind(&Window::on_status_changed, this, _1));

    webview->load_uri("alexa-org:/" + projectdefinitions::getApplicationPrefix() + "alexa/index.html");
    webview->set_visible();

    settings = Gio::Settings::create(projectdefinitions::getApplicationID());

    set_icon(Gdk::Pixbuf::create_from_resource(projectdefinitions::getApplicationPrefix() + "icons/48x48/icon.png"));
    setHeaderBar();
}

Window::~Window() {
}

Window* Window::create() {
    auto builder = Gtk::Builder::create_from_resource(projectdefinitions::getApplicationPrefix() + "ui/window.ui");

    Window* window = nullptr;
    builder->get_widget_derived("window", window);
    if (!window) {
        throw std::runtime_error("No \"window\" object in window.ui");
    }
    return window;
}

void Window::setHeaderBar() {
    auto builder =
        Gtk::Builder::create_from_resource(projectdefinitions::getApplicationPrefix() + "ui/headerbar.ui");
    builder->get_widget("headerBar", headerBar);
    if (!headerBar) {
        throw std::runtime_error("No \"headerBar\" object in headerbar.ui");
    } else {
        headerBar->set_title(projectdefinitions::getProjectName());
        set_titlebar(*headerBar);
    }
}
