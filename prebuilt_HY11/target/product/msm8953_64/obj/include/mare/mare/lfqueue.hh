// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
// Copyright 2013-2015 Qualcomm Technologies, Inc.
// All rights reserved.
// Confidential and Proprietary – Qualcomm Technologies, Inc.
// --~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~----~--~--~--~--
#pragma once

#include <mare/internal/queues/lfqueue.hh>
#include <mare/internal/util/macros.hh>

namespace mare {

template<typename T>
class lfqueue{
public:
  typedef typename internal::lfq::lfq<T> container_type;
  typedef T value_type;

  MARE_DELETE_METHOD(lfqueue(lfqueue const&));
  MARE_DELETE_METHOD(lfqueue(lfqueue &&));
  MARE_DELETE_METHOD(lfqueue& operator=(lfqueue const&));
  MARE_DELETE_METHOD(lfqueue& operator=(lfqueue &&));

  explicit lfqueue(size_t log_size): _c(log_size){}

  bool push(value_type const& v){return (_c.push(v) != 0 ? true:false);}

  bool pop(value_type& r){return (_c.pop(r) != 0 ? true:false);}

private:
  container_type _c;
};

};

