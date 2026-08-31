/**************************** File Read Stream ********************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

A file that presents itself as a terminal to read from, so an input redirect can
hand a command somewhere to take its input. Bytes are fetched a block at a time
rather than held whole, so a file larger than the free heap still reads.

Author          : Suraj I.
created Date    : 26th Aug 2026
******************************************************************************/
#ifndef _FILE_READ_STREAM_H_
#define _FILE_READ_STREAM_H_

#include <config/Config.h>
#include <utility/iIOInterface.h>

#if defined(ENABLE_CMD_SERVICE) && defined(ENABLE_STORAGE_SERVICE)

class FileReadStream : public iTerminalInterface {

public:

  /**
   * Opens the named file for reading. An absent file leaves the stream
   * invalid rather than empty, so the caller can tell the two apart.
   */
  FileReadStream(const char *path);
  virtual ~FileReadStream();

  /**
   * False when the file was missing or there was no room for a buffer, and
   * every read is refused.
   */
  bool isValid() const { return nullptr != m_buffer && m_size >= 0; }

  /**
   * Bytes taken from the file so far, buffered ones included.
   */
  uint32_t consumed() const { return m_consumed; }

  /**
   * Nothing to disconnect, so this only reports the stream is usable.
   */
  int16_t disconnect() override;

  /**
   * Usable while it holds a buffer and the file was found.
   */
  int8_t connected() override;

  using iTerminalInterface::write;
  using iTerminalInterface::read;

  /**
   * Read only, so writes are discarded.
   */
  int32_t write(uint8_t c) override;

  /**
   * Read only, so writes are discarded.
   */
  int32_t write(const uint8_t *c_str) override;

  /**
   * Read only, so writes are discarded.
   */
  int32_t write(const uint8_t *c_str, uint32_t size) override;

  /**
   * Read only, so writes are discarded.
   */
  int32_t write_ro(const char *c_str) override;

  /**
   * Takes one byte, or zero once the file is spent.
   */
  uint8_t read() override;

  /**
   * Takes up to size bytes, refilling from the file as the buffer empties.
   */
  int32_t read(uint8_t *buf, uint32_t size) override;

  /**
   * What is left, buffered bytes plus whatever the file still holds.
   */
  int32_t available() override;

  /**
   * Read only, so there is never room to write.
   */
  bool availableforwrite(uint32_t size) override;

  /**
   * A receive flush abandons what is left rather than reading it.
   */
  void flush(int16_t flushtype = FLUSH_TX) override;

private:

  /**
   * Pulls the next block out of the file. False when nothing came back.
   */
  bool refill();

  pdiutil::string m_path;
  uint8_t *m_buffer;
  uint16_t m_fill;
  uint16_t m_take;
  uint64_t m_offset;
  int64_t m_size;
  uint32_t m_consumed;

  // Held open for the life of the stream when the backend has handles, so a
  // block costs one read instead of an open, a seek and a close. Negative when
  // the backend has none and the path calls are used instead.
  pdi_fhandle_t m_handle;
};

#endif

#endif
