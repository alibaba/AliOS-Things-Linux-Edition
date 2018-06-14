PACKAGECONFIG_append += "\
    ${@bb.utils.contains('DISTRO_FEATURES', 'ima', \
                         'ima', '', d)} \
"
