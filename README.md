# Operating Systems Projects 

## Overview  
This repository contains **three major projects** from **CIS 415: Operating Systems**, where I developed hands-on experience with **systems programming, process scheduling, multithreading, and synchronization** in **C**. Each project progressively built on core OS concepts such as **system calls, process management, signals, and thread synchronization**.

## Projects 📂  

### **Project 1 – Pseudo-Shell (Unix Command-Line Interface)**  
📌 **Description:**  
Developed a **simplified Unix shell**, capable of executing basic file and directory manipulation commands. The shell supports both **interactive mode** (user enters commands via CLI) and **file mode** (batch execution from a script).  

🔹 **Key Features:**  
✅ Implements Unix commands using **system calls** (e.g., `fork()`, `exec()`, `open()`, `read()`, `write()`)  
✅ Supports **file mode execution** for batch processing  
✅ Includes **error handling** for invalid commands and parameters  
✅ Uses **dynamic memory allocation** (`malloc`, `free`) to prevent memory leaks  
✅ Validated with **Valgrind** to ensure correct memory management  

---

### **Project 2 – MCP: Ghost-in-the-Shell (Process Scheduling & Signal Handling)**  
📌 **Description:**  
Implemented a **Master Control Program (MCP)** that launches and manages multiple processes using **system calls** (`fork()`, `exec()`, `wait()`, `kill()`). The MCP schedules processes in a **round-robin fashion** and synchronizes execution using **signals**.  

🔹 **Key Features:**  
✅ Uses `fork()` and `execvp()` to launch processes from a workload file  
✅ Implements **signal handling** (`SIGSTOP`, `SIGCONT`, `SIGUSR1`) for process scheduling  
✅ Uses **alarm signals** (`SIGALRM`) to enforce time slices for scheduling  
✅ Introduces **resource monitoring** via `/proc` for system insights  

---

### **Project 3 – Duck Bank (Multithreading & Synchronization)**  
📌 **Description:**  
Developed a **multi-threaded banking system** that processes thousands of transactions concurrently while maintaining **thread safety** using **mutexes and condition variables**. Transactions include **deposits, withdrawals, and fund transfers**, with periodic balance updates using **thread coordination**.

🔹 **Key Features:**  
✅ Uses **POSIX threads (`pthreads`)** for concurrent transaction processing  
✅ Implements **mutexes (`pthread_mutex_lock`)** to prevent race conditions  
✅ Coordinates **worker threads** and a **bank thread** using **condition variables**  
✅ Uses **barriers (`pthread_barrier_wait`)** for synchronized balance updates  
✅ Implements **inter-process communication (IPC)** via `mmap()` for a secondary bank process  

---

## Lessons Learned 💡  
- **Project 1**: Reinforced **system calls, process creation, and file I/O** in C.  
- **Project 2**: Gained experience in **process scheduling and signal handling**.  
- **Project 3**: Developed strong skills in **thread synchronization and shared memory management**.  

## Author ✨  
👨‍💻 **Nikhar Ramlakhan**  
📚 **CIS 415 - Operating Systems** | University of Oregon  
📝 **Instructor:** Prof. Allen Malony  

---

## License  
This repository contains coursework projects and is intended for **educational purposes** only.
