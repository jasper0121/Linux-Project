# Linux 作業系統 project1

## 前置準備
### 作業題目網址
https://staff.csie.ncu.edu.tw/hsufh/COURSES/FALL2024/linux_project_1.html

### 系統環境
* 作業系統： ubuntu 22.04
* Kernel 版本： 5.15.137


### 想要用superuser權限的vscode開發的話
:::info
```
sudo code --no-sandbox --user-data-dir
```
:::


## 建立自訂syscall並編譯
### 下載 Kernel Source
```b
# 進入 root 模式
sudo su

# 下載kernel
wget -P ~/ https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.137.tar.xz

# 解壓縮到 /usr/src 目錄底下
tar -xvf linux-5.15.137.tar.xz -C /usr/src
```

### 安裝Compile工具
```b
# 更新本地的軟體包數據庫，使系統獲取最新的軟體包信息
sudo apt update

# 檢查本地已安裝的軟體包，並嘗試將它們升級到軟體源中可用的最新版本。
sudo apt upgrade

# build-essential：包含基本編譯工具
# libncurses-dev：用於控制終端窗口顯示的應用程式
# libssl-dev：用於開發需要進行加密或使用安全通訊協議（如 HTTPS）的應用程式
# libelf-dev：用於與 ELF 文件格式相關的開發，比如分析或處理可執行文件、目標文件和共享庫
# bison 和 flex：語法解析器生成器 and 詞法分析器生成器
sudo apt install build-essential libncurses-dev libssl-dev libelf-dev bison flex -y

# 清除安裝的package
sudo apt clean && sudo apt autoremove -y
```

### 新增syscall
```bash
# 進入剛載的linux-5.15.137裡面
cd /usr/src/linux-5.15.137/

# 建立一個名為mygo...叫mycall的資料夾
mkdir mycall
```

1. 在mycall底下建立my_get_physical_addresses.c並新增以下code，之後記得存檔
:::info
[虛擬地址(virtual address)轉實體地址(physical address)程式碼](#Requirement：虛擬地址virtual-address轉實體地址physical-address)
:::

2. 接著繼續在mycall內新增Makefile
```b
obj-y := my_get_physical_addresses.o # 將my_get_physical_addresses.o編入kernel
```
3. **Makefile是一種用來自動化編譯過程的配置檔案，通常與 make 工具搭配使用**。Makefile 定義了如何編譯和鏈接程式碼，以生成可執行檔、庫或其他輸出，並可以設定不同的編譯規則和依賴關係。
4. 以上Makefile指令中：
    * <font color=red>obj-y</font>是用於內核編譯系統中的一個變數，它表示某個 .o 文件（即編譯後的目標文件）會被 內嵌（linked statically） 到內核映像(kernel image)中。
    * <font color=red>:=</font>是賦值運算符，意思是將右側的值（在此例中是 my_get_physical_addresses.o）賦給左側變數（即 obj-y）。
    * <font color=red>my_get_physical_addresses.o</font>是一個目標文件，它由相應的源文件（通常是 my_get_physical_addresses.c）經過編譯後生成。.o 文件是編譯過程中的中間產物，代表編譯後的二進制對象。

### 修改linux-5.15.137本身的Makefile
1. 回到/usr/src/linux-5.15.137並找到其底下的Makefile。
2. 若使用**vscode可直接ctrl+f找core-y字段**，若用vim編輯的話則可輸入 **?core-y**來查找，位置大約在1162行。
3. 找到後在該句後面新增 **mycall/**，具體如下：
```b
原指令：core-y			+= kernel/ certs/ mm/ fs/ ipc/ security/ crypto/

修改成：core-y			+= kernel/ certs/ mm/ fs/ ipc/ security/ crypto/ mycall/
```
4. <font color=red>core-y</font> 是 Linux 核心編譯系統中的一個變數，通常用來定義需要編譯的內核核心部分的子目錄。這些目錄中的代碼會被編譯並靜態鏈接到最終的內核映像中。
5. 這行指令告訴編譯系統，當編譯 Linux 核心時，還需要編譯這些目錄中的代碼並將它們靜態鏈接到內核映像中。換句話說，kernel/、certs/、mm/、fs/、ipc/、security/ 和 crypto/ 這些目錄中的代碼都是內核核心功能的一部分，它們的內容必須被編譯並最終成為內核的一部分。
6. 因此，若我們今天要編譯其他自定義內容，就要在後面追加子目錄，即mycall。

### 將系統調用函數（syscall）的宣告添加到 Linux 核心中
1. 進到 **/usr/src/linux-5.15.137/include/linux/syscalls.h**
2. 在syscalls.h最下面的 **#endif**之前添加以下程式碼：
```c
asmlinkage long sys_my_get_physical_addresses(unsigned long __user *usr_ptr);
```
3. **asmlinkage**的意思是告訴編譯器，這個函數的參數是通過**stack**傳遞的，而不是像普通內核函數那樣通過寄存器傳遞。在 Linux 系統調用中，參數必須從用戶空間（user space）傳遞到內核空間（kernel space），並使用stack來傳遞參數。使用 asmlinkage 確保系統調用能正確接收這些參數，並能跨平台運行。基本上要寫syscall的話都要前綴這個關鍵詞。
4. **long**是函數的返回類型，表示該函數將返回一個 long 整數。
5. **sys_my_get_physical_addresses**是函數的名稱，表明這是一個名為 sys_my_get_physical_addresses 的系統調用。根據命名慣例，sys_ 前綴通常用於標識內核中的系統調用。這個特定的系統調用的功能是獲取物理地址，具體功能將在函數的實現中定義。
6. **unsigned long __user *usr_ptr**是函數的參數。這裡 unsigned long 表示這個指針將指向一個無符號長整型值，而 __user 修飾符表明這個指針指向用戶空間的內存。通過這個指針，系統調用可以訪問user space提供的數據。在kernel中任何訪問user space的數據都需要使用 __user 修飾符來強調這一點，以避免潛在的安全問題。

### 將syscall加入到kernel的syscall table
1. 進到 **/usr/src/linux-5.15.137/arch/x86/entry/syscalls/syscall_64.tbl**
2. 拉到約372行，該區塊編號應該到448，接著下個區塊就從編號512開始，我們在編號448指令的下一行新增449號指令：
```bash
449	common	my_get_physical_addresses		sys_my_get_physical_addresses
```
3. 該指令意義如下：
    * **449**是系統調用編號。(編號可自訂，但注意不要和其他syscall重複)
    * **common**是類型或分類。這通常是用來表示系統調用的類型或屬性。在這個上下文中，common 表示這個系統調用是通用的，可能適用於多個不同的架構或情況。
    * **my_get_physical_addresses**是用戶空間(user space)接口名稱。這是用戶空間中調用的系統調用名稱。當應用程序需要訪問這個系統調用時，它會使用這個名稱來進行調用。這個名稱通常用於用戶空間的函數庫中，應用程序可以通過它來執行系統調用。
    * **sys_my_get_physical_addresses**是內核空間(kernel space)實現名稱。這是實際在內核中實現的系統調用函數的名稱。這個函數處理來自用戶空間的請求，並執行相應的操作。

### 編譯kernel
1. 首先先清理上次編譯(如果有)時產生的各種編譯過程中生成的文件，以便用於全新的編譯或重新配置內核。
```b
sudo make mrproper
```
2. 準備一個新的 Linux 內核構建環境，通過使用當前運行的內核配置來生成適合當前系統的內核配置，以便於後續的內核編譯和優化。
```bash!
# 先回到目標kernel
cd /usr/src/linux-5.15.137/

# 複製當前linux正在使用的kernel的config文件(即使用uname -r)到當前目錄
cp -v /boot/config-$(uname -r) .config

# 將剛剛複製過來的config自動更新此kernel(即linux-5.15.137)的.config。使用 localmodconfig比較快。過程問你什麼都直按Enter就好
make localmodconfig
```
3. 在開始編譯之前，要先在cmd輸入以下指令：
```b
# 刪除憑證
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
```
否則將會出現以下錯誤訊息：
:::danger
sed: can't read modules.order: No such file or directory  
make: *** [Makefile:1544: __modinst_pre] Error 2
:::

原因據其他參考網站是說設定檔內是Debian 官方當初編譯 kernel 時憑證的路徑，若是直接編譯會報錯，因此這邊取消使用憑證，並將值設為空字串。

4. 開始編譯
```bash!
# 查幾核心，據其他參考網站說不要使用全部核心去跑，用1/2 ~ 3/4去跑就好，比較不會當機(ex:16核心的話就用8~12核心去編譯)
nproc

# 用12核心去編譯kernel
make -j12
```

5. 在 Linux 系統上安裝編譯過的內核模組，並且通過並行處理來提高安裝速度。執行**make modules_install -j12**後，內核模組會被安裝到 /lib/modules/ 目錄下，這樣系統就可以在啟動時或運行時加載這些模組以支持相關的硬體和功能。
```bash!
sudo make modules_install -j12

# 上面指令跑完後沒有出現錯誤訊息，就可以繼續安裝kernel。
sudo make install -j12

# 重新啟動
sudo update-grub
sudo reboot
```
![image](https://hackmd.io/_uploads/BJwMqhIg1l.png)
![image](https://hackmd.io/_uploads/HJkN9nUxkg.png)

6. 重新開機後，檢查版本
```bash
uname -rs
```

:::info
若以上過程有任何錯誤或想要有其他修改，可執行以下指令清除make編譯後產生的文件，並回到 **4. 開始編譯**
```b
# 刪除make -j12執行後所產生的文件
make clean
```
:::

---

## Requirement：虛擬地址(virtual address)轉實體地址(physical address)
:::spoiler 題目敘述
* In this project, you need to write a new system call void * my_get_physical_addresses(void *) so that a process can use it to get the physical address of a virtual address of a process.
* The return value of this system call is either 0 or an address value. 0 means that there is no physical address assigned to the logical address currently. A non-zero value means the physical address of the logical address submitted to the system call as its parameter (in fact, this address is only the offset of a logical address).
:::

### ==Kernel Space 程式碼==
**在mycall底下的my_get_physical_addresses.c中寫入以下程式碼：**
```c=
#include <linux/kernel.h>      // 提供內核函數的標頭檔案
#include <linux/syscalls.h>    // 提供系統呼叫的定義
#include <linux/uaccess.h>     // 提供與使用者空間交互的函數 (copy_to_user, copy_from_user)
#include <linux/mm.h>          // 提供內存管理相關的定義
#include <linux/highmem.h>     // 提供與高位記憶體操作相關的定義
#include <linux/sched.h>       // 提供進程相關的結構體與函數 (例如 current)

SYSCALL_DEFINE1(my_get_physical_addresses, unsigned long __user *, usr_ptr)
{
    unsigned long virt_addr;      // 儲存從使用者空間傳入的虛擬地址
    unsigned long phys_addr = 0;  // 儲存計算出來的實體地址
    struct mm_struct *mm = current->mm;  // 取得當前進程的內存描述符 (mm_struct)
    pgd_t *pgd;                   // 頁全局目錄 (Page Global Directory) 的指標
    p4d_t *p4d;                   // 頁 4 目錄 (Page 4 Directory) 的指標
    pud_t *pud;                   // 頁上層目錄 (Page Upper Directory) 的指標
    pmd_t *pmd;                   // 頁中間目錄 (Page Middle Directory) 的指標
    pte_t *pte;                   // 頁表項 (Page Table Entry) 的指標

    // 使用 copy_from_user 函數將使用者空間中的資料複製到內核空間
    // 如果複製操作成功，將 usr_ptr 指向的內容複製到 virt_addr
    // sizeof(unsigned long) 指定要複製的資料大小
    if (copy_from_user(&virt_addr, usr_ptr, sizeof(unsigned long))) {
        // 如果從使用者空間複製資料失敗，則顯示錯誤訊息並返回 -EFAULT
        pr_err("Failed to copy virtual address from user space\n");
        return -EFAULT;  // 返回錯誤碼表示操作失敗
    }

    pr_info("User virtual address: 0x%lx\n", virt_addr);

    // 取得 PGD (Page Global Directory) 頁全局目錄
    pgd = pgd_offset(mm, virt_addr);
    pr_info("MM struct: %p\n", mm);
    pr_info("PGD: %p\n", pgd);
    if (pgd_none(*pgd) || likely(pgd_bad(*pgd))) {
        pr_err("Invalid PGD\n");
        return -EFAULT;  // 如果 PGD 不存在或無效，返回錯誤
    }

    // 取得 P4D (Page 4 Directory) 頁 4 目錄
    p4d = p4d_offset(pgd, virt_addr);
    pr_info("P4D: %p\n", p4d);
    if (p4d_none(*p4d) || likely(p4d_bad(*p4d))) {
        pr_err("Invalid P4D\n");
        return -EFAULT;  // 如果 P4D 不存在或無效，返回錯誤
    }

    // 取得 PUD (Page Upper Directory) 頁上層目錄
    pud = pud_offset(p4d, virt_addr);
    pr_info("PUD: %p\n", pud);
    if (pud_none(*pud) || likely(pud_bad(*pud))) {
        pr_err("Invalid PUD\n");
        return -EFAULT;  // 如果 PUD 不存在或無效，返回錯誤
    }

    // 取得 PMD (Page Middle Directory) 頁中間目錄
    pmd = pmd_offset(pud, virt_addr);
    pr_info("PMD: %p\n", pmd);
    if (pmd_none(*pmd) || likely(pmd_bad(*pmd))) {
        pr_err("Invalid PMD\n");
        return -EFAULT;  // 如果 PMD 不存在或無效，返回錯誤
    }

    // 取得 PTE (Page Table Entry) 頁表項
    pte = pte_offset_kernel(pmd, virt_addr);
    pr_info("PTE: %p\n", pte);
    if (!pte || pte_none(*pte)) {
        pr_err("Invalid PTE\n");
        return -EFAULT;  // 如果 PTE 不存在或無效，返回錯誤
    }

    // 檢查頁框是否有效，確保頁面被映射
    if (!pte_present(*pte)) {
        pr_err("Page not present\n");
        return -EFAULT;  // 如果頁面未被映射（不存在於物理記憶體中），返回錯誤
    }

    // 計算實體地址: (頁框號 << PAGE_SHIFT) + (虛擬地址的頁內偏移量)
    phys_addr = (pte_pfn(*pte) << PAGE_SHIFT) + (virt_addr & ~PAGE_MASK);
    pr_info("Physical address: 0x%lx\n", phys_addr);

    // 將計算出的實體地址複製回使用者空間
    if (copy_to_user(usr_ptr, &phys_addr, sizeof(unsigned long))) {
        pr_err("Failed to copy physical address to user space\n");
        return -EFAULT;  // 如果複製失敗，返回錯誤
    }

    return phys_addr;  // 返回實體地址
}
```

1. current->mm存有PGD的虛擬位址，並且CR3指向PGD的起始實體位址。
2. 在x86-64的5-level架構中，虛擬記憶體內指向PGD、P4D、PUD、PMD、PTE的page table各有9 bits，最後的offset有12 bits。其中5-level和4-level的差別在於多了P4D，且PGD和P4D的值相同。
3. 一開始CR3指向PGD table基地址，加上虛擬記憶體中對應PGD的地址後，就會指向PGD table中的某一格，該格存放的是下一層某個P4D的基地址，接著該地址再加上虛擬位址中對應P4D的地址後，就能得到下層某個PUD的基地址，後續以此類推查找。
4. 最後會查找到PTE（Page Table Entry，頁表項），其內容包含的是映射到實體位址中的頁框號（Page Frame Number，PFN）。PFN映射到實體位址的頁框號，它指向實體位址中對應的頁框(Page Frame)。
5. **`實體位址 = (pte_pfn(*pte) << PAGE_SHIFT) + (virt_addr & ~PAGE_MASK)`**
    * PFN：即查找的PTE內容。
    * PAGE_SHIFT：常數，固定為12。
    * PAGE_MASK = 0xfffffffffffff000，故~PAGE_MASK = 0xfff。
    * (virt_addr & ~PAGE_MASK) = offset，其結果為保留虛擬位址的後12個bits，即保留16進位地址的後3碼。
    * 簡單來說，實體位址的值(二進位) = PFN向左移12 bits後，再加上虛擬位址的後12個bits。也因此，在十六進位表示下，虛擬位址和實體位址的後3碼會相同。
6. 下圖為虛擬位址轉實體位址流程圖，以及過程中轉換得到的各種地址。
![image](https://hackmd.io/_uploads/HkxPNrYe1x.png)
![image](https://hackmd.io/_uploads/ByDRN8FxJe.png)

---

## Question 1：Copy-on-write 寫入時複製
:::spoiler 題目敘述
What follows is an example code which you can use to see the effect of copy-on-write.
:::

### ==User Space 程式碼==
```c=
#include <stdio.h>
#include <unistd.h>   // 提供 fork, sleep 等函數
#include <sys/syscall.h> // 提供 syscall 函數
#include <sys/wait.h> // 提供 wait 函數

// Syscall 449 的接口
void *my_get_physical_addresses(void *vaddr) {
    return (void *)syscall(449, &vaddr);
}

int global_a = 123;  // global variable

int main()
{
    void *parent_use, *child_use;

    printf("===========================Before Fork==================================\n");
    printf("global_a: %d\n", global_a);
    printf("global_a 0x%lx\n", (unsigned long)&global_a);
    parent_use = my_get_physical_addresses(&global_a);
    printf("pid=%d: global variable global_a:\n", getpid());
    printf("Offset of logical address:[%p]   Physical address:[%p]\n", (void *)&global_a, parent_use);
    printf("========================================================================\n\n");

    if (fork())
    { /*parent code*/
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvv  After Fork by parent  vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n");
        parent_use = my_get_physical_addresses(&global_a);
        printf("parent global_a: %d\n", global_a);
        printf("pid=%d: global variable global_a:\n", getpid());
        printf("******* Offset of logical address:[%p]   Physical address:[%p]\n", (void *)&global_a, parent_use);
        printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n\n");
        wait(NULL);  // 等待子進程結束
    }
    else
    { /*child code*/
        printf("llllllllllllllllllllllllll  After Fork by child  llllllllllllllllllllllllllllllll\n");
        child_use = my_get_physical_addresses(&global_a);
        printf("child global_a: %d\n", global_a);
        printf("******* pid=%d: global variable global_a:\n", getpid());
        printf("******* Offset of logical address:[%p]   Physical address:[%p]\n", (void *)&global_a, child_use);
        printf("llllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllllll\n");
        printf("____________________________________________________________________________\n\n");

        /*----------------------- trigger CoW (Copy on Write) -----------------------------------*/
        global_a = 789;

        printf("iiiiiiiiiiiiiiiiiiiiiiiiii  Test copy on write in child  iiiiiiiiiiiiiiiiiiiiiiii\n");
        child_use = my_get_physical_addresses(&global_a);
        printf("child global_a: %d\n", global_a);
        printf("******* pid=%d: global variable global_a:\n", getpid());
        printf("******* Offset of logical address:[%p]   Physical address:[%p]\n", (void *)&global_a, child_use);
        printf("iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
        printf("____________________________________________________________________________\n\n");
        sleep(1000);  // 子進程保持活躍一段時間
    }
    return 0;
}

```