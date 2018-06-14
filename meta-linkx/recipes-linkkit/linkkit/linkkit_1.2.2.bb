FILESEXTRAPATHS_prepend := "${THISDIR}/patches:"
PATCH_FILE_X86_64 = " file://0001-Port-x86-64.patch;patch=1"
PATCH_FILE_RPI = " file://0002-Port-raspberrypi.patch;patch=1"

# patching according to MACHINE
python() {
    bb.note("xiaxiaowen")
    getmachine = d.getVar("MACHINE", True)
    bb.note("%s" % getmachine)
    if getmachine == "qemux86-64":
        bb.note("patching %s" % d.getVar("PATCH_FILE_X86_64", True))
        d.setVar("SRC_URI_append", d.getVar("PATCH_FILE_X86_64", True))
    elif getmachine == "raspberrypi3":
        bb.note("patching %s" % d.getVar("PATCH_FILE_RPI", True))
        d.setVar("SRC_URI_append", d.getVar("PATCH_FILE_RPI", True))
    else:
        bb.note("do not need to patch")
}

SRC_URI = "git://github.com/alibaba/AliOS-Things.git"
SRCREV = "44239bf7888abffe4f4350fcffb174c4ca5968a4"

# source code dir
S = "${WORKDIR}/git"

LIC_FILES_CHKSUM = "file://${S}/LICENSE;md5=e3fc50a88d0a364313df4b21ef20c29e"

require linkkit.inc