#!/bin/bash

base_dir=$PWD
rm -rf $base_dir/cmocka|| exit 1
wget https://cmocka.org/files/1.1/cmocka-1.1.0.tar.xz || exit 1
tar xvf cmocka-1.1.0.tar.xz  || exit 1
mkdir -p cmocka-1.1.0/build|| exit 1
cd cmocka-1.1.0/build  || exit 1

cmake .. -DCMAKE_INSTALL_PREFIX=$base_dir/cmocka  || exit 1

make -j install || exit 1
