FILESEXTRAPATHS_prepend_intel-x86-common := "${THISDIR}/${PN}:"


LTTNG_PATCH = "${@bb.utils.contains_any('PREFERRED_PROVIDER_virtual/kernel','linux-intel linux-intel-rt','file://0002-lttng-modules-PKT-4.9-yocto-build-failed.patch','',d)}"

SRC_URI_append_intel-x86-common = " ${LTTNG_PATCH}"

