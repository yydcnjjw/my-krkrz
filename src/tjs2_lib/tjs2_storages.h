#pragma once

#include <memory>
#include <string>

#include <tjs2_lib/tjs2_lib.h>

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
        std::shared_ptr<my::ResourceLocator> locator;
        std::shared_ptr<AutoPath> archive_auto_path;
        std::shared_ptr<AutoPath> dir_auto_path;
        explicit SearchPathCache(
            std::shared_ptr<my::ResourceLocator> locator,
            std::shared_ptr<AutoPath> archive_auto_path = nullptr,
            std::shared_ptr<AutoPath> dir_auto_path = nullptr)
            : locator(locator), archive_auto_path(archive_auto_path),
              dir_auto_path(dir_auto_path) {}
    };

    static TJS2NativeStorages *get() {
        static TJS2NativeStorages instance;
        return &instance;
    }

    std::optional<std::shared_ptr<SearchPathCache>>
    search_storage(const my::fs::path &path);

    template <typename T>
    std::shared_ptr<T> get_storage(const std::u16string &uri) {
        return this->get_storage<T>(codecvt::utf_to_utf<char>(uri));
    }

    template <typename T>
    std::shared_ptr<T> get_storage(const std::string &search,
                                   const std::string &mode = "") {
        std::shared_ptr<SearchPathCache> search_path{};
        if (!this->_resource_mgr->exist(search)) {
            search_path = this->search_storage(search).value_or(nullptr);
        } else {
            return this->_resource_mgr->load_from_cache<T>(search);
        }

        if (!search_path) {
            throw std::runtime_error(
                (boost::format("%1% is not exist") % search).str());
        }

        size_t offset{0};
        if (!mode.empty()) {
            auto ch = mode.at(0);
            switch (ch) {
            case 'o': {
                offset = std::stoi(mode.substr(1));
                break;
            }
            }
        }
        search_path->locator->set_offset(offset);

        return this->_resource_mgr->load<T>(search_path->locator).get();
    }

    bool is_exist_storage(const std::u16string &utf16_path) {
        return this->get_placed_path(utf16_path).has_value();
    }

    std::optional<std::u16string>
    get_placed_path(const std::u16string &utf16_path);

    void add_auto_path(const std::u16string &utf16_path) {
        this->_add_auto_path(codecvt::utf_to_utf<char>(utf16_path));
    }
    void remove_auto_path(const std::u16string &utf16_path) {
        this->_remove_auto_path(codecvt::utf_to_utf<char>(utf16_path));
    }

    std::u16string extract_storage_name(const std::u16string &utf16_path) {
        return my::fs::path(codecvt::utf_to_utf<char>(utf16_path))
            .filename()
            .u16string();
    }

    std::u16string chop_storage_ext(const std::u16string &utf16_path) {
        auto path = my::fs::path(codecvt::utf_to_utf<char>(utf16_path));
        return (path.parent_path() / path.stem()).u16string();
    }

  private:
    my::ResourceMgr *_resource_mgr;
    my::fs::path _app_path;
    my::fs::path _exec_path;
    my::fs::path _default_storage_data_path;
    std::set<std::shared_ptr<AutoPath>> _auto_paths;

    typedef std::unordered_map<std::string, std::shared_ptr<SearchPathCache>>
        search_path_cache_map;
    search_path_cache_map _search_path_cache;

    TJS2NativeStorages();

    static AutoPathType _check_auto_path_type(const std::string &path);

    void _add_auto_path(const std::string &_path);

    void _remove_auto_path(const std::string &_path);

    template <typename resource_locator, typename... Args>
    std::optional<std::shared_ptr<SearchPathCache>> _add_cache_if_exist(
        const my::fs::path &path, std::shared_ptr<AutoPath> archive_auto_path,
        std::shared_ptr<AutoPath> dir_auto_path, Args &... args) {
        auto locator = resource_locator::make(std::forward<Args>(args)...);
        if (locator->exist()) {
            auto search_path = std::make_shared<SearchPathCache>(
                locator, archive_auto_path, dir_auto_path);
            this->_search_path_cache.insert({path, search_path});
            return search_path;
        } else {
            return std::nullopt;
        }
    }
};

} // namespace krkrz
