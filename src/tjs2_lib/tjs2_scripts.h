#pragma once

#include <shared_mutex>

#include "tjs2_lib.h"
#include "tjs2_storages.h"

#include <my_storage.hpp>

namespace krkrz {
struct TJSIdleEvent {};

class TJSConsoleOutput : public iTJSConsoleOutput {
  public:
    void ExceptionPrint(const tjs_char *msg) override {
        GLOG_I(codecvt::utf_to_utf<char>(msg).c_str());
    }

    void Print(const tjs_char *msg) override {
        GLOG_I(codecvt::utf_to_utf<char>(msg).c_str());
    }
};

class TJS2NativeScripts {
  public:
    static TJS2NativeScripts *get() {
        static TJS2NativeScripts instance;
        return &instance;
    }

    ~TJS2NativeScripts();

    void boot_start();
    bool is_stopping() { return this->_is_stopping; }
    void stop();

    rxcpp::observe_on_one_worker &tjs_worker() { return *this->_tjs_worker; }

    void wake() { this->_cv.notify_one(); }

    void yield() {
        if (!this->_current_sink) {
            throw std::runtime_error("yield error");
        }
        (*this->_current_sink)();
    }

    template <typename Func,
              typename = std::enable_if_t<std::is_invocable_v<Func>>>
    void push_immediate(Func func) {
        this->_cv.notify_one();
        this->_immediate_events.push(func);
    }

    tTJS *engine() { return this->_tjs_engine; }

  private:
    std::thread _tjs_thread{};
    rxcpp::observe_on_one_worker *_tjs_worker{};

    tTJS *_tjs_engine{};
    std::shared_ptr<TJSConsoleOutput> _tjs_console_output{};
    std::atomic_bool _is_stopping{false};
    std::mutex _lock{};
    std::condition_variable _cv{};
    struct CoroCtx {
        my::coro_t::push_type *sink{};
        std::shared_ptr<my::coro_t::pull_type> source{};
    };
    std::list<CoroCtx> _coroutines{};

    my::coro_t::push_type *_current_sink{};

    std::queue<std::function<void()>> _immediate_events{};

    TJS2NativeScripts();

    void _set_default_ppvalue();
    void _load_tjs_lib();
};

class TJS2Script : public my::Resource {
  public:
    size_t used_mem() override { return this->_script.size() * 2; }

    static std::shared_ptr<TJS2Script> make(const std::string &script) {
        return std::make_shared<TJS2Script>(script);
    }
    static std::shared_ptr<TJS2Script> make(std::u16string script) {
        return std::make_shared<TJS2Script>(std::move(script));
    }

    static std::shared_ptr<TJS2Script>
    get_tjs2_script_from_storage(const std::u16string &uri,
                                 const std::u16string &modestr) {
        auto tjs2_script = TJS2NativeStorages::get()->get_storage<TJS2Script>(
            codecvt::utf_to_utf<char>(uri), codecvt::utf_to_utf<char>(modestr));
        return tjs2_script;
    }

    static void exec_storage(const std::u16string &uri,
                             iTJSDispatch2 *context = nullptr,
                             tTJSVariant *result = nullptr,
                             const std::u16string modestr = u"") {
        get_tjs2_script_from_storage(uri, modestr)->exec(result, context, uri);
    }

    static void eval_storage(const std::u16string &uri,
                             iTJSDispatch2 *context = nullptr,
                             tTJSVariant *result = nullptr,
                             const std::u16string modestr = u"") {
        get_tjs2_script_from_storage(uri, modestr)->eval(result, context, uri);
    }

    void eval(tTJSVariant *result = nullptr, iTJSDispatch2 *context = nullptr,
              const std::u16string &name = u"", int lineofs = 0) {
        ttstr short_name = name;
        TJS2Script::_engine()->EvalExpression(this->script(), result, context,
                                              &short_name, lineofs);
    }

    void exec(tTJSVariant *result = nullptr, iTJSDispatch2 *context = nullptr,
              const std::u16string &name = u"", int lineofs = 0) {
        ttstr short_name = name;
        TJS2Script::_engine()->ExecScript(this->script(), result, context,
                                          &short_name, lineofs);
    }

    std::u16string script() { return this->_script; }

    TJS2Script(const std::string &script)
        : _script(codecvt::utf_to_utf<char16_t>(script)) {}

    TJS2Script(std::u16string script) : _script(std::move(script)) {}

  private:
    std::u16string _script;

    static tTJS *_engine() {
        auto v = TJS2NativeScripts::get()->engine();
        assert(v);
        return v;
    }
};

} // namespace krkrz

template <> class my::ResourceProvider<krkrz::TJS2Script> {
  public:
    /**
     * @brief      load from fs
     */
    static std::shared_ptr<krkrz::TJS2Script>
    load(const ResourceFileProvideInfo &info) {
        return ResourceProvider<krkrz::TJS2Script>::load(
            ResourceStreamProvideInfo::make(info));
    }

    /**
     * @brief      load from stream
     */
    static std::shared_ptr<krkrz::TJS2Script>
    load(const ResourceStreamProvideInfo &info) {
        auto blob = my::Blob::make(info);
        static std::vector<std::string> encodes{"UTF-8", "SHIFT_JIS",
                                                "GB18030"};
        auto code = blob->string_view();

        for (const auto &encode : encodes) {
            std::string script{};
            try {
                script = codecvt::to_utf<char>(code.begin(), code.end(), encode,
                                               codecvt::method_type::stop);
                if (encode == "SHIFT_JIS") {
                    auto u16_script = codecvt::utf_to_utf<char16_t>(script);
                    std::replace(u16_script.begin(), u16_script.end(), u'¥',
                                 u'\\');
                    std::replace(u16_script.begin(), u16_script.end(), u'‾',
                                 u'~');
                    script = codecvt::utf_to_utf<char>(u16_script);
                }
            } catch (codecvt::conversion_error &) {
                continue;
            }
            return krkrz::TJS2Script::make(script);
        }
        throw std::runtime_error("tjs2 script codecvt failure");
    }
};
