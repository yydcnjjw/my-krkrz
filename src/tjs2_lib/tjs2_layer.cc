#include "tjs2_layer.h"

#include <skia/include/core/SkMaskFilter.h>
#include <skia/include/core/SkTextBlob.h>

#include <tjsArray.h>

namespace {} // namespace

namespace krkrz {

std::string format_point(const my::IPoint2D &point) {
    return (boost::format("(%1%,%2%)") % point.x() % point.y()).str();
}

std::string format_rect(const my::IRect &rect) {
    return (boost::format("(%1%,%2%)<=>(%3%,%4%)") % rect.x() % rect.y() %
            rect.width() % rect.height())
        .str();
}

void draw_layer(SkCanvas *canvas, TJS2NativeLayer *root, bool check_visible) {
    GLOG_D("render layer %p", root->this_obj());
    canvas->clear(SkColors::kWhite.toSkColor());
    int level = 0;
    auto func = my::y_combinator([canvas, &level, check_visible](
                                     const auto &self, TJS2NativeLayer *layer) {
        if (!layer) {
            return;
        }

        if (!layer->is_visible() && check_visible) {
            return;
        }

        if (layer->call_on_paint) {
            layer->call_on_paint = false;
            TJS::func_call(layer->this_obj(), "onPaint");
        }

        auto [x, y] = layer->pos();

        {
            auto image = layer->image_snapshot();
            canvas->drawImage(image, x, y);
            // SkPaint paint{};
            // paint.setColor(SkColors::kBlue);
            // paint.setAlpha(128);
            // paint.setStyle(SkPaint::Style::kStroke_Style);
            // paint.setStrokeWidth(SkScalar(4));
            // canvas->drawIRect(
            //     my::IRect::MakeXYWH(x, y, image->width(), image->height()),
            //     paint);

            auto w = image->width();
            auto h = image->height();
            std::string indent(level, ' ');

            GLOG_D("%s:%s:%p:%s (%d,%d)", indent.c_str(),
                   codecvt::utf_to_utf<char>(layer->name).c_str(),
                   layer->this_obj(), format_rect(layer->layer_rect()).c_str(),
                   w, h);
        }

        ++level;
        canvas->save();
        canvas->translate(x, y);
        for (const auto child : layer->children()) {
            self(child);
        }
        canvas->restore();
        --level;
    });

    func(root);
}

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
    GLOG_D("invalidate %p:%p:%s", this, this->_this_obj,
           codecvt::utf_to_utf<char>(this->name).c_str());
    this->remove_parent();
    this->_this_obj = nullptr;
    this->_this_action_obj.Release();
    this->_this_action_obj = {nullptr, nullptr};

    this->_event_suject.get_subscriber().on_completed();
    this->_event_suject.get_subscriber().unsubscribe();

    auto font_obj = this->_font->this_obj();
    font_obj->Invalidate(0, nullptr, nullptr, font_obj);
    font_obj->Release();

    this->_main_surface = nullptr;
    this->_province_surface = nullptr;
}

void TJS2NativeLayer::color_rect(const my::IRect &rect, uint32_t color,
                                 int opa) {
    auto face = this->face();

    switch (face) {
    case TJS2DrawFace::dfAlpha:
    case TJS2DrawFace::dfAddAlpha:
    case TJS2DrawFace::dfOpaque: {
        if (face == TJS2DrawFace::dfAlpha) {
            if (opa <= 0) {
                color = 0xffffff;
                opa = -opa;
            }
        }
        auto col = this->color_convert(krkrz::TJSToActualColor(color), opa);
        GLOG_D("%p:color rect face=%d, %s %s color %d %d %d %d",
               this->this_obj(), face, format_rect(rect).c_str(),
               format_point(this->pos()).c_str(), SkColorGetA(col),
               SkColorGetR(col), SkColorGetG(col), SkColorGetB(col));
        SkPaint paint{};
        paint.setColor(col);

        this->canvas()->drawIRect(rect, paint);
        break;
    }
    case TJS2DrawFace::dfProvince: {
        GLOG_D("%p:color rect province, %s %s color %d", this->this_obj(),
               format_rect(rect).c_str(), format_point(this->pos()).c_str(),
               color);

        SkPaint paint{};
        paint.setAlpha(color);
        paint.setBlendMode(SkBlendMode::kSrc);
        this->province_canvas()->drawIRect(rect, paint);
        break;
    }
    case TJS2DrawFace::dfMask:
        break;
    default:
        break;
    }

    this->set_image_modified(true);
}

void TJS2NativeLayer::fill_rect(const my::IRect &rect, uint32_t color) {
    auto face = this->face();

    switch (face) {
    case TJS2DrawFace::dfAlpha:
    case TJS2DrawFace::dfAddAlpha:
    case TJS2DrawFace::dfOpaque: {
        SkPaint paint{};
        paint.setColor(color);
        paint.setBlendMode(SkBlendMode::kSrc);
        this->canvas()->drawIRect(rect, paint);
        break;
    }
    case TJS2DrawFace::dfProvince: {
        SkPaint paint{};
        paint.setAlpha(color);
        paint.setBlendMode(SkBlendMode::kSrc);
        this->province_canvas()->drawIRect(rect, paint);
        break;
    }
    case TJS2DrawFace::dfMask:
        break;
    default:
        break;
    }

    GLOG_D("%p:fill rect face=%d, %s %s color(ARGB) %#x", this->this_obj(),
           face, format_rect(rect).c_str(), format_point(this->pos()).c_str(),
           color);

    this->set_image_modified(true);
}

void TJS2NativeLayer::draw_text(const my::IPoint2D &pos,
                                const std::u16string &_text, uint32_t color,
                                int opa, bool aa, int shadowlevel,
                                uint32_t shadow_color, int shadowwidth,
                                int shadow_x, int shadow_y) {
    color = krkrz::TJSToActualColor(color);
    auto col = this->color_convert(color, opa);
    auto text = codecvt::utf_to_utf<char>(_text);

    GLOG_D(
        "%p:draw text height %d, %d,%d %s, color(ARGB)%#x shadow_color(RGB)%#x",
        this->this_obj(), this->_font->get_height(), pos.x(), pos.y(),
        text.c_str(), col, shadow_color);

    auto [x, y] = pos;
    y += this->_font->get_height();

    SkPaint paint{};
    paint.setColor(col);
    paint.setAntiAlias(true);

    SkPaint blur{};
    blur.setAntiAlias(true);
    blur.setColor(shadow_color);
    blur.setAlpha(255);
    blur.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 1.65f, 0));

    auto blob =
        SkTextBlob::MakeFromString(text.c_str(), this->_font->sk_font());

    this->canvas()->drawTextBlob(blob.get(), x + shadow_x, y + shadow_y, blur);
    this->canvas()->drawTextBlob(blob.get(), x, y, paint);

    this->set_image_modified(true);
}

void TJS2NativeLayer::operate_rect(const my::IPoint2D &d_off,
                                   TJS2NativeLayer *layer,
                                   const my::IPoint2D &s_off,
                                   const my::ISize2D &s_size,
                                   TJS2BlendOperationMode _mode) {
    auto mode = TJS2BlendOperationMode::omAlpha;
    if (_mode == TJS2BlendOperationMode::omAuto) {
        mode = layer->blend_mode();
    }

    // GLOG_D("%p:operate rect: d_off:%d,%d s_off:%d,%d s_size:%d,%d mode:%d "
    //        "%s",
    //        this->this_obj(), d_off.x(), d_off.y(), s_off.x(), s_off.y(),
    //        s_size.width(), s_size.height(), mode,
    //        format_point(this->pos()).c_str());
    SkBitmap src{};
    src.allocN32Pixels(s_size.width(), s_size.height());
    layer->main_surface()->readPixels(src, s_off.x(), s_off.y());

    auto [x, y] = d_off;
    switch (mode) {
    case TJS2BlendOperationMode::omAlpha:
        this->canvas()->drawBitmap(src, x, y);
        break;
    default:
        assert(true);
        break;
    }
    set_image_modified(true);
}

void TJS2NativeLayer::copy_rect(const my::IPoint2D &d_off,
                                TJS2NativeLayer *layer,
                                const my::IPoint2D &s_off,
                                const my::ISize2D &s_size) {
    auto face = this->face();

    sk_sp<SkSurface> dst_surface{};
    sk_sp<SkSurface> src_surface{};

    switch (face) {
    case TJS2DrawFace::dfAlpha:
    case TJS2DrawFace::dfAddAlpha:
    case TJS2DrawFace::dfOpaque: {
        dst_surface = this->main_surface();
        src_surface = layer->main_surface();
        break;
    }
    case TJS2DrawFace::dfProvince: {
        dst_surface = this->province_surface();
        src_surface = layer->province_surface();
        break;
    }
    case TJS2DrawFace::dfMask:
        break;
    default:
        break;
    }

    SkBitmap src{};
    src.allocN32Pixels(s_size.width(), s_size.height());
    src_surface->readPixels(src, s_off.x(), s_off.y());
    dst_surface->writePixels(src, d_off.x(), d_off.y());
    this->set_image_modified(true);
}

void TJS2NativeLayer::piled_copy(const my::IPoint2D dpos,
                                 const my::IRect &src_rect,
                                 TJS2NativeLayer *src_layer) {
    GLOG_D("%p:%s:piled copy: dst: %s src: %s %p:%s", this->this_obj(),
           codecvt::utf_to_utf<char>(this->name).c_str(),
           format_point(dpos).c_str(), format_rect(src_rect).c_str(),
           src_layer->this_obj(),
           codecvt::utf_to_utf<char>(src_layer->name).c_str());

    SkBitmap src{};
    src.allocN32Pixels(src_rect.width(), src_rect.height());
    src_layer->main_surface()->readPixels(src, src_rect.x(), src_rect.y());

    this->canvas()->drawBitmap(src, dpos.x(), dpos.y());
}

void TJS2NativeLayer::stretch_copy(const my::IRect &dst_rect,
                                   const my::IRect &src_rect,
                                   TJS2NativeLayer *src_layer,
                                   TJS2StretchType type) {

    GLOG_D("%p:stretch copy: dst: %s src: %s type: %d", this->this_obj(),
           format_rect(dst_rect).c_str(), format_rect(src_rect).c_str(), type);

    SkBitmap src{};
    src.allocN32Pixels(src_rect.width(), src_rect.height());
    src_layer->main_surface()->readPixels(src, src_rect.x(), src_rect.y());

    this->canvas()->drawBitmapRect(src, my::Rect::Make(dst_rect), nullptr);
}

void TJS2NativeLayer::start_trans(const std::u16string &name, bool withchildren,
                                  TJS2NativeLayer *trans_src,
                                  tTJSVariantClosure options) {
    if (this->is_primary_layer()) {
        GLOG_W("start trans target is primary %p", this->this_obj());
    }

    GLOG_D("%p:start_trans: %p with children %d", this->this_obj(),
           trans_src->this_obj(), withchildren);

    if (this == trans_src) {
        return;
    }

    auto this_primary = this->is_primary_layer();
    auto src_primary = trans_src->is_primary_layer();

    auto src_parent = trans_src->parent();
    auto this_parent = this->parent();

    auto this_pos = this->pos();
    auto src_pos = trans_src->pos();
    auto this_visible = this->is_visible();
    auto src_visible = trans_src->is_visible();

    auto *this_ancestor_child = this->get_ancestor_child(trans_src);
    auto *src_ancestor_child = trans_src->get_ancestor_child(this);

    if (this_primary) {
        this->_win->set_primary_layer(nullptr);
    }

    if (src_primary) {
        trans_src->_win->set_primary_layer(nullptr);
    }

    this->remove_parent();
    trans_src->remove_parent();

    if (this_ancestor_child) {
        if (this_ancestor_child != this)
            this_ancestor_child->remove_parent();

        if (withchildren) {
            this->set_parent(src_parent);

            if (trans_src == this_parent) {
                trans_src->set_parent(this);
            } else {
                trans_src->set_parent(this_parent);
            }

        } else {
            throw std::runtime_error("with children false not be impl");
        }

        if (this_ancestor_child != this)
            this_ancestor_child->set_parent(this);

    } else if (src_ancestor_child) {
        if (src_ancestor_child != trans_src)
            src_ancestor_child->remove_parent();
        if (withchildren) {
            if (this == src_parent) {
                this->set_parent(trans_src);
            } else {
                this->set_parent(src_parent);
            }
            trans_src->set_parent(this_parent);
        } else {
            throw std::runtime_error("with children false not be impl");
        }
        if (src_ancestor_child != trans_src)
            src_ancestor_child->set_parent(trans_src);
    } else {
        if (withchildren) {
            this->set_parent(src_parent);
            trans_src->set_parent(this_parent);
        } else {
            throw std::runtime_error("with children false not be impl");
        }
    }

    this->set_pos(src_pos);
    trans_src->set_pos(this_pos);
    this->set_visible(src_visible);
    trans_src->set_visible(this_visible);

    if (this_primary) {
        this->_win->set_primary_layer(trans_src);
    }

    if (src_primary) {
        trans_src->_win->set_primary_layer(this);
    }

    GLOG_D("trans completed target visible %d %s src visible %d %s",
           this->is_visible(), format_point(this->pos()).c_str(),
           trans_src->is_visible(), format_point(trans_src->pos()).c_str());

    TJS::func_call(this->this_obj(), "onTransitionCompleted",
                   {tTJSVariant(this->this_obj(), this->this_obj()),
                    tTJSVariant(trans_src->this_obj(), trans_src->this_obj())});
}

void TJS2NativeLayer::stop_trans() {}

void TJS2NativeLayer::bind_to_front() {
    auto parent = this->parent();
    if (!parent) {
        return;
    }
    auto &children = parent->_children;

    auto it = std::find(children.begin(), children.end(), this);
    if (it != children.end()) {
        children.remove(*it);
        children.push_front(*it);
        // children.splice(children.begin(), children, it, std::next(it));
    }
}
void TJS2NativeLayer::bind_to_back() {
    auto parent = this->parent();
    if (!parent) {
        return;
    }
    auto &children = parent->_children;

    auto it = std::find(children.begin(), children.end(), this);
    if (it != children.end()) {
        children.remove(*it);
        children.push_back(*it);
        // children.splice(children.end(), children, it, std::next(it));
    }
}

bool TJS2NativeLayer::is_parent_enable() const {
    auto par = this->parent();
    while (par) {
        if (!par->enabled) {
            return false;
        }
        par = par->parent();
    }
    return true;
}

bool TJS2NativeLayer::is_node_enable() const {
    return this->is_parent_enable() && this->enabled;
}

bool TJS2NativeLayer::is_parent_visible() const {
    auto par = this->parent();
    while (par) {
        if (!par->is_visible()) {
            return false;
        }
        par = par->parent();
    }
    return true;
}

bool TJS2NativeLayer::is_node_visible() const {
    return this->is_parent_visible() && this->is_visible();
}

void TJS2NativeLayer::build_surface(
    sk_sp<SkSurface> &surface,
    std::function<sk_sp<SkSurface>(const my::ISize2D &)> &&make_surface) {

    auto size = this->size();
    auto image_size = this->image_size();
    my::ISize2D surface_size = image_size;

    if (image_size.width() < size.width() ||
        image_size.height() < size.height()) {
        surface_size = size;
    }

    if (!surface) {
        surface = make_surface(surface_size);
        return;
    }

    auto current_surface_size =
        my::ISize2D::Make(surface->width(), surface->height());
    if (current_surface_size != surface_size) {
        surface = make_surface(surface_size);
    }

    surface->flush();
}

sk_sp<SkImage> TJS2NativeLayer::image_snapshot() {
    auto surface = this->main_surface();
    auto bound =
        my::IRect::MakeXYWH(-this->image_pos().x(), -this->image_pos().y(),
                            this->size().width(), this->size().height());

    auto surface_bound = my::IRect::MakeWH(surface->width(), surface->height());
    auto image_snapshot = this->main_surface()->makeImageSnapshot(bound);

    assert(image_snapshot);
    return image_snapshot;
}

void TJS2NativeLayer::load_image(const std::u16string &_path) {
    auto path = my::fs::path(codecvt::utf_to_utf<char>(_path));

    GLOG_D("%p:%s:load image: %s", this->this_obj(),
           codecvt::utf_to_utf<char>(this->name).c_str(), path.c_str());

    if (path.has_extension()) {
        this->_image = TJS2NativeStorages::get()->get_storage<my::Image>(path);
    } else {
        for (auto extension : {".png", ".jpg", ".bmp"}) {
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
    }

    GLOG_D("%p:%s:load image finish: %s %d=%d", this->this_obj(),
           codecvt::utf_to_utf<char>(this->name).c_str(), path.c_str(),
           this->_image->size().width(), this->_image->size().height());

    auto image_size = this->_image->size();
    this->set_size(image_size);
    this->set_image_size(image_size);

    // auto [x, y] = this->image_pos();
    SkPaint paint{};
    paint.setBlendMode(SkBlendMode::kSrc);
    this->canvas()->drawImage(this->_image->sk_image(), 0, 0, &paint);
}

void TJS2NativeLayer::save_layer_image(const std::u16string &name,
                                       const std::u16string &type) {
    auto path = my::fs::path(
        my::uri(codecvt::utf_to_utf<char>(name)).encoded_path().to_string());

    auto data = this->image_snapshot()->encodeToData();
    auto blob = my::Blob::make(data->data(), data->size());
    auto image = my::Image::make(blob);
    
    image->export_bmp24(path);

    GLOG_D("save layer image:%p:%s:%s:%s:%s", this,
           codecvt::utf_to_utf<char>(this->name).c_str(),
           codecvt::utf_to_utf<char>(name).c_str(),
           codecvt::utf_to_utf<char>(type).c_str(),
           format_rect(this->layer_rect()).c_str());
}

void TJS2NativeLayer::set_image_pos(const my::IPoint2D &pos) {
    this->_image_pos = pos;

    // if (this->_image) {
    //     auto [x, y] = this->image_pos();
    //     SkPaint paint{};
    //     paint.setBlendMode(SkBlendMode::kSrc);
    //     this->canvas()->drawImage(this->_image->sk_image(), x, y, &paint);
    // }
}

void TJS2NativeLayer::set_face(TJS2DrawFace face) {
    if (face == TJS2DrawFace::dfAuto) {
        this->_face = convert_draw_face(this->type);
    } else {
        this->_face = face;
    }
}

void TJS2NativeLayer::set_parent(TJS2NativeLayer *parent) {

    this->remove_parent();

    this->_parent = parent;

    if (parent) {
        parent->add_children(this);
    }
}

iTJSDispatch2 *TJS2NativeLayer::this_obj() const {
    if (!this->_this_obj) {
        printf("%p:%s\n", this, codecvt::utf_to_utf<char>(this->name).c_str());
        fflush(stdout);
    }
    assert(this->_this_obj);
    return this->_this_obj;
}

iTJSDispatch2 *TJS2NativeLayer::get_children_obj() const {
    iTJSDispatch2 *classobj;
    auto childrens = TJSCreateArrayObject(&classobj);

    int count = 0;
    for (auto &child : this->children()) {
        iTJSDispatch2 *dsp = child->this_obj();
        tTJSVariant val(dsp, dsp);
        childrens->PropSetByNum(TJS_MEMBERENSURE, count, &val, childrens);
        ++count;
    }
    return childrens;
}

TJS2NativeLayer *
TJS2NativeLayer::get_ancestor_child(TJS2NativeLayer *ancestor) {
    auto *c = this;
    auto *p = this->parent();

    while (p) {
        if (p == ancestor)
            return c;
        c = p;
        p = p->parent();
    }
    return nullptr;
}

bool TJS2NativeLayer::hit_test(const my::IPoint2D &pos) {
    bool result{true};
    uint32_t alpha{0};
    switch (this->hit_type) {
    case TJS2HitType::htMask: {
        alpha = SkColorGetA(
            this->get_surface_pixel<SkColor>(this->main_surface(), pos));
        if (alpha < this->hit_threshold) {
            result = false;
        }
        break;
    }
    case TJS2HitType::htProvince: {
        alpha = this->get_surface_pixel<uint8_t>(this->province_surface(), pos);
        if (alpha == 0) {
            result = false;
        }
        break;
    }
    }

    this->hit_test_work = true;

    if (result) {
        TJS::func_call(this->this_obj(), "onHitTest", {pos.x(), pos.y(), true});
        result = this->hit_test_work;
    }

    return result;
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
        _this->hit_type = (TJS2HitType)(tjs_int)*param;
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
        *result = (int)_this->face();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->set_face(((krkrz::TJS2DrawFace)(int)*param));
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
        auto parent = _this->get_parent_obj();
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
    TJS_BEGIN_NATIVE_PROP_DECL(hint) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = TJS_W("");
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        // _this->SetHint(*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(hint)
    TJS_BEGIN_NATIVE_PROP_DECL(showParentHint) {

        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->show_parent_hint;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->show_parent_hint = *param;
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

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ doGrayScale) {
        // TODO: Layer.doGrayScale
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        // _this->DoGrayScale();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ doGrayScale)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ bringToBack) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        _this->bind_to_back();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ bringToBack)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ bringToFront) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        _this->bind_to_front();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ bringToFront)

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
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->enabled;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->enabled = param->operator bool();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(enabled)
    TJS_BEGIN_NATIVE_PROP_DECL(nodeEnabled) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->is_node_enable();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(nodeEnabled)

    TJS_BEGIN_NATIVE_PROP_DECL(focused) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = (tjs_int)_this->focused;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(focused)

    TJS_BEGIN_NATIVE_PROP_DECL(nodeVisible) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        *result = _this->is_node_visible();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_DENY_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(nodeVisible)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ focus) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        _this->focused = true;
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ focus)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getProvincePixel) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 2)
            return TJS_E_BADPARAMCOUNT;
        if (result)
            *result = _this->get_surface_pixel<uint8_t>(
                _this->province_surface(),
                my::IPoint2D::Make(*param[0], *param[1]));
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ getProvincePixel)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setProvincePixel) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 3)
            return TJS_E_BADPARAMCOUNT;
        _this->set_surface_pixel(_this->province_surface(),
                                 my::IPoint2D::Make(*param[0], *param[1]),
                                 (uint8_t)(int)*param[2]);
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
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        // parameters are : <name>, [<withchildren>=true], [<transwith>=null],
        //                  [<options>]
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        ttstr name = *param[0];
        bool withchildren = true;
        if (numparams >= 2 && param[1]->Type() != tvtVoid)
            withchildren = param[1]->operator bool();
        TJS2NativeLayer *trans_src = nullptr;
        if (numparams >= 3 && param[2]->Type() != tvtVoid) {
            tTJSVariantClosure clo = param[2]->AsObjectClosureNoAddRef();
            if (clo.Object) {
                if (TJS_FAILED(clo.Object->NativeInstanceSupport(
                        TJS_NIS_GETINSTANCE, TJS2Layer::ClassID,
                        (iTJSNativeInstance **)&trans_src)))
                    TVPThrowExceptionMessage(TVPSpecifyLayer);
            }
        }
        if (!trans_src)
            TVPThrowExceptionMessage(TVPSpecifyLayer);

        tTJSVariantClosure options(nullptr, nullptr);
        if (numparams >= 4 && param[3]->Type() != tvtVoid)
            options = param[3]->AsObjectClosureNoAddRef();

        _this->start_trans(name.AsStdString(), withchildren, trans_src,
                           options);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ beginTransition)
    //----------------------------------------------------------------------
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ stopTransition) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        _this->stop_trans();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ stopTransition)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ piledCopy) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 7)
            return TJS_E_BADPARAMCOUNT;

        TJS2NativeLayer *src{nullptr};
        tTJSVariantClosure clo = param[2]->AsObjectClosureNoAddRef();
        if (clo.Object) {
            clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
                                              TJS2Layer::ClassID,
                                              (iTJSNativeInstance **)&src);
        }
        if (!src)
            TVPThrowExceptionMessage(TVPSpecifyLayer);

        _this->piled_copy(
            my::IPoint2D::Make(*param[0], *param[1]),
            my::IRect::MakeXYWH(*param[3], *param[4], *param[5], *param[6]),
            src);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ piledCopy)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ stretchCopy) {
        // dx, dy, dw, dh, src, sx, sy, sw, sh, type=0
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 9)
            return TJS_E_BADPARAMCOUNT;

        tTJSVariantClosure clo = param[4]->AsObjectClosureNoAddRef();
        TJS2NativeLayer *srclayer = nullptr;
        if (clo.Object) {
            clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
                                              TJS2Layer::ClassID,
                                              (iTJSNativeInstance **)&srclayer);
        }
        if (!srclayer)
            TVPThrowExceptionMessage(TVPSpecifyLayerOrBitmap);

        auto dest_rect =
            my::IRect::MakeXYWH(*param[0], *param[1], *param[2], *param[3]);

        auto src_rect =
            my::IRect::MakeXYWH(*param[5], *param[6], *param[7], *param[8]);

        auto type = TJS2StretchType::stNearest;
        if (numparams >= 10)
            type = (TJS2StretchType)(int)*param[9];

        tjs_real typeopt = 0.0;
        if (numparams >= 11)
            typeopt = (tjs_real)*param[10];
        else if (type == TJS2StretchType::stFastCubic ||
                 type == TJS2StretchType::stCubic)
            typeopt = -1.0;

        _this->stretch_copy(dest_rect, src_rect, srclayer, type);
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ stretchCopy)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ saveLayerImage) {
        // TODO: Layer.saveLayerImage
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;
        ttstr name(*param[0]);
        ttstr type(TJS_W("bmp"));
        if (numparams >= 2 && param[1]->Type() != tvtVoid)
            type = *param[1];
        _this->save_layer_image(name.AsStdString(), type.AsStdString());
        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ saveLayerImage)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ operateRect) {
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
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ copyRect) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);
        if (numparams < 7)
            return TJS_E_BADPARAMCOUNT;

        if (numparams < 7)
            return TJS_E_BADPARAMCOUNT;
        TJS2NativeLayer *srclayer{};
        tTJSVariantClosure clo = param[2]->AsObjectClosureNoAddRef();
        if (clo.Object) {
            clo.Object->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
                                              TJS2Layer::ClassID,
                                              (iTJSNativeInstance **)&srclayer);
        }
        if (!srclayer)
            TVPThrowExceptionMessage(TVPSpecifyLayerOrBitmap);

        _this->copy_rect({*param[0], *param[1]}, srclayer,
                         {*param[3], *param[4]}, {*param[5], *param[6]});

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ copyRect)

    // event
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseDown) {

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(4, "onMouseDown", objthis);
            TVP_ACTION_INVOKE_MEMBER("x");
            TVP_ACTION_INVOKE_MEMBER("y");
            TVP_ACTION_INVOKE_MEMBER("button");
            TVP_ACTION_INVOKE_MEMBER("shift");
            TVP_ACTION_INVOKE_END(obj);
        }

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

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseMove) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(3, "onMouseMove", objthis);
            TVP_ACTION_INVOKE_MEMBER("x");
            TVP_ACTION_INVOKE_MEMBER("y");
            TVP_ACTION_INVOKE_MEMBER("shift");
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseMove)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onDoubleClick) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(2, "onDoubleClick", objthis);
            TVP_ACTION_INVOKE_MEMBER("x");
            TVP_ACTION_INVOKE_MEMBER("y");
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onDoubleClick)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseUp) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(4, "onMouseUp", objthis);
            TVP_ACTION_INVOKE_MEMBER("x");
            TVP_ACTION_INVOKE_MEMBER("y");
            TVP_ACTION_INVOKE_MEMBER("button");
            TVP_ACTION_INVOKE_MEMBER("shift");
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseUp)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseEnter) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(0, "onMouseEnter", objthis);
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseEnter)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onMouseLeave) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(0, "onMouseLeave", objthis);
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onMouseLeave)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onBeforeFocus) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(3, "onBeforeFocus", objthis);
            TVP_ACTION_INVOKE_MEMBER("layer");
            TVP_ACTION_INVOKE_MEMBER("blurred");
            TVP_ACTION_INVOKE_MEMBER("direction");
            TVP_ACTION_INVOKE_END(obj);
        }

        // set focusable layer back
        // if (param[0]->Type() != tvtVoid) {
        //     tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();
        //     if (clo.Object) {
        //         tTJSNI_BaseLayer *src;
        //         if (clo.Object) {
        //             if (TJS_FAILED(clo.Object->NativeInstanceSupport(
        //                     TJS_NIS_GETINSTANCE, tTJSNC_Layer::ClassID,
        //                     (iTJSNativeInstance **)&src)))
        //                 TVPThrowExceptionMessage(TVPSpecifyLayer);
        //         }
        //         if (!src)
        //             TVPThrowExceptionMessage(TVPSpecifyLayer);
        //         _this->SetFocusWork(src);
        //     } else {
        //         _this->SetFocusWork(NULL);
        //     }
        // } else {
        //     _this->SetFocusWork(NULL);
        // }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onBeforeFocus)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onFocus) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(1, "onFocus", objthis);
            TVP_ACTION_INVOKE_MEMBER("blurred");
            TVP_ACTION_INVOKE_MEMBER("direction");
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onFocus)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onBlur) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(1, "onBlur", objthis);
            TVP_ACTION_INVOKE_MEMBER("focused");
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onBlur)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onHitTest) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        // tTJSVariantClosure obj = _this->GetActionOwnerNoAddRef();
        /*
                this event does not call "action" method
        if(obj.Object)
        {
                TVP_ACTION_INVOKE_BEGIN(3, "onHitTest", objthis);
                TVP_ACTION_INVOKE_MEMBER("x");
                TVP_ACTION_INVOKE_MEMBER("y");
                TVP_ACTION_INVOKE_MEMBER("hit");
                TVP_ACTION_INVOKE_END(obj);
        }
        */
        if (numparams < 3)
            return TJS_E_BADPARAMCOUNT;
        bool b = param[2]->operator bool();

        _this->hit_test_work = b;

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onHitTest)
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
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onTransitionCompleted) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeLayer);

        tTJSVariantClosure obj = _this->this_action_obj();
        if (obj.Object) {
            TVP_ACTION_INVOKE_BEGIN(2, "onTransitionCompleted", objthis);
            TVP_ACTION_INVOKE_MEMBER("dest"); // destination (before exchanging)
            TVP_ACTION_INVOKE_MEMBER(
                "src"); // source (also before exchanging;can be a null)
            TVP_ACTION_INVOKE_END(obj);
        }

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onTransitionCompleted)
    TJS_END_NATIVE_MEMBERS
} // namespace krkrz

} // namespace krkrz
