#ifndef AIDS_FS_H
#define AIDS_FS_H
#include "aids.h"
typedef struct {
    u64 size;
    bool is_directory;
    bool is_regular;
    bool is_symlink;
    u64 modified_time;
} FileInfo;

IMPORT int file_stat(const char* path, FileInfo* info);
IMPORT int directory_create(const char* path);
IMPORT bool exists(const char* path);
EXPORT int directory_remove(const char* path);
#endif /*AIDS_FS_H*/

#ifdef AIDS_FS_IMPLEMENTATION
#ifndef AIDS_FS_IMPLEMENTATION_H
#define AIDS_FS_IMPLEMENTATION_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/stat.h>
#endif

EXPORT int file_stat(const char* path, FileInfo* info) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data;

    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data))
        return -1;

    if (!info)
        return 0;

    info->is_directory =
        (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    info->is_symlink =
        (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    info->is_regular = !info->is_directory;

    info->size = ((u64)data.nFileSizeHigh << 32) | data.nFileSizeLow;

    ULARGE_INTEGER time;
    time.LowPart = data.ftLastWriteTime.dwLowDateTime;
    time.HighPart = data.ftLastWriteTime.dwHighDateTime;

    info->modified_time = (time.QuadPart - 116444736000000000ULL) / 10000000ULL;

    return 0;
#else
    struct stat data;

    if (stat(path, &data) == -1)
        return -1;

    if (!info)
        return 0;

    info->size = (u64)data.st_size;
    info->is_directory = S_ISDIR(data.st_mode);
    info->is_regular = S_ISREG(data.st_mode);
    info->is_symlink = false;
    info->modified_time = (u64)data.st_mtime;

    return 0;
#endif
}

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

EXPORT int directory_create(const char* path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

EXPORT bool exists(const char* path) { return file_stat(path, NULL) == 0; }
#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

EXPORT int directory_remove(const char* path) {
#ifdef _WIN32
    char pattern[4096];
    snprintf(pattern, sizeof(pattern), "%s\\*", path);

    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA(pattern, &data);

    if (find == INVALID_HANDLE_VALUE)
        return RemoveDirectoryA(path) ? 0 : -1;

    do {
        if (strcmp(data.cFileName, ".") == 0 ||
            strcmp(data.cFileName, "..") == 0)
            continue;

        char child[4096];
        snprintf(child, sizeof(child), "%s\\%s", path, data.cFileName);

        if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                if (!RemoveDirectoryA(child)) {
                    FindClose(find);
                    return -1;
                }
            } else {
                if (directory_remove(child) == -1) {
                    FindClose(find);
                    return -1;
                }
            }
        } else {
            if (!DeleteFileA(child)) {
                FindClose(find);
                return -1;
            }
        }
    } while (FindNextFileA(find, &data));

    FindClose(find);
    return RemoveDirectoryA(path) ? 0 : -1;

#else
    DIR* dir = opendir(path);

    if (!dir)
        return remove(path);

    struct dirent* entry;
    char child[4096];

    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);

        struct stat info;
        if (lstat(child, &info) == -1) {
            closedir(dir);
            return -1;
        }

        if (S_ISDIR(info.st_mode)) {
            if (directory_remove(child) == -1) {
                closedir(dir);
                return -1;
            }
        } else {
            if (remove(child) == -1) {
                closedir(dir);
                return -1;
            }
        }
    }

    closedir(dir);
    return rmdir(path);
}
#endif

#endif /*AIDS_FS_IMPLEMENTATION_H*/
#endif /*AIDS_FS_IMPLEMENTATION*/
