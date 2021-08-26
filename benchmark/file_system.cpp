#include "file_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fstream>
#include "path.h"
#ifndef _MSC_VER
#    include <unistd.h>
#    include <dirent.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#else
#    include <io.h>
#    include <direct.h>
#    include <Windows.h>
#endif // !_MSC_VER

static int _walk(const ez::base::path& path, ez::base::file_system::walk_filter_t filter, int depth, ez::base::file_system::paths_t& paths)
{
#ifndef _MSC_VER
    DIR* dir_ptr = opendir(path);
    if (NULL == dir_ptr)
    {
        return -1;
    }

    struct dirent* dir_info = NULL;
    struct stat    file_state;
    while ((dir_info = readdir(dir_ptr)))
    {
        if ((strncmp(dir_info->d_name, ".", 1) == 0) || (strncmp(dir_info->d_name, "..", 2) == 0))
        {
            continue;
        }

        paths.push_back(path.join(dir_info->d_name));
        int rv = lstat(paths.back().c_str(), &file_state);
        if (0 == rv)
        {
            if (depth > 0 && S_ISDIR(file_state.st_mode))
            {
                _walk(paths.back(), filter, depth - 1, paths);
            }
        }
    }
    ::closedir(dir_ptr);
#else
    WIN32_FIND_DATAA data;
    HANDLE           handle = ::FindFirstFileA(path.join("*"), &data);
    if (handle == NULL)
    {
        return -1;
    }
    do
    {
        if ((strncmp(data.cFileName, ".", 1) == 0) || (strncmp(data.cFileName, "..", 2) == 0))
        {
            continue;
        }
        const auto current_path = path.join(data.cFileName);
        if (((filter & ez::base::file_system::walk_filter_t::file) != 0 && (data.dwFileAttributes & _A_SUBDIR) == 0) ||
            ((filter & ez::base::file_system::walk_filter_t::directory) != 0 && (data.dwFileAttributes & _A_SUBDIR) != 0))
        {
            paths.push_back(current_path);
        }
        if (depth > 0 && data.dwFileAttributes & _A_SUBDIR)
        {
            _walk(current_path, filter, depth - 1, paths);
        }
    } while (::FindNextFileA(handle, &data));
    ::FindClose(handle);
#endif
    return paths.size();
}

bool ez::base::file_system::exists(const std::string& path)
{
    return ::access(path.c_str(), 0) == 0;
}

bool ez::base::file_system::mkdir(const std::string& path)
{
#ifdef _MSC_VER
    const ez::base::path native_path = path;
    const ez::base::path parent_path = native_path.parent();
    if (!file_system::exists(parent_path))
    {
        file_system::mkdir(parent_path);
    }
    return 0 == ::mkdir(native_path);
#else
    return 0 == ::mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif // _MSC_VER
}

bool ez::base::file_system::rmdir(const std::string& path)
{
    if (!file_system::exists(path))
    {
        return false;
    }
#ifdef _MSC_VER
    const auto files = file_system::walk(path, file_system::file);
    for (const auto& file : files)
    {
        if (0 != ::rmdir(file.c_str()))
        {
            return false;
        }
    }
    const auto dirs = file_system::walk(path, file_system::directory);
    for (const auto& dir : dirs)
    {
        if (!file_system::rmdir(dir))
        {
            return false;
        }
    }
    return 0 == ::rmdir(path.c_str());
#else
    return 0 == ::rmdir(path.c_str());
#endif // _MSC_VER
}

ez::base::file_system::paths_t ez::base::file_system::walk(const path_t& path, walk_filter_t filter, int depth)
{
    paths_t paths;
    int     count = _walk(path, filter, depth, paths);
    return paths;
}

ez::base::file_system::bytes_t ez::base::file_system::load(const path_t& path)
{
    FILE* file = ::fopen(path.c_str(), "rb");
    if (nullptr == file)
    {
        return bytes_t();
    }

    ::fseek(file, 0L, SEEK_END);
    size_t file_size = ::ftell(file);
    if (0 == file_size)
    {
        ::fclose(file);
        return bytes_t();
    }

    unsigned char* data = (unsigned char*)::malloc(file_size);
    if (nullptr == data)
    {
        ::fclose(file);
        return bytes_t();
    }

    size_t read_size = ::fread(data, 1, file_size, file);

    bytes_t bytes(data, read_size);

    ::free(data);

    ::fclose(file);
    return bytes;
}

size_t ez::base::file_system::save(const path_t& path, const bytes_t& bytes)
{
    return save(path, bytes.data(), bytes.length());
}

size_t ez::base::file_system::save(const ez::base::file_system::path_t& path, const void* src, const size_t& src_size)
{
    FILE* file = ::fopen(path.c_str(), "wb");
    if (nullptr == file)
    {
        return 0;
    }
    size_t output_size = ::fwrite(src, 1, src_size, file);
    ::fclose(file);
    return output_size;
}

size_t ez::base::file_system::copy(const path_t& from, const path_t& to)
{
    if (from.empty() || to.empty())
    {
        return 0;
    }
    if (!exists(from))
    {
        return 0;
    }
    const bytes_t data = load(from);
    if (data.empty())
    {
        return 0;
    }
    return save(to, data);
}
