/******************************** Synthetic FS *********************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
Created Date    : 27th Aug 2026
******************************************************************************/

#include <config/Config.h>

#if defined(ENABLE_PROCFS) || defined(ENABLE_SYSFS)

#include "SynthFs.h"
#include <interface/pdi.h>

SynthFs::SynthFs(iStorageInterface &storage, const char *root)
    : iFileSystemInterface(storage), m_root(root) {}

/**
 * The path with its leading separators removed, relative to the mount.
 */
const char *SynthFs::normalizePath(const char *path) {
    if (nullptr == path) return "";
    while (*path == PATH_SEPARATOR_CHAR) path++;
    return path;
}

/**
 * Consume one path segment when it equals the literal, else leave the cursor.
 */
bool SynthFs::matchSegment(const char *&cursor, const char *literal) {
    uint32_t n = strlen(literal);
    if (strncmp(cursor, literal, n) != 0) return false;
    char after = cursor[n];
    if (after != '\0' && after != PATH_SEPARATOR_CHAR) return false;
    cursor += n;
    return true;
}

/**
 * Consume a numeric path segment, or report why it is not one.
 */
int32_t SynthFs::numberSegment(const char *&cursor) {
    if (*cursor < '0' || *cursor > '9') return PDI_ERR_INVALID_ARG;

    int32_t value = 0;
    while (*cursor >= '0' && *cursor <= '9') {
        value = (int32_t)(value * 10 + (*cursor - '0'));
        cursor++;
        if (value > 65535) return PDI_ERR_RANGE;
    }

    if (*cursor != '\0' && *cursor != PATH_SEPARATOR_CHAR) return PDI_ERR_INVALID_ARG;
    return value;
}

/**
 * Step over the separator between two segments.
 */
bool SynthFs::nextSegment(const char *&cursor) {
    if (*cursor != PATH_SEPARATOR_CHAR) return false;
    cursor++;
    return true;
}

/**
 * Append one directory entry, taking a heap copy of the name because the
 * callers that list a directory free it.
 */
void SynthFs::addEntry(pdiutil::vector<file_info_t> &items, const char *name,
                       file_type_t type, uint16_t perms, int64_t size) {
    if (nullptr == name) return;

    file_info_t info;
    memset(&info, 0, sizeof(info));
    info.m_type = type;
    info.m_size = size;
    info.m_perms = perms;
    info.m_uid = 0;
    info.m_gid = 0;
    info.m_ctime = 0;
    info.m_mtime = 0;

    uint32_t nlen = strlen(name);
    info.m_name = pdiutil::safe_new_array<char>(nlen + 1);
    if (nullptr == info.m_name) return;
    memcpy(info.m_name, name, nlen);
    info.m_name[nlen] = '\0';

    items.push_back(info);
}

/**
 * How long a file node reads. Rendering it is the honest default; a node
 * whose content is expensive to build overrides this and computes instead.
 */
int64_t SynthFs::sizeOf(const char *path) {
    pdiutil::string content = render(path);
    return content.empty() ? PDI_ERR_NOT_FOUND : (int64_t)content.length();
}

/**
 * The permission bits a node carries. Read-only trees keep the default.
 */
uint16_t SynthFs::permsFor(const char *path, synth_node_t kind) {
    return (SYNTH_DIR == kind) ? 0555 : 0444;
}

pdiutil::string SynthFs::basename(const char *path) {
    if (nullptr == path) return pdiutil::string();

    const char *last = path;
    for (const char *p = path; *p; ++p) {
        if (*p == PATH_SEPARATOR_CHAR) last = p + 1;
    }
    return pdiutil::string(last);
}

int SynthFs::readFile(const char *path, uint64_t size,
                      pdiutil::function<bool(char *, uint32_t)> readbackfn,
                      uint64_t offset, const char *readUntilMatchStr,
                      bool *didmatchfound) {
    if (nullptr == path || !readbackfn) return PDI_ERR_INVALID_ARG;

    pdiutil::string content = render(path);
    if (content.empty()) return PDI_ERR_NOT_FOUND;
    if (offset >= content.length()) return 0;

    // size is the per-iteration chunk limit, not a total cap, so the callback
    // is looped until the content is delivered or it asks to stop
    uint32_t total = content.length() - (uint32_t)offset;

    // the count returned stops short of the match, so a caller stepping line by
    // line adds the length of what it matched on to reach the next one
    if (nullptr != readUntilMatchStr && '\0' != readUntilMatchStr[0]) {
        int32_t at = __strstr(content.c_str() + offset, total, readUntilMatchStr,
                              (uint32_t)strlen(readUntilMatchStr), 0);
        if (at >= 0) {
            total = (uint32_t)at;
            if (nullptr != didmatchfound) *didmatchfound = true;
            if (0 == total) return 0;
        }
    }
    uint32_t chunk = (size > 0 && size < total) ? (uint32_t)size : total;
    uint32_t done = 0;

    while (done < total) {
        uint32_t n = total - done;
        if (n > chunk) n = chunk;
        if (!readbackfn((char *)content.c_str() + offset + done, n)) break;
        done += n;
    }

    return (int)done;
}

int64_t SynthFs::getFileSize(const char *path) {
    return (SYNTH_FILE == resolve(path)) ? sizeOf(path) : PDI_ERR_NOT_FOUND;
}

bool SynthFs::isFileExist(const char *path) {
    return SYNTH_FILE == resolve(path);
}

bool SynthFs::isDirExist(const char *path) {
    return SYNTH_DIR == resolve(path);
}

bool SynthFs::isDirectory(const char *path) {
    return isDirExist(path);
}

int SynthFs::getDirFileList(const char *path, pdiutil::vector<file_info_t> &items,
                            const char *pattern) {
    if (SYNTH_DIR != resolve(path)) return STORAGE_ERROR_NOT_A_DIRECTORY;
    return listChildren(path, items);
}

int SynthFs::getFileAttr(const char *path, uint8_t type, void *buffer,
                         uint32_t size) {
    if (nullptr == buffer || 0 == size) return PDI_ERR_INVALID_ARG;

    synth_node_t kind = resolve(path);
    if (SYNTH_NONE == kind) return PDI_ERR_NOT_FOUND;

    if (FILE_ATTR_PERMS == type && size >= sizeof(uint16_t)) {
        *(uint16_t *)buffer = permsFor(path, kind);
        return sizeof(uint16_t);
    }
    if (FILE_ATTR_UID == type && size >= sizeof(uint16_t)) {
        *(uint16_t *)buffer = 0;
        return sizeof(uint16_t);
    }
    if (FILE_ATTR_GID == type && size >= sizeof(uint16_t)) {
        *(uint16_t *)buffer = 0;
        return sizeof(uint16_t);
    }

    return STORAGE_ERROR_ATTR_NOT_FOUND;
}

pdi_err_t SynthFs::getFileMeta(const char *path, file_info_t &out) {
    synth_node_t kind = resolve(path);
    if (SYNTH_NONE == kind) return PDI_ERR_NOT_FOUND;

    // m_name is left untouched per the iFileSystemInterface contract: callers
    // that find it set assume it is heap owned and free it
    out.m_type = (SYNTH_DIR == kind) ? FILE_TYPE_DIR : FILE_TYPE_REG;
    out.m_size = (SYNTH_DIR == kind) ? 0 : sizeOf(path);
    out.m_perms = permsFor(path, kind);
    out.m_uid = 0;
    out.m_gid = 0;
    out.m_ctime = 0;
    out.m_mtime = 0;

    return 0;
}

#endif
