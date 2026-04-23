
Kernel 6.13 added the new ioctl PIDFD_GET_INFO.
Since glibc commit f8dac91ca3b4123e8c853b1bf6b126ad6829687e, pidfd_getpid() uses the new ioctl
