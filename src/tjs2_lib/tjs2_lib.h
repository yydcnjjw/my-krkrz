#pragma once

#include <memory>

#include <tjsNative.h>

#include <my_gui.hpp>
#include <my_storage.hpp>

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
tTJSNativeClass *create_tjs2_plugins();
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

iTJSDispatch2 *create_event_object(const tjs_char *type,
                                   iTJSDispatch2 *targthis,
                                   iTJSDispatch2 *targ);

extern ttstr action_name;
#define TVP_ACTION_INVOKE_BEGIN(argnum, name, object)                          \
    {                                                                          \
        if (numparams < (argnum))                                              \
            return TJS_E_BADPARAMCOUNT;                                        \
        tjs_int arg_count = 0;                                                 \
        iTJSDispatch2 *evobj =                                                 \
            TJS::create_event_object(TJS_W(name), (object), (object));         \
        tTJSVariant evval(evobj, evobj);                                       \
        evobj->Release();

#define TVP_ACTION_INVOKE_MEMBER(name)                                         \
    {                                                                          \
        static ttstr member_name(TJS_W(name));                                 \
        evobj->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP, member_name.c_str(), \
                       member_name.GetHint(), param[arg_count++], evobj);      \
    }

#define TVP_ACTION_INVOKE_END(object)                                          \
    tTJSVariant *pevval = &evval;                                              \
    (object).FuncCall(0, TJS::action_name.c_str(), TJS::action_name.GetHint(), \
                      result, 1, &pevval, NULL);                               \
    }

#define TVP_ACTION_INVOKE_END_NAME(object, name, hint)                         \
    tTJSVariant *pevval = &evval;                                              \
    (object).FuncCall(0, (name), (hint), result, 1, &pevval, NULL);            \
    }

void func_call(iTJSDispatch2 *obj, const std::string &func_name,
               const std::vector<tTJSVariant> &args = {});

} // namespace TJS

} // namespace krkrz
