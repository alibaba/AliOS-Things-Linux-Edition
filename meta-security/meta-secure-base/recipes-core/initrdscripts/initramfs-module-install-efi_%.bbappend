FILESEXTRAPATHS_prepend := "${@ bb.utils.contains(\
    'IMAGE_FEATURES', 'secureboot', '', '${THISDIR}/files:', d)}"
