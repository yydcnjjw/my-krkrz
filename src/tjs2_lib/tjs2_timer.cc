#include "tjs2_lib.h"

#include "codecvt.h"

#include <logger.h>

#include <MsgIntf.h>

namespace {

class TJS2NativeTimer : public tTJSNativeInstance {};

class TJS2Timer : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Timer() : inherited(TJS_W("Timer")) {

        TJS_BEGIN_NATIVE_MEMBERS(Timer)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
            /*var.name*/ _this,
            /*var.type*/ TJS2NativeTimer,
            /*TJS class name*/ Timer) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Timer)

        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() { return new TJS2NativeTimer; }
};

tjs_uint32 TJS2Timer::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_timer() { return new TJS2Timer(); }
} // namespace krkrz
