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
**syscall建立步驟如同上次[作業1](https://hackmd.io/jM6pDq8FRdet6oIutCXXAA?view#%E5%BB%BA%E7%AB%8B%E8%87%AA%E8%A8%82syscall%E4%B8%A6%E7%B7%A8%E8%AD%AF)。**
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

1. 在mycall底下建立my_wait_queue.c並新增以下code，之後記得存檔  
:::info  
[Kernel Space 程式碼](#Kernel-Space-程式碼)
:::  

2. 接著繼續在mycall內新增Makefile
```b
obj-y := my_wait_queue.o # 將my_wait_queue.o編入kernel
```
3. **Makefile是一種用來自動化編譯過程的配置檔案，通常與make工具搭配使用**。Makefile定義如何編譯和鏈接程式碼，以生成可執行檔或其他輸出，並可設定不同的編譯規則和依賴關係。
4. 以上Makefile指令中：
    * <font color=red>obj-y</font>是用於內核編譯系統中的一個變數，它表示某個 .o 文件（即編譯後的目標文件）會被內嵌(linked statically)到內核映像(kernel image)中。
    * <font color=red>:=</font>是賦值運算符，意思是將右側的值(my_wait_queue.o)賦給左側變數(obj-y)。
    * <font color=red>my_wait_queue.o</font>是一個目標文件，它由相應的源文件(通常是 my_wait_queue.c)經過編譯後生成。.o 文件是編譯過程中的中間產物，代表編譯後的二進制對象。

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
asmlinkage long sys_my_wait_queue(int id);
```
3. **asmlinkage**的意思是告訴編譯器，這個函數的參數是通過**stack**傳遞的，而不是像普通內核函數那樣通過寄存器傳遞。在 Linux 系統調用中，參數必須從用戶空間（user space）傳遞到內核空間（kernel space），並使用stack來傳遞參數。使用 asmlinkage 確保系統調用能正確接收這些參數，並能跨平台運行。基本上要寫syscall的話都要前綴這個關鍵詞。

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
# 安裝編譯好的核心模組到系統的模組路徑中
sudo make modules_install -j12

# 上面指令跑完後沒有出現錯誤訊息，就可以繼續安裝kernel。
# 此命令的作用是安裝編譯好的核心和相關文件到系統的啟動目錄中，準備讓新的核心可以被啟動。
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

---

**User Space**  
Except for <font color=red>syscall(xxx, 1);</font> and <font color=red>syscall(yyy, 2);</font> , please do not modify any other parts of the code.  
:::  

### ==Kernel Space 程式碼==  
```c=
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

static DECLARE_WAIT_QUEUE_HEAD(my_wait_queue); // 定義一個全域的wait queue頭，用於管理所有等待的進程

// enter_wait_queue 函數：將當前 thread 加入wait queue
static int enter_wait_queue(void)
{
    DEFINE_WAIT(wait); // 定義wait queue元素
    pr_info("Entering the wait queue thread: %d\n", current->pid); // 打印進程資訊
    add_wait_queue_exclusive(&my_wait_queue, &wait); // 將當前進程添加到wait queue的隊尾
    set_current_state(TASK_INTERRUPTIBLE); // 設置進程為可中斷等待狀態，該進程將進入睡眠，等待被喚醒
    schedule(); // 進程進入睡眠等待喚醒
    
    return 1;
}

// clean_wait_queue 函數：喚醒並移除所有wait queue中的 thread
static int clean_wait_queue(void)
{
    pr_info("-------------------------------------------\n");

    struct wait_queue_entry *entry, *next; // 定義變數來遍歷wait queue中的節點

    // 遍歷wait queue，對每個節點進行操作，使用 list_for_each_entry_safe 確保在遍歷過程中可以安全地移除節點
    list_for_each_entry_safe(entry, next, &my_wait_queue.head, entry) {
        struct task_struct *task = entry->private; // 指向即將要從wait queue移除的節點
        pr_info("Leaving the wait queue thread: %d\n", task->pid);
        list_del(&entry->entry); // 從wait queue中移除當前節點
        wake_up_process(task); // 喚醒等待進程
        msleep(1500); // 休息 1.5 秒
    }

    return 1;
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
        return 0; // 無效 ID
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

1. [**DEFINE_WAIT**](https://elixir.bootlin.com/linux/v5.15.137/source/include/linux/wait.h#L1180)：定義wait queue節點。  
2. [**add_wait_queue_exclusive**](https://elixir.bootlin.com/linux/v5.15.137/source/kernel/sched/wait.c#L29)：實際上內部會執行__add_wait_queue_entry_tail，將節點加入wait queue的最後面。  
3. **set_current_state(TASK_INTERRUPTIBLE)**：將當前進程設置為可中斷的等待狀態(TASK_INTERRUPTIBLE)，進程此時會處於睡眠狀態，表示暫時不需要CPU資源直到被喚醒。  
4. [**schedule()**](https://elixir.bootlin.com/linux/v3.9/source/kernel/sched/core.c#L2966)：讓出 CPU，讓當前進程停止執行，進入調度器，並允許其他進程執行。以下是其程式碼，他會再呼叫__schedule()來執行。
    ```c 
    asmlinkage void __sched schedule(void)
        {
            struct task_struct *tsk = current;

            sched_submit_work(tsk);
            __schedule();
        }
        EXPORT_SYMBOL(schedule);
    ```
    [**__schedule()**](https://elixir.bootlin.com/linux/v3.9/source/kernel/sched/core.c#L2875)：裡面會執行context_switch()，保存當前進程的 CPU 狀態，並加載下一個進程的狀態。以下是程式碼中的片段。  
    ```c
	if (likely(prev != next)) {
            rq->nr_switches++;
            rq->curr = next;
            ++*switch_count;

            context_switch(rq, prev, next); /* unlocks the rq */
            /*
             * The context switch have flipped the stack from under us
             * and restored the local variables which were saved when
             * this task called schedule() in the past. prev == current
             * is still correct, but it can be moved to another cpu/rq.
             */
            cpu = smp_processor_id();
            rq = cpu_rq(cpu);
    ```
5. [**list_for_each_entry_safe**](https://elixir.bootlin.com/linux/v5.15.137/source/include/linux/list.h#L725)：從頭開始安全遍歷wait queue，保證過程中即使刪除節點，也能保證遍歷正常進行。其內部的程式碼如下。
    ```c
    /**
     * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
     * @pos:	the type * to use as a loop cursor.
     * @n:		another type * to use as temporary storage
     * @head:	the head for your list.
     * @member:	the name of the list_head within the struct.
     */
    #define list_for_each_entry_safe(pos, n, head, member)	\
        for (pos = list_first_entry(head, typeof(*pos), member),	\
            n = list_next_entry(pos, member);			\
             !list_entry_is_head(pos, head, member); 		\
             pos = n, n = list_next_entry(n, member))
    ```
6. [**list_del**](https://elixir.bootlin.com/linux/v5.15.137/source/include/linux/list.h#L138)：將當前遍歷到的節點從wait queue中移除。
7. [**wake_up_process**](https://elixir.bootlin.com/linux/v5.15.137/source/kernel/sched/core.c#L4209)：喚醒指定進程(task)，讓其從睡眠狀態恢復到可執行狀態(TASK_RUNNING)，並加入到run queue中。  
8. **msleep**：每次迭代間隔一些時間，以毫秒為單位。

### 執行結果(FIFO)  
根據執行結果可以發現，進出的順序有按照FIFO。  
![image](https://hackmd.io/_uploads/rJuGg59B1e.png)  

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
**編譯與執行的指令如下**
```b
# gcc -o 編譯出來的檔案名 編譯程式碼.c，以下指令是舉例
gcc -o user_space user_space.c

# 執行程式
./user_space
```
:::  

### 執行結果 (FIFO)  
根據kernel space的執行結果，**process從wait queue出來的順序確實有按照FIFO**，但從wait queue出來到在user space印出的這段過程中，仍然可能會因為CPU排程而使得在user space印出的順序不一定照順序，因此**在list_for_each_entry_safe中加入msleep，讓每個process之間的間隔時間長一點來解決在user space中印出的問題。**  
![image](https://hackmd.io/_uploads/Syi0y99Bkl.png)

---

## Project 遇到的問題
**Q**：在kernel space的clean_wait_queue當中，list_for_each_entry_safe迴圈內**若改成先執行wake_up_process(task)再執行list_del(&entry->entry)**，則user space在調用此函式時會當機；反之則正常執行。  
**Ans**：在執行完wake_up_process(task)後，process會被喚醒並進入run queue，但在還未執行list_del(&entry->entry)之前，該process仍位於wait queue。也就是說，<font color=red>**process會有一小段時間同時在wait queue和run queue當中</font>，如同上課提到sleep_on()的race condition也可能會產生這種狀況**。因此必須要先把process從wait queue移出後再執行喚醒，user space的執行才能順利進行。

---

## Reference
[等待队列（一）](https://www.cnblogs.com/zhuyp1015/archive/2012/06/09/2542882.html)  
[linux等待队列wait queue](https://blog.csdn.net/q2519008/article/details/123816780)  
[Linux等待队列（Wait Queue）](https://www.cnblogs.com/hueyxu/p/13745029.html)  
[Process State 與 Wait Queue](https://hackmd.io/@MEME48/Hy65Z5A_h)  
[Linux kernel 的 wait queue 機制](https://www.readfog.com/a/1704747462120542208)  
[linux内核中的等待队列的基本操作](https://blog.51cto.com/weiguozhihui/1566704)  
[进程调度API之add_wait_queue_exclusive](https://blog.csdn.net/tiantao2012/article/details/78791339)  
[Linux 内核等待队列 --- wait_queue_head --- wait_event_interruptible](https://blog.csdn.net/wenjin359/article/details/83002379)  
