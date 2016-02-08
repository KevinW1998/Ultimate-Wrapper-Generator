# CMakeLists files in this project can
# refer to the root source directory of the project as ${HELLO_SOURCE_DIR} and
# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 2.8)
project (Ultimate-Wrapper-Generator)

# Settings
set(LLVM_BUILD_ROOT "" CACHE PATH "The llvm root build directory.")
set(LLVM_SRC_ROOT "" CACHE PATH "The llvm source directory.")

# Defines
add_definitions(-DBOOST_ALL_NO_LIB)











# =========== GENERATION PROCESS ============

# Interal variables
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

# Find packages
find_package(Boost 1.60 COMPONENTS system filesystem program_options)
if(NOT Boost_FOUND)
    message(SEND_ERROR "Boost 1.60 not found!")
endif(NOT Boost_FOUND)

message(STATUS ${Boost_LIBRARIES})

# Includes
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()
include_directories(
    "${LLVM_SRC_ROOT}/include" 
    "${LLVM_SRC_ROOT}/tools/clang/include" 
    "${LLVM_BUILD_ROOT}/include"
    "${LLVM_BUILD_ROOT}/tools/clang/include"
)


IF (MSVC)
    link_directories("${LLVM_BUILD_ROOT}/$(ConfigurationName)/lib")
ELSE()
    link_directories("${LLVM_BUILD_ROOT}/lib")
ENDIF(MSVC)



add_executable (uwgen
    main.cpp
    Core/UStartupArgs.h
    Core/UStartupArgs.cpp
    Core/UASTUtils.h
    Core/UASTUtils.cpp
    Core/WrapperGenerator/UWrapperGenerator.h
    Core/WrapperGenerator/UWrapperGenerator.cpp
    Core/WrapperGenerator/UWrapperGeneratorVB6Declare.h
    Core/WrapperGenerator/UWrapperGeneratorVB6Declare.cpp
    Core/WrapperGenerator/UWrapperGeneratorVB6IDL.h
    Core/WrapperGenerator/UWrapperGeneratorVB6IDL.cpp
    Core/UASTConsumer.h
    Core/UASTConsumer.cpp
)

add_definitions(-std=c++11)


# Boost dependencies
if(Boost_FOUND)
    target_link_libraries(uwgen ${Boost_LIBRARIES})
endif()

# LLVM and Clang dependencies.
target_link_libraries(uwgen
    # Clang
    clangFrontend
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangAnalysis
    clangAST
    clangBasic
    clangEdit
    clangLex
    
    # LLVM
    LLVMX86AsmParser # MC, MCParser, Support, X86CodeGen, X86Desc, X86Info
    LLVMX86CodeGen # Analysis, AsmPrinter, CodeGen, Core, MC, Support, Target, 
                 # X86AsmPrinter, X86Desc, X86Info, X86Utils
    LLVMX86Desc # MC, MCDisassembler, Object, Support, X86AsmPrinter, X86Info
    LLVMX86AsmPrinter # MC, Support, X86Utils
    LLVMX86Info # Support
    LLVMX86Utils # Core, Support
    LLVMCodeGen # Analysis, Core, MC, Scalar, Support, Target, TransformUtils
    LLVMipo
    LLVMScalarOpts
    LLVMInstCombine
    LLVMTransformUtils
    LLVMTarget # Analysis, MC, Core, Support
    LLVMAnalysis # Core, Support
    LLVMOption # Support
    LLVMMCDisassembler # MC, Support
    LLVMMCParser # MC, Support
    LLVMMC # Object, Support
    LLVMObject # BitReader, Core, Support
    LLVMBitReader # Core, Support
    LLVMCore # Support
    LLVMSupport
)


