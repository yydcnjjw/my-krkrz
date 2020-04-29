#include "tjs2_lib.h"

#include <mutex>

#include <MsgIntf.h>
#include <tjs2/tjsDictionary.h>

namespace krkrz {
namespace TJS {
ttstr action_name(TJS_W("action"));
iTJSDispatch2 *create_event_object(const tjs_char *type,
                                   iTJSDispatch2 *targthis,
                                   iTJSDispatch2 *targ) {
    iTJSDispatch2 *object = TJSCreateDictionaryObject();

    static ttstr type_name(TJS_W("type"));
    static ttstr target_name(TJS_W("target"));

    {
        tTJSVariant val(type);
        if (TJS_FAILED(object->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP,
                                       type_name.c_str(), type_name.GetHint(),
                                       &val, object)))
            TVPThrowInternalError;
    }

    {
        tTJSVariant val(targthis, targ);
        if (TJS_FAILED(object->PropSet(TJS_MEMBERENSURE | TJS_IGNOREPROP,
                                       target_name.c_str(),
                                       target_name.GetHint(), &val, object)))
            TVPThrowInternalError;
    }

    return object;
}

void func_call(iTJSDispatch2 *obj, const std::string &func_name,
               const std::vector<tTJSVariant> &args) {
    std::shared_ptr<tTJSVariant *[]> args_ptr(new tTJSVariant *[args.size()]);

    for (int i = 0; i < args.size(); i++) {
        args_ptr[i] = const_cast<tTJSVariant *>(args.data() + i);
    }

    obj->FuncCall(0, codecvt::utf_to_utf<char16_t>(func_name).c_str(), nullptr,
                  nullptr, args.size(), args_ptr.get(), obj);
}
} // namespace TJS
} // namespace krkrz
