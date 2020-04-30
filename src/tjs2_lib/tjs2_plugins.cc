#include "tjs2_lib.h"

#include <util/logger.h>

#include <MsgIntf.h>

namespace {

class TJS2Plugins : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Plugins() : inherited(TJS_W("Plugins")) {

        TJS_BEGIN_NATIVE_MEMBERS(Plugins)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL_NO_INSTANCE(
            /*TJS class name*/ Plugins) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Plugins)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ link) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            ttstr name = *param[0];

            GLOG_D("link plugin %s",
                   codecvt::utf_to_utf<char>(name.AsStdString()).c_str());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ link)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ unlink) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            if (result)
                *result = (tjs_int) true;

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ unlink)

        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        TVPThrowExceptionMessage(TVPCannotCreateInstance);
        return nullptr;
    }
};

tjs_uint32 TJS2Plugins::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_plugins() { return new TJS2Plugins(); }
} // namespace krkrz
