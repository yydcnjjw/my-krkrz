#pragma once

#include <boost/algorithm/string.hpp>

#include <MsgIntf.h>
#include <krkrz_application.h>
#include <tjs2_lib/tjs2_lib.h>

namespace krkrz {

class TJS2NativeFont : public tTJSNativeInstance {
  public:
    TJS2NativeFont() {
        this->_font =
            Application::get()
                ->base_app()
                ->resource_mgr()
                ->load<my::Font>(
                    "/home/yydcnjjw/workspace/code/project/my-gui/assets/fonts/"
                    "NotoSansCJK-Regular.ttc")
                .get();
        this->_sk_font = SkFont(this->_font->get_sk_typeface());
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
        return this->_sk_font.measureText(_text.data(), _text.size() * 2,
                                          SkTextEncoding::kUTF16);
    }
    int get_text_height(std::u16string _text = {}) {
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
        this->_sk_font.setSize(this->_height);
    }

    int get_height() { return this->_height; }

    const SkFont &sk_font() {
        return this->_sk_font;
    }

    int angle{0};
    bool bold{false};
    bool italic{false};
    bool strikeout{false};
    bool underline{false};
    int rasterizer{0};

    std::vector<std::string> faces{};

    SkFont _sk_font{};
    std::shared_ptr<my::Font> _font{};

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
