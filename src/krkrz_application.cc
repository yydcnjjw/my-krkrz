#include "krkrz_application.h"

#include <application.h>
#include <util/logger.h>

#include <tjs2_lib/tjs2_scripts.h>

namespace {} // namespace

namespace krkrz {
Application *Application::_instance = nullptr;
Application::Application(int argc, char *argv[]) {
    Application::_instance = this;

    my::program_options::options_description desc("my krkrz");

    desc.add_options()("debug", my::program_options::value<bool>(), "");

    this->_base_app = my::new_application(argc, argv, desc);
    this->_init_basic_path(argv[0]);
}

void Application::_init_basic_path(const char *exec_path) {
    this->exec_path = my::fs::absolute(exec_path).make_preferred();
    this->app_path = this->exec_path.parent_path();
    this->data_path = this->app_path / "datapath";
    this->app_data_path = this->app_path / "appdata";
    this->personal_path = this->app_path / "user_data";
    this->save_game_path = this->app_path / "save_game";
}

void Application::run() {
    TJS2NativeScripts::get()->boot_start();
    this->_base_app->run();
    TJS2NativeScripts::get()->stop();
}

Application *Application::get() {
    assert(Application::_instance);
    return Application::_instance;
}

} // namespace krkrz
