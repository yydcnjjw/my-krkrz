#include "tjs2_scripts.h"

#include <logger.h>

#include "krkrz_application.h"
#include "tjs2_storages.h"
#include <MsgIntf.h>
#include <MsgLoad.h>
#include <tjs2_plugin/KAGParser.h>

namespace {

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
                    : NULL;

            krkrz::TJS2NativeScripts::get()->exec_storage(
                name.AsStdString(), context, result, modestr.c_str());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ execStorage)

        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
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
        .subscribe([this](const auto &) {
            if (this->_tjs_thread.joinable()) {
                this->_tjs_thread.join();
            }
        });
    this->_tjs_thread = std::thread([this, app]() {
        TVPLoadMessage();
        this->_set_default_ppvalue();
        this->_tjs_engine->SetConsoleOutput(this->_tjs_console_output.get());
        this->_load_tjs_lib();

        try {
            bool is_debug = false;
            my::program_options::variable_value value;
            if (Application::get()->base_app()->get_program_option("debug",
                                                                   value)) {
                is_debug = value.as<bool>();
            }

            if (is_debug) {
                this->exec_storage(u"debug.tjs");
            } else {
                this->exec_storage(u"SysInitScript.tjs");
                this->exec_storage(u"startup.tjs");
            }
            // this->_tjs_engine->Shutdown();
        } catch (eTJSError &e) {
            GLOG_D(
                utf16_codecvt().to_bytes(e.GetMessage().AsStdString()).c_str());
            app->quit();
        }
    });
}

void TJS2NativeScripts::stop() {
    this->_tjs_engine->Shutdown();
}

TJS2NativeScripts::TJS2NativeScripts()
    : _tjs_engine(new tTJS()),
      _tjs_console_output(std::make_shared<TJSConsoleOutput>()) {}

TJS2NativeScripts::~TJS2NativeScripts() {
    this->_tjs_engine->Release();
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

    REGISTER_OBJECT(System, create_tjs2_system());
    REGISTER_OBJECT(Debug, create_tjs2_debug());
    REGISTER_OBJECT(Storages, create_tjs2_storages());
    REGISTER_OBJECT(Scripts, create_tjs2_scripts());
    REGISTER_OBJECT(Window, create_tjs2_window());
    REGISTER_OBJECT(Layer, create_tjs2_layer());
    REGISTER_OBJECT(Timer, create_tjs2_timer());
    REGISTER_OBJECT(AsyncTrigger, create_tjs2_async_trigger());
    REGISTER_OBJECT(Font, TJS2Font::get());
    REGISTER_OBJECT(KAGParser, TVPCreateNativeClass_KAGParser());
}

void TJS2NativeScripts::exec_storage(const std::u16string &path,
                                     iTJSDispatch2 *context,
                                     tTJSVariant *result,
                                     const tjs_char *modestr) {
    auto code = TJS2NativeStorages::get()->storage_read_all(
        codecvt::utf_to_utf<char>(path));
    this->exec(code, context, result, path);
}

void TJS2NativeScripts::exec(const std::u16string &content,
                             iTJSDispatch2 *context, tTJSVariant *result,
                             const std::u16string &name) {
    ttstr shortname = name;
    this->_tjs_engine->ExecScript(content, result, context, &shortname);
}

} // namespace krkrz
