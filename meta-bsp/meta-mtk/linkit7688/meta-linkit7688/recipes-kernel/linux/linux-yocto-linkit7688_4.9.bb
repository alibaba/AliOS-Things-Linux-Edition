# linux-yocto-custom.bb:
#
#   An example kernel recipe that uses the linux-yocto and oe-core
#   kernel classes to apply a subset of yocto kernel management to git
#   managed kernel repositories.
#
#   To use linux-yocto-custom in your layer, copy this recipe (optionally
#   rename it as well) and modify it appropriately for your machine. i.e.:
#
#     COMPATIBLE_MACHINE_yourmachine = "yourmachine"
#
#   You must also provide a Linux kernel configuration. The most direct
#   method is to copy your .config to files/defconfig in your layer,
#   in the same directory as the copy (and rename) of this recipe and
#   add file://defconfig to your SRC_URI.
#
#   To use the yocto kernel tooling to generate a BSP configuration
#   using modular configuration fragments, see the yocto-bsp and
#   yocto-kernel tools documentation.
#
# Warning:
#
#   Building this example without providing a defconfig or BSP
#   configuration will result in build or boot errors. This is not a
#   bug.
#
#
# Notes:
#
#   patches: patches can be merged into to the source git tree itself,
#            added via the SRC_URI, or controlled via a BSP
#            configuration.
#
#   defconfig: When a defconfig is provided, the linux-yocto configuration
#              uses the filename as a trigger to use a 'allnoconfig' baseline
#              before merging the defconfig into the build. 
#
#              If the defconfig file was created with make_savedefconfig, 
#              not all options are specified, and should be restored with their
#              defaults, not set to 'n'. To properly expand a defconfig like
#              this, specify: KCONFIG_MODE="--alldefconfig" in the kernel
#              recipe.
#   
#   example configuration addition:
#            SRC_URI += "file://smp.cfg"
#   example patch addition (for kernel v4.x only):
#            SRC_URI += "file://0001-linux-version-tweak.patch"
#   example feature addition (for kernel v4.x only):
#            SRC_URI += "file://feature.scc"
#

inherit kernel
require recipes-kernel/linux/linux-yocto.inc

KBRANCH="v4.9.73"

# Original openwrt commit (forked from): c5ca1c9ab65bfe1e6fc74230f8c0121230562b1c

# Override SRC_URI in a copy of this recipe to point at a different source
# tree if you do not want to build from Linus' tree.
SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git;protocol=git;nobranch=1;rev=b3e88217e2f95b004da89a0ff931e1dc45d3d094"

SRC_URI += "file://defconfig \
            file://linkit7688.scc \
            file://linkit7688.cfg \
            file://linkit7688-standard.scc \
            file://linkit7688-user-config.cfg \
            file://linkit7688-user-features.scc \
            file://linkit7688-user-patches.scc \
           "
SRC_URI += "file://openwrt_files/target/linux/generic/files"
SRC_URI += "file://openwrt_files/target/linux/ramips/files-4.9"
SRC_URI += "file://openwrt_files/target/linux/ramips/dts"

LINUX_VERSION ?= "4.9"
LINUX_VERSION_EXTENSION_append = "-custom"

# Modify SRCREV to a different commit hash in a copy of this recipe to
# build a different release of the Linux kernel.
# tag: v4.2 64291f7db5bd8150a74ad2036f1037e6a0428df2
# SRCREV_machine="b3e88217e2f95b004da89a0ff931e1dc45d3d094"
# SRCREV="b3e88217e2f95b004da89a0ff931e1dc45d3d094"

PV = "${LINUX_VERSION}+git${SRCPV}"

# Override COMPATIBLE_MACHINE to include your machine in a copy of this recipe
# file. Leaving it empty here ensures an early explicit build failure.
COMPATIBLE_MACHINE_linkit7688 = "linkit7688"

PREFERRED_PROVIDER_virtual/kernel="linux-yocto-linkit7688"

KERNEL_VERSION_SANITY_SKIP="1"

# KERNEL_DEFCONFIG_linkit7688="defconfig"

FILESEXTRAPATHS_prepend:="${THISDIR}/openwrt_files:"

DEPENDS+="image-patch-native xz-native u-boot-mkimage-native openwrt-lzma-native"

do_patch_prepend() {
    cp -r openwrt_files/target/linux/generic/files/* ${S}
    cp -r openwrt_files/target/linux/ramips/files-4.9/* ${S}
    cp -r openwrt_files/target/linux/ramips/dts/* ${S}/arch/mips/boot/dts/ralink
}

do_configure_prepend() {
    rm -rf {B}/.config
}

#KERNEL_DEVICETREE = "${S}/arch/mips/boot/dts/ralink/LINKIT7688.dts"
#KERNEL_DEVICETREE += "LINKIT7688.dtb"

do_compile_append () {
    cd ${B}
    make dtbs
}

do_install_prepend() {
    cd ${B}
    ${OBJCOPY} -O binary -R .reginfo -R .notes -R .note -R .comment -R .mdebug -R .note.gnu.build-id -S vmlinux vmlinux.bin
    cp arch/mips/boot/dts/ralink/LINKIT7688.dtb ${B}
    patch-dtb vmlinux.bin LINKIT7688.dtb
    openwrt-lzma e vmlinux.bin -lc1 -lp2 -pb2 vmlinux.bin.lzma
    # cp vmlinux vmlinux_
    # lzma -z --lzma1=lc=1,lp=2,pb=2 vmlinux
    # lzma -z vmlinux
    # mv vmlinux.lzma vmlinux.bin.lzma
    # mv vmlinux_ vmlinux
    
    mkimage -A mips -O linux -T kernel -C lzma -a 0x80000000 -e 0x80000000 -n LINUX -d vmlinux.bin.lzma uImage
}

kernel_do_deploy_append() {
    install -m 0644 ${B}/uImage ${DEPLOYDIR}/uImage
}

# Disabling stripping.
# Allows kernel modules recompile. Vmlinux cannot be stripped for that.
do_strip() {
}
