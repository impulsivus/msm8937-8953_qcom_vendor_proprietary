// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary – Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#ifdef MARE_HAVE_QTI_HEXAGON

#include <mare/internal/legacy/types.hh>
#include <mare/internal/util/mareptrs.hh>

namespace mare {

namespace internal {

MARE_GCC_IGNORE_BEGIN("-Weffc++");

class hexagondevice : public ref_counted_object<hexagondevice>
{
MARE_GCC_IGNORE_END("-Weffc++");

public:

  static void init();

  static void shutdown();

  static void* allocate(size_t size);

  static void deallocate(void* mem);

};

MARE_GCC_IGNORE_BEGIN("-Weffc++");
class hexagonbuffer : public ref_counted_object<hexagonbuffer>
{
MARE_GCC_IGNORE_END("-Weffc++");
protected:

  hexagondevice* _device;

  void* _host_ptr;

  size_t _size;

  bool _device_allocated;

  bool _fully_shared;

  MARE_DELETE_METHOD(hexagonbuffer(hexagonbuffer const&));
  MARE_DELETE_METHOD(hexagonbuffer(hexagonbuffer&&));
  MARE_DELETE_METHOD(hexagonbuffer& operator=(hexagonbuffer const&));
  MARE_DELETE_METHOD(hexagonbuffer& operator=(hexagonbuffer&&));
public:

  hexagonbuffer(hexagondevice* device, size_t size) :
    _device(device),
    _host_ptr(nullptr),
    _size(size),
    _device_allocated(true),
    _fully_shared(false) {
    _host_ptr = hexagondevice::allocate(size);
    MARE_API_THROW(_host_ptr != nullptr, "rpcmem_alloc failed");
  }

  hexagonbuffer(hexagondevice* device, size_t size, void* ptr) :
    _device(device),
    _host_ptr(ptr),
    _size(size),
    _device_allocated(false),
    _fully_shared(false) {
  }

  virtual ~hexagonbuffer() {

    copy_to_device();

    if (is_device_allocated() && _host_ptr != nullptr)
      hexagondevice::deallocate(_host_ptr);
  }

  void set_fully_shared() {_fully_shared = true;};

  bool is_fully_shared() {return _fully_shared;};

  bool is_device_allocated() {return _device_allocated;};

  void * get_host_ptr() {return _host_ptr;}

  virtual void copy_to_device() {

  }

  virtual void* copy_from_device() {

    return _host_ptr;
  }

  void release() {
    copy_from_device();
  }

};

inline legacy::hexagonbuffer_ptr
create_hexagon_buffer(void *ptr, size_t size)
{

  internal::hexagonbuffer* b_ptr = new internal::hexagonbuffer(nullptr, size, ptr);
  MARE_DLOG("Creating mare::internal::hexagonbuffer %p", b_ptr);
  return legacy::hexagonbuffer_ptr(b_ptr, legacy::hexagonbuffer_ptr::ref_policy::do_initial_ref);
}

  inline legacy::hexagonbuffer_ptr
create_hexagon_buffer(size_t size)
{

  internal::hexagonbuffer* b_ptr = new internal::hexagonbuffer(nullptr, size);
  MARE_DLOG("Creating mare::internal::hexagonbuffer %p", b_ptr);
  return legacy::hexagonbuffer_ptr(b_ptr, legacy::hexagonbuffer_ptr::ref_policy::do_initial_ref);
}
};

};

#endif
