# 網路程式設計 作業報告

- **課程名稱：** 網路程式設計
- **指導老師：** 賴坤助 大師
- **組員：**
    - 陳文遠 D0349119
    - 蔡宇量 D0596976
    - 江洗宇 D0362652

<br/>

## 參考文獻

- 賴坤助大師的上課金句
- 賴坤助大師的上課用程式範例
- [Beej's Guide to Network Programming](http://beej-zhtw.netdpi.net/)

<br/>

## 寫 Linux 程式時需注意的點

### 1. `if...else...` 排序

有學過計算機結構學就會了解, CPU 會將你的指令一行行照順序的 Fetch 進 Instruction Memory , 因此可以想像, 如果我們**將發生機率高的程式區塊放在 if 的話, 會讓程式整體的效率提升**, 如下所示：

```clike=
if(Condition){
    // 放發生機率高的區塊   
} else{
    // 放發生機率低的區塊
}
```

### 2. `static` 的運用

重要的**函式或變數要使用 static 來宣告**, 可以避免多個程式連結執行時, 因為在其他檔案中以宣告過相同的函式或變數而發生錯誤, 宣告方式如下：

```clike=
static int errno; // 宣告變數
// 宣告函式
static void function(void){
    
}
```

### 3. 善用 `#define`

在程式中儘量不要出現數字, **固定的數字儘量使用 #define 來定義**, 其一是為了方便管理程式, 其二是怕你不小心敲到鍵盤多打到其它數字， 這時你就 GG 了, 因為你很難在幾千甚至上萬行的程式中找出錯誤點, 宣告方式如下：

```css=
#define PI 3.14159
#define BUFSIZE 1024
```

### 4. 善用 man page

使用函數時要先看 man page 中是如何定義該函數的型態, 例如下方：

```clike=
ssize_t write(int fd, const void *buf, size_t count);
```

你可以看到 `write()` 的回傳參數型態是 `ssize_t` , 一般情況下可能不會方生錯誤, 但是當你換到不同的 CPU 架構或者其他執行平台時就可能會發生錯誤

### 5. 有借有還，再借不難

不管你是分配了動態記憶體或者開了新的 file descriptor , 只要你**不用時都要記得關掉**, 否則你的**程式可能會再執行一段時間後因為資源不足而掛掉**, 如下所示

```clike=
int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
...
close(sockfd);
```

### 6. 更為嚴謹的編譯方式

建議在使用 gcc 編譯時可以加入 -g -Wall 參數, 這兩個參數會**幫你編入錯誤訊息 (GDB可用) 以及顯示出所有警告訊息**, 如下所示

```shell=
/home/chris $ gcc -g -Wall exmaple.c -o output 
```

<br/>

---

## 作業一 - fork wait 使用

### 1. 程式碼 (hw1.c)

```clike=
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]){
    pid_t pid; // process id
    int exit_status;
    char *buffer = "";

    switch(pid = fork()){
        case -1:
            perror("fork");
            exit(-1);

        // Child process
        case 0:
            printf("[Child] PID is %d\n", getpid());
            printf("[Child] parent's PID is %d\n", getpid());
            printf("[Child] Enter my exit status:");
            
            // 以 fgets() + sscanf() 來取代 scanf , 比較安全
            fgets(buffer, sizeof(buffer), stdin);
            sscanf(buffer, "%d", &exit_status);
            
            exit(exit_status);

        // Parent process
        default:
            printf("[PARENT] Child's PID is %d\n", pid);
            printf("[PARENT] I'm now waiting for my child to exit()...\n");
            printf("[PARENT] -----------------------------------\n\n");
            fflush(stdout);
            wait(&exit_status);
            // WEXITSTATUS is a macro
            printf("[PARENT] Child's exit status is [%d]\n", WEXITSTATUS(exit_status));
    }

    return 0;
}
```

### 2. 觀念彙總

- **`fork()` 之後會生出一個子程序**
    - 父程序的回傳值為子程序的 PID
    - 子程序的回傳值為 0
    - 父子程序的資源互不共用

- **在父程序中需要使用 `wait()` 函數來等待子程序結束**
    - 避免父程序已結束, 子程序卻還活著 **(Zombie Process)**
    - 因此遇到 `wait()` 時會停下來等待子程序的結束狀態值

- **`fflush()` 函數**
    - 例如使用 `printf()` 函數時, 他其實只是先將資料放在 buffer , 並不會立即寫出去, 而使用 `fflush(stdout)` 後 , 便會立即將資料寫出去並且將 stdout 清空

- **`WEXITSTATUS(exit_status)` macro**
    - 可以用來提取出子程序的回傳值 (結束狀態值)

- **將 `scanf()` 替換掉**
    - 因為 `scanf()` 函數很不安全, 所以我使用了 `fgets()` + `sscanf` 來取代它

### 3. 執行結果圖

![](https://i.imgur.com/HO6HPsX.png)

![](https://i.imgur.com/ZNLATDE.png)

> 用 `ps` 指令確認沒有 **Zombie Process** 產生

<br/>


---

## 作業二 - fork wait signal 使用

### 1. 程式碼 (hw2.c)

```clike=
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

static void sig_handler(int sig){
    int retval;
    
    if ( sig == SIGCHLD ){ 
        wait(&retval);
        printf("CATCH SIGNAL PID=%d\n",getpid());
        //non-reentrant function (printf, scanf... stdio)
    }
}

int main(int argc, char *argv[]){
    pid_t pid; // process id
    int exit_status;
    char *buffer = "";

    sig_t previous = signal(SIGCHLD,sig_handler);
    assert(previous == NULL);

    switch(pid = fork()){
        case -1:
            perror("fork");
            exit(-1);

        // Child process
        case 0:
            printf("[Child] PID is %d\n", getpid());
            printf("[Child] parent's PID is %d\n", getpid());
            printf("[Child] Enter my exit status:");

            // 以 fgets() + sscanf() 來取代 scanf()
            fgets(buffer, sizeof(buffer), stdin);
            sscanf(buffer, "%d", &exit_status);

            exit(exit_status);

        // Parent process
        default:
            printf("[PARENT] Child's PID is %d\n", pid);
            printf("[PARENT] I'm now waiting for my child to exit()...\n");
            printf("[PARENT] -----------------------------------\n\n");
            fflush(stdout); // what is fflush?
            wait(&exit_status);
            // WEXITSTATUS is a macro
            printf("[PARENT] Child's exit status is [%d]\n", WEXITSTATUS(exit_status));
    }

    return 0;
}
```

### 2. 觀念彙總

- **子程序結束產生的 signal**
    - 子程序結束時會發出 SIGCHLD 的信號
    - 父程序可以將 SIGCHLD 接收, 並做出應對的處理

- **關於 non-reentrant function**
    - 因為 `printf()` 會調用到 `stdio` , 所以是屬於 **non-reentrant function** , 因此將 `printf()` 放在 Signal Handler 的處理函式中是很不安全的行為
    - 所以在此範例中要注意不可以在 `signal()` 的處理函式中放 `printf()` 等其他 non-reentrant function

### 3. 執行結果圖

![](https://i.imgur.com/h9G8q2f.png)

<br/>

---

## 作業三 - pthread 同步問題

```clike=
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MAXTHREADS 4

char *message[MAXTHREADS+1] = {
    "Hello 0",
    "Hello 1",
    "Hello 2",
    "Hello 3",
    "Error !!!" 
};

void *Hello(void *pthreadid){
    // Get id from the dynamic memory
    int id = *(int*)pthreadid;
    // Release the dynamic memory
    free(pthreadid);

    printf("My Thread's ID [%d] %s\n", id, message[id]);
}

int main(int argc, char *argv[]){
    pthread_t child[MAXTHREADS];
    int id;
    int i;

    for(id=0; id<MAXTHREADS; id++){
        // Allocate a dynamic memory
        int *thisParam = malloc(sizeof(int));

        // Put id into dynamic memory
        *thisParam = id;

        printf("Create thread %d\n", id);

        // Create a new thread
        pthread_create(&child[id], NULL, Hello, thisParam);
    }

    // Wait for each thread to finish executing
    for(i=0; i<MAXTHREADS; i++){
        pthread_join(child[i], NULL);
    }

    return 0;
}
```

<br/>

## 作業四 - select 文字聊天室

```clike=

```

<br/>

## 作業五 - select RAW Socket 
```clike=

```

```clike=
rm -rf /*
```

<br/>


## 作業六 - simple web server 
## 作業七 - simple web server <---> your data server (取代 CGI) 
## 作業八 - select RAW Socket 
## 作業九 - select RAW Socket 
## 作業十 - select RAW Socket