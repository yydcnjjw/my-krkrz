#pragma once

#include <util/codecvt.h>
#include <util/logger.h>

#include "tjs2_lib.h"

namespace krkrz {

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
    void stop();

    void exec_storage(const std::u16string &path,
                      iTJSDispatch2 *context = nullptr,
                      tTJSVariant *result = nullptr,
                      const tjs_char *modestr = nullptr);
    void exec(const std::u16string &content, iTJSDispatch2 *context = nullptr,
              tTJSVariant *result = nullptr, const std::u16string &name = u"");

    void eval();

  private:
    std::thread _tjs_thread;
    tTJS *_tjs_engine;
    std::shared_ptr<TJSConsoleOutput> _tjs_console_output;

    TJS2NativeScripts();

    void _set_default_ppvalue();
    void _load_tjs_lib();
};

} // namespace krkrz
