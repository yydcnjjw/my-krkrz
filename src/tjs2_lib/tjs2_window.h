#pragma once

#include <memory>

#include <my_render.hpp>

#include "krkrz_application.h"
#include "tjs2_lib.h"
#include <tjs2_lib/tjs2_basic_drawdevice.h>
#include <tjs2_lib/tjs2_scripts.h>

namespace krkrz {
enum TJSMouseButton { mbLeft, mbRight, mbMiddle, mbX1, mbX2 };

enum TJSKeymod {
    TJS_SS_NONE = 0,
    TJS_SS_SHIFT = 0x01,
    TJS_SS_ALT = 0x02,
    TJS_SS_CTRL = 0x04,
    TJS_SS_LEFT = 0x08,
    TJS_SS_RIGHT = 0x10,
    TJS_SS_MIDDLE = 0x20,
    TJS_SS_DOUBLE = 0x40,
    TJS_SS_REPEAT = 0x80,
    TJS_SS_X1 = 0x100,
    TJS_SS_X2 = 0x200
};

enum TJSEventType {
    ACTIVATE,
    CLICK,
    CLOSE_QUERY,
    DEACTIVATE,
    DISPLAY_ROTATE,
    DOUBLE_CLICK,
    FILE_DROP,
    HINT_CHANGED,
    KEY_DOWN,
    KEY_PRESS,
    KEY_UP,
    MOUSE_DOWN,
    MOUSE_ENTER,
    MOUSE_LEAVE,
    MOUSE_MOVE,
    MOUSE_UP,
    MOUSE_WHEEL,
    MULTI_TOUCH,
    POPUP_HIDE,
    RESIZE,
    TOUCH_DOWN,
    TOUCH_MOVE,
    TOUCH_ROTATE,
    TOUCH_SCALING,
    TOUCH_UP,
    // layer
    BEFORE_FOCUS,
    BLUR,
    FOCUS,
    HIT_TEST,
    NODE_DISABLED,
    NODE_ENABLED,
    PAINT,
    SEARCH_NEXT_FOCUSABLE,
    SEARCH_PREV_FOCUSABLE,
    TRANSITION_COMPLETED
};

struct TJSEvent {
    TJSEventType type;
    std::vector<tTJSVariant> args;
};

class TJS2NativeLayer;
class TJS2NativeWindow : public tTJSNativeInstance {
  public:
    TJS2NativeWindow();

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj);

    void TJS_INTF_METHOD Invalidate();

    void add(tTJSVariantClosure clo) {
        if (this->_objects.end() ==
            std::find(this->_objects.begin(), this->_objects.end(), clo)) {
            this->_objects.push_back(clo);
            clo.AddRef();
        }
    }

    void remove(tTJSVariantClosure clo) {
        auto i = std::find(this->_objects.begin(), this->_objects.end(), clo);
        if (this->_objects.end() != i) {
            clo.Release();
            this->_objects.erase(i);
        }
    }

    void on_close_query(bool query);

    void close();

    bool get_full_screen() { return this->_is_full_screen; }

    void set_full_screen(bool full_screen) {
        this->_is_full_screen = full_screen;
        this->_window->set_full_screen(full_screen);
    }

    void set_visible(bool v) {
        if (v != this->is_visible()) {
            this->base_window()->set_visible(v);
        }
    }

    void set_size(const my::ISize2D &size) {
        this->base_window()->set_size(size);
    }

    void set_min_size(const my::ISize2D &size) {
        this->base_window()->set_min_size(size);
    }

    void set_max_size(const my::ISize2D &size) {
        this->base_window()->set_max_size(size);
    }

    bool is_visible() { return this->base_window()->is_visible(); }

    my::Window *base_window() { return this->_window; }

    iTJSDispatch2 *this_obj() { return this->_this_obj; }

    std::u16string caption;

    void set_primary_layer(TJS2NativeLayer *layer);
    TJS2NativeLayer *get_primary_layer() { return this->_primary_layer; }

    static TJS2NativeWindow *main_window() {
        return TJS2NativeWindow::_main_window;
    }

    void set_zoom(int number, int denom) {
        this->set_zoom_number(number);
        this->set_zoom_denom(denom);
        GLOG_D("zoom %d/%d", this->get_zoom_number(), this->get_zoom_denom());
    }

    void set_zoom_number(int number) {
        this->_zoom_number = number == 0 ? 1 : number;
    }
    void set_zoom_denom(int denom) {
        this->_zoom_denom = denom == 0 ? 1 : denom;
    }
    int get_zoom_number() { return this->_zoom_number; }
    int get_zoom_denom() { return this->_zoom_denom; }

    void show_modal() {
        while (this->base_window()->is_visible()) {
            TJS2NativeScripts::get()->yield();
        }
        GLOG_D("show modal end");
    }

    iTJSDispatch2 *draw_device_obj() { return this->_draw_device->this_obj(); }

  private:
    static TJS2NativeWindow *_main_window;
    iTJSDispatch2 *_this_obj{};
    TJS2NativeBasicDrawDevice *_draw_device;

    my::Application *_base_app{};
    my::Window *_window{};
    std::vector<tTJSVariantClosure> _objects{};
    TJS2NativeLayer *_primary_layer{};
    TJS2NativeLayer *_current_motion_layer{};
    my::IPoint2D _current_motion_translate{};

    typedef std::shared_ptr<my::AsyncTask::Timer<std::function<void(void)>>>
        RenderTask;
    RenderTask _render_task{};
    bool _is_full_screen{};

    int _zoom_number{1};
    int _zoom_denom{1};

    rxcpp::composite_subscription _mouse_button_cs;
    rxcpp::composite_subscription _mouse_motion_cs;
    rxcpp::composite_subscription _mouse_wheel_cs;
    rxcpp::composite_subscription _window_cs;
    rxcpp::composite_subscription _keyboard_cs;
    rxcpp::composite_subscription _paint_cs;

    rxcpp::composite_subscription _layer_cs;

    std::mutex _lock;

    SkCanvas *_canvas() { return this->_window->get_sk_surface()->getCanvas(); }

    void _subscribe_event(iTJSDispatch2 *obj);
    void _unsubscribe_event();
    void _event_disptach(const std::string &event_name,
                         const std::vector<tTJSVariant> &args);
    void _paint_event_disptach();
    void _mouse_button_event_disptach(const std::string &event_name,
                                      const my::IPoint2D &mouse_pos, int keymod,
                                      TJSMouseButton);

    void _mouse_motion_event_disptach(const my::IPoint2D &mouse_pos, int shift);

    void _render_task_start();
    void _render_task_stop();
    void _draw_layer();
    void _render();
};

class TJS2Window : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Window();
    tTJSNativeInstance *CreateNativeInstance() {
        return new TJS2NativeWindow();
    }
    static tjs_uint32 ClassID;
};

} // namespace krkrz
