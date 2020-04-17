#include "tjs2_layer.h"

#include <tjsArray.h>

namespace {

using krkrz::TJS2NativeLayer;

class TJS2Layer : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Layer() : inherited(TJS_W("Layer")) {

        TJS_BEGIN_NATIVE_MEMBERS(Layer)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
            /*var.name*/ _this,
            /*var.type*/ TJS2NativeLayer,
            /*TJS class name*/ Layer) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Layer)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setPos) // not setPosition
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;
            int left = *param[0];
            int top = *param[1];
            _this->pos = {left, top};

            if (numparams == 4 && param[2]->Type() != tvtVoid &&
                param[3]->Type() != tvtVoid) {
                // set bounds

                int w = (tjs_int)*param[2];
                int h = (tjs_int)*param[3];
                _this->size = my::Size2D(w, h);
            }
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setPos)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setSize) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;
            _this->size = my::Size2D((tjs_int)*param[0], (tjs_int)*param[1]);
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setSize)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setImagePos) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;
            _this->image_pos =
                my::PixelPos{(tjs_int)*param[0], (tjs_int)*param[1]};
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setImagePos)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setImageSize) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;
            _this->image_size =
                my::Size2D((tjs_int)*param[0], (tjs_int)*param[1]);
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setImageSize)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setSizeToImageSize) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->size = _this->image_size;
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setSizeToImageSize)
        TJS_BEGIN_NATIVE_PROP_DECL(left) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->pos.x;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->pos.x = *param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(left)

        TJS_BEGIN_NATIVE_PROP_DECL(top) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->pos.y;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->pos.y = *param;

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(top)

        TJS_BEGIN_NATIVE_PROP_DECL(width) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = (int)_this->size.w;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->size.w = (int)*param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(width)

        TJS_BEGIN_NATIVE_PROP_DECL(height) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = (int)_this->size.h;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);

            _this->size.h = (int)*param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(height)

        TJS_BEGIN_NATIVE_PROP_DECL(imageLeft) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->image_pos.x;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->image_pos.x = *param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(imageLeft)

        TJS_BEGIN_NATIVE_PROP_DECL(imageTop) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->image_pos.y;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->image_pos.y = *param;

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(imageTop)

        TJS_BEGIN_NATIVE_PROP_DECL(imageWidth) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = (int)_this->image_size.w;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->image_size.w = (int)*param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(imageWidth)

        TJS_BEGIN_NATIVE_PROP_DECL(imageHeight) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = (int)_this->image_size.h;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);

            _this->image_size.h = (int)*param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(imageHeight)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ fillRect) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            if (numparams < 5)
                return TJS_E_BADPARAMCOUNT;
            tjs_int x, y, w, h;
            x = *param[0];
            y = *param[1];
            w = *param[2];
            h = *param[3];
            auto color = static_cast<tjs_uint32>((tjs_int64)*param[4]);
            _this->fill_rect(x, y, w, h, color);
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ fillRect)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ colorRect) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            if (numparams < 5)
                return TJS_E_BADPARAMCOUNT;
            tjs_int x, y, w, h;
            x = *param[0];
            y = *param[1];
            w = *param[2];
            h = *param[3];
            auto color = static_cast<tjs_uint32>((tjs_int64)*param[4]);
            auto opa = (numparams >= 6 && param[5]->Type() != tvtVoid)
                           ? (tjs_int)*param[5]
                           : 255;
            _this->fill_rect(x, y, w, h, color, opa);
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ colorRect)

        TJS_BEGIN_NATIVE_PROP_DECL(font) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            iTJSDispatch2 *dsp = _this->get_font();
            *result = tTJSVariant(dsp, dsp);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(font)

        TJS_BEGIN_NATIVE_PROP_DECL(type) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            TJS_GET_NATIVE_INSTANCE(
                /*var. name*/ _this, /*var. type*/ TJS2NativeLayer);
            *result = (tjs_int)_this->type;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->type = ((tjs_int)*param);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(type)
        TJS_BEGIN_NATIVE_PROP_DECL(hitType) {
            // TODO: hit type
            TJS_BEGIN_NATIVE_PROP_GETTER
            TJS_GET_NATIVE_INSTANCE(
                /*var. name*/ _this, /*var. type*/ TJS2NativeLayer);
            *result = (tjs_int)_this->hit_type;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->hit_type = (tjs_int)*param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(hitType)

        TJS_BEGIN_NATIVE_PROP_DECL(hitThreshold) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->hit_threshold;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->hit_threshold = (tjs_int)*param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(hitThreshold)

        TJS_BEGIN_NATIVE_PROP_DECL(window) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);

            iTJSDispatch2 *dsp = _this->get_window()->this_obj();
            *result = tTJSVariant(dsp, dsp);

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(window)

        TJS_BEGIN_NATIVE_PROP_DECL(face) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = (tjs_int)_this->face;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->face = ((tjs_int)*param);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(face)
        TJS_BEGIN_NATIVE_PROP_DECL(cursor) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->cursor;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            if (param->Type() == tvtString)
                _this->cursor = 0;
            // _this->SetCursorByStorage(*param);
            else
                _this->cursor = *param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(cursor)

        TJS_BEGIN_NATIVE_PROP_DECL(cursorX) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->get_window_mgr()->get_mouse_state().pos.x;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            auto pos = _this->get_window_mgr()->get_mouse_state().pos;
            pos.x = (*param);
            _this->get_window()->base_window()->set_mouse_pos(pos);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(cursorX)

        TJS_BEGIN_NATIVE_PROP_DECL(cursorY) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->get_window_mgr()->get_mouse_state().pos.y;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            auto pos = _this->get_window_mgr()->get_mouse_state().pos;
            pos.y = (*param);
            _this->get_window()->base_window()->set_mouse_pos(pos);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(cursorY)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setCursorPos) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);

            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;
            _this->get_window()->base_window()->set_mouse_pos(
                {*param[0], *param[1]});

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setCursorPos)

        TJS_BEGIN_NATIVE_PROP_DECL(name) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->name;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->name = ((ttstr)(*param)).AsStdString();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(name)

        TJS_BEGIN_NATIVE_PROP_DECL(parent) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            auto parent = _this->get_parent();
            if (parent) {
                iTJSDispatch2 *dsp = parent;
                *result = tTJSVariant(dsp, dsp);
            } else {
                *result = tTJSVariant((iTJSDispatch2 *)NULL);
            }
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);

            if (param) {
                TJS_NATIVE_INSTANCE(param, parent_layer, TJS2NativeLayer,
                                    TJS2Layer);
                _this->set_parent(parent_layer);
            }

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(parent)

        TJS_BEGIN_NATIVE_PROP_DECL(children) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            iTJSDispatch2 *dsp = _this->get_children_obj();
            *result = tTJSVariant(dsp, dsp);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(children)

        TJS_BEGIN_NATIVE_PROP_DECL(focusable) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            TJS_GET_NATIVE_INSTANCE(
                /*var. name*/ _this, /*var. type*/ TJS2NativeLayer);
            *result = _this->focusable;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->focusable = param->operator bool();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(focusable)

        TJS_BEGIN_NATIVE_PROP_DECL(visible) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = _this->visible;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->visible = param->operator bool();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(visible)

        TJS_BEGIN_NATIVE_PROP_DECL(imageModified) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = (tjs_int)_this->image_modified;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            _this->image_modified = param->operator bool();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(imageModified)

        TJS_BEGIN_NATIVE_PROP_DECL(showParentHint) {
            // TODO: Layer.showParentHint
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            *result = true;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeLayer);
            // _this->SetShowParentHint(*param);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(showParentHint)

        TJS_END_NATIVE_MEMBERS
    } // namespace
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() { return new TJS2NativeLayer; }
};

tjs_uint32 TJS2Layer::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_layer() { return new TJS2Layer(); }

TJS2NativeLayer::TJS2NativeLayer()
    : _font(TJS2Font::create()),
      _win_mgr(krkrz::Application::get()->base_app()->win_mgr()) {}

tjs_error TJS_INTF_METHOD TJS2NativeLayer::Construct(tjs_int numparams,
                                                     tTJSVariant **param,
                                                     iTJSDispatch2 *tjs_obj) {
    if (numparams < 2) {
        return TJS_E_BADPARAMCOUNT;
    }

    this->_this_obj = tjs_obj;

    TJS_NATIVE_INSTANCE(param[0], win, TJS2NativeWindow, TJS2Window);
    this->_win = win;

    if (*param[1]) {
        TJS_NATIVE_INSTANCE(param[1], parent_layer, TJS2NativeLayer, TJS2Layer);
        this->set_parent(parent_layer);
    }

    return TJS_S_OK;
}

iTJSDispatch2 *TJS2NativeLayer::get_children_obj() const {
    iTJSDispatch2 *classobj;
    auto childrens = TJSCreateArrayObject(&classobj);

    int count = 0;
    for (auto &child : this->_children) {
        iTJSDispatch2 *dsp = child->this_obj();
        tTJSVariant val(dsp, dsp);
        childrens->PropSetByNum(TJS_MEMBERENSURE, count, &val, childrens);
        ++count;
    }
    return childrens;
}

} // namespace krkrz
