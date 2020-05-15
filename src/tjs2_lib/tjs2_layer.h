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

    void assign_images(TJS2NativeLayer *layer) { this->_image = layer->_image; }

    void set_image_modified(bool v) { this->_image_modified = v; }

    bool is_image_modified() { return this->_image_modified; }

    void operate_rect(const my::IPoint2D &d_off, TJS2NativeLayer *layer,
                      const my::IPoint2D &s_off, const my::ISize2D &s_size,
                      TJS2BlendOperationMode mode) {
        GLOG_D("operate rect: d_off:%d,%d s_off:%d,%d s_size:%d,%d mode:%d "
               "layer:%p",
               d_off.x(), d_off.y(), s_off.x(), s_off.y(), s_size.width(),
               s_size.height(), mode, layer->this_obj());
        set_image_modified(true);
    }

    SkCanvas *canvas() {
        if (!this->_surface) {
            this->_surface = SkSurface::MakeRasterN32Premul(
                this->_size.width(), this->_size.height());
        }

        return this->_surface ? this->_surface->getCanvas() : nullptr;
    }

    sk_sp<SkImage> image_snapshot() {
        assert(this->_surface);
        this->_surface->flush();
        return this->_surface->makeImageSnapshot();
    }

    my::IRect layer_rect() {
        return my::IRect::MakeXYWH(this->pos().x(), this->pos().y(),
                                   this->size().width(), this->size().height());
    }

    void update() {
        this->call_on_paint = true;
    }

    void set_visible(bool v) {
        if (this->_visible != v) {
            this->_visible = v;

            if (!this->_surface) {
                this->_surface = SkSurface::MakeRasterN32Premul(
                    this->_size.width(), this->_size.height());
            }

            this->update();
        }
    }

    bool is_visible() { return this->_visible; }

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

    void set_parent(TJS2NativeLayer *parent) {
        if (this->_parent) {
            this->_parent->remove_children(this);
        }

        if (parent) {
            parent->add_children(this);
        }

        this->_parent = parent;
    }

    iTJSDispatch2 *get_parent() { return this->_parent->this_obj(); }

    iTJSDispatch2 *get_children_obj() const;

    const std::list<TJS2NativeLayer *> &get_children() const {
        return this->_children;
    }

    bool is_primary_layer() { return this->_win->get_primary_layer() == this; }

    void set_size(const my::ISize2D &size) {
        this->_size = size;

        if (this->_surface) {
            this->_surface =
                SkSurface::MakeRasterN32Premul(size.width(), size.height());
        }
    }

    my::ISize2D size() { return this->_size; }

    void set_pos(const my::IPoint2D &pos) {
        this->_pos = pos;
    }

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
    TJS2DrawFace face{};

    int cursor;
    std::u16string name;

    bool focusable{false};
    bool absolute_order_mode{false};
    bool hold_alpha{false};
    bool call_on_paint{false};

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
    sk_sp<SkSurface> _surface{};

    bool _visible{false};
    bool _image_modified{false};

    my::ISize2D _size{};
    my::IPoint2D _pos{};
    my::ISize2D _image_size{};
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

} // namespace krkrz
