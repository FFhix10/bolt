option(USE_DYNAMIC_LIBRARY "set use dynamic library or not" OFF)
option(USE_MINSIZEREL ".so lib will be 300KB smaller but performance will be affected" OFF)

option(USE_ANDROID_LOG "set use Android log or not" OFF)
option(USE_DEBUG "set use debug information or not" OFF)
option(USE_PROFILE "set use profile information or not" OFF)
option(USE_PROFILE_STATISTICS "set use profile statistics information or not" OFF)
option(USE_THREAD_SAFE "set use thread safe or not" OFF)

# model_tools variable
option(USE_CAFFE "set use caffe model as input or not" OFF)
option(USE_ONNX "set use onnx model as input or not" OFF)
option(USE_TFLITE "set use tflite model as input or not" OFF)
option(USE_TENSORFLOW "set use tensorflow model as input or not" OFF)

# blas_enhance tensor
option(USE_GENERAL "set use CPU serial code or not" OFF)
option(USE_X86 "set use X86 instruction or not" OFF)
option(USE_NEON "set use ARM NEON instruction or not" OFF)
option(USE_GPU "set use mali for parallel or not" OFF)
option(USE_FP32 "set use ARM NEON FP32 instruction or not" OFF)
option(USE_FP16 "set use ARM NEON FP16 instruction or not" OFF)
option(USE_F16_MIX_PRECISION "set use ARM NEON mix precision f16/f32 instruction or not" ON)
option(USE_INT8 "set use ARM NEON INT8 instruction or not" OFF)
option(USE_INT8_WINOGRAD "set use ARM NEON INT8 winograd" ON)
option(USE_OPENMP "set use openmp to run test(tinybert) or not" OFF)

option(USE_LIBRARY_TUNING "set use algorithm tuning or not" OFF)
option(USE_FLOW "set whether to use flow or not" OFF)

option(USE_JNI "set whether to use Java API or not" OFF)

option(BUILD_TEST "set to build unit test or not" OFF)

function (set_policy)
    if (POLICY CMP0074)
        cmake_policy(SET CMP0074 NEW)
    endif()
endfunction(set_policy)

macro (set_c_cxx_flags)
    set(COMMON_FLAGS "-W -Wextra -O3 -fPIC")
    if (NOT WIN32)
        set(COMMON_FLAGS "${COMMON_FLAGS} -fstack-protector-all")
    endif()
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-unused-command-line-argument -Wno-unused-parameter")
    set(COMMON_FLAGS "${COMMON_FLAGS} -Wno-unused-result -Wno-deprecated-declarations -Wno-unused-variable")

    if (USE_OPENMP)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_OPENMP -fopenmp")
    endif(USE_OPENMP)

    if (USE_THREAD_SAFE OR USE_CAFFE OR USE_ONNX OR USE_FLOW)
        set(COMMON_FLAGS "${COMMON_FLAGS} -pthread")
    endif ()

    if (USE_LIBRARY_TUNING)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_LIBRARY_TUNING")
    endif(USE_LIBRARY_TUNING)

    if (BUILD_TEST)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_BUILD_TEST")
    endif(BUILD_TEST)

    if (USE_DEBUG)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_DEBUG")
    endif(USE_DEBUG)

    if (USE_JNI)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_JNI")
    endif(USE_JNI)

    if (USE_ANDROID_LOG)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_ANDROID_LOG -llog")
    endif(USE_ANDROID_LOG)

    if (USE_PROFILE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_PROFILE")
    endif(USE_PROFILE)

    if (USE_PROFILE_STATISTICS)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_PROFILE_STATISTICS")
    endif(USE_PROFILE_STATISTICS)

    if (USE_THREAD_SAFE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_THREAD_SAFE")
    endif(USE_THREAD_SAFE)

    if (USE_GENERAL)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_GENERAL")
    endif(USE_GENERAL)

    if (USE_GPU)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_GPU")
    endif(USE_GPU)

    if (USE_X86)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_X86 -mavx2 -mfma")
        if (USE_INT8)
            set(COMMON_FLAGS "${COMMON_FLAGS} -mavx512f")
        endif (USE_INT8)
	if (USE_AVX512_VNNI)
            set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_AVX512_VNNI")
	endif(USE_AVX512_VNNI)
    endif(USE_X86)

    if (USE_FP32)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_FP32")
    endif (USE_FP32)

    if (USE_INT8)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_INT8")
    endif (USE_INT8)

    if (USE_NEON)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_NEON")

        if (USE_FP16)
            set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_FP16")
            if (USE_F16_MIX_PRECISION)
                set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_F16_MIX_PRECISION")
            endif (USE_F16_MIX_PRECISION)
            if (USE_INT8)
                if (USE_INT8_WINOGRAD)
                    set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_INT8_WINOGRAD")
                endif ()
                if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
                    set(COMMON_FLAGS "${COMMON_FLAGS} -march=armv8-a+fp16+dotprod")
                else ()
                    set(COMMON_FLAGS "${COMMON_FLAGS} -march=armv8.2-a+fp16+dotprod")
                endif ()
            else (USE_INT8)
                if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
                    set(COMMON_FLAGS "${COMMON_FLAGS} -march=armv8-a+fp16")
                else ()
                    set(COMMON_FLAGS "${COMMON_FLAGS} -march=armv8.2-a+fp16")
                endif ()
            endif (USE_INT8)
        endif (USE_FP16)
        if (USE_INT8)
            set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_INT8")
        endif ()
    endif(USE_NEON)

    if (USE_CAFFE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_CAFFE")
    endif()
    if (USE_ONNX)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_ONNX")
    endif()
    if (USE_TFLITE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_TFLITE")
    endif()
    if (USE_TENSORFLOW)
        set(COMMON_FLAGS "${COMMON_FLAGS} -D_USE_TENSORFLOW")
    endif()

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -std=gnu99")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -std=c++11")
    
    if (USE_DEBUG)
        set(CMAKE_BUILD_TYPE "Debug")
    elseif (USE_MINSIZEREL)
        set(CMAKE_BUILD_TYPE "MinSizeRel")
    endif (USE_DEBUG)
endmacro(set_c_cxx_flags)

macro (set_test_c_cxx_flags)
    if (NOT USE_DYNAMIC_LIBRARY)
        set(COMMON_FLAGS "${COMMON_FLAGS} -static-libstdc++")
        if (NOT "${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "${CMAKE_SYSTEM_PROCESSOR}" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            set(COMMON_FLAGS "${COMMON_FLAGS} -static")
        endif()
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND NOT APPLE)
        set(COMMON_FLAGS "${COMMON_FLAGS} -Wl,-allow-shlib-undefined")
    endif()

    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
endmacro (set_test_c_cxx_flags)

macro (set_project_install_directory)
    set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
    set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
endmacro (set_project_install_directory)

if(USE_DYNAMIC_LIBRARY)
    set(uni_library uni)
    set(model_spec_library model_spec)
    set(gcl_library gcl)
    set(kernelsource_library kernelsource)
    set(blas_enhance_library blas_enhance)
    set(tensor_library tensor)
    set(image_library image)
    set(model_tools_caffe_library model_tools_caffe)
    set(model_tools_onnx_library model_tools_onnx)
    set(model_tools_tflite_library model_tools_tflite)
    set(model_tools_tensorflow_library model_tools_tensorflow)
    set(model_tools_library model_tools)
    set(engine_library engine)
    set(flow_library flow)
else()
    set(uni_library uni_static)
    set(model_spec_library model_spec_static)
    set(gcl_library gcl_static)
    set(kernelsource_library kernelsource_static)
    set(blas_enhance_library blas_enhance_static)
    set(tensor_library tensor_static)
    set(image_library image_static)
    set(model_tools_caffe_library model_tools_caffe_static)
    set(model_tools_onnx_library model_tools_onnx_static)
    set(model_tools_tflite_library model_tools_tflite_static)
    set(model_tools_tensorflow_library model_tools_tensorflow_static)
    set(model_tools_library model_tools_static)
    set(engine_library engine_static)
    set(flow_library flow_static)
endif()

macro(include_uni)
    include_directories(${BOLT_ROOT}/common/uni/include)
endmacro()

macro(link_uni name)
    target_link_libraries(${name} ${uni_library})
endmacro()

macro(include_model_spec)
    include_directories(${BOLT_ROOT}/common/model_spec/include)
    include_memory()
    include_uni()
endmacro()

macro(link_model_spec name)
    target_link_libraries(${name} ${model_spec_library})
endmacro()

macro(include_gcl)
    include_directories(${BOLT_ROOT}/compute/tensor/src/gpu/mali/cl/kernel_option)
    include_directories(${BOLT_ROOT}/common/memory/include)
    include_directories(${BOLT_ROOT}/common/gcl/include)
    include_directories(${BOLT_ROOT}/common/gcl/tools/kernel_source_compile/include)
    include_directories(${OPENCL_INCLUDE_DIR})
    include_uni()
endmacro()

macro(link_opencl name)
    if (USE_GPU)
        target_link_libraries(${name} ${OPENCL_LIBRARIES})
    endif(USE_GPU)
endmacro()

macro(link_gcl name)
    if (USE_GPU)
        target_link_libraries(${name} ${gcl_library} ${kernelsource_library})
        link_opencl(${name})
    endif (USE_GPU)
endmacro()

macro(include_memory)
    include_directories(${BOLT_ROOT}/common/memory/include)
    include_uni()
    include_gcl()
endmacro()

macro(include_blas_enhance)
    include_directories(${BOLT_ROOT}/compute/blas_enhance/include)
    include_uni()
    include_memory()
endmacro()

macro(link_blas_enhance name)
    target_link_libraries(${name} ${blas_enhance_library})
    link_uni(${name})
endmacro()

macro(include_tensor)
    include_directories(${BOLT_ROOT}/compute/tensor/include)
    include_blas_enhance()
    include_gcl()
    include_memory()
endmacro()

macro(link_tensor name)
    target_link_libraries(${name} ${tensor_library} ${blas_enhance_library})
    link_blas_enhance(${name})
    link_gcl(${name})
endmacro()

macro(include_image)
    include_directories(${BOLT_ROOT}/compute/image/include)
    include_tensor()
endmacro()

macro(link_image name)
    target_link_libraries(${name} ${image_library})
    link_tensor(${name})
endmacro()

macro(include_protobuf)
    include_directories(${Protobuf_INCLUDE_DIR})
endmacro()

macro(link_protobuf name)
    target_link_libraries(${name} ${Protobuf_LIBRARY})
    if (ANDROID)
        target_link_libraries(${name} -llog)
    endif()
endmacro()

macro(include_model_tools)
    include_directories(${BOLT_ROOT}/model_tools/include)
    include_model_spec()
    include_uni()
endmacro()

macro(link_model_tools name)
    target_link_libraries(${name} ${model_tools_library})
    if(USE_CAFFE)
        target_link_libraries(${name} ${model_tools_caffe_library})
    endif()
    if(USE_ONNX)
        target_link_libraries(${name} ${model_tools_onnx_library})
    endif()
    if(USE_ONNX)
        target_link_libraries(${name} ${model_tools_tflite_library})
    endif()
    if(USE_TENSORFLOW)
        target_link_libraries(${name} ${model_tools_tensorflow_library})
        target_link_libraries(${name} ${JSONCPP_LIBRARY})
    endif()
    if(USE_CAFFE OR USE_ONNX)
        link_protobuf(${name})
    endif()
    link_model_spec(${name})
    link_uni(${name})
endmacro()

macro(model_tools_test name src_name)
    include_directories(${BOLT_ROOT}/model_tools/include)
    add_executable(${name} ${src_name})
    link_model_tools(${name})
endmacro()

macro(include_engine)
    if (BUILD_TEST)
        include_directories(${JPEG_INCLUDE_DIR})
        include_directories(${OpenCV_INCLUDE_DIRS})
    endif (BUILD_TEST)
    include_directories(${BOLT_ROOT}/inference/engine/include)
    if (USE_JNI)
        include_directories(${JNI_INCLUDE_DIR})
        include_directories(${JNI_MD_INCLUDE_DIR})
    endif (USE_JNI)
    include_model_tools()
    include_tensor()
    include_image()
endmacro()

macro(link_engine name)
    target_link_libraries(${name} ${engine_library})
    if (BUILD_TEST)
        target_link_libraries(${name} ${JPEG_LIBRARY})
    endif ()
    if (BUILD_TEST AND (${name} STREQUAL "ultra_face" OR ${name} STREQUAL "u2net"))
        target_link_libraries(${name} ${OpenCV_LIBS})
    endif ()
    link_model_spec(${name})
    target_link_libraries(${name} ${image_library} ${tensor_library} ${blas_enhance_library})
    link_gcl(${name})
    link_uni(${name})
endmacro()

macro(engine_test name src_name)
    include_engine()
    add_executable(${name} ${src_name})
    link_engine(${name})
endmacro()

macro(include_flow)
    include_directories(${BOLT_ROOT}/inference/flow/include)
    include_engine()
endmacro()

macro(flow_test name src_name)
    include_protobuf()
    include_directories(${BOLT_ROOT}/flow/include)
    if ("${name}" STREQUAL "flow_asr")
        set_policy()
        find_package(FFTS)
        add_executable(${name} ${src_name})
        target_link_libraries(${name} ${FFTS_LIBRARIES})
    else ()
        add_executable(${name} ${src_name})
    endif()
    target_link_libraries(${name} ${flow_library})
    link_engine(${name})
    link_protobuf(${name})
    add_dependencies(${name} flow.pb.h)
endmacro()

macro(include_train)
    include_model_tools()
    include_tensor()
    include_image()
endmacro()

macro(link_train name)
    target_link_libraries(${name} RaulLib)
    link_model_tools(${name})
    target_link_libraries(${name} ${image_library} ${tensor_library} ${blas_enhance_library})
    link_gcl(${name})
    link_uni(${name})
endmacro()

macro(train_test name src_name)
    include_directories(${BOLT_ROOT}/training/include)
    include_directories(${BOLT_ROOT}/training/src)
    add_executable(${name} ${src_name})
    link_train(${name})
endmacro()
