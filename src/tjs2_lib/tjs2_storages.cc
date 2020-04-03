#include "tjs2_storages.h"

#include <fstream>

#include <boost/format.hpp>

#include <logger.h>

#include "krkrz_application.h"
#include "tjs2_lib.h"
#include <MsgIntf.h>

namespace {

class TJS2Storages : public tTJSNativeClass {
    typedef tTJSNativeClass inherited;

  public:
    TJS2Storages() : inherited(TJS_W("Storages")) {

        TJS_BEGIN_NATIVE_MEMBERS(Storages)
        TJS_DECL_EMPTY_FINALIZE_METHOD
        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL_NO_INSTANCE(
            /*TJS class name*/ Storages) {
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/ Storages)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ addAutoPath) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr path = *param[0];
            krkrz::TJS2NativeStorages::get()->add_auto_path(path.AsStdString());

            if (result)
                result->Clear();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ addAutoPath)
        //----------------------------------------------------------------------
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ removeAutoPath) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr path = *param[0];
            krkrz::TJS2NativeStorages::get()->remove_auto_path(
                path.AsStdString());

            if (result)
                result->Clear();

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ removeAutoPath)
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ isExistentStorage) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr path = *param[0];

            if (result)
                *result =
                    (tjs_int)krkrz::TJS2NativeStorages::get()->is_exist_storage(
                        path.AsStdString());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ isExistentStorage)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ getPlacedPath) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr path = *param[0];
            if (result) {
                *result = krkrz::TJS2NativeStorages::get()->get_placed_path(
                    path.AsStdString());
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ getPlacedPath)

        TJS_END_NATIVE_MEMBERS
    }
    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() {
        TVPThrowExceptionMessage(TVPCannotCreateInstance);
        return nullptr;
    }
};

tjs_uint32 TJS2Storages::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {

tTJSNativeClass *create_tjs2_storages() { return new TJS2Storages(); }

TJS2NativeStorages::TJS2NativeStorages()
    : _app_path(krkrz::Application::get()->app_path),
      _default_storage_data_path(this->_app_path / "data") {
    this->_load_default_storage();
}
std::shared_ptr<TJS2NativeStorages::SearchPathCache>
TJS2NativeStorages::search_storage(const my::fs::path &path) {
    // search cache
    auto it = this->_search_path_cache.find(path);
    if (it != this->_search_path_cache.end()) {
        return it->second;
    }

    std::shared_ptr<SearchPathCache> cache = nullptr;

    // search app path/
    my::fs::path full_path = this->_app_path / path;
    if (my::fs::exists(full_path)) {
        cache = std::make_shared<SearchPathCache>(full_path);
        this->_search_path_cache.insert({path, cache});
        return cache;
    }

    // search data/
    full_path = this->_default_storage_data_path / path;
    if (my::fs::exists(full_path)) {
        cache = std::make_shared<SearchPathCache>(full_path);
        this->_search_path_cache.insert({path, cache});
        return cache;
    }

    for (auto &auto_path : this->_auto_paths) {
        if (auto_path->type != AutoPathType::DIR) {
            continue;
        }

        full_path = this->_default_storage_data_path / auto_path->path / path;
        if (my::fs::exists(full_path)) {
            cache = std::make_shared<SearchPathCache>(full_path, nullptr,
                                                      auto_path);
            this->_search_path_cache.insert({path, cache});
            return cache;
        }
    }

    // search archives
    for (auto &auto_path : this->_auto_paths) {
        if (auto_path->type != AutoPathType::ARCHIVE) {
            continue;
        }

        auto it = this->_archives.find(auto_path->path);
        if (it == this->_archives.end()) {
            throw std::runtime_error(
                (boost::format("archive %1% is not exist") % auto_path->path)
                    .str());
        }

        if (it->second->exists(path)) {
            cache = std::make_shared<SearchPathCache>(path, auto_path);
            this->_search_path_cache.insert({path, cache});
            return cache;
        }

        for (auto &dir_auto_path : this->_auto_paths) {
            if (dir_auto_path->type != AutoPathType::DIR) {
                continue;
            }
            full_path = dir_auto_path->path / path;
            if (it->second->exists(full_path)) {
                cache = std::make_shared<SearchPathCache>(full_path, auto_path,
                                                          dir_auto_path);
                this->_search_path_cache.insert({path, cache});
                return cache;
            }
        }
    }

    return cache;
}

std::u16string TJS2NativeStorages::storage_read_all(const my::fs::path &path) {
    auto code = this->_storage_read_all(path);

    try {
        // default utf8 -> utf16
        return codecvt::utf_to_utf<char16_t>(code, codecvt::method_type::stop);
    } catch (codecvt::conversion_error &) {
        // locale to utf8
        std::vector<std::string> encodes{"SHIFT_JIS", "GB18030"};
        for (const auto &encode : encodes) {
            try {
                code = codecvt::to_utf<char>(code, encode,
                                             codecvt::method_type::stop);
            } catch (codecvt::conversion_error &) {
                continue;
            }
            break;
        }
    }
    return codecvt::utf_to_utf<char16_t>(code, codecvt::method_type::stop);
}

std::string TJS2NativeStorages::_storage_read_all(const my::fs::path &path) {
    auto cache = this->search_storage(path);
    if (!cache) {
        throw std::runtime_error(
            (boost::format("storage %1% is not exist") % path).str());
    }

    if (cache->archive_auto_path) {
        GLOG_D("read archive %s>%s",
               cache->archive_auto_path->path.filename().c_str(),
               cache->full_path.c_str());
        return this->_archives.at(cache->archive_auto_path->path)
            ->open(cache->full_path)
            ->read_all();
    }

    GLOG_D("read %s", cache->full_path.filename().c_str());
    // TODO:
    auto size = my::fs::file_size(cache->full_path);
    std::ifstream ifs;
    ifs.exceptions(std::fstream::badbit | std::fstream::failbit);
    ifs.open(cache->full_path);
    std::string s(size, 0);
    ifs.read(s.data(), size);
    return s;
}

std::u16string
TJS2NativeStorages::get_placed_path(const std::u16string &utf16_path) {
    utf16_codecvt codecvt;
    auto path = my::fs::path(codecvt.to_bytes(utf16_path));

    if (".xp3" == path.extension()) {
        // TODO: async load xp3
        if (my::fs::exists(path)) {
            this->_load_storage(path);
            return utf16_path;
        } else {
            return u"";
        }
    }

    auto cache = this->search_storage(path);
    if (cache) {
        return codecvt.from_bytes(cache->full_path);
    }
    return u"";
}

void TJS2NativeStorages::_load_default_storage() {
    my::fs::path default_storage = this->_app_path / "data.xp3";
    this->_load_storage(default_storage);
}

void TJS2NativeStorages::_load_storage(my::fs::path path) {
    GLOG_D("load storage %s", path.c_str());
    if (my::fs::exists(path)) {
        this->_add_auto_path(path.u8string() + ">");
        this->_archives.insert({path, my::Archive::make_xp3(path)});
    }
}
TJS2NativeStorages::AutoPathType
TJS2NativeStorages::_check_auto_path_type(const std::string &path) {
    auto end = path.back();
    if (end == '>') {
        return AutoPathType::ARCHIVE;
    } else if (end == '/') {
        return AutoPathType::DIR;
    } else {
        TVPThrowExceptionMessage(TVPMissingPathDelimiterAtLast);
        // XXX:
        return AutoPathType::DIR;
    }
}

void TJS2NativeStorages::_add_auto_path(const std::string &_path) {
    auto type = this->_check_auto_path_type(_path);
    auto path = _path.substr(0, _path.size() - 1);

    auto auto_path = std::make_shared<AutoPath>(type, path);
    auto it = std::find_if(this->_auto_paths.begin(), this->_auto_paths.end(),
                           [&auto_path](auto &v) { return *v == *auto_path; });

    if (it == this->_auto_paths.end()) {
        this->_auto_paths.insert(auto_path);
    }
}

void TJS2NativeStorages::_remove_auto_path(const std::string &_path) {
    auto type = this->_check_auto_path_type(_path);
    auto path = _path.substr(0, _path.size() - 1);

    AutoPath auto_path{type, path};
    auto it = std::find_if(this->_auto_paths.begin(), this->_auto_paths.end(),
                           [&auto_path](auto &v) { return *v == auto_path; });

    if (it == this->_auto_paths.end()) {
        for (auto cache_it = this->_search_path_cache.begin();
             cache_it != this->_search_path_cache.end();) {
            if (cache_it->second->archive_auto_path == *it ||
                cache_it->second->dir_auto_path == *it) {
                this->_search_path_cache.erase(cache_it);
            } else {
                ++cache_it;
            }
        }
        this->_auto_paths.erase(it);
    }
}

} // namespace krkrz
