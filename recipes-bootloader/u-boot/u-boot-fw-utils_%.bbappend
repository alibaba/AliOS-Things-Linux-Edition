# Keep this separately from the rest of the .bb file in case that .bb file is
# overridden from another layer.

require ${@oe.utils.all_distro_features(d, 'uota', 'u-boot-fw-utils-mender.inc')}
