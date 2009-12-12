// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GPU_COMMAND_BUFFER_SERVICE_COMMAND_BUFFER_SERVICE_H_
#define GPU_COMMAND_BUFFER_SERVICE_COMMAND_BUFFER_SERVICE_H_

#include <set>
#include <vector>

#include "base/linked_ptr.h"
#include "base/scoped_ptr.h"
#include "base/shared_memory.h"
#include "base/task.h"
#include "gpu/command_buffer/common/command_buffer.h"

namespace gpu {

// An object that implements a shared memory command buffer and a synchronous
// API to manage the put and get pointers.
class CommandBufferService : public CommandBuffer {
 public:
  CommandBufferService();
  virtual ~CommandBufferService();

  // CommandBuffer implementation:
  virtual base::SharedMemory* Initialize(int32 size);
  virtual base::SharedMemory* GetRingBuffer();
  virtual int32 GetSize();
  virtual int32 SyncOffsets(int32 put_offset);
  virtual int32 GetGetOffset();
  virtual void SetGetOffset(int32 get_offset);
  virtual int32 GetPutOffset();
  virtual void SetPutOffsetChangeCallback(Callback0::Type* callback);
  virtual int32 CreateTransferBuffer(size_t size);
  virtual void DestroyTransferBuffer(int32 id);
  virtual base::SharedMemory* GetTransferBuffer(int32 handle);
  virtual int32 GetToken();
  virtual void SetToken(int32 token);
  virtual int32 ResetParseError();
  virtual void SetParseError(int32 parse_error);
  virtual bool GetErrorStatus();
  virtual void RaiseErrorStatus();

 private:
  scoped_ptr< base::SharedMemory> ring_buffer_;
  int32 size_;
  int32 get_offset_;
  int32 put_offset_;
  scoped_ptr<Callback0::Type> put_offset_change_callback_;
  std::vector<linked_ptr< base::SharedMemory> > registered_objects_;
  std::set<int32> unused_registered_object_elements_;
  int32 token_;
  int32 parse_error_;
  bool error_status_;
};

}  // namespace gpu

#endif  // GPU_COMMAND_BUFFER_SERVICE_COMMAND_BUFFER_SERVICE_H_
