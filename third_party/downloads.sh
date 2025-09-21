#!/usr/bin/env bash

######################## cJSON ########################
if [ ! -d "cJSON" ]; then
    if ! git clone git@github.com:DaveGamble/cJSON.git ; then
        echo "cloning 'cJson' failed!!!"
        exit 1
    fi
fi

if ! (cd cJSON && make all); then
   echo "cJSON: Failed to compile!!!"
   exit 1
fi

####################### spdlog #########################
if [ ! -d "spdlog" ]; then
    if ! git clone git@github.com:gabime/spdlog.git; then
        echo "cloning 'spdlog' failed!!!"
        exit 1
    fi
fi

if ! (cd spdlog && mkdir build && cd build && cmake .. && cmake --build .) ; then
    echo "spdlog: Failed to compile!!!"
    exit 1
fi

echo "All libraries cloned and compiled successfully."
