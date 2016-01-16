#!/bin/sh
source "$CONFIG_FOLDER/osx-source-qt.sh"

$CMAKE_BIN -GNinja $CMAKE_COMMON_FLAGS -DISCORE_CONFIGURATION=release ..
$CMAKE_BIN --build . --target install/strip --config DynamicRelease
