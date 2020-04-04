#include "tjs2_layer.h"

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
        //----------------------------------------------------------------------
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
        //----------------------------------------------------------------------
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
        //----------------------------------------------------------------------
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
        //----------------------------------------------------------------------
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

tjs_error TJS_INTF_METHOD TJS2NativeLayer::Construct(tjs_int numparams,
                                                     tTJSVariant **param,
                                                     iTJSDispatch2 *tjs_obj) {
    if (numparams < 2) {
        return TJS_E_BADPARAMCOUNT;
    }

    TJS_NATIVE_INSTANCE(param[0], win, TJS2NativeWindow, TJS2Window);

    if (*param[1]) {
        TJS_NATIVE_INSTANCE(param[1], parent_layer, TJS2NativeLayer, TJS2Layer);
        this->_parent = parent_layer;
        this->_parent->add_children(this);
        this->_draw_list = this->_parent->_draw_list;
    } else {
        this->_parent = nullptr;
        this->_draw_list = std::make_shared<my::DrawList>();
    }

    this->_font = TJS2Font::create();

    return TJS_S_OK;
}

} // namespace krkrz
