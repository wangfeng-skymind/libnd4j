set( CMAKE_VERBOSE_MAKEFILE on)

if(LINUX)
    link_directories(/usr/local/lib)
    link_directories(/usr/lib)
    link_directories(/lib)
endif()

if(APPLE)
    message("Using apple")
    link_directories(/usr/local/lib)
    link_directories(/usr/lib)
    link_directories(/lib)
endif()
if(WIN32)
    get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    foreach(dir ${dirs})
        message(STATUS "dir='${dir}'")
    endforeach()
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath,$ORIGIN/")

IF(${ARCH} MATCHES "arm*")
    set(ARCH_TUNE "-march=${ARCH}")
ELSEIF(${ARCH} MATCHES "power*")
    set(ARCH_TUNE "-mcpu=${ARCH} -mtune=${ARCH} -D__POWER")
ELSE()
    set(ARCH_TUNE "-march=${ARCH} -mtune=${ARCH}")
ENDIF()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # using Clang
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ARCH_TUNE}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    # using Intel C++
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ARCH_TUNE} -O3 -fp-model fast")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # using Visual Studio C++

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # using GCC
    #  -fmax-errors=3
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ARCH_TUNE} -fPIC")
endif()


IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    include_directories("/usr/include")
    include_directories("/usr/local/include")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if(!CUDA_BLAS)
    if(!CPU_BLAS)
        set(CUDA_BLAS FALSE)
        set(CPU_BLAS TRUE)
    endif()
endif()


if (APPLE)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if ("${CMAKE_C_COMPILER_VERSION}" VERSION_GREATER 6.0 OR "${CMAKE_C_COMPILER_VERSION}" VERSION_EQUAL 6.0)
            SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wa,-mavx512f ")
        endif()
    endif()
endif()


if(CUDA_BLAS)
    message("Build cublas")
    find_package(CUDA)
    add_definitions(-D__CUDABLAS__=true)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set (CMAKE_CXX_FLAGS "")
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if ("${CMAKE_C_COMPILER_VERSION}" VERSION_GREATER 4.9 AND "$ENV{TRICK_NVCC}" STREQUAL "YES" AND CUDA_VERSION VERSION_LESS "8.0")
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__GNUC__=4 -D__GNUC_MINOR__=9 -D_FORCE_INLINES -D_MWAITXINTRIN_H_INCLUDED")
            set (CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -Xcompiler -std=c++98 -Dnullptr=NULL")
            message("TRICKING CUDA INTO SUPPORTING GCC > 4.9 YOU ARE PROCEEDING AT YOUR OWN RISK")
        endif()
    endif()

    # we want OpenMP to be available for hybrid operations, at least for GCC
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        find_package(OpenMP)
        if (OPENMP_FOUND)
            set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
            set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        endif()
    endif()

    if (CUDA_FOUND)
        message("CUDA found!")
        set( CUDA_ARCHITECTURE_MINIMUM "3.0" CACHE STRING "Minimum required CUDA compute capability" )
        SET(CUDA_VERBOSE_BUILD ON)
        SET(CUDA_SEPARABLE_COMPILATION OFF)
        #set(CUDA_COMPUTE_CAPABILITY "61")
        set(CUDA_COMPUTE_CAPABILITY "35")
        if ("${EXPERIMENTAL}" STREQUAL "yes")
            list(APPEND CUDA_NVCC_FLAGS " -DEXPERIMENTAL ")
        endif()
        if (CMAKE_BUILD_TYPE STREQUAL "Release")
            if(CUDA_VERSION VERSION_GREATER "8.0") # cuda 9
                if ("${COMPUTE}" STREQUAL "all")
                    list(APPEND CUDA_NVCC_FLAGS "-DCUDA_9  -DVERBOSE --cudart=static -O3 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_53,code=sm_53 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_61,code=sm_61 -gencode arch=compute_62,code=sm_62 -gencode arch=compute_70,code=sm_70")
                else()
                    list(APPEND CUDA_NVCC_FLAGS "-DCUDA_9 -DVERBOSE --cudart=static -O3 -gencode arch=compute_${COMPUTE},code=sm_${COMPUTE}")
                endif()
            elseif (CUDA_VERSION VERSION_GREATER "7.5") # cuda 8.0
                if ("${COMPUTE}" STREQUAL "all")
                    list(APPEND CUDA_NVCC_FLAGS "-DCUDA_8  -DVERBOSE --cudart=static -O3 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_53,code=sm_53 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_61,code=sm_61 -gencode arch=compute_62,code=sm_62")
                else()
                    list(APPEND CUDA_NVCC_FLAGS "-DCUDA_8 -DVERBOSE --cudart=static -O3 -gencode arch=compute_${COMPUTE},code=sm_${COMPUTE}")
                endif()
            else()
                if ("${COMPUTE}" STREQUAL "all")
                    list(APPEND CUDA_NVCC_FLAGS "-DCUDA_75 -DVERBOSE --cudart=static -O3 -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_53,code=sm_53")
                else()
                    list(APPEND CUDA_NVCC_FLAGS "-DCUDA_75 -DVERBOSE --cudart=static -O3 -gencode arch=compute_${COMPUTE},code=sm_${COMPUTE}")
                endif()
            endif()

        else()
            if(CUDA_VERSION VERSION_GREATER "8.0") # cuda 9
                if ("${COMPUTE}" STREQUAL "all")
                    list(APPEND CUDA_NVCC_FLAGS " -DCUDA_9  -DVERBOSE -G -g --cudart=static -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_53,code=sm_53 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_61,code=sm_61  -gencode arch=compute_62,code=sm_62 -gencode arch=compute_70,code=sm_70")
                else()
                    list(APPEND CUDA_NVCC_FLAGS " -DCUDA_9  -DVERBOSE -G -g --cudart=static -gencode arch=compute_${COMPUTE},code=sm_${COMPUTE}")
                endif()
            elseif (CUDA_VERSION VERSION_GREATER "7.5") # cuda 8
                if ("${COMPUTE}" STREQUAL "all")
                    list(APPEND CUDA_NVCC_FLAGS " -DCUDA_8  -DVERBOSE -G -g --cudart=static -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_53,code=sm_53 -gencode arch=compute_60,code=sm_60 -gencode arch=compute_61,code=sm_61  -gencode arch=compute_62,code=sm_62")
                else()
                    list(APPEND CUDA_NVCC_FLAGS " -DCUDA_8  -DVERBOSE -G -g --cudart=static -gencode arch=compute_${COMPUTE},code=sm_${COMPUTE}")
                endif()
            else()
                if ("${COMPUTE}" STREQUAL "all")
                    list(APPEND CUDA_NVCC_FLAGS " -DCUDA_75 -DVERBOSE -G -g --cudart=static -gencode arch=compute_30,code=sm_30 -gencode arch=compute_35,code=sm_35 -gencode arch=compute_37,code=sm_37 -gencode arch=compute_50,code=sm_50 -gencode arch=compute_52,code=sm_52 -gencode arch=compute_53,code=sm_53")
                else()
                    list(APPEND CUDA_NVCC_FLAGS " -DCUDA_75 -DVERBOSE -G -g --cudart=static -gencode arch=compute_${COMPUTE},code=sm_${COMPUTE}")
                endif()
            endif()
        endif()
            # ../include/loops/cuda/grid_strided.cu ../include/loops/cuda/grid_shaped.cu
            #cuda/NativeOps.cu ../include/loops/cuda/grid_strided.cu ../include/loops/cuda/grid_shaped.cu ../include/loops/cuda/scalar.cu ../include/loops/cuda/pairwise_transform.cu
            # ../include/loops/cuda/scalar.cu
        CUDA_ADD_LIBRARY(nd4jcuda SHARED cuda/NativeOps.cu ../include/loops/cuda/grid_strided.cu ../include/loops/cuda/grid_shaped.cu  ../include/loops/cuda/pairwise_transform.cu ../include/loops/cpu/scalar.cpp cpu/GraphExecutioner.cpp cpu/NativeOpExcutioner.cpp cpu/NDArray.cpp cpu/NDArrayFactory.cpp  ../include/memory/cpu/Workspace.cpp ../include/cnpy/cnpy.cpp  ../include/nd4jmemset.h ../include/nd4jmalloc.h ../include/ops/declarable/generic/parity_ops.cpp ../include/ops/declarable/generic/third_party.cpp ../include/helpers/helper_hash.h ../include/memory/cpu/Workspace.cpp ../include/ops/declarable/generic/helpers/impl/convolutions.cpp ../include/graph/ShapeList.h ../include/ops/declarable/generic/convo/conv2d_ops.cpp ../include/helpers/ArrayUtils.h ../include/graph/SessionLocalStorage.h  Environment.cpp ../include/graph/impl/Stash.cpp ../include/indexing/impl/NDIndex.cpp ../include/indexing/impl/IndicesList.cpp ../include/graph/impl/Variable.cpp ../include/graph/impl/VariableSpace.cpp ../include/graph/impl/ShapeList.cpp ../include/graph/impl/SessionLocalStorage.cpp ../include/graph/impl/Node.cpp ../include/graph/impl/Graph.cpp ../include/graph/impl/Block.cpp ../include/graph/impl/ArrayList.cpp ../include/helpers/impl/ArrayUtils.cpp ../include/helpers/impl/helper_hash.cpp ../include/helpers/impl/TAD.cpp ../include/ops/impl/specials_sparse.cpp ../include/helpers/impl/shape.cpp ../include/ops/declarable/impl/DeclarableOp.cpp ../include/ops/declarable/impl/OpRegistrator.cpp ../include/ops/declarable/DeclarableReductionOp.h ../include/ops/declarable/impl/DeclarableReductionOp.cpp ../include/ops/declarable/impl/DeclarableCustomOp.cpp ../include/ops/impl/specials.cpp ../include/memory/cpu/MemoryRegistrator.cpp ../include/ops/impl/gemm.cpp ../include/ops/declarable/generic/convo/conv3d_ops.cpp ../include/ops/declarable/generic/convo/fullconv3d.cpp ../include/ops/declarable/generic/convo/pooling2d_ops.cpp ../include/ops/declarable/generic/convo/pooling3d_ops.cpp)
        target_link_libraries(nd4jcuda ${CUDA_LIBRARIES})

        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/cuda)

        install(TARGETS nd4jcuda DESTINATION .)
    endif(CUDA_FOUND)
elseif(CPU_BLAS)
    message("CPU BLAS")
    add_definitions(-D__CPUBLAS__=true)
if(IOS)
    add_library(nd4jcpu STATIC cpu/NativeOps.cpp ../include/loops/cpu/scalar.cpp cpu/GraphExecutioner.cpp cpu/NativeOpExcutioner.cpp cpu/NDArray.cpp cpu/NDArrayFactory.cpp  ../include/memory/cpu/Workspace.cpp ../include/cnpy/cnpy.cpp  ../include/nd4jmemset.h ../include/nd4jmalloc.h ../include/ops/declarable/generic/parity_ops.cpp ../include/ops/declarable/generic/third_party.cpp ../include/helpers/helper_hash.h ../include/memory/cpu/Workspace.cpp ../include/ops/declarable/generic/helpers/impl/convolutions.cpp ../include/graph/ShapeList.h ../include/ops/declarable/generic/convo/conv2d_ops.cpp ../include/helpers/ArrayUtils.h ../include/graph/SessionLocalStorage.h  Environment.cpp ../include/graph/impl/Stash.cpp ../include/indexing/impl/NDIndex.cpp ../include/indexing/impl/IndicesList.cpp ../include/graph/impl/Variable.cpp ../include/graph/impl/VariableSpace.cpp ../include/graph/impl/ShapeList.cpp ../include/graph/impl/SessionLocalStorage.cpp ../include/graph/impl/Node.cpp ../include/graph/impl/Graph.cpp ../include/graph/impl/Block.cpp ../include/graph/impl/ArrayList.cpp ../include/helpers/impl/ArrayUtils.cpp ../include/helpers/impl/helper_hash.cpp ../include/helpers/impl/TAD.cpp ../include/ops/impl/specials_sparse.cpp ../include/helpers/impl/shape.cpp ../include/ops/declarable/impl/DeclarableOp.cpp ../include/ops/declarable/impl/OpRegistrator.cpp ../include/ops/declarable/DeclarableReductionOp.h ../include/ops/declarable/impl/DeclarableReductionOp.cpp ../include/ops/declarable/impl/DeclarableCustomOp.cpp ../include/ops/impl/specials.cpp ../include/memory/cpu/MemoryRegistrator.cpp ../include/ops/impl/gemm.cpp ../include/ops/declarable/generic/convo/conv3d_ops.cpp ../include/ops/declarable/generic/convo/fullconv3d.cpp ../include/ops/declarable/generic/convo/pooling2d_ops.cpp ../include/ops/declarable/generic/convo/pooling3d_ops.cpp)
else()

    add_library(nd4jcpu SHARED cpu/NativeOps.cpp ../include/loops/cpu/scalar.cpp cpu/GraphExecutioner.cpp cpu/NativeOpExcutioner.cpp cpu/NDArray.cpp cpu/NDArrayFactory.cpp  ../include/memory/cpu/Workspace.cpp ../include/cnpy/cnpy.cpp  ../include/nd4jmemset.h ../include/nd4jmalloc.h ../include/ops/declarable/generic/parity_ops.cpp ../include/ops/declarable/generic/third_party.cpp ../include/helpers/helper_hash.h ../include/memory/cpu/Workspace.cpp ../include/ops/declarable/generic/helpers/impl/convolutions.cpp ../include/graph/ShapeList.h ../include/ops/declarable/generic/convo/conv2d_ops.cpp ../include/helpers/ArrayUtils.h ../include/graph/SessionLocalStorage.h  Environment.cpp ../include/graph/impl/Stash.cpp ../include/indexing/impl/NDIndex.cpp ../include/indexing/impl/IndicesList.cpp ../include/graph/impl/Variable.cpp ../include/graph/impl/VariableSpace.cpp ../include/graph/impl/ShapeList.cpp ../include/graph/impl/SessionLocalStorage.cpp ../include/graph/impl/Node.cpp ../include/graph/impl/Graph.cpp ../include/graph/impl/Block.cpp ../include/graph/impl/ArrayList.cpp ../include/helpers/impl/ArrayUtils.cpp ../include/helpers/impl/helper_hash.cpp ../include/helpers/impl/TAD.cpp ../include/ops/impl/specials_sparse.cpp ../include/helpers/impl/shape.cpp ../include/ops/declarable/impl/DeclarableOp.cpp ../include/ops/declarable/impl/OpRegistrator.cpp ../include/ops/declarable/DeclarableReductionOp.h ../include/ops/declarable/impl/DeclarableReductionOp.cpp ../include/ops/declarable/impl/DeclarableCustomOp.cpp ../include/ops/impl/specials.cpp ../include/memory/cpu/MemoryRegistrator.cpp ../include/ops/impl/gemm.cpp ../include/ops/declarable/generic/convo/conv3d_ops.cpp ../include/ops/declarable/generic/convo/fullconv3d.cpp ../include/ops/declarable/generic/convo/pooling2d_ops.cpp ../include/ops/declarable/generic/convo/pooling3d_ops.cpp)
endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 4.9)
      message(FATAL_ERROR "You need at least GCC 4.9")
    endif()

    find_package(OpenMP)
    if (OPENMP_FOUND)
        set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    endif()

    if(APPLE)
        SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    elseif(MSYS)
        SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -static")
    endif()

    #install(TARGETS mySharedLib DESTINATION /some/full/path)
    install(TARGETS nd4jcpu DESTINATION  .)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/cpu)
endif()
