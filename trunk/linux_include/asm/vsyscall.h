#ifndef __X86_64_ASM_STUB__VSYSCALL_H__
#define __X86_64_ASM_STUB__VSYSCALL_H__
#ifdef __x86_64__
#include <asm-x86_64/vsyscall.h>
#else
#warning vsyscall.h is not supported on x86
#endif
#endif
