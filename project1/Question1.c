#include <stdio.h>
#include <unistd.h>   // 提供 fork, sleep 等函數
#include <sys/syscall.h> // 提供 syscall 函數
#include <sys/wait.h> // 提供 wait 函數

// ANSI 控制碼定義
#define RED "\033[31m"     // 紅色
#define YELLOW "\033[33m"  // 黃色
#define GREEN "\033[32m"   // 綠色
#define BLUE "\033[34m"    // 藍色
#define RESET "\033[0m"    // 重設顏色

// Syscall 449 的接口
void *my_get_physical_addresses(void *vaddr) {
    return (void *)syscall(449, &vaddr);
}

int global_a = 123;  // global variable

int main()
{
    void *parent_use, *child_use;

    printf("=================================Before Fork========================================\n");
    parent_use = my_get_physical_addresses(&global_a);
    printf("global_a: " RED "%d" RESET "  |  pid = " YELLOW "%d" RESET "\n", global_a, getpid());
    printf("Offset of logical address:[%s%p%s]   Physical address:[%s%p%s]\n", GREEN, (void *)&global_a, RESET, BLUE, parent_use, RESET);
    printf("====================================================================================\n\n");

    if (fork())
    { /*parent code*/
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvv  After Fork by parent  vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
        parent_use = my_get_physical_addresses(&global_a);
        printf("******* ）parent global_a: " RED "%d" RESET "  |  pid = " YELLOW "%d" RESET "\n", global_a, getpid());
        printf("******* Offset of logical address:[%s%p%s]   Physical address:[%s%p%s]\n", GREEN, (void *)&global_a, RESET, BLUE, parent_use, RESET);
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\n");
        wait(NULL);  // 等待子進程結束
    }
    else
    { /*child code*/
        sleep(1); // 睡1秒鐘，讓printf更整齊
        printf("llllllllllllllllllllllllllll  After Fork by child  lllllllllllllllllllllllllllllllll\n");
        child_use = my_get_physical_addresses(&global_a);
        printf("******* child global_a: " RED "%d" RESET "  |  pid = " YELLOW "%d" RESET "\n", global_a, getpid());
        printf("******* Offset of logical address:[%s%p%s]   Physical address:[%s%p%s]\n", GREEN, (void *)&global_a, RESET, BLUE, child_use, RESET);
        printf("llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll\n");
        printf("____________________________________________________________________________________\n\n");

        /*----------------------- trigger CoW (Copy on Write) -----------------------------------*/
        global_a = 789;

        printf("iiiiiiiiiiiiiiiiiiiiiiiiiiii  Test copy on write in child  iiiiiiiiiiiiiiiiiiiiiiiii\n");
        child_use = my_get_physical_addresses(&global_a);
        printf("******* child global_a: " RED "%d" RESET "  |  pid = " YELLOW "%d" RESET "\n", global_a, getpid());
        printf("******* Offset of logical address:[%s%p%s]   Physical address:[%s%p%s]\n", GREEN, (void *)&global_a, RESET, BLUE, child_use, RESET);
        printf("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
        printf("____________________________________________________________________________________\n\n");
    }
    return 0;
}
