#include "tjs2_window.h"

#include <boost/timer/timer.hpp>

#include <tjs2_lib/tjs2_layer.h>
#include <tjs2_lib/tjs2_lib.h>
#include <tjs2_lib/tjs2_scripts.h>

namespace {
struct PaintEvent {
    my::WindowID win_id;
    explicit PaintEvent(my::WindowID id) : win_id(id) {}
};

bool is_point_at(const krkrz::TJS2NativeLayer *layer,
                 const my::IPoint2D &mouse_pos, const my::IPoint2D &translate) {

    auto rect = layer->layer_rect();
    rect.offset(translate);
    return rect.contains(mouse_pos.x(), mouse_pos.y());
}

} // namespace

namespace krkrz {

tTJSNativeClass *create_tjs2_window() { return new TJS2Window(); }

TJSMouseButton mouse_btn_from_sdl(uint32_t button) {
    switch (button) {
    case BUTTON_LEFT:
        return TJSMouseButton::mbLeft;
    case BUTTON_MIDDLE:
        return TJSMouseButton::mbMiddle;
    case BUTTON_RIGHT:
        return TJSMouseButton::mbRight;
    case BUTTON_X1:
        return TJSMouseButton::mbX1;
    case BUTTON_X2:
        return TJSMouseButton::mbX2;
    default:
        throw std::runtime_error("mouse button key is not invalid");
        break;
    }
}

uint32_t to_shift(const my::Keymod &keymod, uint32_t btn_state) {
    uint32_t shift = TJS_SS_NONE;
    if (keymod & KMOD_SHIFT) {
        shift |= TJS_SS_SHIFT;
    } else if (keymod & KMOD_ALT) {
        shift |= TJS_SS_ALT;
    } else if (keymod & KMOD_CTRL) {
        shift |= TJS_SS_CTRL;
    }

    if (btn_state & BUTTON_LMASK) {
        shift |= TJS_SS_LEFT;
    } else if (btn_state & BUTTON_RMASK) {
        shift |= TJS_SS_RIGHT;
    } else if (btn_state & BUTTON_MMASK) {
        shift |= TJS_SS_MIDDLE;
    }
    return shift;
}

TJS2NativeWindow::TJS2NativeWindow()
    : _draw_device(TJS2BasicDrawDevice::create()),
      _base_app(Application::get()->base_app()) {}

tjs_error TJS_INTF_METHOD TJS2NativeWindow::Construct(tjs_int numparams,
                                                      tTJSVariant **param,
                                                      iTJSDispatch2 *tjs_obj) {
    if (!this->_main_window) {
        this->_main_window = this;
    }
    this->_this_obj = tjs_obj;

    this->_window = this->_base_app->win_mgr()->create_window("", 0, 0);

    this->_subscribe_event(tjs_obj);
    this->_render_task_start();

    return TJS_S_OK;
}

void TJS_INTF_METHOD TJS2NativeWindow::Invalidate() {
    this->_unsubscribe_event();
    this->_base_app->win_mgr()->remove_window(this->_window);
    this->_render_task_stop();

    for (auto &obj : this->_objects) {
        obj.Invalidate(0, nullptr, nullptr, nullptr);
        obj.Release();
    }

    auto draw_device_obj = this->_draw_device->this_obj();
    draw_device_obj->Invalidate(0, nullptr, nullptr, draw_device_obj);
    draw_device_obj->Release();
}

void TJS2NativeWindow::_subscribe_event(iTJSDispatch2 *obj) {
    auto bus = this->_base_app->ev_bus();
    auto win_mgr = this->_base_app->win_mgr();
    auto win_id = this->_window->get_window_id();
    auto error_handle = [this](std::exception_ptr &e) {
        try {
            std::rethrow_exception(e);
        } catch (eTJSError &e) {
            GLOG_D(codecvt::utf_to_utf<char>(e.GetMessage().AsStdString())
                       .c_str());
        } catch (std::exception &e) {
            GLOG_D(e.what());
        }
        this->_base_app->quit();
    };
    auto &tjs_worker = TJS2NativeScripts::get()->tjs_worker();
    this->_mouse_button_cs =
        bus->on_event<my::MouseButtonEvent>()
            .filter(
                [win_id](
                    const std::shared_ptr<my::Event<my::MouseButtonEvent>> &e) {
                    return e->data->win_id == win_id;
                })
            .map([](const auto &e) {
                krkrz::TJS2NativeScripts::get()->wake();
                return e;
            })
            .observe_on(tjs_worker)
            .subscribe(
                [this](
                    const std::shared_ptr<my::Event<my::MouseButtonEvent>> &e) {
                    auto button = e->data;
                    auto keymod = (int)to_shift(
                        this->_base_app->win_mgr()->get_key_mode(), 0);
                    auto btn = mouse_btn_from_sdl(button->button);

                    switch (button->state) {
                    case SDL_PRESSED: {
                        this->_mouse_button_event_disptach(
                            "onMouseDown", button->pos, keymod, btn);
                        switch (button->clicks) {
                        case 1: // single click
                            this->_mouse_button_event_disptach(
                                "onClick", button->pos, keymod, btn);
                            break;
                        case 2: // double click
                            this->_mouse_button_event_disptach(
                                "onDoubleClick", button->pos, keymod, btn);
                            break;
                        default:
                            break;
                        }
                        break;
                    }

                    case SDL_RELEASED:
                        this->_mouse_button_event_disptach(
                            "onMouseUp", button->pos, keymod, btn);
                        break;
                    }
                },
                error_handle);

    this->_mouse_motion_cs =
        bus->on_event<my::MouseMotionEvent>()
            .filter(
                [win_id](
                    const std::shared_ptr<my::Event<my::MouseMotionEvent>> &e) {
                    return e->data->win_id == win_id;
                })
            .map([](const auto &e) {
                krkrz::TJS2NativeScripts::get()->wake();
                return e;
            })
            .observe_on(tjs_worker)
            .subscribe(
                [this, obj](
                    const std::shared_ptr<my::Event<my::MouseMotionEvent>> &e) {
                    auto keymod = this->_base_app->win_mgr()->get_key_mode();
                    auto btn_state = e->data->state;
                    this->_mouse_motion_event_disptach(
                        e->data->pos, (int)to_shift(keymod, btn_state));
                },
                error_handle);

    this->_mouse_wheel_cs =
        bus->on_event<my::MoushWheelEvent>()
            .filter(
                [win_id](
                    const std::shared_ptr<my::Event<my::MoushWheelEvent>> &e) {
                    return e->data->win_id == win_id;
                })
            .map([](const auto &e) {
                krkrz::TJS2NativeScripts::get()->wake();
                return e;
            })
            .observe_on(tjs_worker)
            .subscribe(
                [this, obj](
                    const std::shared_ptr<my::Event<my::MoushWheelEvent>> &e) {
                    auto keymod = this->_base_app->win_mgr()->get_key_mode();
                    auto btn_state =
                        this->_base_app->win_mgr()->get_mouse_state().mask;

                    std::vector<tTJSVariant> args{
                        (int)to_shift(keymod, btn_state), 120, e->data->pos.x(),
                        e->data->pos.y()};
                    TJS::func_call(obj, "onMouseWheel", args);
                },
                error_handle);

    this->_window_cs =
        bus->on_event<my::WindowEvent>()
            .filter(
                [win_id](const std::shared_ptr<my::Event<my::WindowEvent>> &e) {
                    return e->data->win_id == win_id;
                })
            .map([](const auto &e) {
                krkrz::TJS2NativeScripts::get()->wake();
                return e;
            })
            .observe_on(tjs_worker)
            .subscribe(
                [this,
                 obj](const std::shared_ptr<my::Event<my::WindowEvent>> &e) {
                    auto event = e->data;
                    switch (event->event) {
                    case SDL_WINDOWEVENT_ENTER:
                        TJS::func_call(obj, "onActivate");
                        TJS::func_call(obj, "onMouseEnter");
                        break;
                    case SDL_WINDOWEVENT_LEAVE:
                        TJS::func_call(obj, "onMouseLeave");
                        TJS::func_call(obj, "onDeactivate");
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        TJS::func_call(obj, "onCloseQuery");
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        TJS::func_call(obj, "onResize");
                        break;
                    }
                },
                error_handle);

    this->_keyboard_cs =
        bus->on_event<my::KeyboardEvent>()
            .filter(
                [win_id](
                    const std::shared_ptr<my::Event<my::KeyboardEvent>> &e) {
                    return e->data->win_id == win_id;
                })
            .map([](const auto &e) {
                krkrz::TJS2NativeScripts::get()->wake();
                return e;
            })
            .observe_on(tjs_worker)
            .subscribe(
                [this,
                 obj](const std::shared_ptr<my::Event<my::KeyboardEvent>> &e) {
                    auto key = e->data;
                    // TODO: impl Window.onkeyDown Window.onkeyUp
                    // Window.onkeyPress
                    std::vector<tTJSVariant> args{0};
                    switch (key->state) {
                    case SDL_PRESSED:
                        TJS::func_call(obj, "onKeyDown", args);
                        break;
                    case SDL_RELEASED:
                        TJS::func_call(obj, "onKeyUp", args);
                        break;
                    }
                },
                error_handle);
    this->_paint_cs =
        bus->on_event<PaintEvent>()
            .filter([win_id](const std::shared_ptr<my::Event<PaintEvent>> &e) {
                return e->data->win_id == win_id;
            })
            .map([](const auto &e) {
                krkrz::TJS2NativeScripts::get()->wake();
                return e;
            })
            .observe_on(tjs_worker)
            .subscribe(
                [this,
                 win_mgr](const std::shared_ptr<my::Event<PaintEvent>> &e) {
                    if (!win_mgr->has_window(e->data->win_id)) {
                        assert("window is released but call paint");
                        return;
                    }
                    this->_paint_event_disptach();
                },
                error_handle);
}

void TJS2NativeWindow::_unsubscribe_event() {
    this->_mouse_button_cs.unsubscribe();
    this->_mouse_motion_cs.unsubscribe();
    this->_mouse_wheel_cs.unsubscribe();
    this->_window_cs.unsubscribe();
    this->_keyboard_cs.unsubscribe();
    this->_paint_cs.unsubscribe();
}

void TJS2NativeWindow::_mouse_button_event_disptach(
    const std::string &event_name, const my::IPoint2D &mouse_pos, int keymod,
    TJSMouseButton btn) {

    GLOG_D("%s %s", event_name.c_str(), format_point(mouse_pos).c_str());
    std::vector<tTJSVariant> args{mouse_pos.x(), mouse_pos.y(), btn, keymod};

    // window
    TJS::func_call(this->this_obj(), event_name, args);

    auto translate = my::IPoint2D::Make(0, 0);

    auto func = my::y_combinator([&](const auto &self,
                                     const TJS2NativeLayer *layer) -> bool {
        if (!layer) {
            return false;
        }

        if (!layer->is_visible()) {
            return false;
        }

        if (!is_point_at(layer, mouse_pos, translate)) {
            return false;
        }

        bool is_child_handled{false};

        translate += layer->pos();

        const auto &children = layer->children();
        for (auto it = children.crbegin(); it != children.crend(); ++it) {
            if (self(*it)) {
                is_child_handled = true;
                break;
            }
        }

        auto pos = mouse_pos - translate;

        translate -= layer->pos();

        if (const_cast<TJS2NativeLayer *>(layer)->hit_test(pos)) {
            if (layer->is_node_enable() && !is_child_handled) {
                std::vector<tTJSVariant> args{pos.x(), pos.y(), btn, keymod};
                // layer
                TJS::func_call(layer->this_obj(), event_name, args);
                GLOG_D("%p: %s %s %s", layer->this_obj(), event_name.c_str(),
                       format_point(pos).c_str(),
                       format_point(translate).c_str());
            }
        } else {
            return false;
        }

        return true;
    });

    func(this->_primary_layer);
}

void TJS2NativeWindow::_mouse_motion_event_disptach(
    const my::IPoint2D &mouse_pos, int shift) {
    auto translate = my::IPoint2D::Make(0, 0);
    std::vector<tTJSVariant> args{mouse_pos.x(), mouse_pos.y(), shift};
    // window
    TJS::func_call(this->this_obj(), "onMouseMove", args);

    auto layer_visit =
        my::y_combinator([&](const auto &self, TJS2NativeLayer *layer) -> bool {
            if (!layer) {
                return false;
            }

            if (this->_current_motion_layer == layer) {
                return true;
            }

            for (const auto child : layer->children()) {
                if (self(child)) {
                    return true;
                }
            }
            return false;
        });

    if (this->_current_motion_layer == nullptr ||
        !layer_visit(this->_primary_layer)) {
        this->_current_motion_layer = nullptr;
    }

    auto func = my::y_combinator([&](const auto &self, TJS2NativeLayer *layer) {
        if (!layer) {
            return;
        }

        if (!layer->is_visible()) {
            return;
        }

        if (!is_point_at(layer, mouse_pos, translate)) {
            return;
        }

        if (this->_current_motion_layer != layer) {
            if (this->_current_motion_layer) {
                if (!is_point_at(this->_current_motion_layer, mouse_pos,
                                 this->_current_motion_translate)) {
                    this->_current_motion_layer->focused = false;

                    TJS::func_call(this->_current_motion_layer->this_obj(),
                                   "onMouseLeave");

                    if (this->_current_motion_layer->focusable &&
                        this->_current_motion_layer->enabled) {
                        TJS::func_call(this->_current_motion_layer->this_obj(),
                                       "onBlur",
                                       {tTJSVariant(layer->this_obj(),
                                                    layer->this_obj())});
                    }
                }
            }

            if (layer->focusable && layer->enabled) {
                TJS::func_call(
                    layer->this_obj(), "onBeforeFocus",
                    {tTJSVariant(layer->this_obj(), layer->this_obj()),
                     tTJSVariant(this->_current_motion_layer->this_obj(),
                                 this->_current_motion_layer->this_obj()),
                     true});
            }

            TJS::func_call(layer->this_obj(), "onMouseEnter");

            layer->focused = true;
            if (layer->focusable && layer->enabled) {
                TJS::func_call(
                    layer->this_obj(), "onFocus",
                    {tTJSVariant(this->_current_motion_layer->this_obj(),
                                 this->_current_motion_layer->this_obj()),
                     true});
            }

            this->_current_motion_layer = layer;
            this->_current_motion_translate = translate;
        }

        translate += layer->pos();

        auto pos = mouse_pos - translate - layer->pos();
        {
            std::vector<tTJSVariant> args{pos.x(), pos.y(), shift};
            TJS::func_call(layer->this_obj(), "onMouseMove", args);
        }

        {
            auto col =
                layer->get_surface_pixel<uint32_t>(layer->main_surface(), pos);
            bool hit{false};
            if (SkColorGetA(col)) {
                hit = true;
            }
            std::vector<tTJSVariant> args{pos.x(), pos.y(), hit};
            TJS::func_call(layer->this_obj(), "onHitTest", args);
        }

        for (const auto child : layer->children()) {
            self(child);
        }

        translate -= layer->pos();
    });
    func(this->_primary_layer);
}

void TJS2NativeWindow::_paint_event_disptach() {
    if (!this->is_visible()) {
        return;
    }
    this->_draw_layer();
    this->_render();
}

void TJS2NativeWindow::_event_disptach(const std::string &event_name,
                                       const std::vector<tTJSVariant> &args) {}

void TJS2NativeWindow::on_close_query(bool query) {
    if (!query) {
        return;
    }

    this->Invalidate();
}

void TJS2NativeWindow::close() {
    this->base_window()->hide();
    // this->on_close_query(true);
}

void TJS2NativeWindow::_render_task_start() {
    this->_render_task =
        Application::get()->base_app()->async_task()->create_timer_interval(
            std::function<void(void)>([this]() {
                this->_base_app->ev_bus()->post<PaintEvent>(
                    this->base_window()->get_window_id());
            }),
            std::chrono::milliseconds(1000 / 30));
    this->_render_task->start();
}

void TJS2NativeWindow::_render_task_stop() {
    this->_render_task->cancel();
    this->_render_task->wait();
}

void TJS2NativeWindow::_draw_layer() {
    draw_layer(this->_canvas(), this->_primary_layer);
}
void TJS2NativeWindow::_render() {
    this->_canvas()->flush();
    this->base_window()->swap_window();
}

void TJS2NativeWindow::set_primary_layer(TJS2NativeLayer *layer) {
    this->_layer_cs.unsubscribe();

    this->_primary_layer = layer;

    if (this->_primary_layer) {
        this->_primary_layer->set_visible(true);
        this->_layer_cs =
            this->_primary_layer->on_event().subscribe([this](LayerEvent e) {});
    }
}

TJS2NativeWindow *TJS2NativeWindow::_main_window{};

tjs_uint32 TJS2Window::ClassID = (tjs_uint32)-1;

TJS2Window::TJS2Window() : inherited(TJS_W("Window")) {

    TJS_BEGIN_NATIVE_MEMBERS(Window)
    TJS_DECL_EMPTY_FINALIZE_METHOD
    TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/ _this,
                                      /*var.type*/ TJS2NativeWindow,
                                      /*TJS class name*/ Window) {
        return TJS_S_OK;
    }
    TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Window)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ add) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
        _this->add(clo);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ add)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ remove) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
        _this->remove(clo);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ remove)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ close) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        _this->close();
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ close)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ hideMouseCursor) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        _this->base_window()->show_cursor(false);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ hideMouseCursor)

    TJS_BEGIN_NATIVE_PROP_DECL(borderStyle) {
        // TODO: impl Window.borderStyle
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        *result = 0;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(borderStyle)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ postInputEvent) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)

        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        ttstr eventname;
        iTJSDispatch2 *eventparams = nullptr;

        eventname = *param[0];
        if (numparams >= 2)
            eventparams = param[1]->AsObjectNoAddRef();

        return TJS_E_NOTIMPL;

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ postInputEvent)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        int w = *param[0];
        int h = *param[1];
        _this->set_size(my::ISize2D::Make(w, h));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setSize)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setMinSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        int w = *param[0];
        int h = *param[1];
        _this->set_min_size(my::ISize2D::Make(w, h));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setMinSize)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setMaxSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        int w = *param[0];
        int h = *param[1];
        _this->set_max_size(my::ISize2D::Make(w, h));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setMaxSize)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setPos) // not setPosition
    {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        _this->base_window()->set_pos({*param[0], *param[1]});
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setPos)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setInnerSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        int w = *param[0];
        int h = *param[1];
        _this->base_window()->set_frame_buffer_size(my::ISize2D::Make(w, h));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setInnerSize)
    TJS_BEGIN_NATIVE_PROP_DECL(fullScreen) {
        TJS_BEGIN_NATIVE_PROP_GETTER
        TJS_GET_NATIVE_INSTANCE(
            /*var. name*/ _this, /*var. type*/ TJS2NativeWindow)
        *result = _this->get_full_screen();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        _this->set_full_screen(0 != (tjs_int)*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(fullScreen)
    TJS_BEGIN_NATIVE_PROP_DECL(width) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        auto [w, h] = _this->base_window()->get_size();
        *result = (int)w;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        auto [w, h] = _this->base_window()->get_size();
        _this->base_window()->set_size({(int)*param, h});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(width)

    TJS_BEGIN_NATIVE_PROP_DECL(height) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_size();
        *result = (int)h;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_size();
        _this->base_window()->set_size({w, (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(height)

    TJS_BEGIN_NATIVE_PROP_DECL(minWidth) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_min_size();
        *result = (int)w;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        auto [w, h] = _this->base_window()->get_min_size();
        _this->base_window()->set_min_size({(int)*param, h});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(minWidth)

    TJS_BEGIN_NATIVE_PROP_DECL(minHeight) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        auto [w, h] = _this->base_window()->get_min_size();
        *result = (int)h;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        auto [w, h] = _this->base_window()->get_min_size();
        _this->base_window()->set_min_size({w, (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(minHeight)

    TJS_BEGIN_NATIVE_PROP_DECL(maxWidth) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)

        auto [w, h] = _this->base_window()->get_max_size();
        *result = (int)w;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        auto [w, h] = _this->base_window()->get_max_size();
        _this->base_window()->set_max_size({(int)*param, h});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(maxWidth)

    TJS_BEGIN_NATIVE_PROP_DECL(maxHeight) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_max_size();
        *result = (int)h;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_max_size();
        _this->base_window()->set_max_size({w, (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(maxHeight)

    TJS_BEGIN_NATIVE_PROP_DECL(left) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto pos = _this->base_window()->get_pos();
        *result = pos.x();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [x, y] = _this->base_window()->get_pos();
        _this->base_window()->set_pos({(int)*param, y});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(left)

    TJS_BEGIN_NATIVE_PROP_DECL(top) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto pos = _this->base_window()->get_pos();
        *result = pos.y();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [x, y] = _this->base_window()->get_pos();
        _this->base_window()->set_pos({x, (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(top)
    TJS_BEGIN_NATIVE_PROP_DECL(innerWidth) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        auto size = _this->base_window()->get_frame_buffer_size();
        *result = size.width();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_frame_buffer_size();
        _this->base_window()->set_frame_buffer_size({(int)*param, h});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(innerWidth)

    TJS_BEGIN_NATIVE_PROP_DECL(innerHeight) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_frame_buffer_size();
        *result = size.height();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_frame_buffer_size();
        _this->base_window()->set_frame_buffer_size({w, (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(innerHeight)
    TJS_BEGIN_NATIVE_PROP_DECL(caption) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        *result = _this->caption;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        _this->caption = ((ttstr)*param).AsStdString();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(caption)

    TJS_BEGIN_NATIVE_PROP_DECL(innerSunken) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        *result = true;
        GLOG_W("use obsolete prop innerSunken");
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        GLOG_W("use obsolete prop innerSunken");
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(innerSunken)
    TJS_BEGIN_NATIVE_PROP_DECL(showScrollBars) {
        TJS_BEGIN_NATIVE_PROP_GETTER
        TJS_GET_NATIVE_INSTANCE(
            /*var. name*/ _this, /*var. type*/ TJS2NativeWindow);
        *result = false;
        GLOG_W("use obsolete prop showScrollBars");
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        GLOG_W("use obsolete prop showScrollBars");
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(showScrollBars)
    TJS_BEGIN_NATIVE_PROP_DECL(visible) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        *result = _this->base_window()->is_visible();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        _this->base_window()->set_visible(*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(visible)

    TJS_BEGIN_NATIVE_PROP_DECL(mainWindow) /* static */
    {
        TJS_BEGIN_NATIVE_PROP_GETTER

        if (TJS2NativeWindow::main_window()) {
            iTJSDispatch2 *dsp = TJS2NativeWindow::main_window()->this_obj();
            *result = tTJSVariant(dsp, dsp);
        } else {
            *result = tTJSVariant((iTJSDispatch2 *)nullptr);
        }
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(mainWindow)
    TJS_BEGIN_NATIVE_PROP_DECL(primaryLayer) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto pri = _this->get_primary_layer();
        if (!pri)
            TVPThrowExceptionMessage(TVPWindowHasNoLayer);

        *result = pri && pri->this_obj()
                      ? tTJSVariant(pri->this_obj(), pri->this_obj())
                      : tTJSVariant((iTJSDispatch2 *)nullptr);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(primaryLayer)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setZoom) {
        // TODO: Window.setZoom
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        _this->set_zoom(*param[0], *param[1]);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setZoom)
    TJS_BEGIN_NATIVE_PROP_DECL(zoomNumer) {
        // TODO: Window.zoomNumber
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        *result = _this->get_zoom_number();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        _this->set_zoom_number(*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(zoomNumer)

    TJS_BEGIN_NATIVE_PROP_DECL(zoomDenom) {
        // TODO: Window.zoomDenom
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        *result = _this->get_zoom_denom();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        _this->set_zoom_denom(*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(zoomDenom)

    TJS_BEGIN_NATIVE_PROP_DECL(drawDevice) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        *result = _this->draw_device_obj();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        // _this->SetDrawDeviceObject(*param);
        return TJS_E_NOTIMPL;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(drawDevice)

    TJS_BEGIN_NATIVE_PROP_DECL(PassThroughDrawDevice) {
        // compatible for old version kr2
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        *result = _this->draw_device_obj();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        // _this->SetDrawDeviceObject(*param);
        return TJS_E_NOTIMPL;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(PassThroughDrawDevice)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ showModal) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        _this->show_modal();
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ showModal)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onCloseQuery) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        _this->on_close_query(0 != (tjs_int)*param[0]);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onCloseQuery)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseDown) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)

        TVP_ACTION_INVOKE_BEGIN(4, "onMouseDown", objthis)
        TVP_ACTION_INVOKE_MEMBER("x")
        TVP_ACTION_INVOKE_MEMBER("y")
        TVP_ACTION_INVOKE_MEMBER("button")
        TVP_ACTION_INVOKE_MEMBER("shift")
        TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseDown)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onClick) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow)
        TVP_ACTION_INVOKE_BEGIN(2, "onClick", objthis)
        TVP_ACTION_INVOKE_MEMBER("x")
        TVP_ACTION_INVOKE_MEMBER("y")
        TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis))

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onClick)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseMove) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);

        TVP_ACTION_INVOKE_BEGIN(3, "onMouseMove", objthis);
        TVP_ACTION_INVOKE_MEMBER("x");
        TVP_ACTION_INVOKE_MEMBER("y");
        TVP_ACTION_INVOKE_MEMBER("shift");
        TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseMove)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseEnter) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);

        TVP_ACTION_INVOKE_BEGIN(0, "onMouseEnter", objthis);
        TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseEnter)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseLeave) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);

        TVP_ACTION_INVOKE_BEGIN(0, "onMouseLeave", objthis);
        TVP_ACTION_INVOKE_END(tTJSVariantClosure(objthis, objthis));

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseLeave)
    TJS_END_NATIVE_MEMBERS
}

} // namespace krkrz
