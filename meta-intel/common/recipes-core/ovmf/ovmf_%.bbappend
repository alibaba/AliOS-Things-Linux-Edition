FILESEXTRAPATHS_prepend_intel-x86-common := "${THISDIR}/files:"

SRC_URI_append_intel-x86-common = " \
	file://0001-ovmf-RefkitTestCA-TEST-UEFI-SecureBoot.patch \
"
PACKAGECONFIG_append_intel-x86-common = " secureboot"
