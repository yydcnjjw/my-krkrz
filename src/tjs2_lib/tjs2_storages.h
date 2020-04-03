#pragma once

#include <memory>
#include <string>

#include "codecvt.h"
#include <storage/archive.h>

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
        my::fs::path full_path;
        std::shared_ptr<AutoPath> archive_auto_path;
        std::shared_ptr<AutoPath> dir_auto_path;
        SearchPathCache(const std::string &path,
                        std::shared_ptr<AutoPath> archive_auto_path = nullptr,
                        std::shared_ptr<AutoPath> dir_auto_path = nullptr)
            : full_path(path), archive_auto_path(archive_auto_path),
              dir_auto_path(dir_auto_path) {}
    };

    static TJS2NativeStorages *get() {
        static TJS2NativeStorages instance;
        return &instance;
    }

    std::shared_ptr<SearchPathCache> search_storage(const my::fs::path &path);
    std::u16string storage_read_all(const my::fs::path &path);

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
    my::fs::path _app_path;
    my::fs::path _default_storage_data_path;
    std::set<std::shared_ptr<AutoPath>> _auto_paths;

    typedef std::unordered_map<std::string, std::shared_ptr<SearchPathCache>>
        search_path_cache_map;
    search_path_cache_map _search_path_cache;

    std::map<std::string, std::shared_ptr<my::Archive>> _archives;

    TJS2NativeStorages();

    void _load_default_storage();

    void _load_storage(my::fs::path path);

    AutoPathType _check_auto_path_type(const std::string &path);

    void _add_auto_path(const std::string &_path);

    void _remove_auto_path(const std::string &_path);
    
    std::string _storage_read_all(const my::fs::path &path);
};

} // namespace krkrz
