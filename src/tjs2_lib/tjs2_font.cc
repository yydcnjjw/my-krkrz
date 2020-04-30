#include "tjs2_font.h"

namespace krkrz {
tjs_uint32 TJS2Font::ClassID = (tjs_uint32)-1;
TJS2Font::TJS2Font() : inherited(TJS_W("Font")) {

    TJS_BEGIN_NATIVE_MEMBERS(Font)
    TJS_DECL_EMPTY_FINALIZE_METHOD
    TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
        /*var.name*/ _this,
        /*var.type*/ TJS2NativeFont,
        /*TJS class name*/ Font)
    return TJS_S_OK;

    TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Font)

    TJS_BEGIN_NATIVE_PROP_DECL(face) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        *result = _this->get_face();
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        _this->set_face(((ttstr)*param).AsStdString());
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(face)

    TJS_BEGIN_NATIVE_PROP_DECL(height) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        *result = _this->height;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        _this->height = *param;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(height)

    TJS_BEGIN_NATIVE_PROP_DECL(bold) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        *result = _this->bold;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        _this->bold = *param;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(bold)

    TJS_BEGIN_NATIVE_PROP_DECL(italic) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        *result = _this->italic;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        _this->italic = *param;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(italic)

    TJS_BEGIN_NATIVE_PROP_DECL(strikeout) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        *result = _this->strikeout;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        _this->strikeout = *param;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(strikeout)

    TJS_BEGIN_NATIVE_PROP_DECL(underline) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        *result = _this->underline;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        _this->underline = *param;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(underline)

    TJS_BEGIN_NATIVE_PROP_DECL(angle) {
        TJS_BEGIN_NATIVE_PROP_GETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        *result = _this->angle;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_GETTER

        TJS_BEGIN_NATIVE_PROP_SETTER

        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        _this->angle = *param;
        return TJS_S_OK;

        TJS_END_NATIVE_PROP_SETTER
    }
    TJS_END_NATIVE_PROP_DECL(angle)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getTextWidth) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        if (result)
            *result = _this->get_text_width(((ttstr)*param[0]).AsStdString());

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ getTextWidth)
    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getTextHeight) {
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        if (result)
            *result = _this->get_text_height(((ttstr)*param[0]).AsStdString());

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ getTextHeight)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ mapPrerenderedFont) {
        // TODO: Font.mapPrerenderedFont
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        // _this->MapPrerenderedFont(*param[0]);

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ mapPrerenderedFont)

    TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ unmapPrerenderedFont) {
        // TODO: Font.mapPrerenderedFont
        TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                /*var. type*/ TJS2NativeFont);
        if (numparams < 0)
            return TJS_E_BADPARAMCOUNT;

        // _this->UnmapPrerenderedFont();

        return TJS_S_OK;
    }
    TJS_END_NATIVE_METHOD_DECL(/*func. name*/ unmapPrerenderedFont)
    TJS_END_NATIVE_MEMBERS
}
} // namespace krkrz
