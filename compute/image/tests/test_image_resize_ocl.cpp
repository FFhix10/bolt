// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "image.h"
#include "tensor_computing.h"
#include "ut_util_ocl.h"

int resizeTest(int argc, char *argv[], DataType dt)
{
    CHECK_REQUIREMENT(argc == 9);
    // in data
    U32 in = atoi(argv[1]);
    U32 ic = atoi(argv[2]);
    U32 ih = atoi(argv[3]);
    U32 iw = atoi(argv[4]);
    // output
    U32 on = atoi(argv[5]);
    U32 oc = atoi(argv[6]);
    U32 oh = atoi(argv[7]);
    U32 ow = atoi(argv[8]);

    CHECK_REQUIREMENT(in == 1 && on == 1);

    ArchInfo archInfo;
    archInfo.arch = MALI;
    ArchInfo archInfo_org;
    archInfo_org.arch = CPU_GENERAL;

    TensorDesc inputDesc_cpu, inputDesc_gpu, outputDesc_cpu, outputDesc_gpu;
    inputDesc_cpu = tensor4df(dt, DF_NCHW, in, ic, ih, iw);
    inputDesc_gpu = tensor4df(dt, DF_NCHW, in, ic, ih, iw);

    DataType paramDT = DT_U32;
    U32 scales[2];
    scales[0] = oh;
    scales[1] = ow;

    // setup input
    U8 *input_cpu = ut_input_v(in * ic * ih * iw, dt, UT_INIT_RANDOM);

    Tensor inputTensorCpu;
    inputTensorCpu.resize(inputDesc_cpu);
    inputTensorCpu.alloc();
    memcpy(get_ptr_from_tensor(inputTensorCpu, UT_ARCH), input_cpu, tensorNumBytes(inputDesc_cpu));
    Tensor outputTensorCpu;
    Tensor tmpTensorCpu;
    U32 outputBytes;
    CHECK_STATUS(resize_infer_output_size(
        &inputTensorCpu, paramDT, scales, &outputTensorCpu, &outputBytes, &archInfo_org));
    outputTensorCpu.alloc();

    ResizeParamSpec p;
    //p.mode = LINEAR;
    p.mode = NEAREST;
    p.trans_mode = ASYMMETRIC;
    // CPU output
    CHECK_STATUS(resize(inputTensorCpu, tmpTensorCpu, outputTensorCpu, p, &archInfo_org));
    std::shared_ptr<GCLHandle> handleSharedPtr = OCLContext::getInstance().handle;

    GCLHandle_t handle = handleSharedPtr.get();
    std::vector<GCLKernelInfo> kernelVec;
    handle->kernelVec = &kernelVec;
    Tensor inputTensor = Tensor(OCLMem);
    Tensor outputTensor = Tensor(OCLMem);
    Tensor tmpTensor = Tensor(OCLMem);
    inputTensor.resize(inputDesc_gpu);

    MaliPara maliPara;
    maliPara.handle = handle;
    archInfo.archPara = &maliPara;

    CHECK_STATUS(resize_infer_output_size(
        &inputTensor, paramDT, scales, &outputTensor, &outputBytes, &archInfo));
    U32 maxBytes = 0;
    U32 tmpBytes = 0;

    GCLMem_t output = alloc(outputTensor);
    GCLMem_t input = alloc(inputTensor);
    CHECK_STATUS(gcl_fill_memory_zero(handle, input));
    outputDesc_gpu = outputTensor.get_desc();
    U8 *output_gpu = ut_input_v(on * oc * oh * ow, dt, UT_INIT_RANDOM);
    tmpBytes = tensorNumBytes(inputDesc_gpu);
    maxBytes = (tmpBytes > maxBytes) ? tmpBytes : maxBytes;
    tmpBytes = tensorNumBytes(outputDesc_gpu);
    maxBytes = (tmpBytes > maxBytes) ? tmpBytes : maxBytes;
    GCLMem_t tmpbuf = alloc_bytes(tmpTensor, maxBytes);
    CHECK_STATUS(ocl_set_input(handle, input, inputDesc_gpu, input_cpu, tmpbuf, true));

    CHECK_STATUS(resize(inputTensor, tmpTensor, outputTensor, p, &archInfo));
    /*warp up*/
    UNI_INFO_LOG("warm up gpu:\n")
    for (U32 i = 0; i < 2; i++) {
        CHECK_STATUS(gcl_run_kernelVec(handle));
    }

    UNI_INFO_LOG("Run:\n")
#ifdef _DEBUG
    CHECK_STATUS(gcl_run_kernelVec_timing(handle, 0, handle->kernelVec->size()));
    double time = handle->t_execute * 0.001;
#else
    CHECK_STATUS(gcl_run_kernelVec(handle));
#endif
    CHECK_STATUS(ocl_get_output(handle, output, outputDesc_gpu, output_gpu, tmpbuf, true));

    char buffer[150];
    char params[120];
    sprintf(params, "(%u %u %u %u)->(%u %u %u %u)", in, ic, ih, iw, on, oc, oh, ow);
    sprintf(buffer, "%20s, %80s", "bilinear", params);
#ifdef _DEBUG
    double ops = on * oc * oh * ow * 4;  // TO DO
    ut_log(dt, buffer, ops, time);
#endif
    ut_check_a(output_gpu, get_ptr_from_tensor(outputTensorCpu, UT_ARCH), on * oc * ow * oh, dt);
    CHECK_STATUS(gcl_finish(handle));
    CHECK_STATUS(gcl_clean_kernelVec(handle));
    free(input_cpu);
    free(output_gpu);
    return 0;
}

int main(int argc, char **argv)
{
#ifdef _USE_FP16
    resizeTest(argc, argv, DT_F16);
#endif
    return 0;
}
