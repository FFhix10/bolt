// Copyright (C) 2019. Huawei Technologies Co., Ltd. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <iostream>
#include <limits.h>
#include <float.h>
#include "inference.hpp"
#include "tensor.hpp"
#include "data_loader.hpp"
#include "result_format.hpp"
#include "profiling.h"
#include "tensor_computing.h"
#include "model_print.h"
#ifdef _USE_FP16
#include "../../../../compute/tensor/src/cpu/arm/fp16/arm_functions_fp16.h"
#endif
#ifdef _USE_FP32
#include "../../../../compute/tensor/src/cpu/arm/fp32/arm_functions_fp32.h"
#endif

#define BINS 2048
#define NUM_IMAGES_INPUT 100

void print_help(char *argv[])
{
    std::cout << "usage: " << argv[0]
              << " modelPath dataDirectory dataFormat scaleValue affinityPolicyName "
                 "algorithmMapPath"
              << std::endl;
}

int main(int argc, char *argv[])
{
#ifdef _USE_FP16
    UNI_TIME_INIT

    char *modelPath = (char *)"";
    char *dataDir = (char *)"";
    char *affinityPolicyName = (char *)"";
    char *algorithmMapPath = (char *)"";
    ImageFormat imageFormat = RGB;
    F32 scaleValue = 1;
    if (argc < 5) {
        print_help(argv);
        return 1;
    }
    modelPath = argv[1];
    dataDir = argv[2];

    imageFormat = (std::string(argv[3]) == std::string("BGR") ? BGR : RGB);
    if (std::string(argv[3]) == std::string("RGB_SC")) {
        imageFormat = RGB_SC;
    } else if (std::string(argv[3]) == std::string("BGR_SC_RAW")) {
        imageFormat = BGR_SC_RAW;
    } else if (std::string(argv[3]) == std::string("RGB_SC_RAW")) {
        imageFormat = RGB_SC_RAW;
    }

    scaleValue = atof(argv[4]);

    if (argc > 5) {
        affinityPolicyName = argv[5];
    }

    if (argc > 6) {
        algorithmMapPath = argv[6];
    }

    ModelSpec int8Ms;
    CHECK_STATUS(deserialize_model_from_file(modelPath, &int8Ms));
    CHECK_REQUIREMENT(DT_F16_8Q == int8Ms.dt || DT_F16 == int8Ms.dt);
    int8Ms.dt = DT_F16_8Q;

    ModelSpec f16Ms;
    CHECK_STATUS(deserialize_model_from_file(modelPath, &f16Ms));
    f16Ms.dt = DT_F16;

    ModelSpec resultMs;
    CHECK_STATUS(deserialize_model_from_file(modelPath, &resultMs));
    resultMs.dt = DT_F16_8Q;

    auto relationNum = resultMs.num_op_tensor_entries;
    auto relationPtr = resultMs.op_relationship_entries;
    resultMs.num_op_tensor_entries = 0;
    resultMs.op_relationship_entries = nullptr;

    auto int8CNN = createPipelinefromMs(affinityPolicyName, &int8Ms, algorithmMapPath);
    auto f16CNN = createPipelinefromMs(affinityPolicyName, &f16Ms, algorithmMapPath);

    // load images
    std::map<std::string, std::shared_ptr<Tensor>> inMap = int8CNN->get_inputs();
    TensorDesc imageDesc = (*(inMap.begin()->second)).get_desc();
    std::vector<TensorDesc> imageDescs;
    imageDescs.push_back(imageDesc);
    std::vector<std::vector<Tensor>> images;
    std::vector<std::string> imagePaths =
        load_image_with_scale(dataDir, imageDescs, &images, imageFormat, scaleValue);

    std::cout << "[Calibration]:" << std::endl;

    std::vector<U8> dBuf;
    //std::vector<U8> qBuf;
    std::vector<U32> calibratedOpIdx;

    auto curModelInputTensorNames = int8CNN->get_model_input_tensor_names();
    for (int index = 0; index < (int)curModelInputTensorNames.size(); index++) {
        int8CNN->copy_to_named_input(curModelInputTensorNames[index],
            (U8 *)((CpuMemory *)images[0][index].get_memory())->get_ptr());
    }

    U32 opIdx = int8CNN->find_next_dynamic_scale_op(calibratedOpIdx, 0);
    std::map<std::string, std::vector<F32>> tensorScale;

    while (0 != opIdx) {
        auto op = int8CNN->get_operator_by_index(opIdx);
        std::string opName = op->get_name();
        std::cout << "Calibrating OP " << opIdx << ": " << opName << std::endl;
        std::string opsName = int8Ms.ops[opIdx].name;
        CHECK_REQUIREMENT(opName == opsName);

        std::vector<std::vector<F32>> scales;
        auto inputTensors = op->get_input_tensors();
        auto outputTensors = op->get_output_tensors();
        std::cout << "  Inputs:\n";

        for (U32 i = 0; i < int8Ms.ops[opIdx].num_inputs; i++) {
            std::string tensorName = int8Ms.ops[opIdx].input_tensors_name[i];
            TensorDesc inDesc = inputTensors[i].get_desc();

            auto it = tensorScale.find(tensorName);
            if (it != tensorScale.end()) {
                scales.push_back(tensorScale[tensorName]);
                std::cout << "    InputTensor " << i << " " << tensorName << " inherits scale "
                          << tensorScale[tensorName][0] << std::endl;
                continue;
            }

            if (DT_I8 == inDesc.dt) {  // Gets scale from int8 pooling or concat. Label with -1
                std::vector<F32> scale;
                scale.push_back(-1);
                scales.push_back(scale);
                tensorScale[tensorName] = scale;
                std::cout << "    InputTensor " << i << " " << tensorName
                          << " inherits transformed scale " << std::endl;
                continue;
            }

            U32 dBytes = tensorNumBytes(inDesc);
            dBuf.resize(dBytes * NUM_IMAGES_INPUT);
            U8 *d = dBuf.data();
            std::vector<F32> histogram;
            F32 last_max = 0;
            F32 interval = 0;

            for (U32 j = 0; j < images.size(); j++) {
                for (int index = 0; index < (int)curModelInputTensorNames.size(); index++) {
                    int8CNN->copy_to_named_input(curModelInputTensorNames[index],
                        (U8 *)((CpuMemory *)images[j][index].get_memory())->get_ptr());
                }

                int8CNN->run_till_breakpoint(opIdx);
                memcpy(d, ((CpuMemory *)(inputTensors[i].get_memory()))->get_ptr(), dBytes);
                d += dBytes;

                if ((j != images.size() - 1) && ((j + 1) % NUM_IMAGES_INPUT != 0)) {
                    continue;
                }

                if (j == NUM_IMAGES_INPUT - 1 ||
                    ((j == images.size() - 1) && (j < NUM_IMAGES_INPUT - 1))) {
                    UNI_DEBUG_LOG("----------  start getting 1 - %u images input tensors  "
                                  "----------\n",
                        j + 1);
                    F16 *ptr_d = (F16 *)dBuf.data();
                    F32 max = array_maxabs_f16(ptr_d, (I32)(tensorNumElements(inDesc) * (j + 1)));
                    UNI_DEBUG_LOG("      %f is the maximum value\n", max);
                    interval = max / BINS;
                    histogram.resize(BINS, 0.00001f);
                    //update histogram first time
                    update_histogram(tensorNumElements(inDesc) * (j + 1), ptr_d, BINS, interval,
                        histogram.data());
                    last_max = max;
                    d = dBuf.data();
                    dBuf.clear();
                    continue;
                }

                if ((j + 1) % NUM_IMAGES_INPUT == 0 && j != (NUM_IMAGES_INPUT - 1)) {
                    UNI_DEBUG_LOG("----------  start getting %d - %u images input tensors  "
                                  "----------\n",
                        j + 1 - 100, j + 1);
                    F16 *ptr_d = (F16 *)dBuf.data();
                    F32 max = array_maxabs_f16(
                        ptr_d, (I32)(tensorNumElements(inDesc) * NUM_IMAGES_INPUT));
                    if (max <= last_max) {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", last_max);
                        interval = last_max / BINS;
                        //update histogram if no new max
                        update_histogram(tensorNumElements(inDesc) * NUM_IMAGES_INPUT, ptr_d, BINS,
                            interval, histogram.data());
                    } else {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", max);
                        interval = max / BINS;
                        F32 numPerBin = (F32)max / last_max;
                        //last_max = max; -> may optimize accuracy.
                        histogram = compress_histogram(histogram, numPerBin, last_max);
                        last_max = max;
                        update_histogram((tensorNumElements(inDesc) * NUM_IMAGES_INPUT), ptr_d,
                            BINS, interval, histogram.data());
                    }
                    d = dBuf.data();
                    dBuf.clear();
                    continue;
                }

                if ((j == images.size() - 1) && ((j + 1) % NUM_IMAGES_INPUT != 0)) {
                    UNI_DEBUG_LOG("----------  start getting %d - %u images input tensors  "
                                  "----------\n",
                        j + 1 - ((j + 1) % NUM_IMAGES_INPUT), j + 1);
                    dBuf.resize(dBytes * ((j + 1) % NUM_IMAGES_INPUT));
                    F16 *ptr_d = (F16 *)dBuf.data();
                    F32 max = array_maxabs_f16(
                        ptr_d, (I32)(tensorNumElements(inDesc) * ((j + 1) % NUM_IMAGES_INPUT)));
                    if (max <= last_max) {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", last_max);
                        interval = last_max / BINS;
                        //update histogram if no new max
                        update_histogram(tensorNumElements(inDesc) * ((j + 1) % NUM_IMAGES_INPUT),
                            ptr_d, BINS, interval, histogram.data());
                    } else {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", max);
                        interval = max / BINS;
                        F32 numPerBin = (F32)max / last_max;
                        //last_max = max;  -> may optimize accuracy
                        histogram = compress_histogram(histogram, numPerBin, last_max);
                        last_max = max;
                        update_histogram((tensorNumElements(inDesc) * NUM_IMAGES_INPUT), ptr_d,
                            BINS, interval, histogram.data());
                    }
                    d = dBuf.data();
                    dBuf.clear();
                    continue;
                }
            }

            UNI_DEBUG_LOG("----------  compute KL  ----------\n");
            std::vector<F32> scale = compute_scale_with_KL(histogram, interval);
            UNI_DEBUG_LOG("--------- finish compute KL ---------\n");
            scales.push_back(scale);
            tensorScale[tensorName] = scale;
            UNI_DEBUG_LOG("    InputTensor %u %s gets scale %f\n", i, tensorName.c_str(),
                tensorScale[tensorName][0]);
        }

        op->set_feature_scale(scales);
        UNI_DEBUG_LOG("  Outputs:\n");

        for (U32 i = 0; i < int8Ms.ops[opIdx].num_outputs; i++) {
            std::string tensorName = int8Ms.ops[opIdx].output_tensors_name[i];
            TensorDesc desc = outputTensors[i].get_desc();

            auto it = tensorScale.find(tensorName);
            CHECK_REQUIREMENT(it == tensorScale.end());

            if (DT_F16 == desc.dt) {
                continue;
            }

            CHECK_REQUIREMENT(DT_I8 == desc.dt);

            auto opF16 = f16CNN->get_operator_by_index(opIdx);
            auto outputs = opF16->get_output_tensors();

            TensorDesc outDesc = outputs[i].get_desc();
            U32 dBytes = tensorNumBytes(outDesc);
            dBuf.resize(dBytes * NUM_IMAGES_INPUT);
            std::vector<F32> histogram;
            F32 last_max = 0;
            F32 interval = 0;

            U8 *d = dBuf.data();

            for (U32 j = 0; j < images.size(); j++) {
                for (int index = 0; index < (int)curModelInputTensorNames.size(); index++) {
                    f16CNN->copy_to_named_input(curModelInputTensorNames[index],
                        (U8 *)((CpuMemory *)images[j][index].get_memory())->get_ptr());
                }

                f16CNN->run_till_breakpoint(opIdx);
                memcpy(d, ((CpuMemory *)outputs[i].get_memory())->get_ptr(), dBytes);
                d += dBytes;

                if ((j != images.size() - 1) && ((j + 1) % NUM_IMAGES_INPUT != 0)) {
                    continue;
                }

                if (j == NUM_IMAGES_INPUT - 1 ||
                    ((j == images.size() - 1) && (j < NUM_IMAGES_INPUT - 1))) {
                    UNI_DEBUG_LOG("----------  start getting 1 - %u images output tensors  "
                                  "----------\n",
                        j + 1);

                    F16 *ptr_d = (F16 *)dBuf.data();
                    F32 max = array_maxabs_f16(ptr_d, (I32)(tensorNumElements(outDesc) * (j + 1)));
                    UNI_DEBUG_LOG("      %f is the maximum value\n", max);
                    interval = max / BINS;
                    histogram.resize(BINS, 0.00001f);
                    //update histogram first time
                    update_histogram(tensorNumElements(outDesc) * (j + 1), ptr_d, BINS, interval,
                        histogram.data());
                    last_max = max;
                    d = dBuf.data();
                    dBuf.clear();
                    continue;
                }

                if ((j + 1) % NUM_IMAGES_INPUT == 0 && j != (NUM_IMAGES_INPUT - 1)) {
                    F16 *ptr_d = (F16 *)dBuf.data();
                    F32 max =
                        array_maxabs_f16(ptr_d, (I32)tensorNumElements(outDesc) * NUM_IMAGES_INPUT);

                    UNI_DEBUG_LOG("----------  start getting %d - %u images output tensors  "
                                  "----------\n",
                        j + 1 - 100, j + 1);

                    if (max <= last_max) {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", last_max);
                        interval = last_max / BINS;
                        //update histogram if no new max
                        update_histogram(tensorNumElements(outDesc) * NUM_IMAGES_INPUT, ptr_d, BINS,
                            interval, histogram.data());
                    } else {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", max);
                        interval = max / BINS;
                        F32 numPerBin = (F32)max / last_max;
                        //last_max = max;  -> may optimize accuracy
                        histogram = compress_histogram(histogram, numPerBin, last_max);
                        last_max = max;
                        update_histogram(tensorNumElements(outDesc) * NUM_IMAGES_INPUT, ptr_d, BINS,
                            interval, histogram.data());
                    }
                    d = dBuf.data();
                    dBuf.clear();
                    continue;
                }

                if ((j == images.size() - 1) && ((j + 1) % NUM_IMAGES_INPUT != 0)) {
                    UNI_DEBUG_LOG("----------  start getting %d - %u images output tensors  "
                                  "----------\n",
                        j + 1 - ((j + 1) % NUM_IMAGES_INPUT), j + 1);
                    dBuf.resize(dBytes * ((j + 1) % NUM_IMAGES_INPUT));
                    F16 *ptr_d = (F16 *)dBuf.data();
                    F32 max = array_maxabs_f16(
                        ptr_d, (I32)(tensorNumElements(outDesc) * ((j + 1) % NUM_IMAGES_INPUT)));
                    if (max <= last_max) {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", last_max);
                        interval = last_max / BINS;
                        //update histogram if no new max
                        update_histogram(tensorNumElements(outDesc) * ((j + 1) % NUM_IMAGES_INPUT),
                            ptr_d, BINS, interval, histogram.data());
                    } else {
                        UNI_DEBUG_LOG("      %f is the maximum value\n", max);
                        interval = max / BINS;
                        F32 numPerBin = (F32)max / last_max;
                        //last_max = max;  -> may optimize accuracy
                        histogram = compress_histogram(histogram, numPerBin, last_max);
                        last_max = max;
                        update_histogram(tensorNumElements(outDesc) * ((j + 1) % NUM_IMAGES_INPUT),
                            ptr_d, BINS, interval, histogram.data());
                    }
                    d = dBuf.data();
                    dBuf.clear();
                    continue;
                }
            }
            UNI_DEBUG_LOG("----------  compute KL  ----------\n");
            std::vector<F32> scale = compute_scale_with_KL(histogram, interval);
            UNI_DEBUG_LOG("----------  finish compute KL  ---------\n");
            scales.push_back(scale);
            tensorScale[tensorName] = scale;
            UNI_DEBUG_LOG("    OutputTensor %u %s gets scale %f\n", i, tensorName.c_str(),
                tensorScale[tensorName][0]);
        }
        if (int8Ms.ops[opIdx].num_quant_feature == 1 &&
            -2 == int8Ms.ops[opIdx].feature_scale[0].scale[0]) {
            std::vector<F32> outputScale;
            outputScale.push_back(-2);
            scales.push_back(outputScale);
        }

        op->set_feature_scale(scales);

        // Store scales into result model
        if (nullptr != resultMs.ops[opIdx].feature_scale) {  // Could be labelled with -2
            for (U32 i = 0; i < resultMs.ops[opIdx].num_quant_feature; i++) {
                if (nullptr != resultMs.ops[opIdx].feature_scale[i].scale) {
                    delete[] resultMs.ops[opIdx].feature_scale[i].scale;
                }
            }
            delete[] resultMs.ops[opIdx].feature_scale;
        }

        resultMs.ops[opIdx].num_quant_feature = scales.size();
        resultMs.ops[opIdx].feature_scale =
            (QuantSpec *)mt_new_storage(scales.size() * sizeof(QuantSpec));

        for (U32 i = 0; i < scales.size(); i++) {
            resultMs.ops[opIdx].feature_scale[i].num_scale = scales[i].size();
            U32 scaleBytes = scales[i].size() * sizeof(F32);
            resultMs.ops[opIdx].feature_scale[i].scale = (F32 *)mt_new_storage(scaleBytes);
            memcpy(resultMs.ops[opIdx].feature_scale[i].scale, scales[i].data(), scaleBytes);
        }

        calibratedOpIdx.push_back(opIdx);
        opIdx = int8CNN->find_next_dynamic_scale_op(calibratedOpIdx, opIdx);
    }

    print_ms(resultMs);

    std::string modelStorePath = std::string(argv[1]);
    auto suffixPos = modelStorePath.find(".bolt");
    modelStorePath.erase(suffixPos, 5);
    modelStorePath += "_KL.bolt";
    CHECK_STATUS(serialize_model_to_file(&resultMs, modelStorePath.c_str()));

    CHECK_STATUS(mt_destroy_model(&int8Ms));
    CHECK_STATUS(mt_destroy_model(&f16Ms));
    resultMs.num_op_tensor_entries = relationNum;
    resultMs.op_relationship_entries = relationPtr;
    CHECK_STATUS(mt_destroy_model(&resultMs));
#endif
    return 0;
}
