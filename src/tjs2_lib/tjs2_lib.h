#pragma once

#include <memory>

#include <util/codecvt.h>

#include <tjsNative.h>

namespace krkrz {
tTJSNativeClass *create_tjs2_system();
tTJSNativeClass *create_tjs2_debug();
tTJSNativeClass *create_tjs2_storages();
tTJSNativeClass *create_tjs2_scripts();
tTJSNativeClass *create_tjs2_window();
tTJSNativeClass *create_tjs2_timer();
tTJSNativeClass *create_tjs2_async_trigger();
tTJSNativeClass *create_tjs2_wave_sound_buffer();
tTJSNativeClass *create_tjs2_video_overlay();
namespace TJS {

#define TJS_NATIVE_INSTANCE(obj, var, native_cls, tjs_cls)                     \
    if (!(obj))                                                                \
        return TJS_E_NATIVECLASSCRASH;                                         \
    native_cls *var;                                                           \
    {                                                                          \
        tjs_error hr;                                                          \
        hr = (obj)->AsObjectThisNoAddRef()->NativeInstanceSupport(             \
            TJS_NIS_GETINSTANCE, tjs_cls::ClassID,                             \
            (iTJSNativeInstance **)&var);                                      \
        if (TJS_FAILED(hr))                                                    \
            return TJS_E_NATIVECLASSCRASH;                                     \
    }

void func_call(iTJSDispatch2 *obj, const std::string &func_name,
               const std::vector<tTJSVariant> &args = {});

} // namespace TJS

} // namespace krkrz
