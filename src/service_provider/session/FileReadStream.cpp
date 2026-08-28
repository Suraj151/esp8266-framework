/**************************** File Read Stream ********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/

#include "FileReadStream.h"

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

#include <interface/pdi.h>
#include <utility/SafeAlloc.h>

/**
 * Opens the named file for reading. An absent file leaves the stream
 * invalid rather than empty, so the caller can tell the two apart.
 */
FileReadStream::FileReadStream(const char *path) :
  m_buffer(nullptr), m_fill(0), m_take(0), m_offset(0), m_size(-1), m_consumed(0) {

  if (nullptr != path) {
    m_path = path;
  }

  if (m_path.size() > 0 && __i_fs.isFileExist(m_path.c_str())) {
    m_size = __i_fs.getFileSize(m_path.c_str());
    if (m_size >= 0) {
      m_buffer = pdiutil::safe_new_array<uint8_t>(PDI_FILE_STREAM_BUFFER);
    }
  }
}

FileReadStream::~FileReadStream() {
  pdiutil::safe_delete_array(m_buffer);
}

/**
 * Pulls the next block out of the file. False when nothing came back.
 */
bool FileReadStream::refill() {

  if (nullptr == m_buffer || m_size < 0 || m_offset >= (uint64_t)m_size) {
    return false;
  }

  m_fill = 0;
  m_take = 0;

  __i_fs.readFile(m_path.c_str(), PDI_FILE_STREAM_BUFFER, [&](char *data, uint32_t size) -> bool {

    uint32_t room = PDI_FILE_STREAM_BUFFER - m_fill;
    if (size > room) size = room;

    memcpy(m_buffer + m_fill, data, size);
    m_fill += (uint16_t)size;

    // one block is enough; the next call carries on from the new offset
    return false;
  }, m_offset);

  m_offset += m_fill;
  return m_fill > 0;
}

/**
 * Nothing to disconnect, so this only reports the stream is usable.
 */
int16_t FileReadStream::disconnect() {
  return isValid() ? (int16_t)0 : (int16_t)-1;
}

/**
 * Usable while it holds a buffer and the file was found.
 */
int8_t FileReadStream::connected() {
  return isValid() ? (int8_t)1 : (int8_t)0;
}

/**
 * Read only, so writes are discarded.
 */
int32_t FileReadStream::write(uint8_t c) {
  return 0;
}

/**
 * Read only, so writes are discarded.
 */
int32_t FileReadStream::write(const uint8_t *c_str) {
  return 0;
}

/**
 * Read only, so writes are discarded.
 */
int32_t FileReadStream::write(const uint8_t *c_str, uint32_t size) {
  return 0;
}

/**
 * Read only, so writes are discarded.
 */
int32_t FileReadStream::write_ro(const char *c_str) {
  return 0;
}

/**
 * Takes one byte, or zero once the file is spent.
 */
uint8_t FileReadStream::read() {

  uint8_t one = 0;
  return (read(&one, 1) > 0) ? one : (uint8_t)0;
}

/**
 * Takes up to size bytes, refilling from the file as the buffer empties.
 */
int32_t FileReadStream::read(uint8_t *buf, uint32_t size) {

  if (nullptr == buf || 0 == size || !isValid()) {
    return 0;
  }

  uint32_t given = 0;

  while (given < size) {

    if (m_take >= m_fill && !refill()) {
      break;
    }

    uint32_t held = (uint32_t)(m_fill - m_take);
    uint32_t chunk = size - given;
    if (chunk > held) chunk = held;

    memcpy(buf + given, m_buffer + m_take, chunk);
    m_take += (uint16_t)chunk;
    given += chunk;
  }

  m_consumed += given;
  return (int32_t)given;
}

/**
 * What is left, buffered bytes plus whatever the file still holds.
 */
int32_t FileReadStream::available() {

  if (!isValid()) {
    return 0;
  }

  uint32_t buffered = (uint32_t)(m_fill - m_take);
  uint64_t unread = ((uint64_t)m_size > m_offset) ? ((uint64_t)m_size - m_offset) : 0;

  return (int32_t)(buffered + unread);
}

/**
 * Read only, so there is never room to write.
 */
bool FileReadStream::availableforwrite(uint32_t size) {
  return false;
}

/**
 * A receive flush abandons what is left rather than reading it.
 */
void FileReadStream::flush(int16_t flushtype) {

  if (IsFlushRx(flushtype)) {
    m_fill = 0;
    m_take = 0;
    m_offset = (m_size > 0) ? (uint64_t)m_size : 0;
  }
}

#endif
