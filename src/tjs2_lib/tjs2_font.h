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

    void set_face(std::u16string faces) {
        boost::split(this->faces, codecvt::utf_to_utf<char>(faces),
                     boost::is_any_of(","));
        // todo choose font from font mgr
    }

    std::u16string get_face() { return u""; }

    int get_text_width(std::u16string _text) {
        auto text = codecvt::utf_to_utf<wchar_t>(_text);
        int width = 0;
        for (auto ch : text) {
            const auto &glyph = this->_font->get_glyph(ch);
            width += glyph.advance_x;
        }
        return width;
    }
    int get_text_height(std::u16string _text) {
        auto text = codecvt::utf_to_utf<wchar_t>(_text);
        return this->_font->font_size();
    }

    int angle;
    bool bold;
    bool italic;
    bool strikeout;
    bool underline;
    int height;
    int rasterizer;

    std::vector<std::string> faces;

    my::Font *_font;
    my::FontMgr *_font_mgr;
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

    static iTJSDispatch2 *create() {
        iTJSDispatch2 *out;
        auto _this = TJS2Font::getNoRef();
        if (TJS_FAILED(
                _this->CreateNew(0, nullptr, nullptr, &out, 0, nullptr, _this)))
            TVPThrowInternalError;
        return out;
    }

  protected:
    tTJSNativeInstance *CreateNativeInstance() { return new TJS2NativeFont; }
};
} // namespace krkrz
