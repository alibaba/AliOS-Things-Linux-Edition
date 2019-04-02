do_configure_libc-bionic () {
    cd util
    perl perlpath.pl ${STAGING_BINDIR_NATIVE}
    cd ..
    ln -sf apps/openssl.pod crypto/crypto.pod ssl/ssl.pod doc/

    os=${HOST_OS}
    case $os in
    linux-gnueabi |\
    linux-gnuspe |\
    linux-musleabi |\
    linux-muslspe |\
    linux-musl |\
    linux-android )
        os=linux
        ;;
        *)
        ;;
    esac
    target="$os-${HOST_ARCH}"
    case $target in
    linux-arm)
        target=linux-armv4
        ;;
    linux-armeb)
        target=linux-elf-armeb
        ;;
    linux-aarch64*)
        target=linux-aarch64
        ;;
    linux-sh3)
        target=debian-sh3
        ;;
    linux-sh4)
        target=debian-sh4
        ;;
    linux-i486)
        target=debian-i386-i486
        ;;
    linux-i586 | linux-viac3)
        target=debian-i386-i586
        ;;
    linux-i686)
        target=debian-i386-i686/cmov
        ;;
    linux-gnux32-x86_64 | linux-muslx32-x86_64 )
        target=linux-x32
        ;;
    linux-gnu64-x86_64)
        target=linux-x86_64
        ;;
    linux-gnun32-mips*el)
        target=debian-mipsn32el
        ;;
    linux-gnun32-mips*)
        target=debian-mipsn32
        ;;
    linux-mips*64*el)
        target=debian-mips64el
        ;;
    linux-mips*64*)
        target=debian-mips64
        ;;
    linux-mips*el)
        target=debian-mipsel
        ;;
    linux-mips*)
        target=debian-mips
        ;;
    linux-microblaze*|linux-nios2*|linux-gnu*ilp32**)
        target=linux-generic32
        ;;
    linux-powerpc)
        target=linux-ppc
        ;;
    linux-powerpc64)
        target=linux-ppc64
        ;;
    linux-supersparc)
        target=linux-sparcv8
        ;;
    linux-sparc)
        target=linux-sparcv8
        ;;
    darwin-i386)
        target=darwin-i386-cc
        ;;
    esac
    # inject machine-specific flags
    sed -i -e "s|^\(\"$target\",\s*\"[^:]\+\):\([^:]\+\)|\1:${CFLAG}|g" Configure
        useprefix=${prefix}
        if [ "x$useprefix" = "x" ]; then
                useprefix=/
        fi
    perl ./Configure ${EXTRA_OECONF} shared --prefix=$useprefix --openssldir=${libdir}/ssl --libdir=`basename ${libdir}` $target
}
