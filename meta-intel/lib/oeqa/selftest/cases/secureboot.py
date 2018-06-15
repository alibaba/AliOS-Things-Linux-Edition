#!/usr/bin/env python
# ex:ts=4:sw=4:sts=4:et
# -*- tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*-
#
# Copyright (c) 2017, Intel Corporation.
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# AUTHORS
# Mikko Ylinen <mikko.ylinen@linux.intel.com>
#
# Based on meta/lib/oeqa/selftest/* and meta-refkit/lib/oeqa/selftest/*

"""Test cases for secure boot with QEMU running OVMF."""

import os
import unittest
import re
import glob
from shutil import rmtree, copy

from oeqa.core.decorator.depends import OETestDepends
from oeqa.selftest.case import OESelftestTestCase
from oeqa.utils.commands import runCmd, bitbake, get_bb_var, get_bb_vars, runqemu

class SecureBootTests(OESelftestTestCase):
    """Secure Boot test class."""

    ovmf_keys_enrolled = False
    ovmf_qemuparams = ''
    ovmf_dir = ''
    test_image_unsigned = 'secureboot-selftest-image-unsigned'
    test_image_signed = 'secureboot-selftest-image-signed'
    correct_key = 'refkit-db'
    incorrect_key = 'incorrect'

    @classmethod
    def setUpLocal(self):

        if not SecureBootTests.ovmf_keys_enrolled:
            bitbake('ovmf ovmf-shell-image-enrollkeys', output_log=self.logger)

            bb_vars = get_bb_vars(['TMPDIR', 'DEPLOY_DIR_IMAGE'])

            SecureBootTests.ovmf_dir = os.path.join(bb_vars['TMPDIR'], 'oeselftest', 'secureboot', 'ovmf')
            bb.utils.mkdirhier(SecureBootTests.ovmf_dir)

            # Copy (all) OVMF in a temporary location
            for src in glob.glob('%s/ovmf.*' % bb_vars['DEPLOY_DIR_IMAGE']):
                copy(src, SecureBootTests.ovmf_dir)

            SecureBootTests.ovmf_qemuparams = '-drive if=pflash,format=qcow2,file=%s/ovmf.secboot.qcow2' % SecureBootTests.ovmf_dir

            cmd = ("runqemu "
                   "qemuparams='%s' "
                   "ovmf-shell-image-enrollkeys wic intel-corei7-64 "
                   "nographic slirp") % SecureBootTests.ovmf_qemuparams
            print('Running "%s"' % cmd)
            status = runCmd(cmd)

            if not re.search('info: success', status.output, re.M):
                self.fail('Failed to enroll keys. EFI shell log:\n%s' % status.output)
            else:
                # keys enrolled in ovmf.secboot.vars
                SecureBootTests.ovmf_keys_enrolled = True

    @classmethod
    def tearDownLocal(self):
        # Seems this is mandatory between the tests (a signed image is booted
        # when running test_boot_unsigned_image after test_boot_signed_image).
        # bitbake('-c clean %s' % test_image, output_log=self.logger)
        #
        # Whatever the problem was, it no longer seems to be necessary, so
        # we can skip the time-consuming clean + full rebuild (5:04 min instead
        # of 6:55min here).
        pass

    @classmethod
    def tearDownClass(self):
        bitbake('ovmf-shell-image-enrollkeys:do_cleanall', output_log=self.logger)
        rmtree(self.ovmf_dir, ignore_errors=True)

    def secureboot_with_image(self, boot_timeout=300, signing_key=None):
        """Boot the image with UEFI SecureBoot enabled and see the result. """

        config = ""

        if signing_key:
            test_image = self.test_image_signed
            config += 'SECURE_BOOT_SIGNING_KEY = "${THISDIR}/files/%s.key"\n' % signing_key
            config += 'SECURE_BOOT_SIGNING_CERT = "${THISDIR}/files/%s.crt"\n' % signing_key
        else:
            test_image = self.test_image_unsigned

        self.write_config(config)
        bitbake(test_image, output_log=self.logger)
        self.remove_config(config)

        # Some of the cases depend on the timeout to expire. Allow overrides
        # so that we don't have to wait 1000s which is the default.
        overrides = {
            'TEST_QEMUBOOT_TIMEOUT': boot_timeout,
            }

        print('Booting %s' % test_image)

        try:
            with runqemu(test_image, ssh=False,
                          runqemuparams='nographic slirp',
                          qemuparams=self.ovmf_qemuparams,
                          overrides=overrides,
                          image_fstype='wic') as qemu:

                cmd  = 'uname -a'

                status, output = qemu.run_serial(cmd)

                self.assertTrue(status, 'Could not run \'uname -a\' (status=%s):\n%s' % (status, output))

                # if we got this far without a correctly signed image, something went wrong
                if signing_key != self.correct_key:
                    self.fail('The image not give a Security violation when expected. Boot log:\n%s' % output)


        except Exception:

            # Currently runqemu() fails if 'login:' prompt is not seen and it's
            # not possible to login as 'root'. Those conditions aren't met when
            # booting to EFI shell (See [YOCTO #11438]). We catch the failure
            # and parse the boot log to determine the success. Note: the
            # timeout triggers verbose bb.error() but that's normal with some
            # of the test cases.

            workdir = get_bb_var('WORKDIR', test_image)
            bootlog = "%s/testimage/qemu_boot_log" % workdir

            with open(bootlog, "r") as log:

                # This isn't right but all we can do at this point. The right
                # approach would run commands in the EFI shell to determine
                # the BIOS rejects unsigned and/or images signed with keys in
                # dbx key store but that needs changes in oeqa framework.

                output = log.read()

                # PASS if we see a security violation on unsigned or incorrectly signed images, otherwise fail
                if signing_key == self.correct_key:
                    self.fail('Correctly signed image failed to boot. Boot log:\n%s' % output)
                elif not re.search('Security Violation', output):
                    self.fail('The image not give a Security violation when expected. Boot log:\n%s' % output)

    def test_boot_unsigned_image(self):
        """ Boot unsigned image with secureboot enabled in UEFI."""
        self.secureboot_with_image(boot_timeout=120, signing_key=None)

    @OETestDepends(['secureboot.SecureBootTests.test_boot_unsigned_image'])
    def test_boot_incorrectly_signed_image(self):
        """ Boot (correctly) signed image with secureboot enabled in UEFI."""
        self.secureboot_with_image(boot_timeout=120, signing_key=self.incorrect_key)

    @OETestDepends(['secureboot.SecureBootTests.test_boot_incorrectly_signed_image'])
    def test_boot_correctly_signed_image(self):
        """ Boot (correctly) signed image with secureboot enabled in UEFI."""
        self.secureboot_with_image(boot_timeout=150, signing_key=self.correct_key)
