#include "tjs2_lib.h"

#include <util/logger.h>

#include <MsgIntf.h>
#include <krkrz_application.h>
#include <tjs2_lib/tjs2_scripts.h>

namespace {

enum TJS2AsyncTriggerMode { atmNormal, atmExclusive, atmAtIdle };

class TJS2NativeAsyncTrigger : public tTJSNativeInstance {
    typedef tTJSNativeInstance inherited;

  public:
    bool cached;
    TJS2AsyncTriggerMode mode;
    std::u16string action_name;
    tTJSVariantClosure action;

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        this->_this = tjs_obj;

        if (numparams >= 2 && param[1]->Type() != tvtVoid) {
            this->action_name =
                ((ttstr)*param[1])
                    .AsStdString(); // action function to be called
        }
        this->action = param[0]->AsObjectClosure();
        this->_ev_bus = krkrz::Application::get()->base_app()->ev_bus();
        return TJS_S_OK;
    }

    void trigger() {
        GLOG_I("call async trigger");
        this->cancel();
        this->_cs =
            this->_ev_bus->on_event<krkrz::TJSIdleEvent>()
                .map([](const auto &e) {
                    krkrz::TJS2NativeScripts::get()->wake();
                    return e;
                })
                .observe_on(krkrz::TJS2NativeScripts::get()->tjs_worker())
                .subscribe([this](const auto &) {
                    try {
                        GLOG_I("async trigger call");
                        krkrz::TJS::func_call(this->_this, "onFire");
                    } catch (eTJSError &e) {
                        GLOG_E(utf16_codecvt()
                                   .to_bytes(e.GetMessage().AsStdString())
                                   .c_str());
                        krkrz::Application::get()->base_app()->quit(true);
                    }
                    this->cancel();
                });
    }

    void cancel() {
        if (this->_cs.is_subscribed()) {
            this->_cs.unsubscribe();
        }
    }

  private:
    iTJSDispatch2 *_this;
    my::EventBus *_ev_bus;
    rxcpp::composite_subscription _cs{};
};

class TJS2AsyncTrigger : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2AsyncTrigger() : inherited(TJS_W("AsyncTrigger")) {

        TJS_BEGIN_NATIVE_MEMBERS(AsyncTrigger)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
            /*var.name*/ _this,
            /*var.type*/ TJS2NativeAsyncTrigger,
            /*TJS class name*/ AsyncTrigger) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ AsyncTrigger)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ trigger) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeAsyncTrigger);
            _this->trigger();
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ trigger)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ cancel) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeAsyncTrigger);
            _this->cancel();
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ cancel)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onFire) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeAsyncTrigger);
            tTJSVariantClosure obj = _this->action;
            if (obj.Object) {
                ttstr actionname = _this->action_name;
                obj.FuncCall(0,
                             actionname.IsEmpty() ? NULL : actionname.c_str(),
                             actionname.IsEmpty() ? NULL : actionname.GetHint(),
                             nullptr, 0, nullptr, nullptr);
            }
            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onFire)
        TJS_BEGIN_NATIVE_PROP_DECL(cached) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeAsyncTrigger);
            *result = _this->cached;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeAsyncTrigger);
            _this->cached = *param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(cached)

        TJS_BEGIN_NATIVE_PROP_DECL(mode) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeAsyncTrigger);
            *result = (tjs_int)_this->mode;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeAsyncTrigger);
            _this->mode = ((TJS2AsyncTriggerMode)(tjs_int)*param);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(mode)
        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        return new TJS2NativeAsyncTrigger;
    }
};

tjs_uint32 TJS2AsyncTrigger::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_async_trigger() { return new TJS2AsyncTrigger(); }
} // namespace krkrz
