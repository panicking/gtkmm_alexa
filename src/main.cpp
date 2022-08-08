#include <glibmm.h>

#include <iostream>

#include "application.h"
#include "projectdefinitions.h"

int main(int argc, char** argv) {
    auto schema_dir = Glib::getenv("GSETTINGS_SCHEMA_DIR");

    if (!schema_dir.empty()) {
        schema_dir += G_SEARCHPATH_SEPARATOR_S + projectdefinitions::getGeneratedDataDirectory();
    } else {
        schema_dir = projectdefinitions::getGeneratedDataDirectory();
    }

    Glib::setenv("GSETTINGS_SCHEMA_DIR", schema_dir, true);

    auto app = Application::create();
    return app->run(argc, argv);
}
