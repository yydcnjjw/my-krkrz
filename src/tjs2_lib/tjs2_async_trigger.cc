#include "tjs2_lib.h"

#include "tjs2_lib.h"

#include "codecvt.h"

#include <logger.h>

#include <MsgIntf.h>

namespace {
class TJS2Debug : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Debug() : inherited(TJS_W("Debug")) {

        TJS_BEGIN_NATIVE_MEMBERS(Debug)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL_NO_INSTANCE(
            /*TJS class name*/ Debug) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Debug)
            
        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        TVPThrowExceptionMessage(TVPCannotCreateInstance);
        return nullptr;
    }
};

tjs_uint32 TJS2Debug::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_async_trigger() { return new TJS2Debug(); }
} // namespace krkrz
