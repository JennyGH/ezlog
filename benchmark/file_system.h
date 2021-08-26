#ifndef _BASE_FILE_SYSTEM_H_
#define _BASE_FILE_SYSTEM_H_
#include <list>
#include <string>
#include <fstream>

namespace ez
{
    namespace base
    {
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
            paths_t walk(const path_t& path, walk_filter_t filter = walk_filter_t::file, int depth = 0);
            bytes_t load(const path_t& path);
            size_t  save(const path_t& path, const bytes_t& bytes);
            size_t  save(const path_t& path, const void* data, const size_t& length);
            size_t  copy(const path_t& from, const path_t& to);

        } // namespace file_system

    } // namespace base

} // namespace ez

#endif // !_BASE_FILE_SYSTEM_H_