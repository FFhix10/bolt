// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "kernel_def.h"
#define MANGLE_NAME_IMPL(base, FM, IT, OT) base##FM##IT##OT
#define MANGLE_NAME(base, FM, IT, OT) MANGLE_NAME_IMPL(base, FM, IT, OT)

#if defined(INPUT_F16)
#define IT f16_to_
#elif defined(INPUT_I32)
#define IT i32_to_
#endif

#if defined(OUTPUT_F16)
#define OT f16
#elif defined(OUTPUT_I32)
#define OT i32
#endif

#define FM
#if defined(USE_NCHW)
#define FM nchw_
#endif

__kernel void MANGLE_NAME(cast_, FM, IT, OT)(const int w,
    const int iw_str,
    const int ih_str,
    const int i_off,
    const int ow_str,
    const int oh_str,
    const int o_off,
    const int bx,
    const int by,
#if defined(INPUT_F16)
    __global T *in,
#elif defined(INPUT_I32)
    __global int *in,
#endif
#if defined(OUTPUT_F16)
    __global T *out
#elif defined(OUTPUT_I32)
    __global int *out
#endif
)
{
    int idx = get_global_id(0);
    int idy = get_global_id(1);
    int idz = get_global_id(2);
    if (idx >= bx || idy >= by) {
        return;
    }
#if defined(INPUT_F16)
    T4 iv = 0;
#elif defined(INPUT_I32)
    int4 iv = 0;
#endif

#if defined(OUTPUT_F16)
    T4 ov = 0;
#elif defined(OUTPUT_I32)
    int4 ov = 0;
#endif

#if defined(USE_NCHW)
    LOAD_MEM_V4_C1_COMMON(iv, idx, idy, idz, iw_str, ih_str, i_off, w, in);
#else
    LOAD_MEM_V4_COMMON(iv, idx, idy, idz, iw_str, ih_str, i_off, in);
#endif
    ov.x = iv.x;
    ov.y = iv.y;
    ov.z = iv.z;
    ov.w = iv.w;
#if defined(USE_NCHW)
    STORE_MEM_V4_C1_COMMON(ov, idx, idy, idz, ow_str, oh_str, o_off, w, out);
#else
    STORE_MEM_V4_COMMON(ov, idx, idy, idz, ow_str, oh_str, o_off, out);
#endif
}
