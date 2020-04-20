#pragma once

#include <util/logger.h>

#include "tjs2_font.h"
#include "tjs2_lib.h"
#include "tjs2_window.h"
#include <MsgIntf.h>

namespace krkrz {
class TJS2NativeLayer : public tTJSNativeInstance {
  public:
    TJS2NativeLayer();

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) override;

    void fill_rect(int x, int y, int w, int h, uint32_t color, int opa = 255) {
        GLOG_D("file rect %d,%d = %d,%d color %#x, %d", x, y, x + w, y + h,
               color, opa);
        this->_canvas().fill_rect(
            {x, y}, {x + w, y + h},
            {color & 0xff000000, color & 0x00ff0000, color & 0x0000ff00, opa});
    }

    void draw_text(int x, int y, const std::u16string &text, uint32_t color,
                   int opa, bool aa, int shadowlevel, uint32_t shadowcolor,
                   int shadowwidth, int shadowofsx, int shadowofsy) {
        GLOG_D("draw text %d,%d %s", x, y,
               codecvt::utf_to_utf<char>(text).c_str());
        this->_canvas().fill_text(codecvt::utf_to_utf<char>(text), {x, y});
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

    my::Size2D size;
    my::PixelPos pos;
    my::Size2D image_size;
    my::PixelPos image_pos;

    // TODO:
    int type;
    int hit_type;
    int hit_threshold;
    int face;

    int cursor;
    std::u16string name;

    bool focusable = false;
    bool visible = true;
    bool image_modified = false;

  private:
    iTJSDispatch2 *_this_obj = nullptr;
    TJS2NativeLayer *_parent = nullptr;
    std::list<TJS2NativeLayer *> _children;
    iTJSDispatch2 *_font;
    TJS2NativeWindow *_win;
    my::WindowMgr *_win_mgr;

    void add_children(TJS2NativeLayer *layer) {
        this->_children.push_back(layer);
    }

    void remove_children(TJS2NativeLayer *layer) {
        this->_children.remove(layer);
    }

    my::Canvas &_canvas() { return *this->_win->canvas(); }
};
} // namespace krkrz
