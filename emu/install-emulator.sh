#!/usr/bin/env bash
echo "[*] Installing dependencies"

sudo apt-get install libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev libgtk-3-dev

if [ ! -d "qemu_stm32" ]; then
	echo "[*] Cloning the original qemu STM32"

	git clone https://github.com/beckus/qemu_stm32 --depth=1

	echo "[*] Applying TCalc patch"

	cd qemu_stm32
	git apply ../upstream-qemu-patch.patch
else
	echo "[!] QEMU is already cloned!"
fi

echo "[*] Building QEMU"

./configure --enable-debug --target-list="arm-softmmu" --disable-werror
make -j8

echo "[*] Done"
