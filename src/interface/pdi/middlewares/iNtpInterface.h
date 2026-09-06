/************************ i network time Interface ****************************
This file is part of the pdi stack.

This is free software. you can redistribute it and/or modify it but without any
warranty.

Author          : Suraj I.
created Date    : 1st June 2019
******************************************************************************/

#ifndef _I_NTP_INTERFACE_H_
#define _I_NTP_INTERFACE_H_

#include <interface/interface_includes.h>

// forward declaration of derived class for this interface
class NtpInterface;

/**
 * iNtpInterface class
 */
class iNtpInterface
{

public:
  /**
   * iNtpInterface constructor.
   */
  iNtpInterface() {}
  /**
   * iNtpInterface destructor.
   */
  virtual ~iNtpInterface() {}

  virtual void init_ntp_time() = 0;
  virtual bool is_valid_ntptime() = 0;
  virtual pdiutil::epoch_time_t get_ntp_time() = 0;
  virtual bool set_ntp_time(pdiutil::epoch_time_t epoch) = 0;
};

// derived class must define this
extern NtpInterface __i_ntp;

#endif
