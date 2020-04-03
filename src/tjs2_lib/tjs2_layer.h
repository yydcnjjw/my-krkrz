#pragma once

#include <boost/property_tree/ptree.hpp>

#include <logger.h>
#include <draw_list.h>

#include "tjs2_lib.h"
#include "tjs2_window.h"
#include "tjs2_font.h"
#include <MsgIntf.h>

namespace krkrz {

class TJS2NativeLayer : public tTJSNativeInstance {
  public:
    tjs_error TJS_INTF_METHOD
    Construct(tjs_int numparams, tTJSVariant **param,
                               iTJSDispatch2 *tjs_obj) override;
    my::Size2D size;
    my::PixelPos pos;

    void fill_rect(int x, int y, int w, int h, uint32_t color, int opa = 255) {
        
    }

    iTJSDispatch2 *get_font() {
        return this->_font;
    }

  private:
    TJS2NativeLayer *_parent = nullptr;
    std::list<TJS2NativeLayer *> _children;
    iTJSDispatch2 *_font;
    
    std::shared_ptr<my::DrawList> _draw_list;

    void add_children(TJS2NativeLayer *layer) {
        this->_children.push_back(layer);
    }
};
} // namespace krkrz
