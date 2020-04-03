#include <memory>
#include <libintl.h>

#include "CharacterSet.h"
#include "tjs.h"

void TVPGetVersion(void) {
    // TODO:
}

ttstr TVPReadAboutStringFromResource() {
    // TODO:
}


tjs_string getwtext(const char *s) {
    tjs_string str16;
    std::string str(s);
    TVPUtf8ToUtf16(str16, str);
    return str16;
}
