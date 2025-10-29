#!/bin/bash

meson setup build --prefix=$PWD/dist
meson compile -C build
meson install -C build