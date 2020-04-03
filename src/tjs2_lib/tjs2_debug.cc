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
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ addLoggingHandler) {
            // add function to logging handler list

            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ addLoggingHandler)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ removeLoggingHandler) {
            // remove function from logging handler list

            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ removeLoggingHandler)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getLastLog) {

            if (result)
                *result = TJS_W("");

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ getLastLog)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ logAsError) {

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ logAsError)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ message) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            // display the arguments separated with ", "
            ttstr args;
            for (int i = 0; i < numparams; i++) {
                if (i != 0)
                    args += TJS_W(", ");
                args += ttstr(*param[i]);
            }

            static utf16_codecvt code_convert;
            GLOG_D(code_convert.to_bytes(args.AsStdString()).c_str());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ message)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ notice) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr args;
            for (int i = 0; i < numparams; i++) {
                if (i != 0)
                    args += TJS_W(", ");
                args += ttstr(*param[i]);
            }

            static utf16_codecvt code_convert;
            GLOG_I(code_convert.to_bytes(args.AsStdString()).c_str());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ notice)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ startLogToFile) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ startLogToFile)
        TJS_BEGIN_NATIVE_PROP_DECL(clearLogFileOnError) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = true;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(clearLogFileOnError)

        TJS_BEGIN_NATIVE_PROP_DECL(logLocation) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = TJS_W("");
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(logLocation)
        TJS_BEGIN_NATIVE_PROP_DECL(logToFileOnError) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = true;
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(logToFileOnError)
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
tTJSNativeClass *create_tjs2_debug() { return new TJS2Debug(); }
} // namespace krkrz
