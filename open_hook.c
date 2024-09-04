#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example Author");
MODULE_DESCRIPTION("Hook open, openat, and openat2 syscalls to add sleep before the actual call");

typedef asmlinkage long (*sys_open_t)(const char __user *, int, umode_t);
typedef asmlinkage long (*sys_openat_t)(int, const char __user *, int, umode_t);
typedef asmlinkage long (*sys_openat2_t)(int, const char __user *, struct open_how *, size_t);

static sys_open_t original_sys_open;
static sys_openat_t original_sys_openat;
static sys_openat2_t original_sys_openat2;
static void **sys_call_table;

static asmlinkage long hooked_open(const char __user *filename, int flags, umode_t mode) {
    printk(KERN_INFO "hooked_open: Sleeping before actual open\n");
    //ssleep(100);  // Sleep for 100 seconds
    return original_sys_open(filename, flags, mode);
}

static asmlinkage long hooked_openat(int dfd, const char __user *filename, int flags, umode_t mode) {
    printk(KERN_INFO "hooked_openat: Sleeping before actual openat\n");
    //ssleep(100);  // Sleep for 100 seconds
    return original_sys_openat(dfd, filename, flags, mode);
}

static asmlinkage long hooked_openat2(int dfd, const char __user *filename, struct open_how *how, size_t size) {
    printk(KERN_INFO "hooked_openat2: Sleeping before actual openat2\n");
    //ssleep(100);  // Sleep for 100 seconds
    return original_sys_openat2(dfd, filename, how, size);
}

inline void mywrite_cr0(unsigned long val) {
    asm volatile("mov %0,%%cr0" : : "r"(val) : "memory");
}

static void enable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    mywrite_cr0(cr0);
}

static void disable_write_protection(void) {
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    mywrite_cr0(cr0);
}

static int __init syscall_hook_init(void) {
    sys_call_table = (void **)0xffffffffaf200380;

    if (!sys_call_table) {
        printk(KERN_ALERT "Failed to locate sys_call_table\n");
        return -1;
    }

    // 保存されたシステムコールのポインタをバックアップ
    original_sys_open = (sys_open_t)sys_call_table[__NR_open];
    original_sys_openat = (sys_openat_t)sys_call_table[__NR_openat];
    original_sys_openat2 = (sys_openat2_t)sys_call_table[__NR_openat2];

    // システムコールテーブルの書き込み保護を無効にして、フックを設定
    disable_write_protection();
    sys_call_table[__NR_open] = (void *)hooked_open;
    sys_call_table[__NR_openat] = (void *)hooked_openat;
    sys_call_table[__NR_openat2] = (void *)hooked_openat2;
    enable_write_protection();

    printk(KERN_INFO "Syscall hooks installed.\n");
    return 0;
}

static void __exit syscall_hook_exit(void) {
    // システムコールテーブルの書き込み保護を無効にして、元に戻す
    disable_write_protection();
    sys_call_table[__NR_open] = (void *)original_sys_open;
    sys_call_table[__NR_openat] = (void *)original_sys_openat;
    sys_call_table[__NR_openat2] = (void *)original_sys_openat2;
    enable_write_protection();

    printk(KERN_INFO "Syscall hooks removed.\n");
}

module_init(syscall_hook_init);
module_exit(syscall_hook_exit);

