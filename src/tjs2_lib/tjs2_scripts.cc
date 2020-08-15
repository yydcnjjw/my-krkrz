#include "tjs2_scripts.h"

#include <fstream>

#include <boost/timer/timer.hpp>

#include "krkrz_application.h"
#include "tjs2_font.h"
#include "tjs2_layer.h"
#include <MsgIntf.h>
#include <MsgLoad.h>
#include <tjs2_plugin/tjs2_plugin.h>

namespace {

class TJS2TextReadStream : public iTJSTextReadStream {
  public:
    TJS2TextReadStream(const std::u16string &name,
                       const std::u16string &_mode) {
        int off{0};

        auto mode = codecvt::utf_to_utf<char>(_mode);
        if (!mode.empty()) {

            auto ch = mode.at(0);

            switch (ch) {
            case 'o': {
                off = std::stoi(mode.substr(1));
                break;
            }
            default:
                assert(true);
                break;
            }
        }

        auto path =
            my::fs::path(codecvt::utf_to_utf<char>(name)).lexically_normal();
        GLOG_I("read stream %s", path.c_str());
        this->_ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        this->_ifs.open(path);
        this->_ifs.seekg(off, std::ios_base::beg);
    }
    tjs_uint TJS_INTF_METHOD Read(tTJSString &targ, tjs_uint size) override {
        tjs_char *buf = targ.AppendBuffer(size);
        auto len = this->_ifs.readsome(reinterpret_cast<char *>(buf), size * 2);
        assert(len % 2 == 0);
        return len / 2;
    }
    void TJS_INTF_METHOD Destruct() override {}

  private:
    std::ifstream _ifs;
};

class TJS2TextWriteStream : public iTJSTextWriteStream {
  public:
    TJS2TextWriteStream(const std::u16string &name,
                        const std::u16string &_mode) {

        int off{0};

        auto mode = codecvt::utf_to_utf<char>(_mode);
        if (!mode.empty()) {

            auto ch = mode.at(0);

            switch (ch) {
            case 'o': {
                off = std::stoi(mode.substr(1));
                break;
            }
            default:
                assert(true);
                break;
            }
        }

        auto path =
            my::fs::path(codecvt::utf_to_utf<char>(name)).lexically_normal();
        GLOG_I("write stream %s mode %s", path.c_str(), mode.c_str());

        if (!my::fs::exists(path.parent_path())) {
            my::fs::create_directories(my::fs::relative(path.parent_path()));
        }

        this->_ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        if (off > 0) {
            this->_ofs.open(path, std::ios::binary | std::ios::app);
        } else {
            this->_ofs.open(path, std::ios::binary);
        }

        this->_ofs.seekp(off, std::ios_base::beg);
    }
    void TJS_INTF_METHOD Write(const tTJSString &targ) override {
        auto data = codecvt::utf_to_utf<char>(targ.AsStdString());
        this->_ofs.write(data.data(), data.size()).flush();
    }
    void TJS_INTF_METHOD Destruct() override {}

  private:
    std::ofstream _ofs;
};

// class iTJSBinaryStream {
//   public:
//     /* if error, position is not changed */
//     virtual tjs_uint64 TJS_INTF_METHOD Seek(tjs_int64 offset,
//                                             tjs_int whence) = 0;

//     /* returns actually read size */
//     virtual tjs_uint TJS_INTF_METHOD Read(void *buffer, tjs_uint read_size) =
//     0;

//     /* returns actually written size */
//     virtual tjs_uint TJS_INTF_METHOD Write(const void *buffer,
//                                            tjs_uint write_size) = 0;

//     // the default behavior is raising a exception
//     /* if error, raises exception */
//     virtual void TJS_INTF_METHOD SetEndOfStorage() = 0;

//     //-- should re-implement for higher performance
//     virtual tjs_uint64 TJS_INTF_METHOD GetSize() = 0;

//     virtual void TJS_INTF_METHOD Destruct() = 0; // must delete itself

//     virtual tjs_uint64 TJS_INTF_METHOD GetPosition() = 0;

//     virtual void TJS_INTF_METHOD SetPosition(tjs_uint64 pos) = 0;
// };

iTJSTextReadStream *TVPCreateTextStreamForRead(const ttstr &name,
                                               const ttstr &modestr) {
    return new TJS2TextReadStream(name.AsStdString(), modestr.AsStdString());
}

iTJSTextWriteStream *TVPCreateTextStreamForWrite(const ttstr &name,
                                                 const ttstr &modestr) {
    return new TJS2TextWriteStream(name.AsStdString(), modestr.AsStdString());
}

// iTJSBinaryStream *TVPCreateBinaryStreamInterfaceForRead(const ttstr &name,
//                                                         const ttstr &modestr)
//                                                         {}

// iTJSBinaryStream *TVPCreateBinaryStreamInterfaceForWrite(const ttstr &name,
//                                                          const ttstr
//                                                          &modestr) {

// }

class TJS2Scripts : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Scripts() : inherited(TJS_W("Scripts")) {

        TJS_BEGIN_NATIVE_MEMBERS(Scripts)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL_NO_INSTANCE(
            /*TJS class name*/ Scripts) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Scripts)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ exec) {
            // execute given string as a script
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr content = *param[0];

            ttstr name;
            tjs_int lineofs = 0;
            if (numparams >= 2 && param[1]->Type() != tvtVoid)
                name = *param[1];
            if (numparams >= 3 && param[2]->Type() != tvtVoid)
                lineofs = *param[2];

            iTJSDispatch2 *context =
                numparams >= 4 && param[3]->Type() != tvtVoid
                    ? param[3]->AsObjectNoAddRef()
                    : NULL;

            krkrz::TJS2Script::make(content.AsStdString())
                ->exec(result, context, name.AsStdString(), lineofs);
            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ exec)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ execStorage) {
            // execute script which stored in storage
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr name = *param[0];

            ttstr modestr;
            if (numparams >= 2 && param[1]->Type() != tvtVoid)
                modestr = *param[1];

            iTJSDispatch2 *context =
                numparams >= 3 && param[2]->Type() != tvtVoid
                    ? param[2]->AsObjectNoAddRef()
                    : nullptr;

            krkrz::TJS2Script::exec_storage(name.AsStdString(), context, result,
                                            modestr.AsStdString());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ execStorage)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ evalStorage) {
            // execute expression which stored in storage
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr name = *param[0];

            ttstr modestr;
            if (numparams >= 2 && param[1]->Type() != tvtVoid)
                modestr = *param[1];

            iTJSDispatch2 *context =
                numparams >= 3 && param[2]->Type() != tvtVoid
                    ? param[2]->AsObjectNoAddRef()
                    : NULL;

            krkrz::TJS2Script::eval_storage(name.AsStdString(), context, result,
                                            modestr.AsStdString());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ evalStorage)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ eval) {
            // execute given string as a script
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr content = *param[0];

            ttstr name;
            tjs_int lineofs = 0;
            if (numparams >= 2 && param[1]->Type() != tvtVoid)
                name = *param[1];
            if (numparams >= 3 && param[2]->Type() != tvtVoid)
                lineofs = *param[2];

            iTJSDispatch2 *context =
                numparams >= 4 && param[3]->Type() != tvtVoid
                    ? param[3]->AsObjectNoAddRef()
                    : NULL;

            krkrz::TJS2Script::make(content.AsStdString())
                ->eval(result, context, name.AsStdString(), lineofs);

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ eval)

        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() override {
        TVPThrowExceptionMessage(TVPCannotCreateInstance);
        return nullptr;
    }
};

tjs_uint32 TJS2Scripts::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_scripts() { return new TJS2Scripts(); }

void TJS2NativeScripts::boot_start() {
    auto app = krkrz::Application::get()->base_app();
    app->ev_bus()
        ->on_event<my::QuitEvent>()
        .observe_on(app->ev_bus()->ev_bus_worker())
        .subscribe([this](const std::shared_ptr<my::Event<my::QuitEvent>> &ev) {
            this->_is_stopping = true;

            if (this->_tjs_thread.joinable()) {
                this->_tjs_thread.join();
            }
            if (!ev->data->is_on_error) {
                TJS2NativeScripts::get()->stop();
            }
        });

    // std::promise<rxcpp::observe_on_one_worker *> promise;
    // auto future = promise.get_future();

    this->_tjs_thread =
        std::thread([this, app]( // std::promise<rxcpp::observe_on_one_worker
                                 // *> promise
                    ) {
            rxcpp::schedulers::run_loop rlp;
            auto rlp_worker = rxcpp::observe_on_run_loop(rlp);

            this->_tjs_worker = &rlp_worker;

            // promise.set_value(&rlp_worker);
            pthread_setname_np(pthread_self(), "TJS");

            TVPLoadMessage();
            this->_set_default_ppvalue();
            this->_tjs_engine->SetConsoleOutput(
                this->_tjs_console_output.get());
            TJSCreateTextStreamForRead = TVPCreateTextStreamForRead;
            TJSCreateTextStreamForWrite = TVPCreateTextStreamForWrite;

            this->_load_tjs_lib();

            GLOG_D("tjs script exec start");
            try {
                boost::timer::auto_cpu_timer t;
                bool is_debug = false;
                my::program_options::variable_value value;
                if (Application::get()->base_app()->get_program_option("debug",
                                                                       value)) {
                    is_debug = value.as<bool>();
                }

                if (is_debug) {
                    Logger::get()->set_level(Logger::Level::DEBUG);
                } else {
                    Logger::get()->set_level(Logger::Level::INFO);
                }

                TJS2Script::exec_storage(u"SysInitScript.tjs");
                TJS2Script::exec_storage(u"startup.tjs");

            } catch (eTJSError &e) {
                GLOG_D(utf16_codecvt()
                           .to_bytes(e.GetMessage().AsStdString())
                           .c_str());
                app->quit(true);
            }
            GLOG_D("tjs loop start");
            for (;;) {

                while (!this->_immediate_events.empty()) {
                    auto immediate_func = this->_immediate_events.front();

                    auto source = std::make_shared<my::coro_t::pull_type>(
                        [&](my::coro_t::push_type &sink) {
                            this->_current_sink = &sink;
                            immediate_func();
                        });
                    if (*source) {
                        GLOG_D("------------------reschedule----------------");
                        this->_coroutines.push_back(
                            {this->_current_sink, source});
                    }

                    this->_immediate_events.pop();
                }

                while (!rlp.empty() && rlp.peek().when < rlp.now()) {

                    auto source = std::make_shared<my::coro_t::pull_type>(
                        [&](my::coro_t::push_type &sink) {
                            this->_current_sink = &sink;
                            rlp.dispatch();
                        });
                    if (*source) {
                        GLOG_D("------------------reschedule----------------");
                        this->_coroutines.push_back(
                            {this->_current_sink, source});
                    }

                    while (!this->_immediate_events.empty()) {
                        auto immediate_func = this->_immediate_events.front();

                        auto source = std::make_shared<my::coro_t::pull_type>(
                            [&](my::coro_t::push_type &sink) {
                                this->_current_sink = &sink;
                                immediate_func();
                            });
                        if (*source) {
                            GLOG_D(
                                "------------------reschedule----------------");
                            this->_coroutines.push_back(
                                {this->_current_sink, source});
                        }

                        this->_immediate_events.pop();
                    }
                }

                app->ev_bus()->post<TJSIdleEvent>();

                auto it = this->_coroutines.begin();
                while (it != this->_coroutines.end()) {
                    if (*it->source) {
                        this->_current_sink = it->sink;
                        (*it->source)();
                        ++it;
                    } else {
                        it = this->_coroutines.erase(it);
                    }
                }

                {
                    if (this->is_stopping()) {
                        break;
                    }
                    // std::this_thread::yield();
                    std::unique_lock<std::mutex> l_lock(this->_lock);
                    if (rlp.empty() && this->_immediate_events.empty()) {
                        this->_cv.wait(l_lock, [this, &rlp] {
                            return !rlp.empty() ||
                                   !this->_immediate_events.empty() ||
                                   this->_is_stopping;
                        });
                    }
                }
            }
            GLOG_D("tjs loop end");
            GLOG_D("tjs script exec finished");
        } // ,
          // std::move(promise)
        );
}

void TJS2NativeScripts::stop() { this->_tjs_engine->Shutdown(); }

TJS2NativeScripts::TJS2NativeScripts()
    : _tjs_engine(new tTJS()),
      _tjs_console_output(std::make_shared<TJSConsoleOutput>()) {}

TJS2NativeScripts::~TJS2NativeScripts() {
    try {
        this->_tjs_engine->Release();
    } catch (eTJSError &e) {
        GLOG_D(utf16_codecvt().to_bytes(e.GetMessage().AsStdString()).c_str());
    }
}
void TJS2NativeScripts::_set_default_ppvalue() {
    this->_tjs_engine->SetPPValue(TJS_W("kirkiriz"), 1);
    this->_tjs_engine->SetPPValue(TJS_W("linux"), 1);
}

void TJS2NativeScripts::_load_tjs_lib() {
    iTJSDispatch2 *global = this->_tjs_engine->GetGlobalNoAddRef();
    iTJSDispatch2 *cls;
    tTJSVariant cls_val;

#define REGISTER_OBJECT(classname, instance)                                   \
    cls = (instance);                                                          \
    cls_val = tTJSVariant(cls);                                                \
    cls->Release();                                                            \
    global->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP, TJS_W(#classname),      \
                    nullptr, &cls_val, global);
    REGISTER_OBJECT(Debug, create_tjs2_debug());
    REGISTER_OBJECT(Plugins, create_tjs2_plugins());
    REGISTER_OBJECT(System, create_tjs2_system());
    REGISTER_OBJECT(Storages, create_tjs2_storages());
    REGISTER_OBJECT(Scripts, create_tjs2_scripts());
    REGISTER_OBJECT(Window, create_tjs2_window());
    REGISTER_OBJECT(Layer, TJS2Layer::get());
    REGISTER_OBJECT(Timer, create_tjs2_timer());
    REGISTER_OBJECT(AsyncTrigger, create_tjs2_async_trigger());
    REGISTER_OBJECT(WaveSoundBuffer, create_tjs2_wave_sound_buffer());
    REGISTER_OBJECT(VideoOverlay, create_tjs2_video_overlay());
    REGISTER_OBJECT(Font, TJS2Font::get());
    REGISTER_OBJECT(KAGParser, TVPCreateNativeClass_KAGParser());
    REGISTER_OBJECT(MenuItem, create_tjs2_menu_item(global));
}
} // namespace krkrz
