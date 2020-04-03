#include "tjs2_window.h"

namespace krkrz {

tTJSNativeClass *create_tjs2_window() { return new TJS2Window(); }

TJSMouseButton from_sdl(uint32_t button) {
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

void TJS2NativeWindow::_subscribe_event(iTJSDispatch2 *obj) {
    auto bus = this->_base_app->ev_bus();

    bus->on_event<my::MouseButtonEvent>()
        .filter(
            [this](const std::shared_ptr<my::Event<my::MouseButtonEvent>> &e) {
                return e->data->win_id == this->_window->get_window_id();
            })
        .observe_on(bus->ev_bus_worker())
        .subscribe(
            [this,
             obj](const std::shared_ptr<my::Event<my::MouseButtonEvent>> &e) {
                auto button = e->data;
                std::vector<tTJSVariant> args{button->pos.x, button->pos.y};

                auto keymod = this->_base_app->win_mgr()->get_key_mode();

                uint32_t shift = TJS_SS_NONE;
                if (keymod & KMOD_SHIFT) {
                    shift |= TJS_SS_SHIFT;
                } else if (keymod & KMOD_ALT) {
                    shift |= TJS_SS_ALT;
                } else if (keymod & KMOD_CTRL) {
                    shift |= TJS_SS_CTRL;
                }
                args.push_back((int)shift);

                auto btn = from_sdl(button->button);
                args.push_back(btn);

                switch (button->state) {
                case SDL_PRESSED: {
                    switch (button->clicks) {
                    case 1: // single click
                        TJS::func_call(obj, "onClick",
                                       {button->pos.x, button->pos.y});
                        break;
                    case 2: // double click
                        TJS::func_call(obj, "onDoubleClick",
                                       {button->pos.x, button->pos.y});
                        break;
                    }
                    TJS::func_call(obj, "onMouseDown", args);
                    break;
                }

                case SDL_RELEASED:
                    TJS::func_call(obj, "onMouseUp", args);
                    break;
                }
            });

    bus->on_event<my::MouseMotionEvent>()
        .filter(
            [this](const std::shared_ptr<my::Event<my::MouseMotionEvent>> &e) {
                return e->data->win_id == this->_window->get_window_id();
            })
        .observe_on(bus->ev_bus_worker())
        .subscribe(
            [this,
             obj](const std::shared_ptr<my::Event<my::MouseMotionEvent>> &e) {
                auto keymod = this->_base_app->win_mgr()->get_key_mode();
                uint32_t shift = TJS_SS_NONE;
                if (keymod & KMOD_SHIFT) {
                    shift |= TJS_SS_SHIFT;
                } else if (keymod & KMOD_ALT) {
                    shift |= TJS_SS_ALT;
                } else if (keymod & KMOD_CTRL) {
                    shift |= TJS_SS_CTRL;
                }

                auto btn_state = e->data->state;
                if (btn_state & BUTTON_LMASK) {
                    shift |= TJS_SS_LEFT;
                } else if (btn_state & BUTTON_RMASK) {
                    shift |= TJS_SS_RIGHT;
                } else if (btn_state & BUTTON_MMASK) {
                    shift |= TJS_SS_MIDDLE;
                }
                std::vector<tTJSVariant> args{e->data->pos.x, e->data->pos.y,
                                              (int)shift};
                TJS::func_call(obj, "onMouseMove", args);
            });

    bus->on_event<my::MoushWheelEvent>()
        .filter(
            [this](const std::shared_ptr<my::Event<my::MoushWheelEvent>> &e) {
                return e->data->win_id == this->_window->get_window_id();
            })
        .observe_on(bus->ev_bus_worker())
        .subscribe(
            [this,
             obj](const std::shared_ptr<my::Event<my::MoushWheelEvent>> &e) {
                auto keymod = this->_base_app->win_mgr()->get_key_mode();
                uint32_t shift = TJS_SS_NONE;
                if (keymod & KMOD_SHIFT) {
                    shift |= TJS_SS_SHIFT;
                } else if (keymod & KMOD_ALT) {
                    shift |= TJS_SS_ALT;
                } else if (keymod & KMOD_CTRL) {
                    shift |= TJS_SS_CTRL;
                }

                auto btn_state =
                    this->_base_app->win_mgr()->get_mouse_state().mask;
                if (btn_state & BUTTON_LMASK) {
                    shift |= TJS_SS_LEFT;
                } else if (btn_state & BUTTON_RMASK) {
                    shift |= TJS_SS_RIGHT;
                } else if (btn_state & BUTTON_MMASK) {
                    shift |= TJS_SS_MIDDLE;
                }
                std::vector<tTJSVariant> args{(int)shift, 120, e->data->pos.x,
                                              e->data->pos.y};
                TJS::func_call(obj, "onMouseMove", args);
            });

    bus->on_event<my::WindowEvent>()
        .filter([this](const std::shared_ptr<my::Event<my::WindowEvent>> &e) {
            return e->data->win_id == this->_window->get_window_id();
        })
        .observe_on(bus->ev_bus_worker())
        .subscribe(
            [this, obj](const std::shared_ptr<my::Event<my::WindowEvent>> &e) {
                auto event = e->data;
                switch (event->event) {
                case SDL_WINDOWEVENT_ENTER:
                    TJS::func_call(obj, "onActivate");
                    TJS::func_call(obj, "onMouseEnter");
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    TJS::func_call(obj, "onDeactivate");
                    TJS::func_call(obj, "onMouseLeave");
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    TJS::func_call(obj, "onCloseQuery");
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    TJS::func_call(obj, "onResize");
                    break;
                }
            });

    bus->on_event<my::KeyboardEvent>()
        .filter([this](const std::shared_ptr<my::Event<my::KeyboardEvent>> &e) {
            return e->data->win_id == this->_window->get_window_id();
        })
        .observe_on(bus->ev_bus_worker())
        .subscribe([this, obj](
                       const std::shared_ptr<my::Event<my::KeyboardEvent>> &e) {
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
        });
}

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
                                /*var. type*/ TJS2NativeWindow);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
        _this->add(clo);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ add)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ remove) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
        _this->remove(clo);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ remove)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ close) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        _this->base_window()->hide();
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ close)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ hideMouseCursor) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        _this->base_window()->show_cursor(false);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ hideMouseCursor)

    TJS_BEGIN_NATIVE_PROP_DECL(borderStyle) {
        // TODO: impl Window.borderStyle
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        *result = 0;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(borderStyle)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ postInputEvent) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);

        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        ttstr eventname;
        iTJSDispatch2 *eventparams = NULL;

        eventname = *param[0];
        if (numparams >= 2)
            eventparams = param[1]->AsObjectNoAddRef();

        return TJS_E_NOTIMPL;

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ postInputEvent)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        int w = *param[0];
        int h = *param[1];
        _this->base_window()->set_size(my::Size2D(w, h));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setSize)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setMinSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        int w = *param[0];
        int h = *param[1];
        _this->base_window()->set_min_size(my::Size2D(w, h));
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
        _this->base_window()->set_max_size(my::Size2D(w, h));
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
        _this->base_window()->set_frame_buffer_size(my::Size2D(w, h));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setInnerSize)
    TJS_BEGIN_NATIVE_PROP_DECL(fullScreen) {
        TJS_BEGIN_NATIVE_PROP_GETTER
        TJS_GET_NATIVE_INSTANCE(
            /*var. name*/ _this, /*var. type*/ TJS2NativeWindow);
        *result = _this->get_full_screen();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        _this->set_full_screen(0 != (tjs_int)*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(fullScreen)
    TJS_BEGIN_NATIVE_PROP_DECL(width) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_size();
        *result = (int)w;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_size();
        size.w = (int)*param;
        _this->base_window()->set_size(size);
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
        auto size = _this->base_window()->get_size();
        size.h = (int)*param;
        _this->base_window()->set_size(size);

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
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_min_size();
        size.w = (int)*param;
        _this->base_window()->set_min_size(size);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(minWidth)

    TJS_BEGIN_NATIVE_PROP_DECL(minHeight) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto [w, h] = _this->base_window()->get_min_size();
        *result = (int)h;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_min_size();
        size.h = (int)*param;
        _this->base_window()->set_min_size(size);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(minHeight)

    TJS_BEGIN_NATIVE_PROP_DECL(maxWidth) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);

        auto [w, h] = _this->base_window()->get_max_size();
        *result = (int)w;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_max_size();
        size.w = (int)*param;
        _this->base_window()->set_max_size(size);
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
        auto size = _this->base_window()->get_max_size();
        size.h = (int)*param;
        _this->base_window()->set_max_size(size);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(maxHeight)

    TJS_BEGIN_NATIVE_PROP_DECL(left) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto pos = _this->base_window()->get_pos();
        *result = (int)pos.x;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto pos = _this->base_window()->get_pos();
        pos.x = *param;
        _this->base_window()->set_pos(pos);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(left)

    TJS_BEGIN_NATIVE_PROP_DECL(top) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto pos = _this->base_window()->get_pos();
        *result = (int)pos.y;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto pos = _this->base_window()->get_pos();
        pos.y = *param;
        _this->base_window()->set_pos(pos);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(top)
    TJS_BEGIN_NATIVE_PROP_DECL(innerWidth) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_frame_buffer_size();
        *result = (int)size.w;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_frame_buffer_size();
        size.w = (int)*param;
        _this->base_window()->set_frame_buffer_size(size);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(innerWidth)

    TJS_BEGIN_NATIVE_PROP_DECL(innerHeight) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_frame_buffer_size();
        *result = (int)size.h;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeWindow);
        auto size = _this->base_window()->get_frame_buffer_size();
        size.h = (int)*param;
        _this->base_window()->set_frame_buffer_size(size);
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
    TJS_END_NATIVE_MEMBERS
}

} // namespace krkrz
