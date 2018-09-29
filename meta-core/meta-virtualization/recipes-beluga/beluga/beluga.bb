SUMMARY = "beluga is based on moby project. It provide container technology to IoT. It is compatible with docker."
SECTION = "container"
VERSION = "1.13.1"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=aadc30f9c14d876ded7bedc0afd2d3d7"

S = "${WORKDIR}/git/src"
GO_IMPORT = "src"

SRC_URI = "file://beluga-git.tar.gz;subdir=${S} \
              file://${@bb.utils.contains('DISTRO_FEATURES','systemd','beluga.service','beluga.init',d)} \
              file://${@bb.utils.contains('DISTRO_FEATURES','systemd','beluga.service','beluga_prepare.sh',d)} \
              file://0001-fix-docker-top-command-failed.patch \
            "

PV = "beluga-${VERSION}"

inherit systemd update-rc.d
inherit go
inherit goarch
inherit pkgconfig

DEPENDS_remove_class-native = " go-cross-native"
DEPENDS_append_class-native = " go-native"

RRECOMMENDS_${PN} = "kernel-module-xt-conntrack kernel-module-xftrm-user kernel-module-xt-addrtype kernel-module-xt-nat kernel-module-xt-tcpudp"

INITSCRIPT_PACKAGES += "${@bb.utils.contains('DISTRO_FEATURES','sysvinit','${PN}','',d)}"
INITSCRIPT_NAME_${PN} = "${@bb.utils.contains('DISTRO_FEATURES','sysvinit','beluga.init','',d)}"
INITSCRIPT_PARAMS_${PN} = "defaults"

do_compile() {
    export GOHOSTOS="${HOST_GOOS}"
    export GOOS="${TARGET_GOOS}"
    export GOARCH="${TARGET_GOARCH}"
    export GOARM="${TARGET_GOARM}"
    export GO386="${TARGET_GO386}"

    cd ${S}
    rm -rf gopath
    mkdir -p gopath/src/github.com/docker/
    ln -sf ../../../.. gopath/src/github.com/docker/docker
    rm -f /tmp/gopath /tmp/go
    ln -sf ${S}/gopath /tmp/gopath
    ln -sf ${STAGING_DIR_NATIVE}/${nonarch_libdir}/${HOST_SYS}/go /tmp/go
    export GOPATH="/tmp/gopath"
    export GOROOT_FINAL="/opt"

    # Configuration for cgo
    export CGO_ENABLED="1"
    export CGO_CFLAGS="--sysroot=${STAGING_DIR_TARGET}"
    export CGO_LDFLAGS="--sysroot=${STAGING_DIR_TARGET}"
    # Disable unused fs driver
    export DOCKER_BUILDTAGS="exclude_graphdriver_btrfs exclude_graphdirver_zfs exclude_graphdriver_devicemapper exclude_graphdriver_aufs exclude_graphdriver_vfs"

    DOCKER_GITCOMMIT=${SRCREV} VERSION=${VERSION} CFLAGS="" LDFLAGS="" ./hack/make.sh dynbinary-beluga
    cp ${S}/bundles/${VERSION}/dynbinary-beluga/beluga-${VERSION} ${S}/bundles/${VERSION}/dynbinary-beluga/libbeluga.so
    ${CC} -O2 cmd/beluga_entry/main.c -o ${S}/bundles/${VERSION}/dynbinary-beluga/beluga-binary -I ${S}/bundles/${VERSION}/dynbinary-beluga -ldl

    rm /tmp/gopath
    rm /tmp/go
}

do_install() {
    install -d ${D}${bindir}
    install -d ${D}${systemd_system_unitdir}
    install -d ${D}${libdir}

    install -m 0755 ${S}/bundles/${VERSION}/dynbinary-beluga/libbeluga.so ${D}${libdir}/libbeluga.so
    install -m 0755 ${S}/bundles/${VERSION}/dynbinary-beluga/beluga-binary ${D}${bindir}/beluga-binary
    install -m 0755 ${S}/bundles/${VERSION}/dynbinary-beluga/containerd-shim-${VERSION} ${D}${bindir}/docker-containerd-shim
    install -m 0755 ${S}/bundles/${VERSION}/dynbinary-beluga/runc-${VERSION} ${D}${bindir}/docker-runc

    cd ${D}${bindir}
    cp beluga-binary docker-containerd
    cp beluga-binary docker-proxy
    cp beluga-binary dockerd
    cp beluga-binary docker

    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
         install -m 0644 ${WORKDIR}/beluga.service ${D}${systemd_system_unitdir}
    else
        install -d ${D}${sysconfdir}/init.d
        install -m 0744 ${WORKDIR}/beluga_prepare.sh ${D}${bindir}
        install -m 0755 ${WORKDIR}/beluga.init ${D}${sysconfdir}/init.d/beluga.init
    fi
}

RDEPENDS_${PN} = "curl util-linux iptables"
FILES_${PN} = "${bindir} ${libdir} ${sysconfdir}"
SYSTEMD_SERVICE_${PN} = "beluga.service"
