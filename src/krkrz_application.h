#pragma once
#include <application.h>

namespace krkrz {

class Application {
  public:
    Application(int argc, char *argv[]);

    static Application *get();

    my::Application *base_app() { return this->_base_app.get(); }

    void run();

    my::uri exec_path;
    my::uri app_path;
    my::uri data_path;
    my::uri app_data_path;
    my::uri personal_path;
    my::uri save_game_path;
    
  private:
    std::shared_ptr<my::Application> _base_app;
    static Application *_instance;

    void _init_basic_path(const char *exec_path);
};

} // namespace krkrz
