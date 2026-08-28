/****************************** Pipe Stream ***********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 25th Aug 2026
******************************************************************************/

#include "PipeStream.h"

#ifdef ENABLE_CMD_SERVICE

#include <utility/SafeAlloc.h>

namespace {
  const uint8_t RO_WINDOW = 32;
}

/**
 * Takes a buffer of the given capacity, which is released with the object.
 */
PipeStream::PipeStream(uint16_t capacity) :
  m_buffer(nullptr), m_capacity(capacity), m_overflowed(false) {

  if (m_capacity < 2) {
    m_capacity = 2;
  }

  m_buffer = pdiutil::safe_new_array<uint8_t>(m_capacity);
  if (nullptr != m_buffer) {
    RINGBUF_Init(&m_ring, m_buffer, m_capacity);
  }
}

PipeStream::~PipeStream() {
  pdiutil::safe_delete_array(m_buffer);
}

/**
 * A pipe holds no connection of its own, so this always reports success.
 */
int16_t PipeStream::disconnect() {
  return 0;
}

/**
 * Usable for as long as it holds a buffer.
 */
int8_t PipeStream::connected() {
  return isValid() ? (int8_t)1 : (int8_t)0;
}

/**
 * Appends one byte, marking the pipe overflowed when it will not fit.
 */
int32_t PipeStream::write(uint8_t c) {

  if (!isValid()) {
    return 0;
  }

  if (0 != RINGBUF_Put(&m_ring, c)) {
    m_overflowed = true;
    return 0;
  }
  return 1;
}

/**
 * Appends a terminated string, returning how much of it fitted.
 */
int32_t PipeStream::write(const uint8_t *c_str) {

  if (nullptr == c_str) {
    return 0;
  }

  uint32_t len = 0;
  while (c_str[len] != '\0') len++;

  return write(c_str, len);
}

/**
 * Appends a bounded run of bytes, returning how much of it fitted.
 */
int32_t PipeStream::write(const uint8_t *c_str, uint32_t size) {

  if (nullptr == c_str || !isValid()) {
    return 0;
  }

  uint32_t written = 0;
  while (written < size) {
    if (0 != RINGBUF_Put(&m_ring, c_str[written])) {
      m_overflowed = true;
      break;
    }
    written++;
  }
  return (int32_t)written;
}

/**
 * Appends a read only string, copied through a window so the source is
 * never addressed directly.
 */
int32_t PipeStream::write_ro(const char *c_str) {

  if (nullptr == c_str || !isValid()) {
    return 0;
  }

  uint32_t len = strlen_ro(c_str);
  uint32_t done = 0;
  char window[RO_WINDOW];

  while (done < len) {
    uint32_t chunk = len - done;
    if (chunk > RO_WINDOW) chunk = RO_WINDOW;

    memcpy_ro(window, c_str + done, chunk);

    int32_t put = write((const uint8_t *)window, chunk);
    done += (uint32_t)put;

    if ((uint32_t)put < chunk) {
      break;
    }
  }

  return (int32_t)done;
}

/**
 * Takes the oldest byte, or zero when nothing is buffered.
 */
uint8_t PipeStream::read() {

  uint8_t c = 0;
  if (isValid()) {
    RINGBUF_Get(&m_ring, &c);
  }
  return c;
}

/**
 * Takes up to size bytes, returning how many came out.
 */
int32_t PipeStream::read(uint8_t *buf, uint32_t size) {

  if (nullptr == buf || !isValid()) {
    return 0;
  }

  uint32_t got = 0;
  while (got < size) {
    if (0 != RINGBUF_Get(&m_ring, &buf[got])) {
      break;
    }
    got++;
  }
  return (int32_t)got;
}

/**
 * Bytes written into the pipe and not yet read back.
 */
int32_t PipeStream::available() {
  return isValid() ? (int32_t)m_ring.fill_cnt : 0;
}

/**
 * Whether size further bytes would fit without overflowing.
 */
bool PipeStream::availableforwrite(uint32_t size) {
  return isValid() && ((m_capacity - m_ring.fill_cnt) >= size);
}

/**
 * A receive flush discards whatever has not been read; there is nothing
 * buffered on the way out.
 */
void PipeStream::flush(int16_t flushtype) {

  if (IsFlushRx(flushtype) && isValid()) {
    uint8_t c = 0;
    while (0 == RINGBUF_Get(&m_ring, &c)) {}
  }
}

#endif
