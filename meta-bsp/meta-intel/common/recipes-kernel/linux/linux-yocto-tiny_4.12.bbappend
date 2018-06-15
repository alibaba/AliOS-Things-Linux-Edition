FILESEXTRAPATHS_prepend_intel-x86-common := "${THISDIR}/${PN}:"

KERNEL_FEATURES_INTEL_COMMON ?= ""

COMPATIBLE_MACHINE_i586-nlp-32-intel-common = "${MACHINE}"
COMPATIBLE_MACHINE_core2-32-intel-common = "${MACHINE}"
COMPATIBLE_MACHINE_corei7-64-intel-common = "${MACHINE}"

KBRANCH_i586-nlp-32-intel-common = "standard/tiny/base"
KBRANCH_core2-32-intel-common = "standard/tiny/base"
KBRANCH_corei7-64-intel-common = "standard/tiny/base"

KMACHINE_i586-nlp-32-intel-common = "intel-quark"
KMACHINE_core2-32-intel-common = "intel-core2-32"
KMACHINE_corei7-64-intel-common = "intel-corei7-64"

KERNEL_FEATURES_append_i586-nlp-32-intel-common = "${KERNEL_FEATURES_INTEL_COMMON} cfg/fs/ext4.scc"
KERNEL_FEATURES_append_core2-32-intel-common = "${KERNEL_FEATURES_INTEL_COMMON} cfg/fs/ext4.scc"
KERNEL_FEATURES_append_corei7-64-intel-common = "${KERNEL_FEATURES_INTEL_COMMON} cfg/fs/ext4.scc"
