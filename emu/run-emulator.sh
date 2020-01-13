#!/usr/bin/env bash

qemu_stm32/arm-softmmu/qemu-system-arm -M stm32-tcalc -kernel ../stm/.pio/build/tcalc/firmware.bin -gdb tcp::3333 -serial stdio
