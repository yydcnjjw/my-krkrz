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
                *result = krkrz::TJS2NativeStorages::get()
                              ->get_placed_path(path.AsStdString())
                              .value_or(TJS_W(""));
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

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/ searchCD) {
            if (numparams < 1)
                return TJS_E_BADPARAMCOUNT;

            if (result)
                *result = TJS_W("media");

            return TJS_S_OK;
        }
        TJS_END_NATIVE_STATIC_METHOD_DECL(/*func. name*/ searchCD)

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
      _app_path(krkrz::Application::get()->app_path),
      _exec_path(krkrz::Application::get()->exec_path),
      _default_storage_data_path(this->_app_path / "data") {
    this->_add_auto_path(this->_app_path / "data.xp3>");
}

std::optional<std::shared_ptr<TJS2NativeStorages::SearchPathCache>>
TJS2NativeStorages::search_storage(const my::fs::path &path) {
    auto filename = path.filename().generic_string();

    auto low_path = my::fs::path(path).replace_filename(
        boost::algorithm::to_lower_copy(filename));

    // search cache
    auto it = this->_search_path_cache.find(path);
    if (it != this->_search_path_cache.end()) {
        return it->second;
    }

    // search fs system
    std::vector<my::fs::path> fs_paths{
        this->_exec_path.parent_path() / path,      //
        this->_app_path / path,                     // app path/
        this->_default_storage_data_path / path,    // data/
        this->_app_path / low_path,                 // app path/
        this->_default_storage_data_path / low_path // data/
    };

    for (const auto &fs_path : fs_paths) {
        auto search_path = this->_add_cache_if_exist<my::FSResourceLocator>(
            path, nullptr, nullptr, fs_path);
        if (search_path.has_value()) {
            return search_path;
        }
    }

    // search fs system with auto path
    for (auto &auto_path : this->_auto_paths) {
        if (auto_path->type != AutoPathType::DIR) {
            continue;
        }

        std::vector<my::fs::path> fs_paths{
            this->_default_storage_data_path / auto_path->path / path,
            this->_default_storage_data_path / auto_path->path / low_path};

        for (const auto &fs_path : fs_paths) {
            auto search_path = this->_add_cache_if_exist<my::FSResourceLocator>(
                path, nullptr, auto_path, fs_path);
            if (search_path.has_value()) {
                return search_path;
            }
        }
    }

    // search archives
    for (auto &archive_auto_path : this->_auto_paths) {
        if (archive_auto_path->type != AutoPathType::ARCHIVE) {
            continue;
        }

        auto archive_query_paths = {
            path, my::fs::path(path).replace_filename(
                      boost::algorithm::to_lower_copy(filename))};
        for (const auto &query_path : archive_query_paths) {
            auto search_path =
                this->_add_cache_if_exist<my::XP3ResourceLocator>(
                    path, nullptr, nullptr, archive_auto_path->path,
                    query_path);
            if (search_path.has_value()) {
                return search_path;
            }
        }

        for (auto &dir_auto_path : this->_auto_paths) {
            if (dir_auto_path->type != AutoPathType::DIR) {
                continue;
            }

            auto archive_query_paths = {
                dir_auto_path->path / path,
                dir_auto_path->path /
                    my::fs::path(path).replace_filename(
                        boost::algorithm::to_lower_copy(filename))};

            for (const auto &query_path : archive_query_paths) {
                auto search_path =
                    this->_add_cache_if_exist<my::XP3ResourceLocator>(
                        path, archive_auto_path, dir_auto_path,
                        archive_auto_path->path, query_path);
                if (search_path.has_value()) {
                    return search_path;
                }
            }
        }
    }

    return std::nullopt;
}

std::optional<std::u16string>
TJS2NativeStorages::get_placed_path(const std::u16string &utf16_path) {
    auto path =
        my::fs::path(codecvt::utf_to_utf<char>(utf16_path)).lexically_normal();

    auto cache = this->search_storage(path);
    if (cache.has_value()) {
        return codecvt::utf_to_utf<char16_t>(cache.value()->locator->get_id());
    }
    return std::nullopt;
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
    GLOG_D("add auto path %s", _path.c_str());
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
    GLOG_D("remove auto path %s", _path.c_str());    
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
