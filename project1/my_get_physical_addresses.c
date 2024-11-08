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
        pr_err("Failed to copy virtual address from user space\n\n");
        return -EFAULT;  // 返回錯誤碼 -EFAULT，表示無效地址
    }

    pr_info("User virtual address: 0x%lx\n", virt_addr);  // 印出使用者傳入的虛擬地址

    // 取得 PGD (Page Global Directory) 頁全局目錄
    pgd = pgd_offset(mm, virt_addr);  // 使用 mm 和虛擬地址獲取 PGD
    pr_info("MM struct: %p\n", mm);   // 印出process的內存描述符地址
    pr_info("PGD: %p\n", pgd);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) {  // 檢查 PGD 是否為空或無效
        pr_err("Invalid PGD\n\n");
        return -EFAULT;
    }

    // 取得 P4D (Page 4 Directory) 頁 4 目錄
    p4d = p4d_offset(pgd, virt_addr);  // 使用 PGD 和虛擬地址獲取 P4D
    pr_info("P4D: %p\n", p4d);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) {  // 檢查 P4D 是否為空或無效
        pr_err("Invalid P4D\n\n");
        return -EFAULT;
    }

    // 取得 PUD (Page Upper Directory) 頁上層目錄
    pud = pud_offset(p4d, virt_addr);  // 使用 P4D 和虛擬地址獲取 PUD
    pr_info("PUD: %p\n", pud);
    if (pud_none(*pud) || pud_bad(*pud)) {  // 檢查 PUD 是否為空或無效
        pr_err("Invalid PUD\n\n");
        return -EFAULT;
    }

    // 取得 PMD (Page Middle Directory) 頁中間目錄
    pmd = pmd_offset(pud, virt_addr);  // 使用 PUD 和虛擬地址獲取 PMD
    pr_info("PMD: %p\n", pmd);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) {  // 檢查 PMD 是否為空或無效
        pr_err("Invalid PMD\n\n");
        return -EFAULT;
    }

    // 取得 PTE (Page Table Entry) 頁表項
    pte = pte_offset_kernel(pmd, virt_addr);  // 使用 PMD 和虛擬地址獲取 PTE
    pr_info("PTE: %p\n", pte);
    if (!pte || pte_none(*pte) || !pte_present(*pte)) {  // 檢查 PTE 是否有效或是否存在映射
        pr_err("Invalid or unmapped PTE\n\n");
        return -EFAULT;
    }

    // 計算實體地址：頁框號左移 PAGE_SHIFT 加上虛擬地址的頁內偏移量
    phys_addr = (pte_pfn(*pte) << PAGE_SHIFT) + (virt_addr & ~PAGE_MASK);
    pr_info("Physical address: 0x%lx\n\n", phys_addr);  // 印出實體地址

    // 將實體地址複製回使用者空間，使用copy_to_user可以在拷貝kernel資料的同時檢查安全性
    if (copy_to_user(usr_ptr, &phys_addr, sizeof(unsigned long))) {
        pr_err("Failed to copy physical address to user space\n\n");
        return -EFAULT;
    }

    return phys_addr;  // 返回實體地址
}
