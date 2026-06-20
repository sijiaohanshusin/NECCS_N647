#!/bin/bash

# Always create the loader beside the ELF. Installing it into STM32 tools is
# best-effort so a normal project build never depends on administrator rights.
build_dir="$(pwd -P)"
loader_elf="$(find "$build_dir" -maxdepth 1 -type f -name '*ExtMemLoader*.elf' -print -quit)"

if [ -z "$loader_elf" ]; then
    echo "error :: External loader ELF was not found in $build_dir"
    exit 1
fi

loader_name="$(basename "${loader_elf%.elf}.stldr")"
local_loader="$build_dir/$loader_name"

if ! cp -f "$loader_elf" "$local_loader"; then
    echo "error :: Failed to create $local_loader"
    exit 1
fi

echo "External loader created: $local_loader"

install_loader() {
    destination="$1"
    label="$2"

    if [ -z "$destination" ] || [ ! -d "$destination" ]; then
        echo "info :: $label loader directory is unavailable; installation skipped."
        return
    fi

    if cp -f "$local_loader" "$destination/$loader_name" 2>/dev/null; then
        echo "External loader installed for $label."
    else
        echo "info :: $label loader directory is not writable; installation skipped."
    fi
}

if [ -n "$STM32_PRG_PATH" ]; then
    install_loader "$STM32_PRG_PATH/ExternalLoader" "STM32CubeProgrammer"
fi

install_loader "$1" "STM32CubeIDE"

exit 0
