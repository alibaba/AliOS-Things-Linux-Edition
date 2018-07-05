DESCRIPTION = "Alios Things Linux Edtion linkkitapp"
SUMMARY = "Alios Things Linux Edtion linkkitapp"
LICENSE = "Apache-2.0"

# release version
PR = "r1"

FILES_${PN} += "${datadir} ${datadir}/linkkit/ ${datadir}/linkkit/linkkit.kv"
#inherit autotools-brokensep update-rc.d systemd useradd
#inherit update-rc.d systemd

#INITSCRIPT_NAME = "linkkit"
#INITSCRIPT_PARAMS = "defaults 87"

# set ARCH according to MACHINE
python() {
    getmachine = d.getVar("MACHINE", True)
    bb.note("%s" % getmachine)
    if getmachine == "qemux86-64":
        d.setVar("LINKKIT_ARCH", "x86")
    elif getmachine == "raspberrypi3-64":
        d.setVar("LINKKIT_ARCH", "arm64")
    else:
        bb.note("not support")
    bb.note("set ARCH to %s" % d.getVar("LINKKIT_ARCH", True))
}

do_compile() {
    make ARCH=${LINKKIT_ARCH}
}

do_package_qa() {
    # doing nothing,just prevent the qa error
    #echo "do_package_qa"
    bbnote "do_package_qa"
}

do_install() {
    # create linkkit dir
    install -d ${D}/${bindir}/
    install -d ${D}/${datadir}/linkkit/
    install -d ${D}/${sysconfdir}/init.d/
    install -m 0755 ${THISDIR}/linkkit.kv ${D}/${datadir}/linkkit/linkkit.kv
    install -m 0755 ${THISDIR}/linkkit.init ${D}/${sysconfdir}/init.d/linkkit
    install -m 0755 ${S}/linkkitapp ${D}/${bindir}/linkkitapp
}