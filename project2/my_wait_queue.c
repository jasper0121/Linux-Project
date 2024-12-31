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
