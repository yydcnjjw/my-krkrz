#pragma once

#include <util/logger.h>

#include "tjs2_font.h"
#include "tjs2_lib.h"
#include "tjs2_system.h"
#include "tjs2_window.h"
#include <MsgIntf.h>

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

class TJS2NativeLayer : public tTJSNativeInstance {
  public:
    TJS2NativeLayer();

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) override;

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
        GLOG_D("color rect face=%d, %d,%d = %d,%d color %d %d %d %d",
               this->face, x, y, x + w, y + h, color & 0x00ff0000 >> 16,
               color & 0x0000ff00 >> 8, color & 0x000000ff, opa);

        this->_canvas().fill_rect({x, y}, {x + w, y + h}, col);
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
        GLOG_D("draw text %d,%d %s", x, y,
               codecvt::utf_to_utf<char>(text).c_str());
        this->_canvas().fill_text(codecvt::utf_to_utf<char>(text), {x, y},
                                  nullptr, 16, col);
    }

    iTJSDispatch2 *this_obj() {
        assert(this->_this_obj);
        return this->_this_obj;
    }

    iTJSDispatch2 *get_font() { return this->_font; }

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

    my::Size2D size{};
    my::PixelPos pos{};
    my::Size2D image_size{};
    my::PixelPos image_pos{};

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

  private:
    iTJSDispatch2 *_this_obj{};
    TJS2NativeLayer *_parent{};
    std::list<TJS2NativeLayer *> _children{};
    iTJSDispatch2 *_font{};
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
