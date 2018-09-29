FILESEXTRAPATHS_prepend := "${THISDIR}/android-tools:"

ANDROID_MIRROR = "aosp.tuna.tsinghua.edu.cn"

SRC_URI += " \
    file://android-tools-adbd.init \
    "

# By default, disable the adb network and root switch for security consideration.
ADB_NETWORK ??= "0"
ADB_ROOT ??= "0"

SRC_URI += "${@ "file://0001-always-enable-adbd-network-and-listen-on-the-default-.patch" \
    if (d.getVar("ADB_NETWORK") == "1") else "" }"

SRC_URI += "${@ "file://0001-always-enable-adbd-to-be-root-due-to-lack-of-android.patch" \
    if (d.getVar("ADB_ROOT") == "1") else "" }"

# Install init script for sysvinit
inherit update-rc.d

INITSCRIPT_NAME = "adbd"
INITSCRIPT_PARAMS = "defaults 20"

do_install_append() {
    install -d ${D}${sysconfdir}/init.d/
    install -m 0755 ${WORKDIR}/android-tools-adbd.init ${D}${sysconfdir}/init.d/adbd
}
