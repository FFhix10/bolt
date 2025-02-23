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
#include <getopt.h>
#include "model_spec.h"
#include "model_quantization.h"
#include "model_calibration.hpp"
#include "model_data_type_converter.h"
#include "model_optimizer.hpp"
#include "model_print.h"
#include <algorithm>

void print_quantization_usage()
{
    std::cout << "post_training_quantization : "
                 "./post_training_quantization -p <path2Model>\n"
                 "Parameter description:\n"
                 "1. -p <path2Model>: Path to the input bolt model that generated by 'X2bolt -i "
                 "PTQ', and model file suffix is _ptq_input.bolt.\n"
                 "2. -i [inferencePrecision]: The inference precision. Currently, you can only "
                 "choose one of "
                 "{FP32, FP16, INT8_FP16, INT8_FP32}. Default is INT8_FP16. INT8_FP16 is for "
                 "machine(ARMv8.2+) that supports fp16 to compute non quantized operators. "
                 "INT8_FP32 is for machine(ARMv7, v8. Intel AVX512) that supports fp32 to compute "
                 "non quantized operators.\n"
                 "3. -b [BatchNormFusion]: Whether to fuse convolution or FC with BN. Default is "
                 "true.\n"
                 "4. -q [quantStorage]: Store model in quantized form. You can choose one of"
                 "{NOQUANT, FP16, INT8, MIX}. Default is NOQUANT.\n"
                 "5. -c [clipValue]: To clip the input for gemm if clipValue > 0. The default "
                 "value is 0.\n"
                 "6. -s [scaleFileDirectory]: The directory of the scale file. Set tensor clipping "
                 "value according to the file.\n"
                 "7. -o [offlineCalibration]: Whether to use offline calibration. Not compatible "
                 "with "
                 "option 6. Only when -o is set to true, option 8-10 will take effect.\n"
                 "8. -d [datasetPath]: Path to the calibration dataset (e.g. directory of "
                 "images).\n"
                 "9. -f [formatOfInput]: Specify the preprocessing style of the model.\n"
                 "10. -m [mulScale]: The multiplying scale for input preprocessing.\n"
                 "11. -V : Verbose mode.\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    std::cout << "\nEnter './post_training_quantization --help' to get more usage information."
              << std::endl;
    std::vector<std::string> lineArgs(argv, argv + argc);
    for (std::string arg : lineArgs) {
        if (arg == "--help" || arg == "-help" || arg == "--h" || arg == "-h") {
            print_quantization_usage();
            return -1;
        }
    }
    std::string modelPath;
    std::string inferPrecision = "INT8_FP16";
    bool fuseBN = true;
    char *quantStorage = (char *)"NOQUANT";
    F32 clipVal = 0.0;
    char *scaleFile = nullptr;
    bool offCal = false;
    char *dataPath = nullptr;
    ImageFormat imageFormat = RGB;
    F32 mulScale = 1.0;
    bool verbose = false;
    bool hasScale = false;

    int option;
    const char *optionstring = "p:i:b:q:c:s:o:d:f:m:V";
    while ((option = getopt(argc, argv, optionstring)) != -1) {
        switch (option) {
            case 'p':
                std::cout << "option is -p <path2Model>, value is: " << optarg << std::endl;
                modelPath = optarg;
                break;
            case 'i':
                std::cout << "option is -i [inferencePrecision], value is: " << optarg << std::endl;
                inferPrecision = optarg;
                break;
            case 'b':
                std::cout << "option is -b [BatchNormFusion], value is: " << optarg << std::endl;
                fuseBN = (std::string(optarg).compare("false") == 0) ? false : true;
                break;
            case 'q':
                std::cout << "option is -q [quantStorage], value is: " << optarg << std::endl;
                quantStorage = optarg;
                break;
            case 'c':
                std::cout << "option is -c [clipValue], value is: " << optarg << std::endl;
                clipVal = atof(optarg);
                break;
            case 's':
                std::cout << "option is -s [scaleFileDirectory], value is: " << optarg << std::endl;
                scaleFile = optarg;
                hasScale = true;
                break;
#if _USE_INT8
            case 'o':
                std::cout << "option is -o [offlineCalibration], value is: " << optarg << std::endl;
                offCal = (std::string(optarg).compare("true") == 0) ? true : false;
                break;
            case 'd':
                std::cout << "option is -d [datasetPath], value is: " << optarg << std::endl;
                dataPath = optarg;
                break;
            case 'f':
                std::cout << "option is -f [formatOfInput], value is: " << optarg << std::endl;
                if (std::string(optarg) == std::string("RGB")) {
                    imageFormat = RGB;
                } else if (std::string(optarg) == std::string("BGR")) {
                    imageFormat = BGR;
                } else if (std::string(optarg) == std::string("RGB_SC")) {
                    imageFormat = RGB_SC;
                } else if (std::string(optarg) == std::string("BGR_SC_RAW")) {
                    imageFormat = BGR_SC_RAW;
                } else if (std::string(optarg) == std::string("RGB_SC_RAW")) {
                    imageFormat = RGB_SC_RAW;
                } else {
                    imageFormat = RGB;
                    std::cout << "Unsupported image format, default to be RGB" << std::endl;
                }
                break;
            case 'm':
                std::cout << "option is -m [mulScale], value is: " << optarg << std::endl;
                mulScale = atof(optarg);
                break;
#endif
            case 'V':
                verbose = true;
                break;
            default:
                std::cerr << "Input option gets error. Please check the params meticulously. "
                          << "Calibration is only available on ARMv8.2." << std::endl;
                print_quantization_usage();
                exit(1);
        }
    }
    if (modelPath == "") {
        UNI_ERROR_LOG("Please use -p <path2Model> option to give an valid bolt model that "
                      "generated by 'X2bolt -i PTQ', and model file suffix is _ptq_input.bolt.\n");
        exit(1);
    }
    transform(inferPrecision.begin(), inferPrecision.end(), inferPrecision.begin(), toupper);

    if (offCal) {
        CHECK_REQUIREMENT(std::string("INT8_FP16") == inferPrecision ||
            std::string("INT8_FP32") == inferPrecision);
    }

    if (nullptr != scaleFile && offCal) {
        UNI_ERROR_LOG("Mode clash. Please confirm whether to use offline calibration.\n");
        exit(1);
    }

    ModelSpec ms;
    std::string storePath = modelPath;
    CHECK_STATUS(deserialize_model_from_file(storePath.c_str(), &ms));
    if (ms.dt != DT_F32 || std::string::npos == storePath.find("ptq_input.bolt")) {
        CHECK_STATUS(mt_destroy_model(&ms));
        UNI_ERROR_LOG("Input model does not match. Please produce it with: ./X2bolt -i PTQ\n");
        exit(1);
    }
    auto relationNum = ms.num_op_tensor_entries;
    auto relationPtr = ms.op_relationship_entries;
    ms.num_op_tensor_entries = 0;
    ms.op_relationship_entries = nullptr;
#ifdef _DEBUG
    print_ms(ms);
#endif

    DataConvertType converterMode = F32_to_F16;
    std::string storePathSuffix;
    if (inferPrecision == std::string("INT8_FP16")) {
        converterMode = F32_to_F16;
        storePathSuffix = std::string("int8_q.bolt");
    } else if (inferPrecision == std::string("INT8_FP32")) {
        converterMode = F32_to_F32;
        storePathSuffix = std::string("int8_q.bolt");
    } else if (inferPrecision == std::string("HIDDEN")) {
        converterMode = F32_to_F16;
        storePathSuffix = std::string("f16_q.bolt");
    } else if (inferPrecision == std::string("FP16")) {
        converterMode = F32_to_F16;
        storePathSuffix = std::string("f16_q.bolt");
    } else if (inferPrecision == std::string("FP32")) {
        converterMode = F32_to_F32;
        storePathSuffix = std::string("f32_q.bolt");
    } else {
        UNI_ERROR_LOG("Unknown converter data precision : %s.", inferPrecision.c_str());
        exit(1);
    }

    ModelSpecOptimizer msOptimizer;
    msOptimizer.suggest_for_ptq(inferPrecision, fuseBN, clipVal, hasScale);
    msOptimizer.optimize(&ms);

    if (hasScale) {
        add_scale_from_file(&ms, scaleFile);
    }

    ModelSpec *targetMs = new ModelSpec();
    CHECK_STATUS(mt_create_model(targetMs));
    CHECK_STATUS(ms_datatype_converter(&ms, targetMs, converterMode, quantStorage));
    if ("INT8_FP16" == inferPrecision) {
        targetMs->dt = DT_F16_8Q;
    } else if ("INT8_FP32" == inferPrecision) {
        targetMs->dt = DT_F32_8Q;
    }

    auto suffixPos = storePath.find("ptq_input.bolt");
    storePath.erase(suffixPos, 14);
    storePath += storePathSuffix;
    UNI_INFO_LOG("Write bolt model to %s.\n", storePath.c_str());
    CHECK_STATUS(serialize_model_to_file(targetMs, storePath.c_str()));
    CHECK_STATUS(mt_destroy_model(targetMs));
    delete targetMs;
    ms.num_op_tensor_entries = relationNum;
    ms.op_relationship_entries = relationPtr;
    CHECK_STATUS(mt_destroy_model(&ms));

    if (offCal) {
#ifdef _USE_INT8
        ModelSpec calMs;
        DataType calibrateType = DT_F32_8Q;
        if ("INT8" == std::string(inferPrecision)) {
            calibrateType = DT_F16_8Q;
        }
        calibrate_model_with_dataset(
            dataPath, imageFormat, calibrateType, mulScale, storePath, &calMs);
        relationNum = calMs.num_op_tensor_entries;
        relationPtr = calMs.op_relationship_entries;
        calMs.num_op_tensor_entries = 0;
        calMs.op_relationship_entries = nullptr;
        // Overwrite the pre-calibration model
        CHECK_STATUS(serialize_model_to_file(&calMs, storePath.c_str()));
        calMs.num_op_tensor_entries = relationNum;
        calMs.op_relationship_entries = relationPtr;
        CHECK_STATUS(mt_destroy_model(&calMs));
#endif
    }

    if (verbose) {
        ModelSpec resultMs;
        CHECK_STATUS(deserialize_model_from_file(storePath.c_str(), &resultMs));
        print_header(resultMs);
        print_operator_tensor_relationship(resultMs);
        print_weights(resultMs);
        CHECK_STATUS(mt_destroy_model(&resultMs));
    }
    std::cout << "Post Training Quantization Succeeded!" << std::endl;
    return 0;
}
