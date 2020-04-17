#pragma once

#include <memory>
#include <string>

#include <boost/format.hpp>

#include <util/codecvt.h>
#include <storage/resource_mgr.hpp>

namespace krkrz {
class TJS2NativeStorages {
  public:
    enum AutoPathType { ARCHIVE, DIR };
    struct AutoPath {
        AutoPathType type;
        my::fs::path path;

        AutoPath(const AutoPathType &type, const std::string &path)
            : type(type), path(path) {}

        bool operator==(AutoPath &auto_path) {
            return auto_path.type == this->type && auto_path.path == this->path;
        }
    };

    struct SearchPathCache {
        std::shared_ptr<my::uri> search_uri;
        std::shared_ptr<AutoPath> archive_auto_path;
        std::shared_ptr<AutoPath> dir_auto_path;
        explicit SearchPathCache(
            std::shared_ptr<my::uri> search_uri,
            std::shared_ptr<AutoPath> archive_auto_path = nullptr,
            std::shared_ptr<AutoPath> dir_auto_path = nullptr)
            : search_uri(search_uri), archive_auto_path(archive_auto_path),
              dir_auto_path(dir_auto_path) {}
    };

    static TJS2NativeStorages *get() {
        static TJS2NativeStorages instance;
        return &instance;
    }

    std::optional<std::shared_ptr<SearchPathCache>>
    search_storage(const my::fs::path &path);

    template <typename T>
    std::shared_ptr<T> get_storage(const my::fs::path &path) {
        auto search_path = this->search_storage(path);
        if (search_path.has_value()) {
            return this->_resource_mgr
                ->load_from_uri<T>(*search_path.value()->search_uri)
                .get();
        } else {
            throw std::runtime_error(
                (boost::format("%1% is not exist") % path).str());
        }
    }

    bool is_exist_storage(const std::u16string &utf16_path) {
        return !this->get_placed_path(utf16_path).empty();
    }

    std::u16string get_placed_path(const std::u16string &utf16_path);

    void add_auto_path(const std::u16string &utf16_path) {
        this->_add_auto_path(utf16_codecvt().to_bytes(utf16_path));
    }
    void remove_auto_path(const std::u16string &utf16_path) {
        this->_remove_auto_path(utf16_codecvt().to_bytes(utf16_path));
    }

  private:
    my::ResourceMgr *_resource_mgr;
    my::fs::path _app_path;
    my::fs::path _default_storage_data_path;
    std::set<std::shared_ptr<AutoPath>> _auto_paths;

    typedef std::unordered_map<std::string, std::shared_ptr<SearchPathCache>>
        search_path_cache_map;
    search_path_cache_map _search_path_cache;

    TJS2NativeStorages();

    static AutoPathType _check_auto_path_type(const std::string &path);

    void _add_auto_path(const std::string &_path);

    void _remove_auto_path(const std::string &_path);
};

} // namespace krkrz
