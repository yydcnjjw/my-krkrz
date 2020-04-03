#include "tjs2_lib.h"
namespace krkrz {
namespace TJS {
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
