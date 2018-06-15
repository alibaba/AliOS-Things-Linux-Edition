require recipes-core/images/core-image-minimal.bb

# Skip processing of this recipe if linux-intel-rt is not explicitly specified as the
# PREFERRED_PROVIDER for virtual/kernel. This avoids errors when trying
# to build multiple virtual/kernel providers.
python () {
    if d.getVar("PREFERRED_PROVIDER_virtual/kernel") != "linux-intel-rt":
        raise bb.parse.SkipPackage("Set PREFERRED_PROVIDER_virtual/kernel to linux-intel-rt to enable it")
}

DESCRIPTION = "A small image just capable of allowing a device to boot plus a \
real-time test suite and tools appropriate for real-time use."
DEPENDS = "linux-intel-rt"

IMAGE_INSTALL += "rt-tests hwlatdetect"

LICENSE = "MIT"
