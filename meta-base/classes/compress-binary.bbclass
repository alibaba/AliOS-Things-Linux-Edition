COMPRESS_BINARY_FILES ?= ""

DEPENDS_append = " upx-native"

UPX ?= "${STAGING_BINDIR_NATIVE}/upx"

do_compress_binary() {
    if [ -z ${COMPRESS_BINARY_FILES} ]; then
        bbdebug "COMPRESS_BINARY_FILES was empty."
    else
        for binary in ${COMPRESS_BINARY_FILES}; do
            exec=${PKGD}$binary
                if [ -x $exec  ]; then
                    ${UPX} --lzma "$exec"
                else
                    bbfatal "$exec not found"
                fi
        done
        fi
}

PACKAGEBUILDPKGD += "do_compress_binary"

