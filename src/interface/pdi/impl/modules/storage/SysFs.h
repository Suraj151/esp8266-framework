/********************************** SysFS **************************************
This file is part of the PDI Stack.

This is free software. You can redistribute it and/or modify it but without any
warranty.

SysFS is a synthetic filesystem mounted at /sys. It exposes the device surfaces
as a tree of small nodes, and unlike the rest of the synthetic filesystems some
of its leaves are writable.

Author          : Suraj I.
Created Date    : 23rd July 2026
******************************************************************************/

#ifndef _SYS_FS_H
#define _SYS_FS_H

#include <config/Config.h>

#ifdef ENABLE_SYSFS

#include "SynthFs.h"

class SysFs : public SynthFs {
public:
  SysFs();
  virtual ~SysFs() {}

  int writeFile(const char *path, const char *content, uint32_t size,
                bool append = false) override;

protected:
  synth_node_t resolve(const char *path) override;
  pdiutil::string render(const char *path) override;
  int listChildren(const char *path,
                   pdiutil::vector<file_info_t> &items) override;
  uint16_t permsFor(const char *path, synth_node_t kind) override;

private:
  enum NodeKind : uint8_t {
    SYS_INVALID = 0,
    SYS_ROOT,
    SYS_CLASS,
    SYS_GPIODIR,
    SYS_PIN,
    SYS_VALUE,
    SYS_MODE,
    SYS_NETDIR,
    SYS_NETIF,
    SYS_NETATTR
  };

  /**
   * Which node the path names, plus the pin or interface index behind it and
   * which of that node's leaves it is.
   */
  NodeKind classify(const char *path, int16_t &index_out,
                    uint8_t &leaf_out) const;

  /**
   * Whether the pin exists on this board and is safe to drive.
   */
  bool isValidPin(uint8_t pin) const;

  /**
   * What one leaf of a registered network interface currently reads.
   */
  pdiutil::string renderNetAttr(uint8_t index, uint8_t leaf);
};

extern SysFs __i_sysfs;

#endif // ENABLE_SYSFS

#endif
