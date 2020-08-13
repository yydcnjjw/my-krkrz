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

    desc.add_options()("debug", my::program_options::value<bool>(), "")(
        "path", my::program_options::value<my::fs::path>(), "");

    this->_base_app = my::new_application(argc, argv, desc);
    this->_init_basic_path(argv[0]);
}

void Application::_init_basic_path(const char *_exec_path) {
    this->exec_path = my::fs::absolute(_exec_path).make_preferred();
    GLOG_D("exec path %s", this->exec_path.c_str());

    my::program_options::variable_value value;
    my::fs::path app_path;
    if (this->base_app()->get_program_option("path", value)) {
        this->app_path = my::fs::absolute(value.as<my::fs::path>());
    } else {
        this->app_path = exec_path.parent_path();
    }

    GLOG_D("app path %s", this->app_path.c_str());

    this->data_path = this->app_path / "savedata" / "";
    GLOG_D("data path %s", this->data_path.c_str());

    this->app_data_path = this->app_path / "appdata" / "";
    GLOG_D("app data path %s", this->app_data_path.c_str());

    this->personal_path = this->app_path / "user_data" / "";
    GLOG_D("personal path %s", this->personal_path.c_str());

    this->save_game_path = this->app_path / "save_game" / "";
    GLOG_D("save game path %s", this->save_game_path.c_str());
}

void Application::run() {
    TJS2NativeScripts::get()->boot_start();
    this->_base_app->run();
}

Application *Application::get() {
    assert(Application::_instance);
    return Application::_instance;
}

} // namespace krkrz
