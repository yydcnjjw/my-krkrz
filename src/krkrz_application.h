#pragma once
#include <application.h>

#include <aio.h>

namespace krkrz {

class Application {
  public:
    Application(int argc, char *argv[]);

    static Application *get();

    my::Application *base_app() { return this->_base_app.get(); }

    void run();

    my::fs::path exec_path;
    my::fs::path app_path;
    my::fs::path data_path;
    my::fs::path app_data_path;
    my::fs::path personal_path;
    my::fs::path save_game_path;

  private:
    std::shared_ptr<my::Application> _base_app;
    static Application *_instance;

    void _init_basic_path(const char *exec_path);
};

} // namespace krkrz
