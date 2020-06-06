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

void Application::_init_basic_path(const char *_exec_path) {
    auto exec_path = my::fs::absolute(_exec_path).make_preferred();
    my::uri uri;
    uri.set_scheme("file");

    uri.set_encoded_path((exec_path).string());
    this->exec_path.set_encoded_url(uri.encoded_url());
    GLOG_D("exec path %s", this->exec_path.encoded_url().to_string().data());

    auto app_path = exec_path.parent_path();
    uri.set_encoded_path((app_path / "").string());
    this->app_path.set_encoded_url(uri.encoded_url());
    GLOG_D("app path %s", this->app_path.encoded_url().to_string().data());

    uri.set_encoded_path((app_path / "savedata" / "").string());
    this->data_path.set_encoded_url(uri.encoded_url());
    GLOG_D("data path %s", this->data_path.encoded_url().to_string().data());

    uri.set_encoded_path((app_path / "appdata" / "").string());
    this->app_data_path.set_encoded_url(uri.encoded_url());
    GLOG_D("app data path %s",
           this->app_data_path.encoded_url().to_string().data());

    uri.set_encoded_path((app_path / "user_data" / "").string());
    this->personal_path.set_encoded_url(uri.encoded_url());
    GLOG_D("personal path %s",
           this->personal_path.encoded_url().to_string().data());

    uri.set_encoded_path((app_path / "save_game" / "").string());
    this->save_game_path.set_encoded_url(uri.encoded_url());
    GLOG_D("save game path %s",
           this->save_game_path.encoded_url().to_string().data());
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
