#pragma once

#include <util/logger.h>

#include "tjs2_font.h"
#include "tjs2_lib.h"
#include "tjs2_system.h"
#include "tjs2_window.h"
#include <MsgIntf.h>
#include <tjs2_lib/tjs2_storages.h>

namespace krkrz {

enum TJS2DrawFace {
    dfBoth = 0,
    dfAlpha = 0,
    dfAddAlpha = 4,
    dfMain = 1,
    dfOpaque = 1,
    dfMask = 2,
    dfProvince = 3,
    dfAuto = 128 // face is chosen automatically from the layer type
};

enum TJS2LayerType {
    ltBinder = 0,
    ltCoverRect = 1,
    ltOpaque = 1,      // the same as ltCoverRect
    ltTransparent = 2, // alpha blend
    ltAlpha = 2,       // the same as ltTransparent
    ltAdditive = 3,
    ltSubtractive = 4,
    ltMultiplicative = 5,
    ltEffect = 6,
    ltFilter = 7,
    ltDodge = 8,
    ltDarken = 9,
    ltLighten = 10,
    ltScreen = 11,
    ltAddAlpha = 12, // additive alpha blend
    ltPsNormal = 13,
    ltPsAdditive = 14,
    ltPsSubtractive = 15,
    ltPsMultiplicative = 16,
    ltPsScreen = 17,
    ltPsOverlay = 18,
    ltPsHardLight = 19,
    ltPsSoftLight = 20,
    ltPsColorDodge = 21,
    ltPsColorDodge5 = 22,
    ltPsColorBurn = 23,
    ltPsLighten = 24,
    ltPsDarken = 25,
    ltPsDifference = 26,
    ltPsDifference5 = 27,
    ltPsExclusion = 28
};

enum TJS2BlendOperationMode {
    omPsNormal = ltPsNormal,
    omPsAdditive = ltPsAdditive,
    omPsSubtractive = ltPsSubtractive,
    omPsMultiplicative = ltPsMultiplicative,
    omPsScreen = ltPsScreen,
    omPsOverlay = ltPsOverlay,
    omPsHardLight = ltPsHardLight,
    omPsSoftLight = ltPsSoftLight,
    omPsColorDodge = ltPsColorDodge,
    omPsColorDodge5 = ltPsColorDodge5,
    omPsColorBurn = ltPsColorBurn,
    omPsLighten = ltPsLighten,
    omPsDarken = ltPsDarken,
    omPsDifference = ltPsDifference,
    omPsDifference5 = ltPsDifference5,
    omPsExclusion = ltPsExclusion,
    omAdditive = ltAdditive,
    omSubtractive = ltSubtractive,
    omMultiplicative = ltMultiplicative,
    omDodge = ltDodge,
    omDarken = ltDarken,
    omLighten = ltLighten,
    omScreen = ltScreen,
    omAlpha = ltTransparent,
    omAddAlpha = ltAddAlpha,
    omOpaque = ltCoverRect,

    omAuto = 128 // operation mode is guessed from the source layer type
};

enum TJS2StretchType {
    stNearest = 0, // primal method; nearest neighbor method
    stFastLinear =
        1,        // fast linear interpolation (does not have so much precision)
    stLinear = 2, // (strict) linear interpolation
    stCubic = 3,  // cubic interpolation
    stSemiFastLinear = 4,
    stFastCubic = 5,
    stLanczos2 = 6, // Lanczos 2 interpolation
    stFastLanczos2 = 7,
    stLanczos3 = 8, // Lanczos 3 interpolation
    stFastLanczos3 = 9,
    stSpline16 = 10, // Spline16 interpolation
    stFastSpline16 = 11,
    stSpline36 = 12, // Spline36 interpolation
    stFastSpline36 = 13,
    stAreaAvg = 14, // Area average interpolation
    stFastAreaAvg = 15,
    stGaussian = 16,
    stFastGaussian = 17,
    stBlackmanSinc = 18,
    stFastBlackmanSinc = 19,

    stTypeMask = 0x0000ffff, // stretch type mask
    stFlagMask = 0xffff0000, // flag mask

    stRefNoClip =
        0x10000 // referencing source is not limited by the given rectangle
                // (may allow to see the border pixel to interpolate)
};

enum class LayerEventType { LAYER_RESIZE, LAYER_POS_CHANGE };

struct LayerEvent {
    LayerEventType type;
    TJS2NativeLayer *layer;
};

class TJS2NativeLayer : public tTJSNativeInstance {
  public:
    TJS2NativeLayer();

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) override;

    void TJS_INTF_METHOD Invalidate() override;

    SkColor color_convert(uint32_t color, int opa) {
        return SkColorSetARGB(opa, color & 0x00ff0000 >> 16,
                              color & 0x0000ff00 >> 8, color & 0x000000ff);
    }

    void color_rect(const my::IRect &, uint32_t color, int opa = 255);

    void fill_rect(const my::IRect &, uint32_t color);

    void draw_text(const my::IPoint2D &, const std::u16string &text,
                   uint32_t color, int opa, bool aa, int shadowlevel,
                   uint32_t shadowcolor, int shadowwidth, int shadowofsx,
                   int shadowofsy);

    void load_image(const std::u16string &_path);

    void assign_images(TJS2NativeLayer *layer) {
        this->_main_surface = layer->_main_surface;
        this->_province_surface = layer->_province_surface;
        this->_image = layer->_image;
    }

    std::shared_ptr<my::Image> image() { return this->_image; }

    void set_image_modified(bool v) { this->_image_modified = v; }

    bool is_image_modified() { return this->_image_modified; }

    void operate_rect(const my::IPoint2D &d_off, TJS2NativeLayer *layer,
                      const my::IPoint2D &s_off, const my::ISize2D &s_size,
                      TJS2BlendOperationMode mode);

    void copy_rect(const my::IPoint2D &d_off, TJS2NativeLayer *layer,
                   const my::IPoint2D &s_off, const my::ISize2D &s_size);

    void stretch_copy(const my::IRect &dst, const my::IRect &src,
                      TJS2NativeLayer *src_layer, TJS2StretchType type);

    void start_trans(const std::u16string &name, bool withchildren,
                     TJS2NativeLayer *trans_src, tTJSVariantClosure options);
    void stop_trans();

    void bind_to_front();
    void bind_to_back();

    bool is_parent_visible() const;
    bool is_node_visible() const;

    SkCanvas *canvas() { return this->main_surface()->getCanvas(); }
    SkCanvas *province_canvas() {
        return this->province_surface()->getCanvas();
    }

    sk_sp<SkSurface> province_surface() {
        build_surface(&this->_province_surface, [](const my::ISize2D &size) {
            return SkSurface::MakeRaster(
                SkImageInfo::MakeA8(size.width(), size.height()));
        });
        return this->_province_surface;
    }

    template <typename T>
    T get_surface_pixel(sk_sp<SkSurface> surface, const my::IPoint2D &pos) {
        SkBitmap bitmap;
        bitmap.allocPixels(surface->imageInfo().makeWH(1, 1));
        surface->readPixels(bitmap, pos.x(), pos.y());
        return *(T *)bitmap.getAddr(0, 0);
    }

    template <typename T>
    void set_surface_pixel(sk_sp<SkSurface> surface, const my::IPoint2D &pos,
                           T v) {
        SkBitmap bitmap;
        bitmap.allocPixels(surface->imageInfo().makeWH(1, 1));
        auto addr = bitmap.getAddr(0, 0);
        *(T *)addr = v;
        surface->writePixels(bitmap, pos.x(), pos.y());
    }

    sk_sp<SkSurface> main_surface() {
        build_surface(&this->_main_surface, [](const my::ISize2D &size) {
            return SkSurface::MakeRasterN32Premul(size.width(), size.height());
        });
        return this->_main_surface;
    }

    void build_surface(sk_sp<SkSurface> *surface,
                       std::function<sk_sp<SkSurface>(const my::ISize2D &)> &&);

    sk_sp<SkImage> image_snapshot() {
        return this->main_surface()->makeImageSnapshot();
    }

    my::IRect layer_rect() {
        return my::IRect::MakeXYWH(this->pos().x(), this->pos().y(),
                                   this->size().width(), this->size().height());
    }

    void update() { this->call_on_paint = true; }

    void set_visible(bool v) {
        if (this->_visible != v) {
            this->_visible = v;

            this->update();
        }
    }

    bool is_visible() const { return this->_visible; }

    void set_face(TJS2DrawFace face);

    TJS2DrawFace face() {
        return this->_face;
    }

    TJS2BlendOperationMode blend_mode() {

        // returns corresponding blend operation mode from layer type

        switch (this->type) {
            //	case ltBinder:
        case ltOpaque:
            return omOpaque;
        case ltAlpha:
            return omAlpha;
        case ltAdditive:
            return omAdditive;
        case ltSubtractive:
            return omSubtractive;
        case ltMultiplicative:
            return omMultiplicative;
            //	case ltEffect:
            //	case ltFilter:
        case ltDodge:
            return omDodge;
        case ltDarken:
            return omDarken;
        case ltLighten:
            return omLighten;
        case ltScreen:
            return omScreen;
        case ltAddAlpha:
            return omAddAlpha;
        case ltPsNormal:
            return omPsNormal;
        case ltPsAdditive:
            return omPsAdditive;
        case ltPsSubtractive:
            return omPsSubtractive;
        case ltPsMultiplicative:
            return omPsMultiplicative;
        case ltPsScreen:
            return omPsScreen;
        case ltPsOverlay:
            return omPsOverlay;
        case ltPsHardLight:
            return omPsHardLight;
        case ltPsSoftLight:
            return omPsSoftLight;
        case ltPsColorDodge:
            return omPsColorDodge;
        case ltPsColorDodge5:
            return omPsColorDodge5;
        case ltPsColorBurn:
            return omPsColorBurn;
        case ltPsLighten:
            return omPsLighten;
        case ltPsDarken:
            return omPsDarken;
        case ltPsDifference:
            return omPsDifference;
        case ltPsDifference5:
            return omPsDifference5;
        case ltPsExclusion:
            return omPsExclusion;

        default:
            return omOpaque;
        }
    }

    iTJSDispatch2 *this_obj() {
        assert(this->_this_obj);
        return this->_this_obj;
    }

    tTJSVariantClosure this_action_obj() const {
        return this->_this_action_obj;
    }

    iTJSDispatch2 *get_font() { return this->_font->this_obj(); }

    TJS2NativeWindow *get_window() {
        assert(this->_win);
        return this->_win;
    }

    my::WindowMgr *get_window_mgr() { return this->_win_mgr; }

    void set_parent(TJS2NativeLayer *parent);

    iTJSDispatch2 *get_parent_obj() {
        if (this->parent()) {
            return this->parent()->this_obj();
        } else {
            return nullptr;
        }
    }

    iTJSDispatch2 *get_children_obj() const;

    TJS2NativeLayer *parent() const { return this->_parent; }
    const std::list<TJS2NativeLayer *> &children() const {
        return this->_children;
    }

    TJS2NativeLayer *get_ancestor_child(TJS2NativeLayer *);

    bool is_primary_layer() { return this->_win->get_primary_layer() == this; }

    void set_size(const my::ISize2D &size) { this->_size = size; }

    my::ISize2D size() { return this->_size; }

    void set_pos(const my::IPoint2D &pos) { this->_pos = pos; }

    my::IPoint2D pos() { return this->_pos; }
    void set_image_size(const my::ISize2D &size) { this->_image_size = size; }

    my::ISize2D image_size() { return this->_image_size; }

    void set_image_pos(const my::IPoint2D &pos) { this->_image_pos = pos; }

    my::IPoint2D image_pos() { return this->_image_pos; }

    rxcpp::observable<LayerEvent> on_event() {
        return this->_event_suject.get_observable();
    }

    void post(LayerEventType type) {
        this->_event_suject.get_subscriber().on_next(LayerEvent{type, this});
    };

    // TODO:
    TJS2LayerType type{};
    int hit_type{};
    int hit_threshold{};

    int cursor;
    std::u16string name;

    bool focusable{false};
    bool focused{false};
    bool enabled{false};
    bool absolute_order_mode{false};
    bool hold_alpha{false};
    bool call_on_paint{false};
    bool show_parent_hint{false};

  private:
    iTJSDispatch2 *_this_obj{};
    tTJSVariantClosure _this_action_obj{nullptr, nullptr};
    TJS2NativeLayer *_parent{};
    std::list<TJS2NativeLayer *> _children{};
    TJS2NativeFont *_font{};
    TJS2NativeWindow *_win{};
    my::WindowMgr *_win_mgr{};

    rxcpp::subjects::subject<LayerEvent> _event_suject{};

    std::shared_ptr<my::Image> _image{};
    sk_sp<SkSurface> _province_surface{};
    sk_sp<SkSurface> _main_surface{};

    bool _visible{false};
    bool _image_modified{false};
    TJS2DrawFace _face{};
    
    my::ISize2D _size{32, 32};
    my::IPoint2D _pos{};
    my::ISize2D _image_size{32, 32};
    my::IPoint2D _image_pos{};

    void add_children(TJS2NativeLayer *layer) {
        if (!layer) {
            return;
        }
        this->_children.push_back(layer);

        layer->on_event().subscribe([](LayerEvent e) {
            switch (e.type) {
            case LayerEventType::LAYER_POS_CHANGE:

                break;
            case LayerEventType::LAYER_RESIZE:

                break;
            }
        });
    }

    void remove_children(TJS2NativeLayer *layer) {
        this->_children.remove(layer);
    }

    void remove_parent() {
        if (this->parent()) {
            this->parent()->remove_children(this);
            this->_parent = nullptr;
        }
    }
};

class TJS2Layer : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Layer();
    static tjs_uint32 ClassID;
    static TJS2Layer *get() {
        auto instance = getNoRef();
        instance->AddRef();
        return instance;
    }
    static TJS2Layer *getNoRef() {
        static TJS2Layer instance;
        return &instance;
    }

    static iTJSDispatch2 *create() {
        iTJSDispatch2 *out;
        auto _this = TJS2Layer::getNoRef();
        if (TJS_FAILED(
                _this->CreateNew(0, nullptr, nullptr, &out, 0, nullptr, _this)))
            TVPThrowInternalError;
        return out;
    }

  protected:
    tTJSNativeInstance *CreateNativeInstance() { return new TJS2NativeLayer; }
};

std::string format_point(const my::IPoint2D &point);

std::string format_rect(const my::IRect &rect);

void draw_layer(SkCanvas *, TJS2NativeLayer *, bool check_visible = true);

} // namespace krkrz
