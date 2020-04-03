#pragma once

#include <codecvt.h>
#include <window_mgr.h>

#include "krkrz_application.h"

#include "tjs2_lib.h"

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

class TJS2NativeWindow : public tTJSNativeInstance {
  public:
    TJS2NativeWindow() : _base_app(Application::get()->base_app()) {}
    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        this->_window =
            this->_base_app->win_mgr()->create_window("test", 800, 600);
        this->_subscribe_event(tjs_obj);
        return TJS_S_OK;
    }
    void TJS_INTF_METHOD Invalidate() {
        for (auto &obj : this->_objects) {
            obj.Invalidate(0, nullptr, nullptr, nullptr);
            obj.Release();
        }
        this->_base_app->win_mgr()->remove_window(this->_window);
    }

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

    bool get_full_screen() { return this->_is_full_screen; }

    void set_full_screen(bool full_screen) {
        this->_is_full_screen = full_screen;
        this->_window->set_full_screen(full_screen);
    }

    my::Window *base_window() { return this->_window; }

    std::u16string caption;

  private:
    my::Application *_base_app;
    my::Window *_window;
    std::vector<tTJSVariantClosure> _objects;
    bool _is_full_screen;

    void _subscribe_event(iTJSDispatch2 *obj);
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
