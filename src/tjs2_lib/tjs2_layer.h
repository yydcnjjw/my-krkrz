#pragma once

#include <render/canvas.h>
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

class TJS2NativeLayer : public tTJSNativeInstance {
  public:
    TJS2NativeLayer();

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) override;

    void TJS_INTF_METHOD Invalidate() override {
        this->_this_action_obj.Release();
        this->_this_action_obj = {nullptr, nullptr};
        auto font_obj = this->_font->this_obj();
        font_obj->Invalidate(0, nullptr, nullptr, font_obj);
        font_obj->Release();
    }

    my::ColorRGBAub color_convert(uint32_t color, int opa) {
        return my::ColorRGBAub{color & 0x00ff0000 >> 16,
                               color & 0x0000ff00 >> 8, color & 0x000000ff,
                               opa};
    }

    void color_rect(int x, int y, int w, int h, uint32_t color, int opa = 255) {
        x = x + this->pos.x;
        y = y + this->pos.y;

        color = krkrz::TJSToActualColor(color);

        if (opa < 0) {
            if (this->face == TJS2DrawFace::dfAlpha) {
                color = 0xffffff;
                opa = 255 + opa;
            } else {
                opa = 255;
            }
        }

        my::ColorRGBAub col = this->color_convert(color, opa);

        // bound check
        if (x < 0) {
            x = 0;
        }
        if (y < 0) {
            y = 0;
        }
        int right{x + w};
        if (right > this->size.w) {
            right = this->size.w;
        }
        int bottom{y + h};
        if (bottom > this->size.h) {
            bottom = this->size.h;
        }

        GLOG_D("color rect face=%d, {%d,%d}={%d,%d} color %d %d %d %d",
               this->face, x, y, right, bottom, color & 0x00ff0000 >> 16,
               color & 0x0000ff00 >> 8, color & 0x000000ff, opa);

        this->_canvas().fill_rect({x, y}, {right, bottom}, col);
    }

    void fill_rect(int x, int y, int w, int h, uint32_t color) {
        color = krkrz::TJSToActualColor(color);
        this->color_rect(x, y, w, h, color & 0x00ffffff, color & 0xff000000);
    }

    void draw_text(int x, int y, const std::u16string &text, uint32_t color,
                   int opa, bool aa, int shadowlevel, uint32_t shadowcolor,
                   int shadowwidth, int shadowofsx, int shadowofsy) {
        x = x + this->pos.x;
        y = y + this->pos.y;

        color = krkrz::TJSToActualColor(color);

        my::ColorRGBAub col = this->color_convert(color, opa);
        GLOG_D("draw text height %d, %d,%d %s", this->_font->get_height(), x, y,
               codecvt::utf_to_utf<char>(text).c_str());
        this->_canvas().fill_text(codecvt::utf_to_utf<char>(text), {x, y},
                                  nullptr, this->_font->get_height(), col);
    }

    void load_image(const std::u16string &_path) {
        auto path = my::fs::path(codecvt::utf_to_utf<char>(_path));

        GLOG_D("load image: %s", path.c_str());

        if (path.has_extension()) {
            this->_image =
                TJS2NativeStorages::get()->get_storage<my::Image>(path);
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
            this->on_paint();
        }
    }

    void assign_images(TJS2NativeLayer *layer) { this->_image = layer->_image; }

    auto get_layer_image_data(const my::PixelPos &_off,
                              const my::Size2D &size) {
        auto off = this->pos + _off;
        // TODO: check overflow
        return this->_canvas().get_image_data(off, size);
    }

    void put_layer_image_data(std::shared_ptr<my::RGBAImage> image,
                              const my::PixelPos &off) {
        // TODO: check overflow
        this->_canvas().put_image_data(image, off);
    }

    auto get_layer_texture() {
        return this->get_layer_image_data({0, 0}, this->size);
    }

    void get_layer_pixel(const my::PixelPos &off) {
        this->get_layer_image_data(off, {1, 1});
    }

    void operate_rect(const my::PixelPos &d_off, TJS2NativeLayer *layer,
                      const my::PixelPos &s_off, const my::Size2D &s_size,
                      TJS2BlendOperationMode mode) {
        if (s_size.w * s_size.h == 0) {
            return;
        }
        auto s_image = layer->get_layer_image_data(s_off, s_size);
        this->put_layer_image_data(s_image, d_off);
    }

    void on_paint() {
        if (!this->visible) {
            return;
        }
        if (this->_image) {
            int x = this->image_pos.x;
            int y = this->image_pos.y;
            GLOG_D("draw image {%d, %d}={%d, %d}", x, y, this->image_size.w, this->image_size.h);
            this->_canvas().draw_image(
                this->_image, {x, y},
                {x + this->image_size.w, y + this->image_size.h});
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

    my::Size2D size{};
    my::PixelPos pos{};
    my::Size2D image_size{};
    my::PixelPos image_pos{};
    std::shared_ptr<my::Image> _image;

    // TODO:
    int type{};
    int hit_type{};
    int hit_threshold{};
    TJS2DrawFace face{};

    int cursor;
    std::u16string name;

    bool focusable{false};
    bool visible{false};
    bool image_modified{false};
    bool absolute_order_mode{false};
    bool hold_alpha{false};

  private:
    iTJSDispatch2 *_this_obj{};
    tTJSVariantClosure _this_action_obj{nullptr, nullptr};
    TJS2NativeLayer *_parent{};
    std::list<TJS2NativeLayer *> _children{};
    TJS2NativeFont *_font{};
    TJS2NativeWindow *_win{};
    my::WindowMgr *_win_mgr{};

    void add_children(TJS2NativeLayer *layer) {
        this->_children.push_back(layer);
    }

    void remove_children(TJS2NativeLayer *layer) {
        this->_children.remove(layer);
    }

    my::Canvas &_canvas() { return *this->_win->canvas(); }
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
