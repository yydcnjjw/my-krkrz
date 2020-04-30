#pragma once

#include <shared_mutex>

#include <boost/coroutine2/all.hpp>

#include <util/codecvt.h>
#include <util/logger.h>

#include "tjs2_lib.h"

namespace krkrz {

typedef boost::coroutines2::coroutine<void> coro_t;

struct TJSIdleEvent {};

class TJSConsoleOutput : public iTJSConsoleOutput {
  public:
    void ExceptionPrint(const tjs_char *msg) override {
        GLOG_D(this->_code_cvt.to_bytes(msg).c_str());
    }

    void Print(const tjs_char *msg) override {
        GLOG_D(this->_code_cvt.to_bytes(msg).c_str());
    }

  private:
    utf16_codecvt _code_cvt;
};

class TJS2NativeScripts {
  public:
    static TJS2NativeScripts *get() {
        static TJS2NativeScripts instance;
        return &instance;
    }

    ~TJS2NativeScripts();

    void boot_start();
    bool is_stopping() {
        std::shared_lock<std::shared_mutex> l_lock(this->_lock);
        return this->_is_stopping;
    }
    void stop();

    void exec_storage(const std::u16string &path,
                      iTJSDispatch2 *context = nullptr,
                      tTJSVariant *result = nullptr,
                      const tjs_char *modestr = nullptr);
    void exec(const std::u16string &content, iTJSDispatch2 *context = nullptr,
              tTJSVariant *result = nullptr, const std::u16string &name = u"",
              int lineofs = 0);

    void eval(const std::u16string &content, iTJSDispatch2 *context = nullptr,
              tTJSVariant *result = nullptr, const std::u16string &name = u"",
              int lineofs = 0);

    rxcpp::observe_on_one_worker &tjs_worker() { return *this->_tjs_worker; }

    // template <
    //     typename Func,
    //     std::enable_if_t<std::is_invocable<Func, coro_t::push_type
    //     &>::value>>
    // void make_coroutine(Func &&func) {
    //     this->_coroutines.push_back(
    //         std::bind(coro_t::pull_type([](coro_t::push_type &sink,
    //                                        Func &func) { func(sink); }),
    //                   std::forward(func)));
    // }

    void yield() {
        if (!this->_current_sink) {
            throw std::runtime_error("yield error");
        }
        (*this->_current_sink)();
    }

  private:
    std::thread _tjs_thread;
    rxcpp::observe_on_one_worker *_tjs_worker{};

    tTJS *_tjs_engine{};
    std::shared_ptr<TJSConsoleOutput> _tjs_console_output{};
    bool _is_stopping{false};
    std::shared_mutex _lock;
    std::condition_variable_any _cv;
    struct CoroCtx {
        coro_t::push_type *sink{};
        std::shared_ptr<coro_t::pull_type> source;
    };
    std::list<CoroCtx> _coroutines{};

    coro_t::push_type *_current_sink{};

    TJS2NativeScripts();

    void _set_default_ppvalue();
    void _load_tjs_lib();
};

} // namespace krkrz
