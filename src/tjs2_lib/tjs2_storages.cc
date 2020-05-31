#include "tjs2_storages.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <util/logger.h>

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
        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ extractStorageName) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr path = *param[0];

            if (result)
                *result =
                    krkrz::TJS2NativeStorages::get()->extract_storage_name(
                        path.AsStdString());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ extractStorageName)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ chopStorageExt) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            ttstr path = *param[0];

            if (result)
                *result = krkrz::TJS2NativeStorages::get()->chop_storage_ext(
                    path.AsStdString());

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ chopStorageExt)

        TJS_END_NATIVE_MEMBERS
    }

    static tjs_uint32 ClassID;

  protected:
    tTJSNativeInstance *CreateNativeInstance() override {
        TVPThrowExceptionMessage(TVPCannotCreateInstance);
        return nullptr;
    }
};

tjs_uint32 TJS2Storages::ClassID = (tjs_uint32)-1;

} // namespace

namespace krkrz {

tTJSNativeClass *create_tjs2_storages() { return new TJS2Storages(); }

TJS2NativeStorages::TJS2NativeStorages()
    : _resource_mgr(Application::get()->base_app()->resource_mgr()),
      _app_path(krkrz::Application::get()->app_path.encoded_path().to_string()),
      _default_storage_data_path(this->_app_path / "data") {
    this->_add_auto_path(this->_app_path / "data.xp3>");
}

std::optional<std::shared_ptr<TJS2NativeStorages::SearchPathCache>>
TJS2NativeStorages::search_storage(const my::fs::path &path) {
    // search cache
    auto it = this->_search_path_cache.find(path);
    if (it != this->_search_path_cache.end()) {
        return it->second;
    }

    // search fs system
    std::vector<my::fs::path> fs_paths{
        this->_app_path / path,                 // app path/
        this->_default_storage_data_path / path // data/
    };

    for (const auto &fs_path : fs_paths) {
        if (my::ResourceMgr::exist(fs_path)) {
            auto search_path = std::make_shared<SearchPathCache>(
                my::make_path_search_uri(fs_path));
            this->_search_path_cache.insert({path, search_path});
            return search_path;
        }
    }

    // search fs system with auto path
    for (auto &auto_path : this->_auto_paths) {
        if (auto_path->type != AutoPathType::DIR) {
            continue;
        }

        auto fs_path =
            this->_default_storage_data_path / auto_path->path / path;
        if (my::ResourceMgr::exist(fs_path)) {
            auto search_path = std::make_shared<SearchPathCache>(
                my::make_path_search_uri(fs_path), nullptr, auto_path);
            this->_search_path_cache.insert({path, search_path});
            return search_path;
        }
    }

    // search archives
    for (auto &archive_auto_path : this->_auto_paths) {
        if (archive_auto_path->type != AutoPathType::ARCHIVE) {
            continue;
        }

        auto filename = path.filename().generic_string();

        auto archive_path_uris = {
            my::make_archive_search_uri(archive_auto_path->path, path),
            my::make_archive_search_uri(
                archive_auto_path->path,
                my::fs::path(path).replace_filename(
                    boost::algorithm::to_lower_copy(filename)))};
        for (const auto &archive_path_uri : archive_path_uris) {
            if (this->_resource_mgr->exist(archive_path_uri)) {
                auto search_path = std::make_shared<SearchPathCache>(
                    archive_path_uri, archive_auto_path);
                this->_search_path_cache.insert({path, search_path});
                return search_path;
            }
        }

        for (auto &dir_auto_path : this->_auto_paths) {
            if (dir_auto_path->type != AutoPathType::DIR) {
                continue;
            }

            auto archive_path_uris = {
                my::make_archive_search_uri(archive_auto_path->path,
                                            dir_auto_path->path / path),
                my::make_archive_search_uri(
                    archive_auto_path->path,
                    dir_auto_path->path /
                        my::fs::path(path).replace_filename(
                            boost::algorithm::to_lower_copy(filename)))};

            for (const auto &archive_path_uri : archive_path_uris) {
                if (this->_resource_mgr->exist(archive_path_uri)) {
                    auto search_path = std::make_shared<SearchPathCache>(
                        archive_path_uri, archive_auto_path, dir_auto_path);
                    this->_search_path_cache.insert({path, search_path});
                    return search_path;
                }
            }
        }
    }

    return std::nullopt;
}

std::u16string
TJS2NativeStorages::get_placed_path(const std::u16string &utf16_uri) {
    auto uri = my::uri(codecvt::utf_to_utf<char>(utf16_uri));
        
    auto cache = this->search_storage(
        my::fs::path(uri.encoded_path().to_string()).lexically_normal());
    if (cache.has_value()) {
        return codecvt::utf_to_utf<char16_t>(
            cache.value()->search_uri.encoded_url().to_string());
    }
    return u"";
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
