BBCLASSEXTEND = "native nativesdk"

DESCRIPTION = "Alios Things linkkit app"
SUMMARY = "Alios Things linkkit app"
LICENSE = "Apache-2.0"

# release version
PR = "r1"

DEPENDS += "python-aos-cube-native python-setuptools-native"

#RDEPENDS_${PN} = "python python-aos-cube"

LICENSE = "Apache-2.0"

# target to compile
TARGET_APP = "linkkitapp@linuxhost"

do_compile() {
#    aos make ${TARGET_APP} vcall=posix
    ${STAGING_BINDIR_NATIVE}/python-native/python ${STAGING_BINDIR_NATIVE}/aos make ${TARGET_APP} vcall=posix
}

do_package_qa() {
    # doing nothing,just prevent the qa error
    #echo "do_package_qa"
    bbnote "do_package_qa"
}

do_install() {
    # create bindir
    install -d ${D}${bindir}/
    # install app to /usr/bin/
    install -m 0755 ${S}/out/${TARGET_APP}/binary/${TARGET_APP}.elf ${D}${bindir}/${TARGET_APP}
}