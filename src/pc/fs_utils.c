#include <stdio.h>
#include <string.h>

#include "fs_utils.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(TARGET_PSP)
#include <pspiofilemgr.h>
#elif !defined(TARGET_PSP) && !defined(TARGET_WEB)
#include <unistd.h>
#endif

static char s_fs_base_path[FS_MAX_PATH];

static void fs_copy_string(char *dst, size_t size, const char *src) {
    if (size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    snprintf(dst, size, "%s", src);
}

static void fs_join_path(char *dst, size_t size, const char *base, const char *leaf) {
    size_t baseLen;
    size_t leafLen;

    if (size == 0) {
        return;
    }

    if (base == NULL || base[0] == '\0') {
        fs_copy_string(dst, size, leaf);
        return;
    }

    if (leaf == NULL || leaf[0] == '\0') {
        fs_copy_string(dst, size, base);
        return;
    }

    baseLen = strlen(base);
    leafLen = strlen(leaf);

    if (baseLen + leafLen + 2 > size) {
        fs_copy_string(dst, size, leaf);
        return;
    }

    snprintf(dst, size, "%s/%s", base, leaf);
}

static int fs_has_root_separator(const char *path) {
    return strchr(path, '/') != NULL || strchr(path, '\\') != NULL;
}

static int fs_is_absolute_path(const char *path) {
    if (path == NULL || path[0] == '\0') {
        return 0;
    }

#if defined(_WIN32) || defined(_WIN64)
    return (path[0] == '\\' && path[1] == '\\')
        || ((path[0] != '\0' && path[1] == ':')
            && (path[2] == '\\' || path[2] == '/'));
#else
    return path[0] == '/';
#endif
}

static void fs_strip_filename(char *path) {
    char *lastForwardSlash = strrchr(path, '/');
    char *lastBackSlash = strrchr(path, '\\');
    char *lastSlash = lastForwardSlash;

    if (lastBackSlash != NULL && (lastSlash == NULL || lastBackSlash > lastSlash)) {
        lastSlash = lastBackSlash;
    }

    if (lastSlash == NULL) {
        path[0] = '\0';
        return;
    }

    if (lastSlash == path) {
        lastSlash[1] = '\0';
        return;
    }

#if defined(_WIN32) || defined(_WIN64)
    if (lastSlash == path + 2 && path[1] == ':') {
        lastSlash[1] = '\0';
        return;
    }
#endif

    *lastSlash = '\0';
}

static void fs_ensure_directory(const char *path) {
#if defined(TARGET_PSP)
    if (path != NULL && path[0] != '\0') {
        sceIoMkdir(path, 0777);
    }
#else
    (void) path;
#endif
}

void fs_init(const char *argv0) {
    char path[FS_MAX_PATH];

    s_fs_base_path[0] = '\0';
    path[0] = '\0';

#if defined(TARGET_PSP)
    fs_copy_string(s_fs_base_path, sizeof(s_fs_base_path), "ms0:/PSP/SAVEDATA/SM64PORT");
    fs_ensure_directory("ms0:/PSP");
    fs_ensure_directory("ms0:/PSP/SAVEDATA");
    fs_ensure_directory(s_fs_base_path);
    return;
#endif

#if defined(_WIN32) || defined(_WIN64)
    if (GetModuleFileNameA(NULL, path, sizeof(path)) > 0) {
        fs_strip_filename(path);
        fs_copy_string(s_fs_base_path, sizeof(s_fs_base_path), path);
        return;
    }
#endif

    if (argv0 == NULL || argv0[0] == '\0') {
#if !defined(TARGET_PSP) && !defined(TARGET_WEB)
        if (getcwd(path, sizeof(path)) != NULL) {
            fs_copy_string(s_fs_base_path, sizeof(s_fs_base_path), path);
        }
#endif
        return;
    }

    fs_copy_string(path, sizeof(path), argv0);

#if !defined(_WIN32) && !defined(_WIN64) && !defined(TARGET_PSP) && !defined(TARGET_WEB)
    if (!fs_is_absolute_path(path) && fs_has_root_separator(path)) {
        char cwd[FS_MAX_PATH];
        char absolutePath[FS_MAX_PATH];

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            fs_join_path(absolutePath, sizeof(absolutePath), cwd, path);
            fs_copy_string(path, sizeof(path), absolutePath);
        }
    }
#endif

    if (!fs_has_root_separator(path)) {
#if !defined(TARGET_PSP) && !defined(TARGET_WEB)
        if (getcwd(path, sizeof(path)) != NULL) {
            fs_copy_string(s_fs_base_path, sizeof(s_fs_base_path), path);
        }
#endif
        return;
    }

    fs_strip_filename(path);
    fs_copy_string(s_fs_base_path, sizeof(s_fs_base_path), path);
}

void fs_resolve_path(char *buffer, size_t size, const char *filename) {
    if (size == 0) {
        return;
    }

    if (filename == NULL || filename[0] == '\0') {
        buffer[0] = '\0';
        return;
    }

    if (s_fs_base_path[0] == '\0' || fs_is_absolute_path(filename)) {
        fs_copy_string(buffer, size, filename);
        return;
    }

    fs_join_path(buffer, size, s_fs_base_path, filename);
}

void fs_prepare_parent_path(const char *filename) {
    char path[FS_MAX_PATH];

    fs_resolve_path(path, sizeof(path), filename);
    fs_strip_filename(path);
    fs_ensure_directory(path);
}
