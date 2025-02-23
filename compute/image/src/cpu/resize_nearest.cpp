// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "cpu/image_cpu.h"

template <typename IT, typename OT, ResizeCoordinateTransMode coordinate_transformation_mode>
inline static EE resize_nearest_kernel(
    const TensorDesc &inputDesc, IT *inArray, const TensorDesc &outputDesc, OT *outArray)
{
    DataType idt, odt;
    DataFormat idf, odf;
    U32 in, ic, ih, iw;
    U32 on, oc, oh, ow;
    CHECK_STATUS(tensor4dGet(inputDesc, &idt, &idf, &in, &ic, &ih, &iw));
    CHECK_STATUS(tensor4dGet(outputDesc, &odt, &odf, &on, &oc, &oh, &ow));

    int ic_align = 1, oc_align = 1;
    if (idf == DF_NCHWC8) {
        ic_align = 8;
    }
    if (odf == DF_NCHWC8) {
        oc_align = 8;
    }
    int ic_d = ic / ic_align;
    int oc_d = oc / oc_align;

    float hs0 = ih * 1.0 / oh;
    float ws0 = iw * 1.0 / ow;
    float hs1 = (ih - 1.0) / (oh - 1.0);
    float ws1 = (iw - 1.0) / (ow - 1.0);

    int srcX, srcY, src;
    for (U32 n = 0, dst = 0; n < on; n++) {
        for (I32 c = 0; c < oc_d; c++) {
            for (U32 h = 0; h < oh; h++) {
                for (U32 w = 0; w < ow; w++) {
                    for (int k = 0; k < oc_align; k++, dst++) {
                        switch (coordinate_transformation_mode) {
                            case HALF_PIXEL: {
                                srcX = (h + 0.5) * hs0 - 0.5;
                                srcY = (w + 0.5) * ws0 - 0.5;
                                if (srcX < 0) {
                                    srcX = 0;
                                }
                                if (srcY < 0) {
                                    srcY = 0;
                                }
                                break;
                            }
                            case PYTORCH_HALF_PIXEL: {
                                srcX = oh > 1 ? (h + 0.5) * hs0 - 0.5 : 0;
                                srcY = ow > 1 ? (w + 0.5) * ws0 - 0.5 : 0;
                                if (srcX < 0) {
                                    srcX = 0;
                                }
                                if (srcY < 0) {
                                    srcY = 0;
                                }
                                break;
                            }
                            case ALIGN_CORNERS: {
                                srcX = h * hs1;
                                srcY = w * ws1;
                                break;
                            }
                            case ASYMMETRIC: {
                                srcX = h * hs0;
                                srcY = w * ws0;
                                break;
                            }
                            default:
                                UNI_ERROR_LOG("Resize currently not support this coordinate "
                                              "transformation mode.\n");
                                break;
                        }
                        U32 cc = c * oc_align + k;
                        if (idf == DF_NCHWC8) {
                            U32 cc1 = cc / ic_align;
                            U32 cc2 = cc % ic_align;
                            src = (((n * ic_d + cc1) * ih + srcX) * iw + srcY) * ic_align + cc2;
                        } else {
                            src = ((n * ic + cc) * ih + srcX) * iw + srcY;
                        }
                        outArray[dst] = (OT)inArray[src];
                    }
                }
            }
        }
    }
    return SUCCESS;
}

template <typename IT, typename OT>
inline static EE resize_nearest_wrapper(const TensorDesc &inputDesc,
    IT *inArray,
    const ResizeParamSpec &p,
    const TensorDesc &outputDesc,
    OT *outArray)
{
    EE ret = SUCCESS;
    switch (p.trans_mode) {
        case HALF_PIXEL: {
            resize_nearest_kernel<IT, OT, HALF_PIXEL>(inputDesc, inArray, outputDesc, outArray);
            break;
        }
        case PYTORCH_HALF_PIXEL: {
            resize_nearest_kernel<IT, OT, PYTORCH_HALF_PIXEL>(
                inputDesc, inArray, outputDesc, outArray);
            break;
        }
        case ALIGN_CORNERS: {
            resize_nearest_kernel<IT, OT, ALIGN_CORNERS>(inputDesc, inArray, outputDesc, outArray);
            break;
        }
        case ASYMMETRIC: {
            resize_nearest_kernel<IT, OT, ASYMMETRIC>(inputDesc, inArray, outputDesc, outArray);
            break;
        }
        default:
            UNI_ERROR_LOG("Resize currently not support this coordinate transformation mode.\n");
            ret = NOT_SUPPORTED;
            break;
    }
    return ret;
}

EE resize_nearest_cpu(
    TensorDesc inputDesc, void *input, ResizeParamSpec p, TensorDesc outputDesc, void *output)
{
    EE ret = NOT_SUPPORTED;
    switch (inputDesc.dt) {
#ifdef _USE_FP16
        case DT_F16: {
            ret = resize_nearest_wrapper<F16, F16>(
                inputDesc, (F16 *)input, p, outputDesc, (F16 *)output);
            break;
        }
#endif
#ifdef _USE_FP32
        case DT_F32: {
            ret = resize_nearest_wrapper<F32, F32>(
                inputDesc, (F32 *)input, p, outputDesc, (F32 *)output);
            break;
        }
#endif
        case DT_U8: {
#ifdef _USE_FP16
            if (DT_F16 == outputDesc.dt) {
                ret = resize_nearest_wrapper<U8, F16>(
                    inputDesc, (U8 *)input, p, outputDesc, (F16 *)output);
            }
#endif
#ifdef _USE_FP32
            if (DT_F32 == outputDesc.dt) {
                ret = resize_nearest_wrapper<U8, F32>(
                    inputDesc, (U8 *)input, p, outputDesc, (F32 *)output);
            }
#endif
            break;
        }
        default:
            break;
    }
    return ret;
}
