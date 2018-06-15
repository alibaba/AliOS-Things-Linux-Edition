# The kernel build is 64-bit regardless, so include both common overrides.
# Without this, the kernel will be missing vars that make it buildable for the
# intel-corei7-64 machine.
MACHINEOVERRIDES_prepend_corei7-64-x32-intel-common = "corei7-64-intel-common:"
