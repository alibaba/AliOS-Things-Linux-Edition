FILESEXTRAPATHS_prepend := "${THISDIR}/ltp:"

inherit module-base

addtask make_scripts after do_prepare_recipe_sysroot before do_configure

EXTRA_OEMAKE += "KERNEL_SRC=${STAGING_KERNEL_DIR}"

DEPENDS += " keyutils"
SRC_URI += "file://0001-disable-16bit-syscall-tests.patch \
            file://0002-disable-arch-cases.patch \
            file://0003-disable-numa-test-cases.patch \
            file://0004-disable-unsupported-syscall-cases.patch \
            file://0005-disable-oom-cases.patch \
            file://0006-disable-driver-cases-runtest-kernel_misc.patch \
            file://0007-distinguish-funtional-stress-skipped-and-notmpfs.patch \
            file://0008-disable-can-swap-cpuhotplug-and-isofs.patch \
            file://0009-fix-TCONF-to-TPASS.patch \
            file://0010-disable-cases.patch \
            file://0011-modify-ar01-and-tar01-scripts.patch \
            file://0012-fix-arguments-in-netns_helper.sh.patch \
            file://0013-disable-4-netns_comm-netlink-cases.patch \
            file://0014-disable-strange-cases.patch \
            file://0015-move-syscalls-ipc-from-functional-to-skipped.patch \
            file://0016-disable-dio4-dio10-getxattr04.patch \
            file://0017-add-func_test.sh-and-stress_test.sh.patch \
            file://0018-modify-Makefile-of-insmod-dummy_del_mod-dummy_del_mo.patch \
            file://0019-ignore-signal-32-33-and-34-when-GLIBC-is-not-used.patch \
            file://0020-move-chdir01A-gf01-gf14-gf15-gf18-to-notmpfs.others.patch \
            file://0021-disable-unsupported-syscalls.patch \
            file://0022-disable-arch-mismatch-cases.patch \
            file://0023-disable-rtc01-and-cpufreq_boost-in-kernel_misc.patch \
            file://0024-disable-memory-too-small-cases.patch \
            file://0025-disable-space-too-small-cases.patch \
            file://0026-disable-openat02-case.patch \
            file://0027-disable-strange-cases.patch \
            file://0028-move-crashme-from-functional-to-skipped.patch \
            file://0029-disable-madvise01-and-fanotify03.patch \
            file://0030-move-controllers-and-hugetlb-from-functional-to-skip.patch \
            file://0031-move-pids-cases-to-front-in-controllers.patch \
            file://0032-rm-cve-2016-4997.c-when-LIBC-musl.patch \
            file://0033-add-musl_patch.patch \
            file://0034-disable-cases-for-adb.patch \
           "

do_install_append(){
    # Copy functional,stress,notmpfs,skipped to /opt/ltp manually
    install functional ${D}/opt/ltp/
    install stress ${D}/opt/ltp/
    install notmpfs ${D}/opt/ltp/
    install skipped ${D}/opt/ltp/
    install func_test.sh ${D}/opt/ltp/
    install stress_test.sh ${D}/opt/ltp/
    install musl_patch ${D}/opt/ltp/
    install testcases/commands/insmod/insmod01.sh ${D}/opt/ltp/testcases/bin
    install testcases/commands/insmod/ltp_insmod01.ko ${D}/opt/ltp/testcases/bin
    install testcases/kernel/module/dummy_del_mod/dummy_del_mod.ko ${D}/opt/ltp/testcases/bin
    install testcases/kernel/module/dummy_del_mod_dep/dummy_del_mod_dep.ko ${D}/opt/ltp/testcases/bin
}
