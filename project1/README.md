# Linux 作業系統 project1

## 前置準備
### 小組名單 - 第24組
> **113522118 韓志鴻  
> 113522096 李婕綾  
> 113522089 邱奕鋒   
> 111521052 徐浩軒**  
### 作業題目網址
[Project 1](https://staff.csie.ncu.edu.tw/hsufh/COURSES/FALL2024/linux_project_1.html)

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
    * <font color=red>obj-y</font>是用於內核編譯系統中的一個變數，它表示某個 .o 文件（即編譯後的目標文件）會被內嵌（linked statically）到內核映像(kernel image)中。
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
5. 這行指令告訴編譯系統，當編譯 Linux 核心時，還需要編譯這些目錄中的代碼並將它們靜態鏈接到內核映像中。換句話說，kernel/、certs/、mm/、fs/、ipc/、security/ 和 crypto/ 這些目錄中的代碼都是內核核心功能的一部分，它們的內容必須被編譯並最終成為內核的一部分。因此若我們今天要編譯其他自定義內容，就要在後面追加子目錄，即mycall。

### 將系統調用函數（syscall）的宣告添加到 Linux 核心中
1. 進到 **/usr/src/linux-5.15.137/include/linux/syscalls.h**
2. 在syscalls.h最下面的 **#endif**之前添加以下程式碼：
```c
asmlinkage long sys_my_get_physical_addresses(unsigned long __user *usr_ptr);
```
3. **asmlinkage**的意思是告訴編譯器，這個函數的參數是通過**stack**傳遞的，而不是像普通內核函數那樣通過寄存器傳遞。在 Linux 系統調用中，參數必須從用戶空間（user space）傳遞到內核空間（kernel space），並使用stack來傳遞參數。使用 asmlinkage 確保系統調用能正確接收這些參數，並能跨平台運行。基本上要寫syscall的話都要前綴這個關鍵詞。
4. **long**是函數的返回類型，表示該函數將返回一個 long 整數。
5. **sys_my_get_physical_addresses**是函數的名稱，表明這是一個系統調用。根據命名慣例，sys_ 前綴通常用於標識內核中的系統調用。這個特定的系統調用的功能是獲取物理地址，具體功能將在函數的實現中定義。
6. **unsigned long __user *usr_ptr**是函數的參數。其中 __user 修飾符表示這個pointer指向user space的內存。通過這個pointer，系統調用可以訪問user space提供的數據。在kernel中任何訪問user space的數據使用 __user 修飾符可以避免潛在的安全問題。

### 將syscall加入到kernel的syscall table
1. 進到 **/usr/src/linux-5.15.137/arch/x86/entry/syscalls/syscall_64.tbl**
2. 拉到約372行，該區塊編號應該到448，接著下個區塊就從編號512開始，我們在編號448指令的下一行新增449號指令：
```bash
449	common	my_get_physical_addresses		sys_my_get_physical_addresses
```
3. 該指令意義如下：
    * **449**是系統調用編號。(編號可自訂，但注意不要和其他syscall重複)
    * **common**是類型或分類。這通常是用來表示系統調用的類型或屬性。common 表示這個系統調用是通用的，可能適用於多個不同的架構或情況。
    * **my_get_physical_addresses**是用戶空間(user space)接口名稱。這是user space中調用的系統調用名稱。當應用程序需要訪問這個系統調用時，它會使用這個名稱來進行調用。
    * **sys_my_get_physical_addresses**是內核空間(kernel space)實現名稱。這是實際在內核中實現的系統調用函數名稱。此函數處理來自user space的請求，並執行相應的操作。

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
3. 在開始編譯之前，要先在cmd輸入以下4個指令：
```bash
# 刪除憑證
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
scripts/config --set-str CONFIG_SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str CONFIG_SYSTEM_REVOCATION_KEYS ""
```

否則將會出現以下錯誤訊息:  
:::danger  
sed: can't read modules.order: No such file or directory  
make: *** [Makefile:1544: _modinst_pre] Error 2
:::

原因據其他參考網站是說設定檔內是Debian官方當初編譯kernel時憑證的路徑，若是直接編譯會
報錯，因此這邊取消使用憑證，並將值設為空字串。

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

:::info  
補充：
若reboot沒有出現以上選單畫面需輸入
```
vim /etc/default/grub
```
將GRUB_TIMEOUT_STYLE改成menu、GRUB_TIMEOUT改成-1，然後**再回到上述第5點的重新啟動**。
![image](https://hackmd.io/_uploads/SyHTZ0rZJx.png)  
:::
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

// 定義一個新的系統呼叫 my_get_physical_addresses，用於將虛擬地址轉換為實體地址
SYSCALL_DEFINE1(my_get_physical_addresses, unsigned long __user *, usr_ptr) {
    unsigned long virt_addr;         // 儲存從使用者空間傳入的虛擬地址
    unsigned long phys_addr = 0;     // 儲存計算出來的實體地址
    struct mm_struct *mm = current->mm;  // 獲取當前進程的內存描述符，用於訪問內存映射
    pgd_t *pgd;                      // 頁全局目錄 (Page Global Directory) 的指標
    p4d_t *p4d;                      // 頁 4 目錄 (Page 4 Directory) 的指標
    pud_t *pud;                      // 頁上層目錄 (Page Upper Directory) 的指標
    pmd_t *pmd;                      // 頁中間目錄 (Page Middle Directory) 的指標
    pte_t *pte;                      // 頁表項 (Page Table Entry) 的指標

    // 從使用者空間複製虛擬地址到內核空間，使用copy_from_user可以在拷貝user資料的同時檢查安全性
    if (copy_from_user(&virt_addr, usr_ptr, sizeof(unsigned long))) {
        pr_err("Failed to copy virtual address from user space\n");
        return -EFAULT;  // 返回錯誤碼 -EFAULT，表示無效地址
    }

    pr_info("User virtual address: 0x%lx\n", virt_addr);  // 印出使用者傳入的虛擬地址

    // 取得 PGD (Page Global Directory) 頁全局目錄
    pgd = pgd_offset(mm, virt_addr);  // 使用 mm 和虛擬地址獲取 PGD
    pr_info("MM struct: %p\n", mm);   // 印出process的內存描述符地址
    pr_info("PGD: %p\n", pgd);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {  // 檢查 PGD 是否為空或無效
        pr_err("Invalid PGD\n");
        return -EFAULT;
    }

    // 取得 P4D (Page 4 Directory) 頁 4 目錄
    p4d = p4d_offset(pgd, virt_addr);  // 使用 PGD 和虛擬地址獲取 P4D
    pr_info("P4D: %p\n", p4d);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {  // 檢查 P4D 是否為空或無效
        pr_err("Invalid P4D\n");
        return -EFAULT;
    }

    // 取得 PUD (Page Upper Directory) 頁上層目錄
    pud = pud_offset(p4d, virt_addr);  // 使用 P4D 和虛擬地址獲取 PUD
    pr_info("PUD: %p\n", pud);
    if (pud_none(*pud) || pud_bad(*pud)) {  // 檢查 PUD 是否為空或無效
        pr_err("Invalid PUD\n");
        return -EFAULT;
    }

    // 取得 PMD (Page Middle Directory) 頁中間目錄
    pmd = pmd_offset(pud, virt_addr);  // 使用 PUD 和虛擬地址獲取 PMD
    pr_info("PMD: %p\n", pmd);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {  // 檢查 PMD 是否為空或無效
        pr_err("Invalid PMD\n");
        return -EFAULT;
    }

    // 取得 PTE (Page Table Entry) 頁表項
    pte = pte_offset_kernel(pmd, virt_addr);  // 使用 PMD 和虛擬地址獲取 PTE
    pr_info("PTE: %p\n", pte);
    if (!pte || pte_none(*pte) || !pte_present(*pte)) {  // 檢查 PTE 是否有效或是否存在映射
        pr_err("Invalid or unmapped PTE\n");
        return -EFAULT;
    }

    // 計算實體地址：頁框號左移 PAGE_SHIFT 加上虛擬地址的頁內偏移量
    phys_addr = (pte_pfn(*pte) << PAGE_SHIFT) + (virt_addr & ~PAGE_MASK);
    pr_info("Physical address: 0x%lx\n\n", phys_addr);  // 印出實體地址

    // 將實體地址複製回使用者空間，使用copy_to_user可以在拷貝kernel資料的同時檢查安全性
    if (copy_to_user(usr_ptr, &phys_addr, sizeof(unsigned long))) {
        pr_err("Failed to copy physical address to user space\n");
        return -EFAULT;
    }

    return phys_addr;  // 返回實體地址
}
```

### 虛擬位址轉實體位址流程圖
![image](https://hackmd.io/_uploads/HkxPNrYe1x.png)  

1. current->mm存有PGD的虛擬位址，並且CR3指向PGD的起始實體位址。
2. 在x86-64的5-level架構中，**虛擬地址內指向PGD、P4D、PUD、PMD、PTE的page table各有9 bits，最後的offset有12 bits**。其中5-level和4-level的差別在於多了P4D，且PGD和P4D的值相同。
3. 一開始CR3指向PGD table基地址，加上虛擬記憶體中對應PGD的地址後，就會指向PGD table中的某一格，該格存放的是下一層某個P4D的基地址，接著該地址再加上虛擬位址中對應P4D的地址後，就能得到下層某個PUD的基地址，後續以此類推查找。
4. 最後會查找到PTE（Page Table Entry，頁表項），其內容包含的是映射到實體位址中的PFN（Page Frame Number，頁框號）。**PFN 指的是實體記憶體中的頁框號，並指向實體位址中對應的頁框（Page Frame）**。
5. <font color=red>**實體位址 = (pte_pfn(*pte) << PAGE_SHIFT) + (virt_addr & ~PAGE_MASK)**</font>  
    * **pte_pfn(*pte)**：即從PTE查找得到PFN。
    * PAGE_SHIFT：常數，固定為12。
    * PAGE_MASK = 0xfffffffffffff000，故 ~PAGE_MASK = 0xfff。
    * **(virt_addr & ~PAGE_MASK) = offset**，其結果為保留虛擬位址的後12個bits，即保留16進位地址的後3碼。
    * 也就是說，**實體位址的值(二進位) = PFN向左移12 bits後，再加上虛擬位址的後12個bits**。因此**在十六進位表示下，虛擬位址和實體位址的後3碼會相同**。  
6. __pte_pfn(*pte)__ v.s __pte_val(*pte)__
    * **pte_pfn(*pte)**： 這個function是一個專門從 PTE 中提取 PFN 的function，可以確保只獲取正確的 PFN，而不會受到低位控制位的影響。
    * **pte_val(*pte)**：這個function會將整個 PTE 數值提取出來，但 PTE 中包含了低位的控制位，這會導致後面計算的 page address錯誤。
    * PTE架構圖  
    ![image](https://0dr3f.github.io/images/Pasted%20image%2020240925104151.png)

### 執行結果
下圖顯示從虛擬地址轉成實體地址過程中的各種值，若有任何層轉換出現問題則顯示該層Invalid。
![image](https://hackmd.io/_uploads/Hy5S5xSZ1g.png)  

:::info  
**想查看kernel訊息等相關指令(如上述執行結果)的話可輸入以下指令**
```bash
# 查看kernel訊息
sudo dmesg

# 可即時顯示kernel訊息，其中的0.1表示更新間隔時間
sudo watch -n0.1 dmesg

# 清除kernel訊息
sudo dmesg -C
```
:::

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
```

:::info  
**編譯與執行的指令如下，或是直接vscode右上角的執行按鍵**
![image](https://hackmd.io/_uploads/ryWrHie-kx.png)

```b
# gcc -o 編譯出來的檔案名 編譯程式碼.c，以下指令是舉例
gcc -o Question1 Question1.c

# 執行程式
./Question1
```
:::

### 執行結果
![image](https://hackmd.io/_uploads/B1j05er-kl.png)  
1. **執行流程**： 建立全域變數global_a → 進入main() → fork()分成父子process → 父子process各自取global_a的虛擬位址和實體位址 → 子process修改global_a的值觸發copy on write。
2. 進入**Test copy on write in child之前**，除了父子的pid不一樣之外，因global_a變數沒有改變，故父子的虛擬地址和實體地址的值都一樣。
3. 進入**Test copy on write in child之後**，因為global_a變數在child被改成789，但parent的global_a仍是123，因此<font color=red>**child將原本指向的global_a = 123複製到另一個實體位址後，再將其改成789**</font>，因此可以發現此時的實體位址已經改變了。

### Copy-on-write示意圖
![image](https://hackmd.io/_uploads/Sy-l0IslJe.png)  
原本2個processes都內的變數都指向同一個實體記憶體，但當其中一個process更改值之後，就會複製原先的值(page 5)到其他地方，然後再把值修改上去(copy of page 5)。

---

## Question 2：Demand paging & Page Fault
:::spoiler 題目敘述
What follows is an example code which you can use to check whether a loader loads all data of a process before executing it.
:::

### ==User Space 程式碼==
```c=
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
```

### 執行結果

1. user space 結果顯示  
![image](https://hackmd.io/_uploads/rky8KHHWye.png)  
2. kernel space 過程顯示(分別表示a[1999999]未存取值和已存取值下取得實體位址的情況)  
![image](https://hackmd.io/_uploads/r16KiSB-kg.png)  
3. 在 Linux 作業系統中，分頁管理（paging）是惰性分配的（lazy allocation）。意思是當宣告一個大陣列時，**記憶體並不會馬上為陣列的每個元素分配實體頁面，而是當實際存取這些元素時，系統才會將對應的虛擬頁面載入並映射到實體頁面。**
    * **a[0]可以找的到實體位址**：原因是當定義一個全域變數，它通常位於「靜態資料區」（Static Data Segment），這部分記憶體在程式啟動時會被初始化。因為是全域變數，在程式啟動時就會為該變數分配記憶體頁面，並將其對應到實體位址。因此a[0] 的實體位址可以馬上取得，即使a[0]尚未被存取。
    * **a[1999999] 沒有實體位址**：對這麼大的陣列來說，作業系統雖然預先分配一部分頁面，但可能並不會馬上分配所有頁面，特別是對於像 a[1999999] 這樣遠端的元素。如果從未對該元素進行存取，它對應的頁面仍然可能處於未分配狀態。此時kernel space就會無法查找PTE內部的值。
    * 根據上圖，在**a[1999999]** 給值之後，就找的到實體位址，其上一格陣列和往前數143格也有實體位址，但當往前找第144格(a[1999999 - 144])時卻又找不到實體位址。之後可以發現**每1024個間隔就會必須賦予值才能取得該位置的實體位址**。這是因為一旦有尚未分配實體位址的陣列寫入資料後，其所屬的page才會被載入。**page的大小是4KB，且int陣列每一格是4bytes，因此可以說每次載入1個page等同載入(4096 / 4 = 1024)格陣列至實體記憶體**，所以才會有每隔1024格陣列後無法取得實體位址，必須再寫入資料後才能取得實體位址的現象。

---

## Project 遇到的問題
1. **Q**：編譯過程中記憶體不足，導致整個VM壞掉。  
   **Ans**：重新配置VM(磁碟給100GB)，並在每次重新編譯前先清除之前的的文件(make clean)。  
2. **Q**：在make時有出現過以下error
    :::danger
    .tmp_vmlinux.btf: pahole(pahole) is not available  
    Failed to generate BTF for vmlinux  
    Try to disable CONFIG_DEBUG_INFO_BTF  
    make:***[Makefile:1227:vmlinux] Error1
    :::
    **Ans**：
   ```bash
    # 安裝dwarves
    sudo apt-get install dwarves
    ```
    這個問題主要是因為內核在編譯過程中嘗試生成 BTF (BPF Type Format) 調試資訊，但缺少必要的工具 pahole。  
    BTF (BPF Type Format) 是一種二進制格式，用於支持 BPF（Berkeley Packet Filter）的調試和追蹤功能。生成 BTF 需要使用 pahole 工具，它是 Dwarves 工具集的一部分。  
3. **Q**：一開始發現轉出來的實體位址過大，如下圖的第1行：  
![image](https://hackmd.io/_uploads/BkaqXyS-ye.png)  
上圖第1行是我們一開始所使用的轉換實體位址公式，但地址太大了，懷疑這個地址是不是不乾淨，還藏有其他東西。  
   **Ans**：  
   a. 上[bootlin](https://elixir.bootlin.com/linux/v5.15.137/source)找對應kernel版本的資料夾查詢函式。  
   b. 查pte_val()底層是什麼東西，發現執行pte_val()時，實際等同執行native_pte_val()。  
   ![image](https://hackmd.io/_uploads/SkPO7hEWkg.png)  
   繼續往下追查發現：  
   ![image](https://hackmd.io/_uploads/HkERE3EWJl.png)  
   雖然native_pte_val()最後回傳的pte.pte只是unsigned long型態，但往下發現pte_flags()函式回傳的是native_pte_val(pte) & PTE_FLAGS_MASK，意味著**從pte裡面取flags出來，綜合在網路上找其他資料說pte_val()回傳的值還包括flags和權限位之類的東西，證實pte_val()回傳的東西確實除了PFN外還包含其他東西。**  
   c. 因此<font color=red>**要確實取得PFN的話，則要再多and一個PTE_PFN_MASK(如問題圖中的第2行)**</font>，然後再照常加上offset就好了。  
   d. 然而在trace的過程中發現以下pte_pfn()函式：  
   ![image](https://hackmd.io/_uploads/BkC622EW1x.png)  
其中的protnone_mask代表取得權限位，一般情況下根據[bootlin](https://elixir.bootlin.com/linux/v5.15.137/source/arch/x86/include/asm/pgtable-2level.h#L100)顯示是回傳0：  
   ![image](https://hackmd.io/_uploads/HJVzkTE-yg.png)  
   實測結果也是回傳0：  
   ![image](https://hackmd.io/_uploads/SJLoGp4bkx.png)  
   這也意味著**在pte_pfn()當中，pfn ^= protnone_mask(pfn)的結果不變，因為等同和0做XOR**。所以**pte_pfn()和原先問題中的第1行寫法只差在多一個向右移PAGE_SHIFT位(12位)後再回傳真正的PFN而已**，因此要取得實體位址的話，<font color=red>**可以改寫成問題圖中的第3行寫法，使用pte_pfn()並向左移12位後再加上offset即可**</font>，同樣也能得出和第2行寫法一樣的實體位址。

---

## Reference
[add a system call to kernel (v5.15.137)](https://hackmd.io/@cookie0416/rJaPXCvf6)  
[Kernel 的替換 & syscall 的添加](https://satin-eyebrow-f76.notion.site/Kernel-syscall-3ec38210bb1f4d289850c549def29f9f)  
[Linux 核心 Copy On Write 實作機制](https://hackmd.io/@linD026/Linux-kernel-COW-Copy-on-Write)  
[作業系統CH9 Virtual Memory Management](https://hackmd.io/@Chang-Chia-Chi/OS-CH9)  
[一文聊透 Linux 缺页异常的处理 —— 图解 Page Faults](https://www.cnblogs.com/binlovetech/p/17918733.html)  
[Linux Kernel demand paging: mapping anonymous memory into a user process's address space.](https://www.ryanstan.com/linux-demand-paging-anon-memory.html)  
[Linux内存管理 (19)总结内存管理数据结构和API](https://www.cnblogs.com/arnoldlu/p/8335568.html)  
[Linux arm64 pte相关宏](https://blog.csdn.net/weixin_45030965/article/details/132905393)  
[Demystifying Physical Memory Primitive Exploitation on Windows](https://0dr3f.github.io/Demystifying_Physical_Memory_Primitive_Exploitation_on_Windows)  
[Page Table Walk in Linux](https://stackoverflow.com/questions/46782797/page-table-walk-in-linux)  