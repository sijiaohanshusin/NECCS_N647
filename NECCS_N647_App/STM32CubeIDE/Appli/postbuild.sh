#!/bin/bash

ProjectDir="$(dirname "$(readlink -f "$0")")"
BuildDir="$(pwd)"

for j in $(find "$BuildDir" -maxdepth 1 -name "*_Appli.bin"); do
    ProjectOut="$j"
done

if [ -z "$ProjectOut" ]; then
    echo "Cannot find *_Appli.bin under $BuildDir" >&2
    exit 1
fi

arm-none-eabi-objcopy -I binary "$ProjectOut" --change-addresses 0x70100400 -O ihex "$ProjectDir"/../../Binary/appli.hex
