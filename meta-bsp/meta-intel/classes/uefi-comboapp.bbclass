# This class brings a more generic version of the UEFI combo app from refkit to meta-intel.
# It uses a combo file, containing kernel, initramfs and
# command line, presented to the BIOS as UEFI application, by prepending
# it with the efi stub obtained from systemd-boot.

# Don't add syslinux or build an ISO
PCBIOS_forcevariable = "0"

# image-live.bbclass will default INITRD_LIVE to the image INITRD_IMAGE creates.
# We want behavior to be consistent whether or not "live" is in IMAGE_FSTYPES, so
# we default INITRD_LIVE to the INITRD_IMAGE as well.
INITRD_IMAGE ?= "core-image-minimal-initramfs"
INITRD_LIVE ?= " ${@ ('${DEPLOY_DIR_IMAGE}/' + d.getVar('INITRD_IMAGE', expand=True) + '-${MACHINE}.cpio.gz') if d.getVar('INITRD_IMAGE', True) else ''}"

do_uefiapp[depends] += " \
                         intel-microcode:do_deploy \
                         systemd-boot:do_deploy \
                         virtual/kernel:do_deploy \
                       "

# INITRD_IMAGE is added to INITRD_LIVE, which we use to create our initrd, so depend on it if it is set
do_uefiapp[depends] += "${@ '${INITRD_IMAGE}:do_image_complete' if d.getVar('INITRD_IMAGE') else ''}"

# The image does without traditional bootloader.
# In its place, instead, it uses a single UEFI executable binary, which is
# composed by:
#   - an UEFI stub
#     The linux kernel can generate a UEFI stub, however the one from systemd-boot can fetch
#     the command line from a separate section of the EFI application, avoiding the need to
#     rebuild the kernel.
#   - the kernel
#   - an initramfs (optional)

def create_uefiapp(d, uuid=None, app_suffix=''):
    import glob, re
    from subprocess import check_call

    build_dir = d.getVar('B')
    deploy_dir_image = d.getVar('DEPLOY_DIR_IMAGE')
    image_link_name = d.getVar('IMAGE_LINK_NAME')

    cmdline = '%s/cmdline.txt' % build_dir
    linux = '%s/%s' % (deploy_dir_image, d.getVar('KERNEL_IMAGETYPE'))
    initrd = '%s/initrd' % build_dir

    stub_path = '%s/linux*.efi.stub' % deploy_dir_image
    stub = glob.glob(stub_path)[0]
    m = re.match(r"\S*(ia32|x64)(.efi)\S*", os.path.basename(stub))
    app = "boot%s%s%s" % (m.group(1), app_suffix, m.group(2))
    executable = '%s/%s.%s' % (deploy_dir_image, image_link_name, app)

    if d.getVar('INITRD_LIVE'):
        with open(initrd, 'wb') as dst:
            for cpio in d.getVar('INITRD_LIVE').split():
                with open(cpio, 'rb') as src:
                    dst.write(src.read())
        initrd_cmd = "--add-section .initrd=%s --change-section-vma .initrd=0x3000000 " % initrd
    else:
        initrd_cmd = ""

    root = 'root=PARTUUID=%s' % uuid if uuid else ''

    with open(cmdline, 'w') as f:
        f.write('%s %s' % (d.getVar('APPEND'), root))

    objcopy_cmd = ("objcopy "
        "--add-section .cmdline=%s --change-section-vma .cmdline=0x30000 "
        "--add-section .linux=%s --change-section-vma .linux=0x40000 "
        "%s %s %s") % \
        (cmdline, linux, initrd_cmd, stub, executable)

    check_call(objcopy_cmd, shell=True)

python create_uefiapps () {
    # We must clean up anything that matches the expected output pattern, to ensure that
    # the next steps do not accidentally use old files.
    import glob
    pattern = d.expand('${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.boot*.efi')
    for old_efi in glob.glob(pattern):
        os.unlink(old_efi)
    uuid = d.getVar('DISK_SIGNATURE_UUID')
    create_uefiapp(d, uuid=uuid)
}

# This is intentionally split into different parts. This way, derived
# classes or images can extend the individual parts. We can also use
# whatever language (shell script or Python) is more suitable.
python do_uefiapp() {
    bb.build.exec_func('create_uefiapps', d)
}

do_uefiapp[vardeps] += "APPEND DISK_SIGNATURE_UUID INITRD_LIVE KERNEL_IMAGETYPE IMAGE_LINK_NAME"

uefiapp_deploy_at() {
    dest=$1
    for i in ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.boot*.efi; do
        target=`basename $i`
        target=`echo $target | sed -e 's/${IMAGE_LINK_NAME}.//'`
        cp  --preserve=timestamps -r $i $dest/$target
    done
}

fakeroot do_uefiapp_deploy() {
    rm -rf ${IMAGE_ROOTFS}/boot/*
    dest=${IMAGE_ROOTFS}/boot/EFI/BOOT
    mkdir -p $dest
    uefiapp_deploy_at $dest
}

do_uefiapp_deploy[depends] += "${PN}:do_uefiapp virtual/fakeroot-native:do_populate_sysroot"


# This decides when/how we add our tasks to the image
python () {
    image_fstypes = d.getVar('IMAGE_FSTYPES', True)
    initramfs_fstypes = d.getVar('INITRAMFS_FSTYPES', True)

    # Don't add any of these tasks to initramfs images
    if initramfs_fstypes not in image_fstypes:
        bb.build.addtask('uefiapp', 'do_image', 'do_rootfs', d)
        bb.build.addtask('uefiapp_deploy', 'do_image', 'do_rootfs', d)
}

SIGN_AFTER ?= "do_uefiapp"
SIGN_BEFORE ?= "do_uefiapp_deploy"
SIGNING_DIR ?= "${DEPLOY_DIR_IMAGE}"
SIGNING_BINARIES ?= "${IMAGE_LINK_NAME}.boot*.efi"
inherit uefi-sign

# Legacy hddimg support below this line
efi_hddimg_populate() {
    uefiapp_deploy_at "$1"
}

IMAGE_FEATURES[validitems] += "secureboot"
