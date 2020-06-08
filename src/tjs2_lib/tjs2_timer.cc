#include "tjs2_lib.h"

#include <MsgIntf.h>
#include <krkrz_application.h>
#include <tjs2_lib/tjs2_scripts.h>

namespace {

class TJS2NativeInstance : public tTJSNativeInstance {
    typedef tTJSNativeInstance inherited;

  public:
    virtual tjs_error TJS_INTF_METHOD
    Construct(tjs_int numparams, tTJSVariant **param,
              iTJSDispatch2 *tjs_obj) override {
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

enum TJS2AsyncTriggerMode { atmNormal, atmExclusive, atmAtIdle };

class TJS2NativeTimer : public TJS2NativeInstance {
  public:
    int capacity{6};
    TJS2AsyncTriggerMode mode;
    std::u16string action_name;
    tTJSVariantClosure action;

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        TJS2NativeInstance::Construct(numparams, param, tjs_obj);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        if (numparams >= 2 && param[1]->Type() != tvtVoid)
            this->action_name =
                ((ttstr)*param[1])
                    .AsStdString(); // action function to be called

        this->action = param[0]->AsObjectClosure();
        this->ev_bus = krkrz::Application::get()->base_app()->ev_bus();

        this->_subscribe_timer();
        this->_timer = krkrz::Application::get()
                           ->base_app()
                           ->async_task()
                           ->create_timer_interval(
                               std::function<void(void)>([this]() -> void {
                                   // this->_reset_finish();
                                   if (this->_wait_events > this->capacity) {
                                       return;
                                   }
                                   this->_post();
                                   // this->_wait_finish();
                                   this->log();
                               }),
                               std::chrono::milliseconds(this->_interval));
        return TJS_S_OK;
    }

    void TJS_INTF_METHOD Invalidate() {
        this->_unsubscribe_timer();
        this->disable();
        this->action.Release();
    }

    void log() {
        GLOG_D("timer callback: %p, cap %d interval %d, %Li", this,
               this->capacity, this->get_interval(),
               std::chrono::system_clock::now().time_since_epoch().count());
    }

    bool is_enable() { return !this->_timer->is_cancel(); }

    void disable() {
        GLOG_D("direct disable timer");
        this->_timer->cancel();
    }

    void enable() {
        GLOG_D("direct enable timer %d", this->get_interval());
        // this->_subscribe_timer();
        if (this->get_interval() == 0) {
            this->_post();
            this->disable();
        } else {
            this->_timer->start();
        }
    }

    void set_interval(int64_t mil) {
        GLOG_D("interval %d", mil);

        this->_interval = mil;

        if (mil == 0) {
            this->disable();
            return;
        }

        this->_timer->set_interval(std::chrono::milliseconds(mil));
        if (!this->is_enable()) {
            this->enable();
        }
    }

    int64_t get_interval() { return this->_interval; }

  private:
    typedef std::shared_ptr<my::AsyncTask::Timer<std::function<void(void)>>>
        TJSTimerEvent;
    std::shared_ptr<my::AsyncTask::Timer<std::function<void(void)>>> _timer;
    int64_t _interval{};
    my::EventBus *ev_bus{};

    rxcpp::composite_subscription _timer_cs{};

    std::mutex _lock{};
    std::condition_variable _cv{};
    bool _is_finish{false};

    std::atomic_int _wait_events{0};

    void _reset_finish() {
        std::unique_lock<std::mutex> l_lock(this->_lock);
        this->_is_finish = false;
    }

    void _notify_finish() {
        std::unique_lock<std::mutex> l_lock(this->_lock);
        this->_is_finish = true;
        this->_cv.notify_one();
    }

    void _wait_finish() {
        std::unique_lock<std::mutex> l_lock(this->_lock);
        this->_cv.wait(l_lock, [this]() { return this->_is_finish; });
    }

    void _post() {
        ++this->_wait_events;
        this->ev_bus->post<TJSTimerEvent>(this->_timer);
    }

    void _subscribe_timer() {
        // this->_unsubscribe_timer();

        this->_timer_cs =
            this->ev_bus->on_event<TJSTimerEvent>()
                .filter(
                    [this](const std::shared_ptr<my::Event<TJSTimerEvent>> &e) {
                        return *e->data == this->_timer;
                    })
                .map([](const auto &e) {
                    krkrz::TJS2NativeScripts::get()->wake();
                    return e;
                })
                .observe_on(krkrz::TJS2NativeScripts::get()->tjs_worker())
                .subscribe(
                    [this](const std::shared_ptr<my::Event<TJSTimerEvent>> &e) {
                        if (!krkrz::TJS2NativeScripts::get()
                                 ->is_stopping() // &&
                                                 // !this->is_enable()
                        ) {
                            GLOG_I("call timecallback %p", this);
                            try {
                                krkrz::TJS::func_call(this->this_obj(),
                                                      "onTimer");
                            } catch (eTJSError &e) {
                                GLOG_E(
                                    utf16_codecvt()
                                        .to_bytes(e.GetMessage().AsStdString())
                                        .c_str());
                                this->disable();
                                krkrz::Application::get()->base_app()->quit(
                                    true);
                            }
                            // this->_notify_finish();
                        }
                        --this->_wait_events;
                    });
    }

    void _unsubscribe_timer() {
        if (this->_timer_cs.is_subscribed()) {
            this->_timer_cs.unsubscribe();
        }
    }
};

constexpr auto TJS_SUBMILLI_FRAC_BITS = 16;

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
    tTJSNativeInstance *CreateNativeInstance() override {
        return new TJS2NativeTimer;
    }
};

tjs_uint32 TJS2Timer::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_timer() { return new TJS2Timer(); }
} // namespace krkrz
