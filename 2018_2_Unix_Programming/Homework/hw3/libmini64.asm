
%macro gensys 2
    global sys_%2:function
sys_%2:
    push r10
    mov r10, rcx
    mov rax, %1
    syscall
    pop r10
    ret
%endmacro

; RDI, RSI, RDX, RCX, R8, R9

extern    errno

    section .data

    section .text

    gensys   0, read
    gensys   1, write
    gensys   2, open
    gensys   3, close
    gensys   9, mmap
    gensys  10, mprotect
    gensys  11, munmap
    gensys  22, pipe
    gensys  32, dup
    gensys  33, dup2
    gensys  34, pause
    gensys  35, nanosleep
    gensys  57, fork
    gensys  60, exit
    gensys  79, getcwd
    gensys  80, chdir
    gensys  82, rename
    gensys  83, mkdir
    gensys  84, rmdir
    gensys  85, creat
    gensys  86, link
    gensys  88, unlink
    gensys  89, readlink
    gensys  90, chmod
    gensys  92, chown
    gensys  95, umask
    gensys  96, gettimeofday
    gensys 102, getuid
    gensys 104, getgid
    gensys 105, setuid
    gensys 106, setgid
    gensys 107, geteuid
    gensys 108, getegid

    ; generate system calls in homework 3
    gensys 13, rt_sigaction
    gensys 14, rt_sigprocmask
    gensys 15, rt_sigreturn
    gensys 37, alarm
    gensys 127, rt_sigpending

    global open:function
open:
    call sys_open
    cmp rax, 0
    jge open_success    ; no error :)
open_error:
    neg rax
%ifdef NASM
    mov rdi, [rel errno wrt ..gotpc]
%else
    mov rdi, [rel errno wrt ..gotpcrel]
%endif
    mov [rdi], rax    ; errno = -rax
    mov rax, -1
    jmp open_quit
open_success:
%ifdef NASM
    mov rdi, [rel errno wrt ..gotpc]
%else
    mov rdi, [rel errno wrt ..gotpcrel]
%endif
    mov QWORD [rdi], 0    ; errno = 0
open_quit:
    ret

    ; setjmp, longjmp, restorer in homework 3
    global setjmp:function
setjmp:
    push r8
    push rsi
    push rdx
    push rcx
    mov r8, rdi
    mov rdi, 0
    mov rsi, 0
    lea rdx, [r8+64]
    mov rcx, 8
    call sys_rt_sigprocmask
    mov rdi, r8
    pop rcx
    pop rdx
    pop rsi
    pop r8

    mov [rdi], rbx
    push rdx
    lea rdx, [rsp+16]
    mov [rdi+8], rdx
    mov [rdi+16], rbp
    mov [rdi+24], r12
    mov [rdi+32], r13
    mov [rdi+40], r14
    mov [rdi+48], r15
    mov rdx, [rsp+8]
    mov [rdi+56], rdx
    pop rdx
    xor rax, rax
    ret

    global longjmp:function
longjmp:
    push r8
    push rsi
    push rdx
    push rcx
    mov r8, rdi
    mov rdi, 2
    lea rsi, [r8+64]
    mov rdx, 0
    mov rcx, 8
    call sys_rt_sigprocmask
    mov rdi, r8
    pop rcx
    pop rdx
    pop rsi
    pop r8

    mov rax, rsi
    test rax, rax
    jnz longjmp2
    inc rax
longjmp2:
    mov rbx, [rdi]
    mov rsp, [rdi+8]
    mov rbp, [rdi+16]
    mov r12, [rdi+24]
    mov r13, [rdi+32]
    mov r14, [rdi+40]
    mov r15, [rdi+48]
    jmp [rdi+56]

    global restorer:function
restorer:
    mov rax, 15
    syscall