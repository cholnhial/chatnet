#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_binary=$(mktemp /tmp/chatnet-network-events.XXXXXX)
trap 'rm -f "$test_binary"' EXIT HUP INT TERM
${CXX:-c++} -std=c++11 -Wno-deprecated-declarations \
    -Iclient/chatnetgui/src $(pkg-config --cflags gtkmm-2.4) \
    tests/network_events.cpp client/chatnetgui/src/network.cpp \
    client/chatnetgui/src/common.cpp -o "$test_binary" \
    $(pkg-config --libs gtkmm-2.4)
"$test_binary"
