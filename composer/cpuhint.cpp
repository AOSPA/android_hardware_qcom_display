/* Copyright (c) 2015, 2020-2021, The Linux Foundataion. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
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
*
*/

#include <cutils/properties.h>
#include <dlfcn.h>
#include <utils/debug.h>

#include "cpuhint.h"
#include "hwc_debugger.h"

#define __CLASS__ "CPUHint"

namespace sdm {

DisplayError CPUHint::Init(HWCDebugHandler *debug_handler) {
  char path[PROPERTY_VALUE_MAX];
  if (debug_handler->GetProperty("ro.vendor.extension_library", path) != kErrorNone) {
    DLOGI("Vendor Extension Library not enabled");
    return kErrorNotSupported;
  }

  if (vendor_ext_lib_.Open(path)) {
    if (!vendor_ext_lib_.Sym("perf_event_offload", reinterpret_cast<void **> \
        (&fn_perf_event_offload_))) {
      DLOGW("Failed to load symbols for Vendor Extension Library");
      return kErrorNotSupported;
    }
    DLOGI("Successfully Loaded Vendor Extension Library symbols");
    enabled_ = (fn_perf_event_offload_ != NULL);
  } else {
    DLOGW("Failed to open %s : %s", path, vendor_ext_lib_.Error());
  }

  return enabled_ ? kErrorNone : kErrorNotSupported;
}

void CPUHint::ReqHintsOffload(int hint, int duration) {
  if(enabled_ && hint > 0) {
    int args[] = {0, duration};
    int handle = fn_perf_event_offload_(hint, NULL, 2, args);
    if (handle < 0) {
      DLOGW("Failed to send hint 0x%x. handle = %d", hint, handle);
    }
  }
}
}  // namespace sdm
