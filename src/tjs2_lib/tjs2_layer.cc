#include "tjs2_layer.h"

#include <tjsArray.h>

namespace {
std::string format_point(const my::IPoint2D &point) {
    return (boost::format("(%1%,%2%)") % point.x() % point.y()).str();
}

std::string format_rect(const my::IRect &rect) {
    return (boost::format("(%1%,%2%)<=>(%3%,%4%)") % rect.x() % rect.y() %
            rect.width() % rect.height())
        .str();
}
} // namespace

namespace krkrz {

TJS2DrawFace convert_draw_face(TJS2LayerType type) {
    TJS2DrawFace face;
    switch (type) {
        //	case ltBinder:
    case ltOpaque:
        face = dfOpaque;
        break;
    case ltAlpha:
        face = dfAlpha;
        break;
    case ltAdditive:
        face = dfOpaque;
        break;
    case ltSubtractive:
        face = dfOpaque;
        break;
    case ltMultiplicative:
        face = dfOpaque;
        break;
        //	case ltEffect:
        //	case ltFilter:
    case ltDodge:
        face = dfOpaque;
        break;
    case ltDarken:
        face = dfOpaque;
        break;
    case ltLighten:
        face = dfOpaque;
        break;
    case ltScreen:
        face = dfOpaque;
        break;
    case ltAddAlpha:
        face = dfAddAlpha;
        break;
    case ltPsNormal:
        face = dfAlpha;
        break;
    case ltPsAdditive:
        face = dfAlpha;
        break;
    case ltPsSubtractive:
        face = dfAlpha;
        break;
    case ltPsMultiplicative:
        face = dfAlpha;
        break;
    case ltPsScreen:
        face = dfAlpha;
        break;
    case ltPsOverlay:
        face = dfAlpha;
        break;
    case ltPsHardLight:
        face = dfAlpha;
        break;
    case ltPsSoftLight:
        face = dfAlpha;
        break;
    case ltPsColorDodge:
        face = dfAlpha;
        break;
    case ltPsColorDodge5:
        face = dfAlpha;
        break;
    case ltPsColorBurn:
        face = dfAlpha;
        break;
    case ltPsLighten:
        face = dfAlpha;
        break;
    case ltPsDarken:
        face = dfAlpha;
        break;
    case ltPsDifference:
        face = dfAlpha;
        break;
    case ltPsDifference5:
        face = dfAlpha;
        break;
    case ltPsExclusion:
        face = dfAlpha;
        break;
    default:
        face = dfOpaque;
        break;
    }
    return face;
}

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
    this->_this_action_obj = param[0]->AsObjectClosure();

    TJS_NATIVE_INSTANCE(param[0], win, TJS2NativeWindow, TJS2Window);
    this->_win = win;

    if (*param[1]) {
        TJS_NATIVE_INSTANCE(param[1], parent_layer, TJS2NativeLayer, TJS2Layer);
        this->set_parent(parent_layer);
    } else {
        this->_win->set_primary_layer(this);
    }

    return TJS_S_OK;
}

void TJS_INTF_METHOD TJS2NativeLayer::Invalidate() {
    this->_event_suject.get_subscriber().on_completed();
    this->_event_suject.get_subscriber().unsubscribe();

    this->_this_action_obj.Release();
    this->_this_action_obj = {nullptr, nullptr};

    auto font_obj = this->_font->this_obj();
    font_obj->Invalidate(0, nullptr, nullptr, font_obj);
    font_obj->Release();
}

void TJS2NativeLayer::color_rect(const my::IRect &rect, uint32_t color,
                                 int opa) {
    color = krkrz::TJSToActualColor(color);

    auto face = this->face;
    if (face == TJS2DrawFace::dfAuto) {
        face = convert_draw_face(this->type);
    }

    if (face == TJS2DrawFace::dfAlpha) {
        if (opa <= 0) {
            color = 0x000000;
            opa = -opa;
        }
    } else {
        if (opa == 0) {
            return;
        }
    }

    this->set_image_modified(true);
    if (face == TJS2DrawFace::dfProvince || face == TJS2DrawFace::dfMask) {
        return;
    }
    auto col = this->color_convert(color, opa);
    SkPaint paint{};
    paint.setColor(col);
    auto canvas = this->canvas();
    if (canvas) {
        canvas->drawIRect(rect, paint);
    }

    GLOG_D("color rect face=%d, %s color %d %d %d %d", face,
           format_rect(rect).c_str(), SkColorGetA(col), SkColorGetR(col),
           SkColorGetG(col), SkColorGetB(col));
}

void TJS2NativeLayer::fill_rect(const my::IRect &rect, uint32_t color) {
    auto face = this->face;
    if (face == TJS2DrawFace::dfAuto) {
        face = convert_draw_face(this->type);
    }

    this->set_image_modified(true);
    if (face == TJS2DrawFace::dfProvince || face == TJS2DrawFace::dfMask) {
        return;
    }

    SkPaint paint{};
    paint.setBlendMode(SkBlendMode::kSrc);
    auto canvas = this->canvas();
    if (canvas) {
        canvas->drawIRect(rect, paint);
    }

    GLOG_D("fill rect face=%d, %s color(ARGB) %#x", face,
           format_rect(rect).c_str(), color);
}

void TJS2NativeLayer::draw_text(const my::IPoint2D &pos,
                                const std::u16string &_text, uint32_t color,
                                int opa, bool aa, int shadowlevel,
                                uint32_t shadowcolor, int shadowwidth,
                                int shadowofsx, int shadowofsy) {
    color = krkrz::TJSToActualColor(color);
    auto col = this->color_convert(color, opa);
    auto text = codecvt::utf_to_utf<char>(_text);
    GLOG_D("draw text height %d, %d,%d %s, color(ARGB)%#x",
           this->_font->get_height(), pos.x(), pos.y(), text.c_str(), col);
    SkPaint paint{};
    paint.setColor(col);
    auto [x, y] = pos;
    y += this->_font->get_height();
    auto canvas = this->canvas();
    if (canvas) {
        canvas->drawString(text.c_str(), x, y, this->_font->sk_font(), paint);
    }

    this->set_image_modified(true);
}

void TJS2NativeLayer::load_image(const std::u16string &_path) {
    auto path = my::fs::path(codecvt::utf_to_utf<char>(_path));

    GLOG_D("load image: %s", path.c_str());

    if (path.has_extension()) {
        this->_image = TJS2NativeStorages::get()->get_storage<my::Image>(path);
    } else {
        for (auto extension : {".png", ".jpg"}) {
            path.replace_extension(extension);
            try {
                this->_image =
                    TJS2NativeStorages::get()->get_storage<my::Image>(path);
            } catch (...) {
                this->_image = nullptr;
                continue;
            }
            break;
        }
    }

    if (!this->_image) {
        throw std::runtime_error(
            (boost::format("load image failure %1%") % path).str());
    } else {
    }
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

tjs_uint32 TJS2Layer::ClassID = (tjs_uint32)-1;

TJS2Layer::TJS2Layer() : inherited(TJS_W("Layer")) {

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
        _this->set_pos({left, top});

        if (numparams == 4 && param[2]->Type() != tvtVoid &&
            param[3]->Type() != tvtVoid) {
            // set bounds

            int w = (tjs_int)*param[2];
            int h = (tjs_int)*param[3];
            _this->set_size({w, h});
        }
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setPos)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        _this->set_size({(tjs_int)*param[0], (tjs_int)*param[1]});
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setSize)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setImagePos) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        _this->set_image_pos({(tjs_int)*param[0], (tjs_int)*param[1]});
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setImagePos)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setImageSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        _this->set_image_size({(tjs_int)*param[0], (tjs_int)*param[1]});
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setImageSize)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setSizeToImageSize) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_size(_this->image_size());
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setSizeToImageSize)
    TJS_BEGIN_NATIVE_PROP_DECL(left) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->pos().x();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_pos({(int)*param, _this->pos().y()});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(left)

    TJS_BEGIN_NATIVE_PROP_DECL(top) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->pos().y();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_pos({_this->pos().x(), (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(top)

    TJS_BEGIN_NATIVE_PROP_DECL(width) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (int)_this->size().width();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_size({(int)*param, _this->size().height()});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(width)

    TJS_BEGIN_NATIVE_PROP_DECL(height) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (int)_this->size().height();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_size({_this->size().width(), (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(height)

    TJS_BEGIN_NATIVE_PROP_DECL(imageLeft) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->image_pos().x();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_image_pos({(int)*param, _this->image_pos().y()});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(imageLeft)

    TJS_BEGIN_NATIVE_PROP_DECL(imageTop) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->image_pos().y();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_image_pos({_this->image_pos().x(), (int)*param});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(imageTop)

    TJS_BEGIN_NATIVE_PROP_DECL(imageWidth) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (int)_this->image_size().width();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_image_size({(int)*param, _this->image_size().height()});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(imageWidth)

    TJS_BEGIN_NATIVE_PROP_DECL(imageHeight) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (int)_this->image_size().height();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_image_size({_this->image_size().width(), (int)*param});
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
        _this->fill_rect(my::IRect::MakeXYWH(x, y, w, h), color);
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
        _this->color_rect(my::IRect::MakeXYWH(x, y, w, h), color, opa);
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
        *result = (int)_this->type;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->type = ((krkrz::TJS2LayerType)(int)*param);
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
        *result = (int)_this->face;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->face = ((krkrz::TJS2DrawFace)(int)*param);
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
        *result = _this->get_window_mgr()->get_mouse_state().pos.x();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        auto pos = _this->get_window_mgr()->get_mouse_state().pos;
        _this->get_window()->base_window()->set_mouse_pos(
            {(int)*param, pos.y()});
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(cursorX)

    TJS_BEGIN_NATIVE_PROP_DECL(cursorY) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->get_window_mgr()->get_mouse_state().pos.y();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        auto pos = _this->get_window_mgr()->get_mouse_state().pos;
        _this->get_window()->base_window()->set_mouse_pos(
            {pos.x(), (int)*param});
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
        *result = _this->is_visible();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_visible(param->operator bool());
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(visible)

    TJS_BEGIN_NATIVE_PROP_DECL(imageModified) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (tjs_int)_this->is_image_modified();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_image_modified(param->operator bool());
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

    TJS_BEGIN_NATIVE_PROP_DECL(opacity) {
        // TODO: Layer.opacity
        TJS_BEGIN_NATIVE_PROP_GETTER
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = 255; //_this->GetOpacity();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        // _this->SetOpacity(*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(opacity)
    TJS_BEGIN_NATIVE_PROP_DECL(absolute) // not absoluteOrderIndex
    {
        // TODO: Layer.absolute
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = 0; //(tjs_int)_this->GetAbsoluteOrderIndex();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        // _this->SetAbsoluteOrderIndex(*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(absolute)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setMode) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        // _this->SetMode();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setMode)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ removeMode) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        // _this->RemoveMode();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ removeMode)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ drawText) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 4)
            return TJS_E_BADPARAMCOUNT;
        _this->draw_text(
            my::IPoint2D::Make(*param[0], *param[1]),
            ((ttstr)*param[2]).c_str(),
            static_cast<tjs_uint32>((tjs_int64)*param[3]),
            (numparams >= 5 && param[4]->Type() != tvtVoid) ? (tjs_int)*param[4]
                                                            : (tjs_int)255,
            (numparams >= 6 && param[5]->Type() != tvtVoid)
                ? param[5]->operator bool()
                : true,
            (numparams >= 7 && param[6]->Type() != tvtVoid) ? (tjs_int)*param[6]
                                                            : 0,
            (numparams >= 8 && param[7]->Type() != tvtVoid)
                ? static_cast<tjs_uint32>((tjs_int64)*param[7])
                : 0,
            (numparams >= 9 && param[8]->Type() != tvtVoid) ? (tjs_int)*param[8]
                                                            : 0,
            (numparams >= 10 && param[9]->Type() != tvtVoid)
                ? (tjs_int)*param[9]
                : 0,
            (numparams >= 11 && param[10]->Type() != tvtVoid)
                ? (tjs_int)*param[10]
                : 0);

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ drawText)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ update) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->update();
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ update)
    TJS_BEGIN_NATIVE_PROP_DECL(callOnPaint) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->call_on_paint;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->call_on_paint = param->operator bool();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(callOnPaint)

    TJS_BEGIN_NATIVE_PROP_DECL(enabled) {
        // TODO: Layer.enabled
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = true;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        // _this->SetEnabled(param->operator bool());
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(enabled)
    TJS_BEGIN_NATIVE_PROP_DECL(nodeEnabled) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = true;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(nodeEnabled)

    TJS_BEGIN_NATIVE_PROP_DECL(focused) {
        // TODO: Layer.focused
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (tjs_int) true;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(focused)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ focus) { return TJS_S_OK; }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ focus)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getProvincePixel) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        if (result)
            *result = 0;
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ getProvincePixel)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setProvincePixel) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 3)
            return TJS_E_BADPARAMCOUNT;
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ setProvincePixel)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ loadImages) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        ttstr name(*param[0]);
        _this->load_image(name.AsStdString());

        if (result)
            *result = tTJSVariant(); // TODO: image tag information(dict)

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ loadImages)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ assignImages) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        TJS2NativeLayer *src;
        tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
        if (clo.Object) {
            if (TJS_FAILED(clo.Object->NativeInstanceSupport(
                    TJS_NIS_GETINSTANCE, TJS2Layer::ClassID,
                    (iTJSNativeInstance **)&src)))
                TVPThrowExceptionMessage(TVPSpecifyLayer);
        }
        if (!src)
            TVPThrowExceptionMessage(TVPSpecifyLayer);

        _this->assign_images(src);

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ assignImages)
    TJS_BEGIN_NATIVE_PROP_DECL(isPrimary) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (tjs_int)_this->is_primary_layer();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(isPrimary)

    TJS_BEGIN_NATIVE_PROP_DECL(absoluteOrderMode) // not absoluteOrderIndexMode
    {
        // TODO: Layer.absoluteOrderMode
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (tjs_int)_this->absolute_order_mode;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->absolute_order_mode = (bool)(*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(absoluteOrderMode)

    TJS_BEGIN_NATIVE_PROP_DECL(holdAlpha) {
        // TODO: Layer.holdAlpha
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (tjs_int)_this->hold_alpha;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->hold_alpha = (0 != (tjs_int)*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(holdAlpha)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ adjustGamma) {
        // TODO: Layer.adjustGamma
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        if (numparams == 0)
            return TJS_S_OK;

        // tTVPGLGammaAdjustData data;
        // memcpy(&data, &TVPIntactGammaAdjustData, sizeof(data));

        // if(numparams >= 1 && param[0]->Type() != tvtVoid)
        // 	data.RGamma = static_cast<float>((double)*param[0]);
        // if(numparams >= 2 && param[1]->Type() != tvtVoid)
        // 	data.RFloor = *param[1];
        // if(numparams >= 3 && param[2]->Type() != tvtVoid)
        // 	data.RCeil  = *param[2];
        // if(numparams >= 4 && param[3]->Type() != tvtVoid)
        // 	data.GGamma = static_cast<float>((double)*param[3]);
        // if(numparams >= 5 && param[4]->Type() != tvtVoid)
        // 	data.GFloor = *param[4];
        // if(numparams >= 6 && param[5]->Type() != tvtVoid)
        // 	data.GCeil  = *param[5];
        // if(numparams >= 7 && param[6]->Type() != tvtVoid)
        // 	data.BGamma = static_cast<float>((double)*param[6]);
        // if(numparams >= 8 && param[7]->Type() != tvtVoid)
        // 	data.BFloor = *param[7];
        // if(numparams >= 9 && param[8]->Type() != tvtVoid)
        // 	data.BCeil  = *param[8];

        // _this->AdjustGamma(data);

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ adjustGamma)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ beginTransition) {
        // TODO: Layer.beginTransition
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        // parameters are : <name>, [<withchildren>=true], [<transwith>=null],
        //                  [<options>]
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        // ttstr name = *param[0];
        // bool withchildren = true;
        // if(numparams >= 2 && param[1]->Type() != tvtVoid)
        // 	withchildren = param[1]->operator bool();
        // tTJSNI_BaseLayer * transsrc = NULL;
        // if(numparams >= 3 && param[2]->Type() != tvtVoid)
        // {
        // 	tTJSVariantClosure clo = param[2]->AsObjectClosureNoAddRef();
        // 	if(clo.Object)
        // 	{
        // 		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
        // 			tTJSNC_Layer::ClassID,
        // (iTJSNativeInstance**)&transsrc)))
        // 			TVPThrowExceptionMessage(TVPSpecifyLayer);
        // 	}
        // }
        // if(!transsrc) TVPThrowExceptionMessage(TVPSpecifyLayer);

        // tTJSVariantClosure options(NULL, NULL);
        // if(numparams >= 4 && param[3]->Type() != tvtVoid)
        // 	options = param[3]->AsObjectClosureNoAddRef();

        // _this->StartTransition(name, withchildren, transsrc, options);

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ beginTransition)
    //----------------------------------------------------------------------
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ stopTransition) {
        // TODO: Layer.stopTransition
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        // _this->StopTransition();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ stopTransition)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ piledCopy) {
        // TODO: Layer.piledCopy
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 7)
            return TJS_E_BADPARAMCOUNT;

        // tTJSNI_BaseLayer * src = NULL;
        // tTJSVariantClosure clo = param[2]->AsObjectClosureNoAddRef();
        // if(clo.Object)
        // {
        // 	if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
        // 		tTJSNC_Layer::ClassID, (iTJSNativeInstance**)&src)))
        // 		TVPThrowExceptionMessage(TVPSpecifyLayer);
        // }
        // if(!src) TVPThrowExceptionMessage(TVPSpecifyLayer);

        // tTVPRect rect(*param[3], *param[4], *param[5], *param[6]);
        // rect.right += rect.left;
        // rect.bottom += rect.top;

        // _this->PiledCopy(*param[0], *param[1], src, rect);
        GLOG_D("call Layer.PiledCopy");
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ piledCopy)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ stretchCopy) {
        // TODO: Layer.stretchCopy
        // dx, dy, dw, dh, src, sx, sy, sw, sh, type=0
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 9)
            return TJS_E_BADPARAMCOUNT;

        // tTVPBaseBitmap* src = NULL;
        // tTJSVariantClosure clo = param[4]->AsObjectClosureNoAddRef();
        // if(clo.Object)
        // {
        // 	tTJSNI_BaseLayer * srclayer = NULL;
        // 	if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
        // 		tTJSNC_Layer::ClassID,
        // (iTJSNativeInstance**)&srclayer))) 		src = NULL; 	else
        // src = srclayer->GetMainImage();

        // 	if( src == NULL )
        // 	{	// try to get bitmap interface
        // 		tTJSNI_Bitmap * srcbmp = NULL;
        // 		if(TJS_FAILED(clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
        // 			tTJSNC_Bitmap::ClassID,
        // (iTJSNativeInstance**)&srcbmp))) 			src = NULL;
        // else 			src = srcbmp->GetBitmap();
        // 	}
        // }
        // if(!src) TVPThrowExceptionMessage(TVPSpecifyLayerOrBitmap);

        // tTVPRect destrect(*param[0], *param[1], *param[2], *param[3]);
        // destrect.right += destrect.left;
        // destrect.bottom += destrect.top;

        // tTVPRect srcrect(*param[5], *param[6], *param[7], *param[8]);
        // srcrect.right += srcrect.left;
        // srcrect.bottom += srcrect.top;

        // tTVPBBStretchType type = stNearest;
        // if(numparams >= 10)
        // 	type = (tTVPBBStretchType)(tjs_int)*param[9];

        // tjs_real typeopt = 0.0;
        // if(numparams >= 11)
        // 	typeopt = (tjs_real)*param[10];
        // else if( type == stFastCubic || type == stCubic )
        // 	typeopt = -1.0;

        // _this->StretchCopy(destrect, src, srcrect, type, typeopt);
        GLOG_D("call Layer.StretchCopy");

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ stretchCopy)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ saveLayerImage) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        ttstr name(*param[0]);
        ttstr type(TJS_W("bmp"));
        if (numparams >= 2 && param[1]->Type() != tvtVoid)
            type = *param[1];
        // _this->SaveLayerImage(name, type);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ saveLayerImage)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ operateRect) {
        // TODO: Layer.operateRect
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 7)
            return TJS_E_BADPARAMCOUNT;
        TJS2NativeLayer *srclayer{};
        tTJSVariantClosure clo = param[2]->AsObjectClosureNoAddRef();
        if (clo.Object) {
            if (TJS_FAILED(clo.Object->NativeInstanceSupport(
                    TJS_NIS_GETINSTANCE, TJS2Layer::ClassID,
                    (iTJSNativeInstance **)&srclayer))) {
                throw std::runtime_error(
                    "TODO: Layer.operateRect only support Layer");
            }
        }

        // if(!src) TVPThrowExceptionMessage(TVPSpecifyLayerOrBitmap);

        TJS2BlendOperationMode mode;
        if (numparams >= 8 && param[7]->Type() != tvtVoid)
            mode = (TJS2BlendOperationMode)(tjs_int)(*param[7]);
        else
            mode = omAuto;

        if (mode == omAuto)
            mode = omAlpha;

        _this->operate_rect({*param[0], *param[1]}, srclayer,
                            {*param[3], *param[4]}, {*param[5], *param[6]},
                            mode);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ operateRect)

    // event
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseDown) {
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseDown)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onClick) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(2, "onClick", objthis);
            TVP_ACTION_INVOKE_MEMBER("x");
            TVP_ACTION_INVOKE_MEMBER("y");
            TVP_ACTION_INVOKE_END(obj);
        }
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onClick)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onDoubleClick) {
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onDoubleClick)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseUp) { return TJS_S_OK; }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseUp)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onPaint) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        // tTJSVariantClosure obj = _this->GetActionOwnerNoAddRef();
        // if(obj.Object)
        // {
        // 	TVP_ACTION_INVOKE_BEGIN(0, "onPaint", objthis);
        // 	TVP_ACTION_INVOKE_END(obj);
        // }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onPaint)
    TJS_END_NATIVE_MEMBERS
} // namespace krkrz

} // namespace krkrz
