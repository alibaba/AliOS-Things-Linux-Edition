SUMMUARY = "Package for Moby"
DESCRIPTION = "Moby is an open-source project created by Docker to enable and \
                   accelerate software containerization. This package integrate many \
                   container tools into moby tree, like containerd, runc, docker CLI \
                   and compose"
SECTION = "Moby"
VERSION = "18.05"

LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/Apache-2.0;md5=89aea4e17d99a7cacdbeed46a0096b10"

S = "${WORKDIR}/git/src"
GO_IMPORT = "src"

SRCREV  = "8e2f9203065987116aec9e2d2a1d5c7039e1a5d4"
SRC_URI = " \
              file://moby-git.tar.gz;subdir=${S} \
              file://moby.init \
              file://moby.service \
              file://moby_prepare.sh \
              file://check-config.sh \
		   "

PV = "Moby+git${SRCREV}"

inherit systemd update-rc.d
inherit go
inherit goarch
inherit pkgconfig

DEPENDS_remove_class-native = " go-cross-native"
DEPENDS_append_class-native = " go-native"

RRECOMMENDS_${PN} = "kernel-module-xt-conntrack kernel-module-xftrm-user kernel-module-xt-addrtype tini"

INITSCRIPT_PACKAGES += "${@bb.utils.contains('DISTRO_FEATURES','sysvinit','${PN}','',d)}"
INITSCRIPT_NAME_${PN} = "${@bb.utils.contains('DISTRO_FEATURES','sysvinit','moby.init','',d)}"
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
    export GOPATH="${S}/gopath:${S}/vendor:${STAGING_DIR_TARGET}/${prefix}/lib/go"
    export GOROOT="${STAGING_DIR_NATIVE}/${nonarch_libdir}/${HOST_SYS}/go"

    # Configuration for cgo
    export CGO_ENABLED="1"
    export CGO_CFLAGS="--sysroot=${STAGING_DIR_TARGET}"
    export CGO_LDFLAGS="--sysroot=${STAGING_DIR_TARGET}"
    # Disable do not use fs driver 
    export DOCKER_BUILDTAGS="exclude_graphdriver_btrfs exclude_graphdirver_zfs exclude_graphdriver_devicemapper"

    DOCKER_GITCOMMIT=${SRCREV} VERSION=${VERSION} CFLAGS="" LDFLAGS="" ./hack/make.sh dynbinary-moby
}

do_install() {
    install -d ${D}${bindir}
    install -d ${D}${systemd_system_unitdir}

    install -m 0755 ${S}/bundles/dynbinary-moby/moby-${VERSION} ${D}${bindir}/moby
    install -m 0755 ${S}/bundles/dynbinary-moby/runc-${VERSION} ${D}${bindir}/docker-runc
    install -m 0755 ${S}/bundles/dynbinary-moby/containerd-${VERSION} ${D}${bindir}/docker-containerd
    install -m 0755 ${S}/bundles/dynbinary-moby/containerd-shim-${VERSION} ${D}${bindir}/docker-containerd-shim
    install -m 0755 ${S}/bundles/dynbinary-moby/docker-${VERSION} ${D}${bindir}/docker

    cd ${D}${bindir}
    ln -sf moby docker-compose 
    ln -sf moby dockerd
    ln -sf moby docker-proxy
    ln -sf docker-containerd docker-containerd-ctr
    if ${@bb.utils.contains('DISTRO_FEATURES','systemd','true','false',d)}; then
        install -m 0644 ${WORKDIR}/moby.service ${D}${systemd_system_unitdir}
        install -m 0755 ${WORKDIR}/check-config.sh ${D}${bindir}
    else
        install -d ${D}${sysconfdir}/init.d
        install -m 0744 ${WORKDIR}/moby_prepare.sh ${D}${bindir}
        install -m 0755 ${WORKDIR}/moby.init ${D}${sysconfdir}/init.d/moby.init
    fi
}

inherit useradd
USERADD_PACKAGES = "${PN}"
GROUPADD_PARAM_${PN} = "-r docker"

RDEPENDS_${PN} = "curl util-linux iptables"
FILES_${PN} = "${bindir} ${sysconfdir}"
SYSTEMD_SERVICE_${PN} = "moby.service"
