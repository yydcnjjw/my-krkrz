#pragma once

#include "tjs2_lib.h"

#include <boost/algorithm/string.hpp>

#include <util/codecvt.h>
#include <util/logger.h>

#include <MsgIntf.h>

namespace krkrz {

class TJS2NativeFont : public tTJSNativeInstance {
  public:
    void set_face(std::u16string faces) {
        boost::split(this->faces, codecvt::utf_to_utf<char>(faces),
                     boost::is_any_of(","));
        // todo choose font from font mgr
    }

    std::u16string get_face() { return u""; }

    int angle;
    bool bold;
    bool italic;
    bool strikeout;
    bool underline;
    int height;
    int rasterizer;

    std::vector<std::string> faces;
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
