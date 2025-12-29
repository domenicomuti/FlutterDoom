#!/bin/bash

# Copyright (C) 2025 Domenico Muti
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 31 Milk St # 960789 Boston, MA 02196 USA.

CURRENT_DIR=$(basename "$PWD")

if [[ "$CURRENT_DIR" != "macos" && "$CURRENT_DIR" != "ios" ]]; then
    echo "This script must be executed from the macos or ios directory only"
    exit 1
fi

rm -rf linuxdoom-1.10
mkdir linuxdoom-1.10
cd ../linuxdoom-1.10

if [[ "$CURRENT_DIR" == "macos" ]]; then
    find . -type f -exec sh -c 'dest="../macos/linuxdoom-1.10/{}"; mkdir -p "$(dirname "$dest")" && ln "{}" "$dest"' \;
else
    find . -type f -exec sh -c 'dest="../ios/linuxdoom-1.10/{}"; mkdir -p "$(dirname "$dest")" && ln "{}" "$dest"' \;
    mv ../ios/linuxdoom-1.10/i_sound.c ../ios/linuxdoom-1.10/i_sound.m
fi

cd ../$CURRENT_DIR

exit 0