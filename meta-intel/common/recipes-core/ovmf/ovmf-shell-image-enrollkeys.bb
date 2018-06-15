require recipes-core/ovmf/ovmf-shell-image.bb

WKS_SEARCH_PATH_append = ":${COREBASE}/meta/recipes-core/ovmf"

QB_DRIVE_TYPE = "/dev/vd"

do_image_append() {
    cat > ${IMAGE_ROOTFS}/startup.nsh << EOF
EnrollDefaultKeys
reset
EOF

}
