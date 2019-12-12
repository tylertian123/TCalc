import code
import os
from gendbgblob import run as create_blob

Import("env")

def blob_action(target, source, env):
    source_elf = os.path.join(env.subst("$BUILD_DIR"), env.subst("$PROGNAME")) + ".elf"
    tgt_blob   = os.path.join(env.subst("$BUILD_DIR"), env.subst("$PROGNAME")) + ".dbgblob"

    create_blob(source_elf, tgt_blob)

# ref: arm-none-eabi-objcopy --set-section-flags .fw_dbg=ALLOC,CONTENTS,READONLY,DATA --update-section .fw_dbg=a1.bin.gz .pio/build/tcalc/firmware.elf

extra_actions = [
        env.VerboseAction(blob_action, "Creating debug blob."),
        env.VerboseAction(" ".join([
            "$OBJCOPY",
            "--set-section-flags",
            ".fw_dbg=ALLOC,CONTENTS,READONLY,DATA",
            "--update-section",
            ".fw_dbg=${BUILD_DIR}/${PROGNAME}.dbgblob",
            "${BUILD_DIR}/${PROGNAME}.elf"
        ]), "Adding blob to elf")
]

env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", extra_actions)
