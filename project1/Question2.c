#include <stdio.h>
#include <sys/syscall.h> // 提供 syscall 函數
#include <unistd.h>      // 提供標準系統函數

// ANSI 顏色碼
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

// Syscall 449 的接口
unsigned long my_get_physical_addresses(void *vaddr) {
    return (unsigned long) syscall(449, &vaddr);
}

// 打印邏輯地址與物理地址
void print_physical_address(int *addr, const char *label) {
    unsigned long phy_add = my_get_physical_addresses(addr);
    printf("%s:\n", label);
    printf("Offset of logical address: [" GREEN "%p" RESET "]   Physical address: [" BLUE "0x%lx" RESET "]\n", (void *)addr, phy_add);
    printf("==============================================================================\n");
}

int a[2000000]; // 定義一個大陣列

int main() {
    print_physical_address(&a[0], "global element " RED "a[0]" RESET);

    printf("\na[1999999]未存取：\n");
    print_physical_address(&a[1999999], "global element " RED "a[1999999]" RESET);

    a[1999999] = 1;
    printf("a[1999999]已存取：\n");
    print_physical_address(&a[1999999], "global element " RED "a[1999999]" RESET);

    printf("\n");
    print_physical_address(&a[1999999 - 1], "global element " RED "a[1999999 - 1]" RESET);
    print_physical_address(&a[1999999 - 143], "global element " RED "a[1999999 - 143]" RESET);
    print_physical_address(&a[1999999 - 144], "global element " RED "a[1999999 - 144]" RESET);

    a[1999999 - 144] = 2;
    printf("\na[1999999 - 144]已存取：\n");
    print_physical_address(&a[1999999 - 144], "global element " RED "a[1999999 - 144]" RESET);
    print_physical_address(&a[1999999 - 144 - 1023], "global element " RED "a[1999999 - 144 - 1023]" RESET);
    print_physical_address(&a[1999999 - 144 - 1024], "global element " RED "a[1999999 - 144 - 1024]" RESET);

    return 0;
}
