FILESEXTRAPATHS_prepend_intel-x86-common := "${THISDIR}/${PN}:"

EXTRA_OEMAKE = "LD=${STAGING_BINDIR_NATIVE}/${HOST_SYS}/${TARGET_PREFIX}ld AR=${STAGING_BINDIR_NATIVE}/${HOST_SYS}/${TARGET_PREFIX}gcc-ar"

LINUX_VERSION_i586-nlp-32-intel-common = "4.9.13"
LINUX_VERSION_core2-32-intel-common = "4.9.13"
LINUX_VERSION_corei7-64-intel-common = "4.9.13"

SRCREV_meta_i586-nlp-32-intel-common = "8f3bc608ae61c5333043167fa31bac33be93c3de"
SRCREV_meta_core2-32-intel-common = "8f3bc608ae61c5333043167fa31bac33be93c3de"
SRCREV_meta_corei7-64-intel-common = "8f3bc608ae61c5333043167fa31bac33be93c3de"

SRCREV_machine_i586-nlp-32-intel-common = "95c0a80ee83f1cf8e59d733f36e8a9dfd50a0098"
SRCREV_machine_core2-32-intel-common = "95c0a80ee83f1cf8e59d733f36e8a9dfd50a0098"
SRCREV_machine_corei7-64-intel-common = "95c0a80ee83f1cf8e59d733f36e8a9dfd50a0098"

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
