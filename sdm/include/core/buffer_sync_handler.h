/*
* Copyright (c) 2015, 2019-2020, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*  * Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above
*    copyright notice, this list of conditions and the following
*    disclaimer in the documentation and/or other materials provided
*    with the distribution.
*  * Neither the name of The Linux Foundation nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*! @file buffer_sync_handler.h
  @brief Interface file for platform specific buffer allocator.

  @details SDM will use this interface to wait for buffer sync fd to be signaled/merge
  the two buffer sync fds into one.
*/

#ifndef __BUFFER_SYNC_HANDLER_H__
#define __BUFFER_SYNC_HANDLER_H__

#include <sstream>

namespace sdm {

/*! @brief Buffer sync handler implemented by the client

  @details This class declares prototype for BufferSyncHandler methods which must be
  implemented by the client. SDM will use these methods to wait for buffer sync fd to be
  signaled/merge two buffer sync fds into one.

  @sa CoreInterface::CreateCore
*/
class BufferSyncHandler {
 public:
  /*! @brief Method to wait specified time for ouput buffer to be released.

    @details This method waits for fd to be signaled by the producer/consumer with a specified
    timeout in milliseconds. It is the responsibility of the caller to close the file descriptor.

    @param[in] fd file descriptor

    @return \link int \endlink
  */

  virtual int SyncWait(int fd, int timeout) = 0;

  /*! @brief Method to merge two sync fds into one sync fd

    @details This method merges two buffer sync fds into one sync fd, if a producer/consumer
    requires to wait for more than one sync fds. It is responsibility of the caller to close file
    descriptor.

    @param[in] fd1 file descriptor 1
    @param[in] fd2 file descriptor 2
    @param[out] merged_fd

    @return \link int \endlink
 */

  virtual int SyncMerge(int fd1, int fd2, int *merged_fd) = 0;

  /*! @brief Method to get fence info associated to given file descriptor

    @details This method writes fence info such as driver name, status etc associated to given
    file descriptor into output stream.

    @param[in] fd file descriptor
    @param[in] os output stream
 */
  virtual void GetSyncInfo(int fd, std::ostringstream *os) = 0;

 protected:
  virtual ~BufferSyncHandler() { }
};

}  // namespace sdm

#endif  // __BUFFER_SYNC_HANDLER_H__
