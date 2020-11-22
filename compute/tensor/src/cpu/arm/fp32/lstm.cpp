// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <string.h>
#include "cpu/arm/fp32/tensor_computing_fp32.h"

void mvm_nkn32(U32 fn, U32 fk, const F32 *filterArray, F32 *input, F32 *output)
{
#ifdef _USE_OPENMP
#pragma omp parallel for num_threads(OMP_NUM_THREADS)
#endif
    for (U32 n = 0; n < fn; n++) {
        F32 *in = input;
        const F32 *f = filterArray + n * fk * 32;
        F32 *out = output + n * 32;
#ifdef __aarch64__
        __asm__ __volatile__("ldr d0, [%[in]]\n"
                             "ldr  q1, [%[out]]\n"
                             "ldr  q2, [%[out], #16]\n"
                             "ldr  q3, [%[out], #32]\n"
                             "ldr  q4, [%[out], #48]\n"
                             "ldr q13, [%[out], #64]\n"
                             "ldr q14, [%[out], #80]\n"
                             "ldr q15, [%[out], #96]\n"
                             "ldr q16, [%[out], #112]\n"
                             "mov x0, %[k]\n"
                             "ldr  q5, [%[f]]\n"
                             "ldr  q6, [%[f], #16]\n"
                             "ldr  q7, [%[f], #32]\n"
                             "ldr  q8, [%[f], #48]\n"
                             "ldr q17, [%[f], #64]\n"
                             "ldr q18, [%[f], #80]\n"
                             "ldr q19, [%[f], #96]\n"
                             "ldr q20, [%[f], #112]\n"
                             "0:\n"
                             "prfm pldl2strm, [%[f], #4096]\n"
                             "prfm pldl1strm, [%[f], #1024]\n"
                             "ldr d9, [%[f], #128]\n"
                             "fmla v1.4s, v5.4s, v0.s[0]\n"
                             "ldr x9, [%[f], #136]\n"
                             "ins v9.d[1], x9\n"
                             "ldr d10, [%[f], #144]\n"
                             "fmla v2.4s, v6.4s, v0.s[0]\n"
                             "ldr x10, [%[f], #152]\n"
                             "ins v10.d[1], x10\n"
                             "ldr d11, [%[f], #160]\n"
                             "fmla v3.4s, v7.4s, v0.s[0]\n"
                             "ldr x11, [%[f], #168]\n"
                             "ins v11.d[1], x11\n"
                             "ldr d12, [%[f], #176]\n"
                             "fmla v4.4s, v8.4s, v0.s[0]\n"
                             "ldr x12, [%[f], #184]\n"
                             "ins v12.d[1], x12\n"
                             "ldr d21, [%[f], #192]\n"
                             "fmla v13.4s, v17.4s, v0.s[0]\n"
                             "ldr x9, [%[f], #200]\n"
                             "ins v21.d[1], x9\n"
                             "ldr d22, [%[f], #208]\n"
                             "fmla v14.4s, v18.4s, v0.s[0]\n"
                             "ldr x10, [%[f], #216]\n"
                             "ins v22.d[1], x10\n"
                             "ldr d23, [%[f], #224]\n"
                             "fmla v15.4s, v19.4s, v0.s[0]\n"
                             "ldr x11, [%[f], #232]\n"
                             "ins v23.d[1], x11\n"
                             "ldr d24, [%[f], #240]\n"
                             "fmla v16.4s, v20.4s, v0.s[0]\n"
                             "ldr x12, [%[f], #248]\n"
                             "ins v24.d[1], x12\n"

                             "add %[f], %[f], #256\n"
                             "ldr d5, [%[f]]\n"
                             "fmla v1.4s, v9.4s, v0.s[1]\n"
                             "ldr x5, [%[f], #8]\n"
                             "ins v5.d[1], x5\n"
                             "ldr d6, [%[f], #16]\n"
                             "fmla v2.4s, v10.4s, v0.s[1]\n"
                             "ldr x6, [%[f], #24]\n"
                             "ins v6.d[1], x6\n"
                             "ldr d7, [%[f], #32]\n"
                             "fmla v3.4s, v11.4s, v0.s[1]\n"
                             "ldr x7, [%[f], #40]\n"
                             "ins v7.d[1], x7\n"
                             "ldr d8, [%[f], #48]\n"
                             "fmla v4.4s, v12.4s, v0.s[1]\n"
                             "ldr x8, [%[f], #56]\n"
                             "ins v8.d[1], x8\n"
                             "ldr d17, [%[f], #64]\n"
                             "fmla v13.4s, v21.4s, v0.s[1]\n"
                             "ldr x5, [%[f], #72]\n"
                             "ins v17.d[1], x5\n"
                             "ldr d18, [%[f], #80]\n"
                             "fmla v14.4s, v22.4s, v0.s[1]\n"
                             "ldr x6, [%[f], #88]\n"
                             "ins v18.d[1], x6\n"
                             "ldr d19, [%[f], #96]\n"
                             "fmla v15.4s, v23.4s, v0.s[1]\n"
                             "ldr x7, [%[f], #104]\n"
                             "ins v19.d[1], x7\n"
                             "ldr d20, [%[f], #112]\n"
                             "fmla v16.4s, v24.4s, v0.s[1]\n"
                             "ldr x8, [%[f], #120]\n"
                             "add %[in], %[in], #8\n"
                             "ins v20.d[1], x8\n"

                             "ldr d0, [%[in]]\n"
                             "sub x0, x0, #2\n"

                             "cmp x0, #3\n"
                             "bgt 0b\n"
                             "ldr  q9, [%[f], #128]\n"
                             "ldr q10, [%[f], #144]\n"
                             "ldr q11, [%[f], #160]\n"
                             "ldr q12, [%[f], #176]\n"
                             "ldr q21, [%[f], #192]\n"
                             "ldr q22, [%[f], #208]\n"
                             "ldr q23, [%[f], #224]\n"
                             "ldr q24, [%[f], #240]\n"
                             "fmla  v1.4s,  v5.4s, v0.s[0]\n"
                             "fmla  v2.4s,  v6.4s, v0.s[0]\n"
                             "fmla  v3.4s,  v7.4s, v0.s[0]\n"
                             "fmla  v4.4s,  v8.4s, v0.s[0]\n"
                             "fmla v13.4s,  v17.4s, v0.s[0]\n"
                             "fmla v14.4s,  v18.4s, v0.s[0]\n"
                             "fmla v15.4s,  v19.4s, v0.s[0]\n"
                             "fmla v16.4s,  v20.4s, v0.s[0]\n"
                             "fmla  v1.4s,  v9.4s, v0.s[1]\n"
                             "fmla  v2.4s, v10.4s, v0.s[1]\n"
                             "fmla  v3.4s, v11.4s, v0.s[1]\n"
                             "fmla  v4.4s, v12.4s, v0.s[1]\n"
                             "fmla v13.4s, v21.4s, v0.s[1]\n"
                             "fmla v14.4s, v22.4s, v0.s[1]\n"
                             "fmla v15.4s, v23.4s, v0.s[1]\n"
                             "fmla v16.4s, v24.4s, v0.s[1]\n"
                             "cmp x0, #3\n"
                             "bne 1f\n"
                             "add %[f], %[f], #256\n"
                             "ldr s0, [%[in], #8]\n"
                             "ldr  q5, [%[f]]\n"
                             "ldr  q6, [%[f], #16]\n"
                             "ldr  q7, [%[f], #32]\n"
                             "ldr  q8, [%[f], #48]\n"
                             "ldr q17, [%[f], #64]\n"
                             "ldr q18, [%[f], #80]\n"
                             "ldr q19, [%[f], #96]\n"
                             "ldr q20, [%[f], #112]\n"
                             "fmla v1.4s,  v5.4s, v0.s[0]\n"
                             "fmla v2.4s,  v6.4s, v0.s[0]\n"
                             "fmla v3.4s,  v7.4s, v0.s[0]\n"
                             "fmla v4.4s,  v8.4s, v0.s[0]\n"
                             "fmla v13.4s,  v17.4s, v0.s[0]\n"
                             "fmla v14.4s,  v18.4s, v0.s[0]\n"
                             "fmla v15.4s,  v19.4s, v0.s[0]\n"
                             "fmla v16.4s,  v20.4s, v0.s[0]\n"

                             "1:\n"
                             "str  q1, [%[out]]\n"
                             "str  q2, [%[out], #16]\n"
                             "str  q3, [%[out], #32]\n"
                             "str  q4, [%[out], #48]\n"
                             "str q13, [%[out], #64]\n"
                             "str q14, [%[out], #80]\n"
                             "str q15, [%[out], #96]\n"
                             "str q16, [%[out], #112]\n"
                             : [out] "+r"(out), [f] "+r"(f), [in] "+r"(in)
                             : [k] "r"((I64)fk)
                             : "memory", "cc", "x0", "x5", "x6", "x7", "x8", "x9", "x10", "x11",
                             "x12", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9",
                             "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19",
                             "v20", "v21", "v22", "v23", "v24");
#else
        __asm__ __volatile__("vld1.f32 {d0[0]}, [%[in]]!\n"
                             "mov r2, %[out]\n"
                             "mov r3, %[out]\n"
                             "vld1.f32   {d2-d3}, [r2]!\n"
                             "vld1.f32   {d4-d5}, [r2]!\n"
                             "vld1.f32   {d6-d7}, [r2]!\n"
                             "vld1.f32   {d8-d9}, [r2]!\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"
                             "vld1.f32 {d22-d23}, [%[f]]!\n"
                             "vld1.f32 {d24-d25}, [%[f]]!\n"
                             "mov r4, %[k]\n"
                             "vld1.f32 {d10-d11}, [r2]!\n"
                             "vld1.f32 {d12-d13}, [r2]!\n"
                             "vld1.f32 {d14-d15}, [r2]!\n"
                             "vld1.f32 {d16-d17}, [r2]\n"
                             "vld1.f32 {d26-d27}, [%[f]]!\n"
                             "vld1.f32 {d28-d29}, [%[f]]!\n"
                             "0:\n"
                             "cmp r4, #3\n"
                             "ble 3f\n"
                             "pld [%[f], #374]\n"
                             "vld1.f32 {d30[0]}, [%[in]]!\n"
                             "vmla.f32 q1,  q9, d0[0]\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vmla.f32 q2, q10, d0[0]\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"
                             "vmla.f32 q3, q11, d0[0]\n"
                             "vld1.f32 {d22-d23}, [%[f]]!\n"
                             "vmla.f32 q4, q12, d0[0]\n"
                             "vld1.f32 {d24-d25}, [%[f]]!\n"
                             "vmla.f32 q5, q13, d0[0]\n"
                             "vld1.f32 {d26-d27}, [%[f]]!\n"
                             "vmla.f32 q6, q14, d0[0]\n"
                             "vld1.f32 {d28-d29}, [%[f]]!\n"
                             "vmla.f32 q7,  q9, d0[0]\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vmla.f32 q8, q10, d0[0]\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"

                             "pld [%[f], #374]\n"
                             "vmov.f32 q0, q15\n"
                             "vld1.f32 {d30[0]}, [%[in]]!\n"
                             "vmla.f32 q1, q11, d0[0]\n"
                             "vld1.f32 {d22-d23}, [%[f]]!\n"
                             "vmla.f32 q2, q12, d0[0]\n"
                             "vld1.f32 {d24-d25}, [%[f]]!\n"
                             "vmla.f32 q3, q13, d0[0]\n"
                             "vld1.f32 {d26-d27}, [%[f]]!\n"
                             "vmla.f32 q4, q14, d0[0]\n"
                             "vld1.f32 {d28-d29}, [%[f]]!\n"
                             "vmla.f32 q5,  q9, d0[0]\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vmla.f32 q6, q10, d0[0]\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"
                             "vmla.f32 q7, q11, d0[0]\n"
                             "vld1.f32 {d22-d23}, [%[f]]!\n"
                             "vmla.f32 q8, q12, d0[0]\n"
                             "vld1.f32 {d24-d25}, [%[f]]!\n"

                             "sub r4, r4, #3\n"

                             "pld [%[f], #374]\n"
                             "vmov.f32 q0, q15\n"
                             "vld1.f32 {d30[0]}, [%[in]]!\n"
                             "vmla.f32 q1, q13, d0[0]\n"
                             "vld1.f32 {d26-d27}, [%[f]]!\n"
                             "vmla.f32 q2, q14, d0[0]\n"
                             "vld1.f32 {d28-d29}, [%[f]]!\n"
                             "vmla.f32 q3,  q9, d0[0]\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vmla.f32 q4, q10, d0[0]\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"
                             "vmla.f32 q5, q11, d0[0]\n"
                             "vld1.f32 {d22-d23}, [%[f]]!\n"
                             "vmla.f32 q6, q12, d0[0]\n"
                             "vld1.f32 {d24-d25}, [%[f]]!\n"
                             "vmla.f32 q7, q13, d0[0]\n"
                             "vld1.f32 {d26-d27}, [%[f]]!\n"
                             "vmla.f32 q8, q14, d0[0]\n"
                             "vld1.f32 {d28-d29}, [%[f]]!\n"
                             "vmov.f32 q0, q15\n"
                             "b 0b\n"
                             "3:\n"
                             "sub r4, r4, #1\n"
                             "vmla.f32 q1,  q9, d0[0]\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vmla.f32 q2, q10, d0[0]\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"
                             "vmla.f32 q3, q11, d0[0]\n"
                             "vmla.f32 q4, q12, d0[0]\n"
                             "vmla.f32 q5, q13, d0[0]\n"
                             "vmla.f32 q6, q14, d0[0]\n"
                             "vmla.f32 q7,  q9, d0[0]\n"
                             "vmla.f32 q8, q10, d0[0]\n"

                             "1:\n"
                             "cmp r4, #0\n"
                             "beq 2f\n"
                             "sub r4, r4, #1\n"
                             "vld1.f32 {d0[0]}, [%[in]]!\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"
                             "vld1.f32 {d22-d23}, [%[f]]!\n"
                             "vld1.f32 {d24-d25}, [%[f]]!\n"
                             "vmla.f32 q1,  q9, d0[0]\n"
                             "vld1.f32 {d18-d19}, [%[f]]!\n"
                             "vmla.f32 q2, q10, d0[0]\n"
                             "vld1.f32 {d20-d21}, [%[f]]!\n"
                             "vmla.f32 q3, q11, d0[0]\n"
                             "vld1.f32 {d22-d23}, [%[f]]!\n"
                             "vmla.f32 q4, q12, d0[0]\n"
                             "vld1.f32 {d24-d25}, [%[f]]!\n"
                             "vmla.f32 q5,  q9, d0[0]\n"
                             "vmla.f32 q6, q10, d0[0]\n"
                             "vmla.f32 q7, q11, d0[0]\n"
                             "vmla.f32 q8, q12, d0[0]\n"
                             "b 1b\n"

                             "2:\n"
                             "vst1.f32   {d2-d3}, [r3]!\n"
                             "vst1.f32   {d4-d5}, [r3]!\n"
                             "vst1.f32   {d6-d7}, [r3]!\n"
                             "vst1.f32   {d8-d9}, [r3]!\n"
                             "vst1.f32 {d10-d11}, [r3]!\n"
                             "vst1.f32 {d12-d13}, [r3]!\n"
                             "vst1.f32 {d14-d15}, [r3]!\n"
                             "vst1.f32 {d16-d17}, [r3]\n"
                             : [f] "+r"(f), [in] "+r"(in)
                             : [k] "r"(fk), [out] "r"(out)
                             : "memory", "cc", "r2", "r3", "r4", "q0", "q1", "q2", "q3", "q4", "q5",
                             "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15");
#endif
    }
}

EE rnncell_fp32(TensorDesc xDesc,
    const void *currentX,
    const TensorDesc *filterDesc,
    const void **filter,
    const TensorDesc *biasDesc,
    const void **bias,
    void *state,
    U32 tmpBytes,
    void *tmp,
    RNNParamSpec rnnParamSpec,
    U32 batchStrideX,
    U32 batchStrideH,
    TensorDesc hDesc,
    void *output,
    Arch arch)
{
    UNUSED(biasDesc);
    UNUSED(tmpBytes);
    UNUSED(arch);
    if (nullptr == currentX || nullptr == filter || nullptr == bias || nullptr == state ||
        nullptr == tmp || nullptr == output) {
        CHECK_STATUS(NULL_POINTER);
    }

    DataType idt, fdt, odt;
    DataFormat idf, fdf, odf;
    U32 in, ix;
    U32 on, oh;
    U32 fk, fn;
    CHECK_STATUS(tensor2dGet(xDesc, &idt, &idf, &in, &ix));
    CHECK_STATUS(tensor2dGet(filterDesc[0], &fdt, &fdf, &fn, &fk));
    CHECK_STATUS(tensor2dGet(hDesc, &odt, &odf, &on, &oh));
    if (fdf != DF_NKN32) {
        CHECK_STATUS(NOT_MATCH);
    }
    fn /= 32;

    U32 batch = in;
    I32 xDim = ix;
    I32 hDim = rnnParamSpec.numOutput;
    I32 column = (rnnParamSpec.numProjection > 0) ? rnnParamSpec.numProjection
                                                  : rnnParamSpec.numOutput;
    if (!(idt == DT_F32 && fdt == DT_F32 && odt == DT_F32)) {
        CHECK_STATUS(NOT_MATCH);
    }
    if (!(4 * column == (I32)fn * 32 && (ix + oh) == fk && in == on)) {
        CHECK_STATUS(NOT_MATCH);
    }
    F32 forgetBias = rnnParamSpec.forgetBias;
    ActivationMode activationMode = rnnParamSpec.activationMode;
    if (activationMode != ACTIVATION_TANH) {
        CHECK_STATUS(NOT_SUPPORTED);
    }

    const F32 *currentXArray = (const F32 *)currentX;
    F32 *lastStateArray = (F32 *)state;
    F32 *lastHArray = lastStateArray + column;
    F32 *tmpArray = (F32 *)tmp;
    F32 *currentStateArray = (F32 *)state;
    F32 *currentHArray = currentStateArray + column;
    F32 *outputArray = (F32 *)output;
    F32 *xhArray = tmpArray;
    F32 *intermediateH = xhArray + (xDim + hDim);
    U32 lastStateStride = column + hDim;
    U32 lastHStride = column + hDim;
    U32 currentStateStride = column + hDim;
    U32 currentHStride = column + hDim;
    float32x4_t forgetBiasVector = vdupq_n_f32(forgetBias);
    for (U32 m = 0; m < batch; m++) {
        F32 *lastBatchH = lastHArray + m * lastHStride;
        memcpy(xhArray, currentXArray + m * batchStrideX, xDim * sizeof(F32));
        memcpy(xhArray + xDim, lastBatchH, hDim * sizeof(F32));

        memcpy(intermediateH, bias[0], column * 4 * sizeof(F32));
        mvm_nkn32(fn, fk, (const F32 *)filter[0], xhArray, intermediateH);
        F32 *out_i = intermediateH;
        F32 *out_g = out_i + column;
        F32 *out_f = out_i + column * 2;
        F32 *out_o = out_i + column * 3;

        F32 *lastBatchState = lastStateArray + m * lastStateStride;
        F32 *currentBatchState = currentStateArray + m * currentStateStride;
        F32 *currentBatchH = currentHArray + m * currentHStride;
        F32 *currentOutput = outputArray + m * batchStrideH;

        F32 *tmpState, *tmpHH, *tmpH;
        if (rnnParamSpec.zoneoutCell == 0) {
            tmpState = currentBatchState;
        } else {
            tmpState = out_i;
        }
        if (rnnParamSpec.numProjection > 0) {
            tmpHH = out_g;
            tmpH = currentOutput;
        } else {
            tmpHH = currentOutput;
            tmpH = out_g;
        }

        I32 h = 0;
        for (; h < column - 3; h += 4) {
            float32x4_t out_i_v = vld1q_f32(out_i + h);
            float32x4_t out_g_v = vld1q_f32(out_g + h);
            float32x4_t out_f_v = vld1q_f32(out_f + h);
            float32x4_t out_o_v = vld1q_f32(out_o + h);
            float32x4_t C_v = vld1q_f32(lastBatchState + h);
            float32x4_t I_v = vsigmoidq_f32(out_i_v);
            float32x4_t F_v = vsigmoidq_f32(vaddq_f32(out_f_v, forgetBiasVector));
            float32x4_t O_v = vsigmoidq_f32(out_o_v);
            float32x4_t G_v = vtanhq_f32(out_g_v);
            C_v = vaddq_f32(vmulq_f32(C_v, F_v), vmulq_f32(I_v, G_v));
            float32x4_t out_hidden_v = vmulq_f32(O_v, vtanhq_f32(C_v));
            vst1q_f32(tmpState + h, C_v);
            vst1q_f32(tmpHH + h, out_hidden_v);
        }
        for (; h < column; h++) {
            F32 C_s = lastBatchState[h];
            F32 I_s = 1.0 / (1.0 + exp(-out_i[h]));
            F32 F_s = 1.0 / (1.0 + exp(-(out_f[h] + forgetBias)));
            F32 O_s = 1.0 / (1.0 + exp(-out_o[h]));
            F32 G_s = tanh(out_g[h]);
            C_s = C_s * F_s + I_s * G_s;
            F32 value = O_s * tanh(C_s);
            tmpState[h] = C_s;
            tmpHH[h] = value;
        }
        if (rnnParamSpec.zoneoutCell != 0) {
            array_scale_f32(tmpState, tmpState, column, 1 - rnnParamSpec.zoneoutCell, 0);
            array_scale_f32(lastBatchState, lastBatchState, column, rnnParamSpec.zoneoutCell, 0);
            array_add_f32(tmpState, lastBatchState, currentBatchState, column);
        }

        if (rnnParamSpec.numProjection > 0) {
            memset(tmpH, 0, sizeof(F32) * hDim);
            mvm_nkn32(hDim / 32, rnnParamSpec.numProjection, (const F32 *)filter[1], tmpHH, tmpH);
        }
        if (rnnParamSpec.zoneoutOutput != 0) {
            if (rnnParamSpec.numProjection > 0) {
                array_scale_f32(tmpH, out_f, hDim, 1 - rnnParamSpec.zoneoutOutput, 0);
            } else {
                array_scale_f32(tmpHH, out_f, hDim, 1 - rnnParamSpec.zoneoutOutput, 0);
            }
            array_scale_f32(lastBatchH, lastBatchH, hDim, rnnParamSpec.zoneoutOutput, 0);
            array_add_f32(out_f, lastBatchH, currentBatchH, hDim);
        } else {
            memcpy(currentBatchH, currentOutput, sizeof(F32) * hDim);
        }
    }
    return SUCCESS;
}
