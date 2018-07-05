SUMMARY = "TPM 2.0 Simulator Extraction Script"
DESCRIPTION = "The result of the extraction scripts is a complete set \
of the source files for a Trusted Platform Module (TPM) 2.0 Simulator, \
which runs under Windows, Linux, as well as Genode (by applying the \
appropriate patches). \
"

LICENSE = "BSD-2-Clause"
LIC_FILES_CHKSUM = "file://LICENSE;md5=1415f7be284540b81d9d28c67c1a6b8b"

DEPENDS += "\
    python-native \
    python-beautifulsoup4-native \
"

PV = "1.38+git${SRCPV}"

SRC_URI = "\
    git://github.com/stwagnr/tpm2simulator.git \
"
SRCREV = "b9646b90ce26ad34f7fc5c7099f28053eab94333"

S = "${WORKDIR}/git"

inherit native pythonnative lib_package cmake

EXTRA_OECMAKE = "\
    -DCMAKE_BUILD_TYPE=Debug \
    -DSPEC_VERSION=116 \
"

OECMAKE_SOURCEPATH = "${S}/cmake"

do_configure_prepend() {
    sed -i 's/^SET = False/SET = True/' "${S}/scripts/settings.py"
}
