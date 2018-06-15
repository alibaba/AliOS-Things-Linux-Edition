do_install_armmultilib () {

        oe_multilib_header bits/endian.h bits/fcntl.h bits/fenv.h  bits/hwcap.h bits/ipc.h bits/link.h bits/wordsize.h
        oe_multilib_header bits/local_lim.h bits/mman.h bits/msq.h bits/pthreadtypes.h  bits/sem.h  bits/semaphore.h bits/setjmp.h
        oe_multilib_header bits/shm.h bits/sigstack.h bits/stat.h bits/statfs.h bits/typesizes.h

        oe_multilib_header fpu_control.h gnu/lib-names.h gnu/stubs.h ieee754.h

        oe_multilib_header sys/elf.h sys/procfs.h sys/ptrace.h sys/ucontext.h sys/user.h
}
