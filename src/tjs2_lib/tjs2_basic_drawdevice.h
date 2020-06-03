#pragma once

#include <MsgIntf.h>
#include <tjs2_lib/tjs2_lib.h>

namespace krkrz {

enum TJS2DeviceDrawType {
    dtNone,    //!< drawer
    dtDrawDib, //
    dtDBGDI,   // GDI
    dtDBDD,    // DirectDraw
    dtDBD3D    // Direct3D
};

class TJS2NativeBasicDrawDevice : public tTJSNativeInstance {
  public:
    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        this->_this_obj = tjs_obj;
        return TJS_S_OK;
    }
    iTJSDispatch2 *this_obj() {
        assert(this->_this_obj);
        return this->_this_obj;
    }

  private:
    iTJSDispatch2 *_this_obj;
};

class TJS2BasicDrawDevice : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2BasicDrawDevice();
    static tjs_uint32 ClassID;
    static TJS2BasicDrawDevice *get() {
        auto instance = getNoRef();
        instance->AddRef();
        return instance;
    }
    static TJS2BasicDrawDevice *getNoRef() {
        static TJS2BasicDrawDevice instance;
        return &instance;
    }

    static TJS2NativeBasicDrawDevice *create() {
        auto _this = TJS2BasicDrawDevice::getNoRef();
        iTJSDispatch2 *out{};
        if (TJS_FAILED(
                _this->CreateNew(0, nullptr, nullptr, &out, 0, nullptr, _this)))
            TVPThrowInternalError;
        TJS2NativeBasicDrawDevice *device;
        out->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
                                   TJS2BasicDrawDevice::ClassID,
                                   (iTJSNativeInstance **)&device);
        return device;
    }

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        return new TJS2NativeBasicDrawDevice;
    }
};

} // namespace krkrz
