SUMMARY = "A performance and memory usage benchmark for comparing libc implementation"
HOMEPAGE = "https://www.etalabs.net/libc-bench.html"

LICENSE="MIT"
LIC_FILES_CHKSUM = "file://${S}/COPYRIGHT;md5=9a825c63897c53f487ef900598c31527"

SRC_URI = "git://git.musl-libc.org/libc-bench"
SRC_URI += "file://0001-Remove-default-flags.patch"
SRCREV = "b6b2ce5f9f87a09b14499cb00c600c601f022634"

S = "${WORKDIR}/git"

CFLAGS += "-fPIE"
LDFLAGS += "-pie"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${S}/libc-bench ${D}${bindir}
}
