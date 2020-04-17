#include "tjs2_lib.h"

#include <util/codecvt.h>
#include <util/logger.h>

#include "krkrz_application.h"
#include <MsgIntf.h>

namespace {

enum TJS2AsyncTriggerMode { atmNormal, atmExclusive, atmAtIdle };

class TJS2NativeTimer : public tTJSNativeInstance {
    typedef tTJSNativeInstance inherited;

  public:
    int capacity;
    TJS2AsyncTriggerMode mode;
    std::u16string action_name;
    tTJSVariantClosure action;

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        if (numparams >= 2 && param[1]->Type() != tvtVoid)
            this->action_name =
                ((ttstr)*param[1])
                    .AsStdString(); // action function to be called

        this->action = param[0]->AsObjectClosure();

        this->_timer = krkrz::Application::get()
                           ->base_app()
                           ->async_task()
                           ->create_timer_interval(
                               std::function<void(void)>([tjs_obj]() -> void {
                                   krkrz::TJS::func_call(tjs_obj, "onTimer");
                               }),
                               std::chrono::milliseconds(this->_interval));
        return TJS_S_OK;
    }

    void TJS_INTF_METHOD Invalidate() {
        this->disable();
        this->action.Release();
    }

    bool is_enable() { return this->_enabled; }

    void disable() {
        this->_enabled = false;
        this->_timer->cancel();
    }

    void enable() {
        this->_enabled = true;
        this->_timer->start();
    }

    void set_interval(int64_t mil) {
        this->_interval = mil;
        this->_timer->set_interval(std::chrono::milliseconds(mil));
    }

    ino64_t get_interval() { return this->_interval; }

  private:
    std::shared_ptr<my::AsyncTask::Timer<std::function<void(void)>>> _timer;
    bool _enabled = false;
    int64_t _interval;
};

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
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ onTimer) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
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
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ onTimer)
        TJS_BEGIN_NATIVE_PROP_DECL(interval) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            /*
                    bcc 5.5.1 has an inliner bug which cannot treat 64bit
               integer return value properly in some occasions. OK : tjs_uint64
               interval = _this->GetInterval(); *result = (tjs_int64)interval;
                    NG : *result = (tjs_int64)interval;
            */
            *result = (double)_this->get_interval();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            _this->set_interval(*param);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(interval)

        TJS_BEGIN_NATIVE_PROP_DECL(enabled) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            *result = _this->is_enable();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            if ((bool)*param) {
                _this->enable();
            } else {
                _this->disable();
            }
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(enabled)

        TJS_BEGIN_NATIVE_PROP_DECL(capacity) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            *result = _this->capacity;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            _this->capacity = *param;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(capacity)

        TJS_BEGIN_NATIVE_PROP_DECL(mode) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            *result = (tjs_int)_this->mode;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeTimer);
            _this->mode = ((TJS2AsyncTriggerMode)(tjs_int)*param);
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(mode)

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
