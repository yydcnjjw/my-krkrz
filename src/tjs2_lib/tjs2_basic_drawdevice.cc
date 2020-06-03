#include "tjs2_basic_drawdevice.h"

namespace krkrz {
tjs_uint32 TJS2BasicDrawDevice::ClassID = (tjs_uint32)-1;

TJS2BasicDrawDevice::TJS2BasicDrawDevice()
    : inherited(TJS_W("BasicDrawDevice")) {

    TJS_BEGIN_NATIVE_MEMBERS(BasicDrawDevice)
    TJS_DECL_EMPTY_FINALIZE_METHOD
    TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
        /*var.name*/ _this,
        /*var.type*/ TJS2NativeBasicDrawDevice,
        /*TJS class name*/ BasicDrawDevice)
    return TJS_S_OK;

    TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ BasicDrawDevice)
#define TVP_REGISTER_PTDD_ENUM(name)                                           \
    TJS_BEGIN_NATIVE_PROP_DECL(name) {                                         \
        TJS_BEGIN_NATIVE_PROP_GETTER                                           \
        *result = (tjs_int64)TJS2DeviceDrawType::name;                         \
        return TJS_S_OK;                                                       \
        TJS_END_NATIVE_PROP_GETTER                                             \
        TJS_DENY_NATIVE_PROP_SETTER                                            \
    }                                                                          \
    TJS_END_NATIVE_PROP_DECL(name)
    // compatible for old kirikiri2
    TVP_REGISTER_PTDD_ENUM(dtNone)
    TVP_REGISTER_PTDD_ENUM(dtDrawDib)
    TVP_REGISTER_PTDD_ENUM(dtDBGDI)
    TVP_REGISTER_PTDD_ENUM(dtDBDD)
    TVP_REGISTER_PTDD_ENUM(dtDBD3D)

    TJS_BEGIN_NATIVE_PROP_DECL(preferredDrawer) {
        // compatible for old kirikiri2
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeBasicDrawDevice);
        // *result = (tjs_int64)(_this->GetDevice()->GetPreferredDrawerType());
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeBasicDrawDevice);
        // _this->GetDevice()->SetPreferredDrawerType(
        //     (tTVPBasicDrawDevice::tDrawerType)(tjs_int)*param);
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(preferredDrawer)
    TJS_END_NATIVE_MEMBERS
}

} // namespace krkrz
