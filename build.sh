#!/usr/bin/env bash

######################### TIPS #######################
# THIS SCRIPT SHOULD BE CALLED BY FATHER_DIR'S MAKEFILE,
# SHOULD NOT BE CALLED BY ITSELF.
#######################################################


build_dir="$(pwd)/build"
project_lib_path="$build_dir/lib"
project_include_path="$build_dir/include"
third="$(pwd)/third_party"

# parameters:
#   $1: The name of lib or module
function cp_lib_and_include() {
    lib_name=$1
    if [ -z "$lib_name" ]; then
        echo "Inputs cp_lib_and_include parameters error!!!"
        return 1
    fi

    tr_lib_name=$(echo "$lib_name" | tr 'A-Z' 'a-z')
    echo "  Lib name: $tr_lib_name"
    echo "  Finding lib directory: $third/$lib_name"

    static_lib_name="lib$tr_lib_name.a"
    static_lib_path=$(find "$third/$lib_name" -name "$static_lib_name" -print -quit)
    if [ -z "$static_lib_path" ]; then
        echo "Can't find file: $static_lib_name!!! Please check compiling status!!!"
        return 1
    fi

    if [ -n "$static_lib_path" ]; then
        if ! cp "$static_lib_path" "$project_lib_path" ; then
            echo "Failed to copy static library: $static_lib_path" 
            return 1  
        fi
    fi

    c_include_file_name="$lib_name.h"
    cpp_include_file_name="$lib_name.hpp"

    c_include_file_path=$(find "$third/$lib_name" -name "$c_include_file_name" -type f -print)
    cpp_include_file_path=$(find "$third/$lib_name" -name "$cpp_include_file_name" -type f -print)
    
    if [ -n "$c_include_file_path" ]; then
        if ! cp "$c_include_file_path" "$project_include_path" ; then
            echo "Failed to copy C include file:$c_include_file_path"
        fi
    fi

    if [ -n "$cpp_include_file_path" ]; then
        if ! cp "$cpp_include_file_path" "$project_include_path" ; then
            echo "Failed to copy C++ include file:$cpp_include_file_path"
        fi
    fi

    echo "$lib_name: Operating success!!!"
}

# parameters:
#   $1: Directory's Path which should be checked.
function check_dir() {
    dir_name=$1
    if [ ! -d "$dir_name" ]; then
        mkdir -p "$dir_name"
        echo "Auto mkdir: $dir_name"
    fi
}   

function main() {
    (cd third_party && . downloads.sh)

    check_dir "$build_dir"
    check_dir "$project_include_path"
    check_dir "$project_lib_path"

    echo "------------------------------"
    for dir in "$third"/*; do
        if [ -d "$dir" ]; then
            lib_name=$(basename "$dir")
            if ! cp_lib_and_include "$lib_name"; then
                echo "Error: $lib_name"
                exit 1
            fi
            echo "------------------------------"
        fi
    done

    echo "Building complete!!!"
}

main