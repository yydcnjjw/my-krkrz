#include "tjs2_lib.h"

#include <util/logger.h>

#include <MsgIntf.h>

namespace {

class TJS2NativeVideoOverlay : public tTJSNativeInstance {
    typedef tTJSNativeInstance inherited;

  public:
    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        return TJS_S_OK;
    }
  private:
};

class TJS2VideoOverlay : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2VideoOverlay() : inherited(TJS_W("VideoOverlay")) {

        TJS_BEGIN_NATIVE_MEMBERS(VideoOverlay)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
            /*var.name*/ _this,
            /*var.type*/ TJS2NativeVideoOverlay,
            /*TJS class name*/ VideoOverlay) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ VideoOverlay)
        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        return new TJS2NativeVideoOverlay;
    }
};

tjs_uint32 TJS2VideoOverlay::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_video_overlay() { return new TJS2VideoOverlay(); }
} // namespace krkrz
