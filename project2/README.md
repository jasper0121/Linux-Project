# Linux 作業系統 project 2

## 前置準備
### 小組名單 - 第24組
> **113522118 韓志鴻  
> 113522096 李婕綾  
> 113522089 邱奕鋒   
> 111521052 徐浩軒**  
### 作業題目下載
[Project 2](https://raw.githubusercontent.com/jasper0121/Linux-Project/9a9f56fd7caf4074bdb171eadece5cd41b1fe888/project2/Project_2.pdf)

### 系統環境
* 虛擬機器：VMware Workstation Pro
* 作業系統：ubuntu 22.04
* Kernel 版本：5.15.137
* 記憶體：16GB

### 想要用vscode開發的話
1. 在虛擬機內搜尋[vscode(點擊此連結)](https://code.visualstudio.com/Download)，並**點選.deb(Debian, Ubuntu)按鈕**來下載。
2. 想要用superuser權限的vscode開發的話，可在cmd輸入以下指令：  
    :::info  
    ```
    sudo code --no-sandbox --user-data-dir
    ```
    :::

## 建立自訂syscall並編譯
:::success
**更詳細的步驟與介紹請點擊[這裡](https://hackmd.io/jM6pDq8FRdet6oIutCXXAA?view#%E5%BB%BA%E7%AB%8B%E8%87%AA%E8%A8%82syscall%E4%B8%A6%E7%B7%A8%E8%AD%AF)查看上一次的作業內容，這裡會根據本次作業內容簡單帶過。**
:::  

### 下載 Kernel Source
```b
# 進入 root 模式
sudo su

# 下載kernel
wget -P ~/ https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.137.tar.xz

# 解壓縮到 /usr/src 目錄底下
tar -xvf linux-5.15.137.tar.xz -C /usr/src
```

### 新增syscall
```bash
# 進入剛載的linux-5.15.137裡面
cd /usr/src/linux-5.15.137/

# 建立一個名為mygo...叫mycall的資料夾
mkdir mycall
```

1. 在mycall底下建立my_wait_queue.c並新增以下code，之後記得存檔  
:::info  
[之後再回來補](#Requirement：虛擬地址virtual-address轉實體地址physical-address)
:::  

2. 接著繼續在mycall內新增Makefile
```b
obj-y := my_wait_queue.o # 將my_wait_queue.o編入kernel
```

### 修改linux-5.15.137本身的Makefile
1. 回到/usr/src/linux-5.15.137並找到其底下的Makefile。
2. 若使用**vscode可直接ctrl+f找core-y字段**，若用vim編輯的話則可輸入 **?core-y**來查找，位置大約在1162行。
3. 找到後在該句後面新增 **mycall/**，具體如下：
```b
原指令：core-y			+= kernel/ certs/ mm/ fs/ ipc/ security/ crypto/

修改成：core-y			+= kernel/ certs/ mm/ fs/ ipc/ security/ crypto/ mycall/
```

### 將系統調用函數（syscall）的宣告添加到 Linux 核心中
1. 進到 **/usr/src/linux-5.15.137/include/linux/syscalls.h**
2. 在syscalls.h最下面的 **#endif**之前添加以下程式碼：
```c
asmlinkage long sys_my_wait_queue(int id);
```

### 將syscall加入到kernel的syscall table
1. 進到 **/usr/src/linux-5.15.137/arch/x86/entry/syscalls/syscall_64.tbl**
2. 拉到約372行，我們新增450號指令(編號可自訂，但注意不要和其他syscall重複)：
```bash
450 common  my_wait_queue    sys_my_wait_queue
```
3. 該指令意義如下：
    * **common**是類型或分類。這通常是用來表示系統調用的類型或屬性。common 表示這個系統調用是通用的，可能適用於多個不同的架構或情況。
    * **my_wait_queue**是用戶空間(user space)接口名稱。這是user space中調用的系統調用名稱。當應用程序需要訪問這個系統調用時，它會使用這個名稱來進行調用。
    * **sys_my_wait_queue**是內核空間(kernel space)實現名稱。這是實際在kernel中實現的系統調用函數名稱。此函數處理來自user space的請求，並執行相應的操作。  

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
補充：若reboot沒有出現以上選單畫面需輸入
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
若以上過程有任何錯誤或想要有其他修改，可執行以下指令清除make編譯後產生的文件，並回到 **4. 開始編譯**。
```b
# 刪除make -j12執行後所產生的文件
make clean
```
:::  

---

## Project 2：Wait Queue
:::spoiler 題目敘述  
A Wait Queue is a synchronization mechanism in the Linux kernel used to put processes to sleep while waiting for a specific condition to be met. Once the condition is satisfied, the processes are awakened. It is commonly used to avoid busy-waiting, thereby improving system efficiency.  

Under normal circumstances, a function using a wait queue must know the name or the pointer of the wait queue. This is because a wait queue is represented as a variable (usually of type <font color=red>wait_queue_head_t</font>), and the function needs the address of this variable to perform operations.  

> **In this lab, you need to implement a custom wait queue-like functionality in kernel space, allowing user applications to operate through the system call.**  

---

The target output is as follows: threads exit the wait queue in FIFO order.  

> :::success  
> enter wait queue thread_id: 0  
> enter wait queue thread_id: 1  
> enter wait queue thread_id: 2  
> enter wait queue thread_id: 6  
> enter wait queue thread_id: 7  
> enter wait queue thread_id: 8  
> enter wait queue thread_id: 9  
> enter wait queue thread_id: 5  
> enter wait queue thread_id: 3  
> enter wait queue thread_id: 4  
> start clean queue ...  
> exit wait queue thread_id: 0  
> exit wait queue thread_id: 1  
> exit wait queue thread_id: 2  
> exit wait queue thread_id: 6  
> exit wait queue thread_id: 7  
> exit wait queue thread_id: 8  
> exit wait queue thread_id: 9  
> exit wait queue thread_id: 5  
> exit wait queue thread_id: 3  
> exit wait queue thread_id: 4  
> :::  

---

**Todo**
> :::info
> Implement a system call <font color=red> call_my_wait_queue(int id)</font> . This function takes an argument <font color=red>id</font> to determine which of the following two operations to perform:  
> 1. **Add a thread to the wait queue to wait.** If an error occurs, return 0; on success, return 1.  
> 2. **Remove threads from the wait queue, allowing them exit.** If an error occurs, return 0 ; on success, return 1. The removal order must follow the **FIFO (First In, First Out)** principle.
> :::  

**Hints**  
> :::info
> You can use kernel-provided wait queue related functions to implement this functionality.
> You need to declare a <font color=red>my_wait_queue</font> in kernel space.
> :::  
:::  

### ==Kernel Space 程式碼==
```c=
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/delay.h>

static DECLARE_WAIT_QUEUE_HEAD(my_wait_queue);  // 定義等待隊列頭
static DEFINE_MUTEX(wait_queue_mutex);         // 定義互斥鎖保護等待隊列

// enter_wait_queue 函數：將當前 thread 加入等待隊列
static int enter_wait_queue(void)
{
    DEFINE_WAIT(wait); // 定義等待隊列元素

    mutex_lock(&wait_queue_mutex); // 獲取互斥鎖
    pr_info("Thread %d is entering the wait queue\n", current->pid); // 打印進程資訊
    add_wait_queue_exclusive(&my_wait_queue, &wait); // 將當前進程添加到等待隊列
    set_current_state(TASK_INTERRUPTIBLE); // 設置進程為可中斷等待狀態
    mutex_unlock(&wait_queue_mutex); // 釋放互斥鎖

    schedule(); // 進程進入睡眠等待喚醒

    return 0; // 喚醒後返回
}

// clean_wait_queue 函數：喚醒並移除所有等待隊列中的 thread
static int clean_wait_queue(void)
{
    struct wait_queue_entry *entry, *next;

    list_for_each_entry_safe(entry, next, &my_wait_queue.head, entry) {
        struct task_struct *task = entry->private; // 獲取等待進程的 task_struct
        pr_info("Thread %d is leaving the wait queue\n", task->pid); // 打印進程資訊
        mutex_lock(&wait_queue_mutex); // 獲取互斥鎖
        list_del(&entry->entry); // 從等待隊列中移除
        wake_up_process(task); // 喚醒等待進程
        mutex_unlock(&wait_queue_mutex); // 釋放互斥鎖
        msleep(1000); // 休息 1 秒
    }

    return 0;
}

// 系統呼叫實作
SYSCALL_DEFINE1(my_wait_queue, int, id)
{
    switch (id) {
    case 1:
        return enter_wait_queue(); // ID 為 1，執行 enter_wait_queue
    case 2:
        return clean_wait_queue(); // ID 為 2，執行 clean_wait_queue
    default:
        return -EINVAL; // 無效 ID
    }
}
```
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
### 執行結果  
![image](https://hackmd.io/_uploads/HkkRg8KE1x.png)  



### ==User Space 程式碼==
```c=
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

#define NUM_THREADS 10
#define SYSCALL_WAIT_QUEUE 450

void *enter_wait_queue(void *thread_id)
{
    fprintf(stderr, "enter wait queue thread_id: %d\n", *(int *)thread_id);

    // your syscall here 
    syscall(SYSCALL_WAIT_QUEUE, 1);

    fprintf(stderr, "exit wait queue thread_id: %d\n", *(int *)thread_id);
    return NULL;
}

void *clean_wait_queue()
{
    // your syscall here 
    syscall(SYSCALL_WAIT_QUEUE, 2);
    return NULL;
}

int main()
{
    void *ret;
    pthread_t id[NUM_THREADS];
    int thread_args[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_args[i] = i;
        pthread_create(&id[i], NULL, enter_wait_queue, (void *)&thread_args[i]);
    }

    sleep(1);

    fprintf(stderr,  "start clean queue ...\n");
    clean_wait_queue();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(id[i], &ret);
    }

    return 0;
}

```
:::info  
**編譯與執行的指令如下，或是直接vscode右上角的執行按鍵**
![image](https://hackmd.io/_uploads/ryWrHie-kx.png)

```b
# gcc -o 編譯出來的檔案名 編譯程式碼.c，以下指令是舉例
gcc -o user_space user_space.c

# 執行程式
./user_space
```
:::  

### 執行結果
![image](https://hackmd.io/_uploads/Hk89o7uE1l.png)



---

## Project 遇到的問題
add_wait_queue_exclusive獨佔與否差別(flag的差異)？

list_del和wake_up_process執行順序相反，user space執行石容易當機，原因？

kernel space確定是FIFO，但從kernel出來到user space印出這段期間，可能因為CPU排程的關係，導致在user space印出的順序不是FIFO。除了在kernel端使用sleep的方式外，還有沒有其他更好的方法？

---

## Reference
[linux等待队列wait queue](https://blog.csdn.net/q2519008/article/details/123816780)