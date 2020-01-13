#!/usr/bin/env bash
set +e
set -v

echo "[*] Cloning the original qemu STM32"

git clone https://github.com/beckus/qemu_stm32 --depth=1

echo "[*] Applying TCalc patch"

cd qemu_stm32
git apply ../upstream-qemu-patch.patch

echo "[*] Building QEMU"

./configure --enable-debug --target-list="arm-softmmu" --disable-werror
make -j8

echo "[*] Done"
