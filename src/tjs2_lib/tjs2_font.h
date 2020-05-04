#pragma once

#include <boost/algorithm/string.hpp>

#include <storage/font_mgr.h>
#include <util/codecvt.h>
#include <util/logger.h>

#include <MsgIntf.h>
#include <krkrz_application.h>
#include <tjs2_lib/tjs2_lib.h>

namespace krkrz {

class TJS2NativeFont : public tTJSNativeInstance {
  public:
    TJS2NativeFont() : _font_mgr(Application::get()->base_app()->font_mgr()) {
        this->_font = this->_font_mgr->add_font(
            "/home/yydcnjjw/workspace/code/project/my-gui/assets/fonts/"
            "NotoSansCJK-Regular.ttc");
    }

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        this->_this_obj = tjs_obj;
        return TJS_S_OK;
    }

    void set_face(std::u16string faces) {
        boost::split(this->faces, codecvt::utf_to_utf<char>(faces),
                     boost::is_any_of(","));
        // todo choose font from font mgr
    }

    std::u16string get_face() { return u""; }

    int get_text_width(std::u16string _text) {
        float scale = this->get_height() / this->_font->font_size();
        auto text = codecvt::utf_to_utf<wchar_t>(_text);
        float width = 0;
        for (auto ch : text) {
            const auto &glyph = this->_font->get_glyph(ch);
            width += glyph.advance_x * scale;
        }
        return width + 1;
    }
    int get_text_height(std::u16string _text) {
        // auto text = codecvt::utf_to_utf<wchar_t>(_text);
        return this->get_height();
    }

    iTJSDispatch2 *this_obj() {
        assert(this->_this_obj);
        return this->_this_obj;
    }

    void set_height(int height) {
        if (height < 0) {
            height = -height;
        }
        this->_height = height;
    }

    int get_height() {
        return this->_height;
    }

    int angle{0};
    bool bold{false};
    bool italic{false};
    bool strikeout{false};
    bool underline{false};
    int rasterizer{0};

    std::vector<std::string> faces;

    my::Font *_font;
    my::FontMgr *_font_mgr;

  private:
    iTJSDispatch2 *_this_obj;
    int _height{16};    
};

class TJS2Font : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Font();
    static tjs_uint32 ClassID;
    static TJS2Font *get() {
        auto instance = getNoRef();
        instance->AddRef();
        return instance;
    }
    static TJS2Font *getNoRef() {
        static TJS2Font instance;
        return &instance;
    }

    static TJS2NativeFont *create() {
        auto _this = TJS2Font::getNoRef();
        iTJSDispatch2 *out{};
        if (TJS_FAILED(
                _this->CreateNew(0, nullptr, nullptr, &out, 0, nullptr, _this)))
            TVPThrowInternalError;
        TJS2NativeFont *font;
        out->NativeInstanceSupport(TJS_NIS_GETINSTANCE, TJS2Font::ClassID,
                                   (iTJSNativeInstance **)&font);
        return font;
    }

  protected:
    tTJSNativeInstance *CreateNativeInstance() { return new TJS2NativeFont; }
};
} // namespace krkrz
