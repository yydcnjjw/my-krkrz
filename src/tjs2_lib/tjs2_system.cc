#include "tjs2_lib.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <util/codecvt.h>
#include <util/logger.h>

#include "krkrz_application.h"
#include <MsgIntf.h>

namespace {

enum {
    clScrollBar = 0x80000000,
    clBackground = 0x80000001,
    clActiveCaption = 0x80000002,
    clInactiveCaption = 0x80000003,
    clMenu = 0x80000004,
    clWindow = 0x80000005,
    clWindowFrame = 0x80000006,
    clMenuText = 0x80000007,
    clWindowText = 0x80000008,
    clCaptionText = 0x80000009,
    clActiveBorder = 0x8000000a,
    clInactiveBorder = 0x8000000b,
    clAppWorkSpace = 0x8000000c,
    clHighlight = 0x8000000d,
    clHighlightText = 0x8000000e,
    clBtnFace = 0x8000000f,
    clBtnShadow = 0x80000010,
    clGrayText = 0x80000011,
    clBtnText = 0x80000012,
    clInactiveCaptionText = 0x80000013,
    clBtnHighlight = 0x80000014,
    cl3DDkShadow = 0x80000015,
    cl3DLight = 0x80000016,
    clInfoText = 0x80000017,
    clInfoBk = 0x80000018,
    clNone = 0x1fffffff,
    clAdapt = 0x01ffffff,
    clPalIdx = 0x3000000,
    clAlphaMat = 0x4000000,
};

inline unsigned long ColorToRGB(unsigned long col) {
    switch (col) {
    case clScrollBar:
        return 0xffc8c8c8;
    case clBackground:
        return 0xff000000;
    case clActiveCaption:
        return 0xff99b4d1;
    case clInactiveCaption:
        return 0xffbfcddb;
    case clMenu:
        return 0xfff0f0f0;
    case clWindow:
        return 0xffffffff;
    case clWindowFrame:
        return 0xff646464;
    case clMenuText:
        return 0xff000000;
    case clWindowText:
        return 0xff000000;
    case clCaptionText:
        return 0xff000000;
    case clActiveBorder:
        return 0xffb4b4b4;
    case clInactiveBorder:
        return 0xfff4f7fc;
    case clAppWorkSpace:
        return 0xffababab;
    case clHighlight:
        return 0xff3399ff;
    case clHighlightText:
        return 0xffffffff;
    case clBtnFace:
        return 0xfff0f0f0;
    case clBtnShadow:
        return 0xffa0a0a0;
    case clGrayText:
        return 0xff6d6d6d;
    case clBtnText:
        return 0xff000000;
    case clInactiveCaptionText:
        return 0xff434e54;
    case clBtnHighlight:
        return 0xffffffff;
    case cl3DDkShadow:
        return 0xff696969;
    case cl3DLight:
        return 0xffe3e3e3;
    case clInfoText:
        return 0xff000000;
    case clInfoBk:
        return 0xffffffe1;
    case clNone: // black for WinNT
        return 0xff000000;
    case clAdapt:
        return clAdapt;
    case clPalIdx:
        return clPalIdx;
    case clAlphaMat:
        return clAlphaMat;
    }
    return col; // unknown, passthru
}

uint32_t TJSToActualColor(uint32_t color) {
    if (color & 0xff000000) {
        color = ColorToRGB(color); // system color to RGB
        // convert byte order to 0xRRGGBB since ColorToRGB's return value is in
        // a format of 0xBBGGRR.
        return ((color & 0xff) << 16) + (color & 0xff00) +
               ((color & 0xff0000) >> 16);
    } else {
        return color;
    }
}
//---------------------------------------------------------------------------
uint32_t TJSFromActualColor(u_int32_t color) {
    color &= 0xffffff;
    return color;
}

class TJS2NativeSystem {
  public:
    static TJS2NativeSystem *get() {
        static TJS2NativeSystem instance;
        return &instance;
    }

    std::u16string generate_uuid() {
        auto uuid = this->_uuid_generator();
        return this->_code_convert.from_bytes(boost::uuids::to_string(uuid));
    }

    bool create_app_lock() {
        // TODO: 
        return true;
    }

    void exit(int code) {
        std::exit(code);
    }

    void terminal(int code) { krkrz::Application::get()->base_app()->quit(); }

    bool get_program_option_value(const tjs_char *name, tTJSVariant *value) {
        my::program_options::variable_value option_value;
        if (krkrz::Application::get()->base_app()->get_program_option(
                this->_code_convert.to_bytes(name), option_value)) {

            if (typeid(std::string) == option_value.value().type()) {
                *value = this->_code_convert.from_bytes(
                    option_value.as<std::string>());
            } else if (typeid(bool) == option_value.value().type()) {
                *value = option_value.as<bool>();
            }
            return true;
        } else {
            return false;
        }
    }

    int64_t get_tick_count() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    bool shell_exec(const std::u16string &target, const std::u16string &param) {
        return false;
    }

  private:
    boost::uuids::random_generator_pure _uuid_generator;
    utf16_codecvt _code_convert;
    TJS2NativeSystem() : _uuid_generator(boost::uuids::random_generator()) {}
};

static bool terminate_on_window_close = true;

class TJS2System : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2System() : inherited(TJS_W("System")) {

        TJS_BEGIN_NATIVE_MEMBERS(System)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL_NO_INSTANCE(
            /*TJS class name*/ System) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ System)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ addContinuousHandler) {
            // add function to continus handler list

            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            // TODO: impl System.addContinuousHandler
            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ addContinuousHandler)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ removeContinuousHandler) {
            // remove function from continuous handler list

            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            // TODO: impl System.removeContinuousHandler
            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ removeContinuousHandler)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ assignMessage) {
            // assign system message

            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;

            ttstr id(*param[0]);
            ttstr msg(*param[1]);

            bool res = TJSAssignMessage(id.c_str(), msg.c_str());

            if (result)
                *result = tTJSVariant((tjs_int)res);

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ assignMessage)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ createAppLock) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            if (!result)
                return TJS_S_OK;

            ttstr lockname = *param[0];

            if (result)
                *result = (tjs_int)TJS2NativeSystem::get()->create_app_lock();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ createAppLock)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ createUUID) {
            if (result)
                *result = tTJSVariant(TJS2NativeSystem::get()->generate_uuid());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ createUUID)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ doCompact) {
            // TODO: impl System.doCompact
            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ doCompact)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ dumpHeap) {
            // TODO: impl System.dumpHeap
            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ dumpHeap)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ exit) {
            // this method does not return

            int code = numparams > 0 ? static_cast<int>(*param[0]) : 0;
            std::terminate();
            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ exit)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getArgument) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            if (!result)
                return TJS_S_OK;

            ttstr name = *param[0];

            bool res = TJS2NativeSystem::get()->get_program_option_value(
                name.c_str(), result);

            if (!res)
                result->Clear();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ getArgument)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ setArgument) {
            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;

            ttstr name = *param[0];
            ttstr value = *param[1];

            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ setArgument)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getKeyState) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            tjs_uint code = (tjs_int)*param[0];

            bool getcurrent = true;
            if (numparams >= 2)
                getcurrent = 0 != (tjs_int)*param[1];

            // if(result) *result = (tjs_int)res;
            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ getKeyState)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getTickCount) {
            if (result) {
                *result = (tjs_int64)TJS2NativeSystem::get()->get_tick_count();
            }
            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ getTickCount)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ inform) {
            // TODO: impl System.inform
            // show simple message box
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr text = *param[0];

            ttstr caption;
            if (numparams >= 2 && param[1]->Type() != tvtVoid)
                caption = *param[1];
            else
                caption = TJS_W("Information");

            if (result)
                result->Clear();

            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ inform)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ readRegValue) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            if (!result)
                return TJS_S_OK;

            ttstr key = *param[0];

            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ readRegValue)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ shellExecute) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr target = *param[0];
            ttstr execparam;

            if (numparams >= 2)
                execparam = *param[1];

            bool res = TJS2NativeSystem::get()->shell_exec(target.c_str(),
                                                           execparam.c_str());

            if (result)
                *result = (tjs_int)res;
            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ shellExecute)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ showVersion) {
            return TJS_E_NOTIMPL;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(
            /*func. name*/ showVersion)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ terminate) {
            int code = numparams > 0 ? static_cast<int>(*param[0]) : 0;
            TJS2NativeSystem::get()->terminal(code);
            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ terminate)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ toActualColor) {
            // convert color codes to 0xRRGGBB format.

            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            if (result) {
                tjs_uint32 color = (tjs_int)(*param[0]);
                color = TJSToActualColor(color);
                *result = (tjs_int)color;
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ toActualColor)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ touchImages) {
            // try to cache graphics

            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ touchImages)

        TJS_BEGIN_NATIVE_PROP_DECL(appDataPath) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result =
                (krkrz::Application::get()->app_data_path / "").u16string();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(appDataPath)
        TJS_BEGIN_NATIVE_PROP_DECL(dataPath) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = (krkrz::Application::get()->data_path / "").u16string();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(dataPath)

        TJS_BEGIN_NATIVE_PROP_DECL(desktopHeight) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = krkrz::Application::get()
                          ->base_app()
                          ->win_mgr()
                          ->get_desktop_display_mode()
                          .h;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(desktopHeight)
        TJS_BEGIN_NATIVE_PROP_DECL(desktopWidth) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = krkrz::Application::get()
                          ->base_app()
                          ->win_mgr()
                          ->get_desktop_display_mode()
                          .w;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(desktopWidth)
        TJS_BEGIN_NATIVE_PROP_DECL(desktopLeft) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = 0;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(desktopLeft)

        TJS_BEGIN_NATIVE_PROP_DECL(desktopTop) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = 0;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(desktopTop)
        TJS_BEGIN_NATIVE_PROP_DECL(drawThreadNum) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = 0;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER
            TJS_BEGIN_NATIVE_PROP_SETTER

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(drawThreadNum)
        TJS_BEGIN_NATIVE_PROP_DECL(eventDisabled) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = false;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(eventDisabled)

        TJS_BEGIN_NATIVE_PROP_DECL(exeBits) {
            // TODO: impl System.exeBits
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = 64;
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER
            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(exeBits)
        TJS_BEGIN_NATIVE_PROP_DECL(exeName) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = krkrz::Application::get()->exec_path.u16string();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(exeName)
        TJS_BEGIN_NATIVE_PROP_DECL(exePath) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = (krkrz::Application::get()->app_path / "").u16string();
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(exePath)
        TJS_BEGIN_NATIVE_PROP_DECL(exitOnWindowClose) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = terminate_on_window_close;
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            terminate_on_window_close = 0 != (tjs_int)*param;
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(exitOnWindowClose)

        TJS_BEGIN_NATIVE_PROP_DECL(graphicCacheLimit) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = 0;
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(graphicCacheLimit)
        TJS_BEGIN_NATIVE_PROP_DECL(osBits) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            // impl System.osBits
            *result = 64;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER
            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(osBits)
        TJS_BEGIN_NATIVE_PROP_DECL(osName) {
            // impl System.osName
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = TJS_W("linux");
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(osName)
        TJS_BEGIN_NATIVE_PROP_DECL(personalPath) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result =
                (krkrz::Application::get()->personal_path / "").u16string();
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(personalPath)
        TJS_BEGIN_NATIVE_PROP_DECL(platformName) {
            // TODO: impl System.platformName
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = TJS_W("linux");
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(platformName)
        TJS_BEGIN_NATIVE_PROP_DECL(savedGamesPath) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result =
                (krkrz::Application::get()->save_game_path / "").u16string();
            return TJS_S_OK;
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(savedGamesPath)
        TJS_BEGIN_NATIVE_PROP_DECL(screenWidth) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = krkrz::Application::get()
                          ->base_app()
                          ->win_mgr()
                          ->get_desktop_display_mode()
                          .w;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(screenWidth)
        TJS_BEGIN_NATIVE_PROP_DECL(screenHeight) {
            TJS_BEGIN_NATIVE_PROP_GETTER
            *result = krkrz::Application::get()
                          ->base_app()
                          ->win_mgr()
                          ->get_desktop_display_mode()
                          .h;
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(screenHeight)

        TJS_BEGIN_NATIVE_PROP_DECL(title) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = TJS_W("test");
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(title)
        TJS_BEGIN_NATIVE_PROP_DECL(versionInformation) {
            // TODO: impl System.versionInformation
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = TJS_W("1.0.0");
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(versionInformation)

        TJS_BEGIN_NATIVE_PROP_DECL(versionString) {
            // TODO: impl System.versionString
            TJS_BEGIN_NATIVE_PROP_GETTER

            *result = TJS_W("version");
            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_STATIC_PROP_DECL(versionString)

        TJS_END_NATIVE_MEMBERS
        // register default "exceptionHandler" member
        tTJSVariant val((iTJSDispatch2 *)NULL, (iTJSDispatch2 *)NULL);
        PropSet(TJS_MEMBERENSURE, TJS_W("exceptionHandler"), NULL, &val, this);

        // and onActivate, onDeactivate
        PropSet(TJS_MEMBERENSURE, TJS_W("onActivate"), NULL, &val, this);
        PropSet(TJS_MEMBERENSURE, TJS_W("onDeactivate"), NULL, &val, this);
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        TVPThrowExceptionMessage(TVPCannotCreateInstance);
        return nullptr;
    }
};

tjs_uint32 TJS2System::ClassID = (tjs_uint32)-1;
} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_system() { return new TJS2System(); }
} // namespace krkrz
