#===============================================================================
# Copyright 2020-2021 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#===============================================================================

macro(exec_cmd_and_check CMD1 CMD2 CMD3 OUT)
    execute_process(COMMAND ${CMD1} ${CMD2} ${CMD3}
        RESULT_VARIABLE _sc_exit_code
        OUTPUT_VARIABLE _sc_cmd_out)
    if(NOT ${_sc_exit_code} STREQUAL 0)
        message(FATAL_ERROR "Failed with exit code ${_sc_exit_code}: ${CMD}")
    endif()
    set(${OUT} ${_sc_cmd_out})
endmacro()

macro(find_llvm)
    if(SC_LLVM_CONFIG STREQUAL "AUTO")
        find_package(LLVM CONFIG)
        if(NOT LLVM_FOUND)
            message(FATAL_ERROR "LLVM not found.")
        else()
            llvm_map_components_to_libnames(LLVM_LIBS all)
            set(SC_LLVM_VERSION ${LLVM_VERSION_MAJOR})
            set(SC_LLVM_INCLUDE_PATH ${LLVM_INCLUDE_DIRS})
            set(SC_LLVM_LIB_NAME ${LLVM_LIBS})
            message(STATUS "Found LLVM version: ${SC_LLVM_VERSION}")
        endif()
    else()
        message(STATUS "Finding LLVM using ${SC_LLVM_CONFIG}")
        set(__sc_llvm_link "--link-static")
        set(SC_LLVM_CONFIG_RETURN_STATIC ON)
        if(NOT MSVC)
            if(SC_LLVM_TRY_DYN_LINK OR (NOT SC_LIBRARY_TYPE STREQUAL "STATIC"))
                # try link with shared library
                execute_process(COMMAND ${SC_LLVM_CONFIG} "--libfiles" "--link-shared"
                    RESULT_VARIABLE _sc_exit_code
                    OUTPUT_VARIABLE _sc_cmd_out)
                if(${_sc_exit_code} STREQUAL 0)
                    set(__sc_llvm_link "--link-shared")
                    set(SC_LLVM_CONFIG_RETURN_STATIC OFF)
                endif()
            else()
                execute_process(COMMAND ${SC_LLVM_CONFIG} "--libfiles" "--link-static"
                    RESULT_VARIABLE _sc_exit_code
                    OUTPUT_VARIABLE _sc_cmd_out)
                if(NOT ${_sc_exit_code} STREQUAL 0)
                    set(__sc_llvm_link "--link-shared")
                    set(SC_LLVM_CONFIG_RETURN_STATIC OFF)
                endif()
            endif()
        endif()

        message(STATUS "Decided to link LLVM with ${__sc_llvm_link}")

        exec_cmd_and_check(${SC_LLVM_CONFIG} "--system-libs" ""
            _sc_llvm_system_libs)
        exec_cmd_and_check(${SC_LLVM_CONFIG} "--libfiles"  "${__sc_llvm_link}"
            _sc_llvm_libs)

        # libs to link
        string(STRIP ${_sc_llvm_libs} _sc_llvm_libs)
        string(STRIP ${_sc_llvm_system_libs} _sc_llvm_system_libs)
        set(SC_LLVM_LIB_NAME "${_sc_llvm_libs} ${_sc_llvm_system_libs}")
        separate_arguments(SC_LLVM_LIB_NAME)

        # version
        exec_cmd_and_check(${SC_LLVM_CONFIG} "--version" ""
            _sc_llvm_version)
        string(REGEX MATCH "^([^.]+)\\.([^.])+\\.[^.]+.*$" _ ${_sc_llvm_version})
        set(SC_LLVM_VERSION ${CMAKE_MATCH_1})
        string(STRIP ${SC_LLVM_VERSION} SC_LLVM_VERSION)
        
        # include directory
        exec_cmd_and_check(${SC_LLVM_CONFIG} "--includedir" "" SC_LLVM_INCLUDE_PATH)
    endif()
endmacro()
