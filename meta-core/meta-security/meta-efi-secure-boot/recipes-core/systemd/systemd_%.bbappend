DEPENDS += "\
    ${@bb.utils.contains('MACHINE_FEATURES', 'efi', 'gnu-efi', '', d)} \
"

EXTRA_OECONF += "\
    ${@bb.utils.contains('MACHINE_FEATURES', 'efi', \
                         '--enable-efi --enable-gnuefi --with-efi-libdir=${STAGING_LIBDIR} --with-efi-ldsdir=${STAGING_LIBDIR} --with-efi-includedir=${STAGING_INCDIR}', \
                         '', d)} \
"
