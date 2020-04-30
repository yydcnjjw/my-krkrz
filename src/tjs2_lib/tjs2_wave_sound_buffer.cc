#include "tjs2_lib.h"

#include <media/audio_mgr.h>
#include <util/codecvt.h>
#include <util/logger.h>

#include <MsgIntf.h>
#include <krkrz_application.h>
#include <tjs2_lib/tjs2_storages.h>

namespace {

class TJS2NativeWaveSoundBuffer : public tTJSNativeInstance {
    typedef tTJSNativeInstance inherited;

  public:
    TJS2NativeWaveSoundBuffer()
        : _audio_mgr(krkrz::Application::get()->base_app()->audio_mgr()) {}

    tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param,
                                        iTJSDispatch2 *tjs_obj) {
        if (numparams < 1)
            return TJS_E_BADPARAMCOUNT;

        return TJS_S_OK;
    }

    void open(const std::u16string &path) {
        this->_audio = krkrz::TJS2NativeStorages::get()->get_storage<my::Audio>(
            codecvt::utf_to_utf<char>(path));
    }

    void play() {
        if (!this->_audio) {
            GLOG_W("warn: audio is not be opened");
            return;
        }
        this->_audio_mgr->play(this->_audio.get());
    }
    void stop() {
        if (!this->_audio) {
            GLOG_W("warn: audio is not be opened");
            return;
        }
        this->_audio_mgr->pause(this->_audio.get());
    }

    void fade(int ms) {
        if (!this->_audio) {
            GLOG_W("warn: audio is not be opened");
            return;
        }
        this->_fade_time = ms;
        this->_audio_mgr->play_fade(this->_audio.get(), this->_fade_time);
    }

    void stop_fade() {
        if (!this->_audio) {
            GLOG_W("warn: audio is not be opened");
            return;
        }
        this->_audio_mgr->stop_fade(this->_audio.get(), this->_fade_time);
    }

    bool is_paused() {
        if (!this->_audio) {
            return true;
        }
        return this->_audio_mgr->is_paused(this->_audio.get());
    }

    void set_paused(bool paused) {
        if (paused == this->is_paused()) {
            return;
        }
        if (paused) {
            pause();
        } else {
            if (!this->_audio) {
                GLOG_W("warn: audio is not be opened");
                return;
            }
            this->_audio_mgr->resume(this->_audio.get());
        }
    }

    void set_panning(int pan) {
        if (this->_pan == pan) {
            return;
        }
        // TODO:
        // this->_audio_mgr->set_panning(this->_audio.get(), , );
    }

    int get_panning() { return this->_pan; }

  private:
    my::AudioMgr *_audio_mgr;
    std::shared_ptr<my::Audio> _audio;
    int _fade_time{0};
    int _pan{0};
};

class TJS2WaveSoundBuffer : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2WaveSoundBuffer() : inherited(TJS_W("WaveSoundBuffer")) {

        TJS_BEGIN_NATIVE_MEMBERS(WaveSoundBuffer)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
            /*var.name*/ _this,
            /*var.type*/ TJS2NativeWaveSoundBuffer,
            /*TJS class name*/ WaveSoundBuffer) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ WaveSoundBuffer)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ open) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;
            _this->open(((ttstr)*param[0]).AsStdString());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ open)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ play) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            _this->play();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ play)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ stop) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            _this->stop();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ stop)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ fade) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);
            if (numparams < 2)
                return TJS_E_BADPARAMCOUNT;

            tjs_int to;
            tjs_int time;
            tjs_int delay = 0;
            to = (tjs_int)(*param[0]);
            time = (tjs_int)(*param[1]);
            if (numparams >= 3 && param[2]->Type() != tvtVoid)
                delay = (tjs_int)(*param[2]);

            _this->fade(time);

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ fade)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ stopFade) {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            _this->stop_fade();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/ stopFade)

        TJS_BEGIN_NATIVE_PROP_DECL(paused) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            *result = _this->is_paused();

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            _this->set_paused(*param);

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(paused)
        TJS_BEGIN_NATIVE_PROP_DECL(pan) {
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            *result = _this->get_panning();

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            _this->set_panning(*param);

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(pan)
        TJS_BEGIN_NATIVE_PROP_DECL(volume) {
            // TODO: WaveSoundBuffer.volume
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            // *result = _this->GetVolume();

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            // _this->SetVolume(*param);

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(volume)

        TJS_BEGIN_NATIVE_PROP_DECL(volume2) {
            // TODO: WaveSoundBuffer.volume2
            TJS_BEGIN_NATIVE_PROP_GETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            // *result = _this->GetVolume2();

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_GETTER

            TJS_BEGIN_NATIVE_PROP_SETTER

            TJS_GET_NATIVE_INSTANCE(/*var. name*/ _this,
                                    /*var. type*/ TJS2NativeWaveSoundBuffer);

            // _this->SetVolume2(*param);

            return TJS_S_OK;

            TJS_END_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(volume2)

        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        return new TJS2NativeWaveSoundBuffer;
    }
};

tjs_uint32 TJS2WaveSoundBuffer::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {
tTJSNativeClass *create_tjs2_wave_sound_buffer() {
    return new TJS2WaveSoundBuffer();
}
} // namespace krkrz
