#include <tjs2_plugin/tjs2_plugin.h>

#include <MsgIntf.h>
#include <tjsArray.h>

namespace {
class TJS2NativeMenuItem : public tTJSNativeInstance {};

class TJS2MenuItem : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    static TJS2MenuItem *get() {
        auto instance = getNoRef();
        instance->AddRef();
        return instance;
    }
    static TJS2MenuItem *getNoRef() {
        static TJS2MenuItem instance;
        return &instance;
    }

    static iTJSDispatch2 *create() {
        iTJSDispatch2 *out;
        auto _this = TJS2MenuItem::getNoRef();
        if (TJS_FAILED(
                _this->CreateNew(0, nullptr, nullptr, &out, 0, nullptr, _this)))
            TVPThrowInternalError;
        return out;
    }

    static void init(iTJSDispatch2 *global) {
        auto menu = TJS2MenuItem::create();
        tTJSVariant val = tTJSVariant(menu);
        menu->Release();
        tTJSVariant win;
        if (TJS_SUCCEEDED(
                global->PropGet(0, TJS_W("Window"), nullptr, &win, global))) {
            iTJSDispatch2 *obj = win.AsObjectNoAddRef();
            obj->PropSet(TJS_MEMBERENSURE, TJS_W("menu"), NULL, &val, obj);
            win.Clear();
        }
        val.Clear();
    }

    TJS2MenuItem() : inherited(TJS_W("MenuItem")) {

        TJS_BEGIN_NATIVE_MEMBERS(MenuItem) // constructor
        TJS_DECL_EMPTY_FINALIZE_METHOD

        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(/*var.name*/ _this,
                                          /*var.type*/ TJS2NativeMenuItem,
                                          /*TJS class name*/ MenuItem) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ MenuItem)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ add) { return TJS_S_OK; }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ add)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ insert) { return TJS_S_OK; }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ insert)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ remove) { return TJS_S_OK; }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ remove)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ popup) { return TJS_S_OK; }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ popup)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onClick) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onClick)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ fireClick) {

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ fireClick)

        TJS_BEGIN_NATIVE_PROP_DECL(caption) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            ttstr res;
            *result = res;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(caption)
        TJS_BEGIN_NATIVE_PROP_DECL(children) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            auto obj = TJSCreateArrayObject();
            *result = tTJSVariant(obj, obj);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(children)
        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        return new TJS2NativeMenuItem;
    }
};

tjs_uint32 TJS2MenuItem::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_menu_item(iTJSDispatch2 *global) {
    TJS2MenuItem::init(global);
    return TJS2MenuItem::get();
}
} // namespace krkrz
