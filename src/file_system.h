#ifndef _BASE_FILE_SYSTEM_H_
#define _BASE_FILE_SYSTEM_H_
#include <list>
#include <string>
#include <fstream>
EZLOG_NAMESPACE_BEGIN
namespace file_system
{
    using path_t  = std::string;
    using paths_t = std::list<path_t>;
    using bytes_t = std::basic_string<unsigned char>;

    enum walk_filter_t
    {
        file      = (1 << 0),
        directory = (1 << 1),
        all       = file | directory
    };

    bool    exists(const std::string& path);
    bool    mkdir(const std::string& path);
    bool    rmdir(const std::string& path);
    bool    rename(const std::string& from, const std::string& to);
    paths_t walk(const path_t& path, walk_filter_t filter = walk_filter_t::file, int depth = 0);
    bytes_t load(const path_t& path);
    size_t  save(const path_t& path, const bytes_t& bytes);
    size_t  save(const path_t& path, const void* data, const size_t& length);
    template <typename T>
    static inline size_t save(const path_t& path, const std::basic_string<T>& data)
    {
        return file_system::save(data.data(), data.length());
    }
    size_t copy(const path_t& from, const path_t& to);

} // namespace file_system
EZLOG_NAMESPACE_END
#endif // !_BASE_FILE_SYSTEM_H_