#
# bionic specific configuration
# NOTICE: When using bionic as the standard C library, you should put meta-core/meta-base
# before meta-yp/meta in BBLAYERS to override bbclasses
# For now, bionic supports alios-tiny on qemux86-64, qemuarm64 and raspberrypi3-64

LIBCEXTENSION = "-android"

# Add bionic libc overrides to the overrides.
LIBCOVERRIDE = ":libc-bionic"
OVERRIDES .= "${LIBCOVERRIDE}"

PREFERRED_PROVIDER_virtual/libc ?= "bionic"
PREFERRED_PROVIDER_virtual/libintl ?= "gettext"
PREFERRED_PROVIDER_virtual/libiconv ?= "mini-iconv"

# Currently, bionic need to use its own linux headers.
PREFERRED_PROVIDER_linux-libc-headers = "bionic-linux-libc-headers"

PREFERRED_PROVIDER_virtual/nativesdk-libintl ?= "nativesdk-glibc"
PREFERRED_PROVIDER_virtual/nativesdk-libiconv ?= "nativesdk-glibc"

PACKAGE_INSTALL_append +=  "bionic-linker"
DISTRO_FEATURES_append += "ld-is-gold"

IMAGE_LINGUAS = ""

LIBC_DEPENDENCIES = "\
    bionic \
    bionic-dbg \
    bionic-dev \
    "