// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "tensor_computing.h"
#ifdef _USE_GPU
#include "gpu/mali/tensor_computing_mali.h"
#endif

EE preallocated_memory_infer_output_size(Tensor *outputTensor, ArchInfo_t archInfo)
{
    if (outputTensor == nullptr) {
        CHECK_STATUS(NULL_POINTER);
    }
    TensorDesc outputDesc = outputTensor->get_desc();
    outputTensor->resize(outputDesc);
    return SUCCESS;
}

EE preallocated_memory(Tensor outputTensor, ArchInfo_t archInfo)
{
    auto arch = archInfo->arch;
    TensorDesc outputDesc = outputTensor.get_desc();
    void *output = get_ptr_from_tensor(outputTensor, arch);

    EE ret = NOT_SUPPORTED;
    if (IS_GPU(arch)) {
#ifdef _USE_GPU
        ret = preallocated_memory_mali(
            ((MaliPara_t)(archInfo->archPara))->handle, outputDesc, (GCLMem_t)output);
#endif
#ifdef _USE_CPU
    } else {
        memset(output, 0, tensorNumBytes(outputDesc));
        ret = SUCCESS;
#endif
    }
    return ret;
}
