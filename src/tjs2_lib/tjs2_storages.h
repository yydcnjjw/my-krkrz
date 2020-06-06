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
        my::uri search_uri;
        std::shared_ptr<AutoPath> archive_auto_path;
        std::shared_ptr<AutoPath> dir_auto_path;
        explicit SearchPathCache(
            const my::uri &search_uri,
            std::shared_ptr<AutoPath> archive_auto_path = nullptr,
            std::shared_ptr<AutoPath> dir_auto_path = nullptr)
            : search_uri(search_uri.encoded_url().to_string()),
              archive_auto_path(archive_auto_path),
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
        my::uri full_uri{search};
        if (!this->_resource_mgr->exist(full_uri)) {
            auto search_path = this->search_storage(
                my::fs::path(full_uri.encoded_path().to_string())
                    .lexically_normal());
            if (search_path.has_value()) {
                full_uri.set_encoded_url(
                    search_path.value()->search_uri.encoded_url());
            } else {
                throw std::runtime_error(
                    (boost::format("%1% is not exist") % search).str());
            }
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

        const auto &params = full_uri.params();

        // TODO: use search storage struct
        my::uri fix_uri{};
        if (params.find("path") != params.end()) {
            fix_uri.set_encoded_url(
                my::make_archive_search_uri(full_uri.encoded_path().to_string(),
                                            params.at("path"), offset)
                    .encoded_url());
        } else {
            fix_uri.set_encoded_url(
                my::make_path_search_uri(full_uri.encoded_path().to_string(),
                                         offset)
                    .encoded_url());
        }

        return this->_resource_mgr->load<T>(fix_uri).get();
    }

    bool is_exist_storage(const std::u16string &utf16_uri) {
        return !this->get_placed_path(utf16_uri).empty();
    }

    std::u16string get_placed_path(const std::u16string &utf16_uri);

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
