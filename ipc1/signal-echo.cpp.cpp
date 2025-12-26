#include <iostream>
#include <unistd.h>     
#include <signal.h>     
#include <sys/types.h>  
#include <pwd.h>        
#include <string.h>    
#include <ucontext.h>   
#include <atomic>

struct SignalInfo {
    pid_t pid;
    uid_t uid;
    unsigned long rip;
    unsigned long rax;
    unsigned long rbx;
};

std::atomic<bool> signal_received(false);
SignalInfo last_info = {0};

const char* get_username_by_uid(uid_t uid) {
    struct passwd *pw = getpwuid(uid);
    return (pw) ? pw->pw_name : "Unknown User";
}

void sig_handler(int signo, siginfo_t *info, void *context) {
    ucontext_t *uc = (ucontext_t *)context;
    
    last_info.pid = info->si_pid;
    last_info.uid = info->si_uid;

    #ifdef __x86_64__
        last_info.rip = uc->uc_mcontext.gregs[REG_RIP];
        last_info.rax = uc->uc_mcontext.gregs[REG_RAX];
        last_info.rbx = uc->uc_mcontext.gregs[REG_RBX];
    #elif __i386__
        last_info.rip = uc->uc_mcontext.gregs[REG_EIP];
        last_info.rax = uc->uc_mcontext.gregs[REG_EAX];
        last_info.rbx = uc->uc_mcontext.gregs[REG_EBX];
    #endif

    signal_received.store(true);
    
\
    const char* msg = "\n[Signal Handled]\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); 
    
    sa.sa_sigaction = sig_handler; 
    sa.sa_flags = SA_SIGINFO | SA_RESTART; 
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    std::cout << "--- Signal Echo Program ---\n";
    std::cout << "PID: " << getpid() << "\n";
    std::cout << "Command: kill -SIGUSR1 " << getpid() << "\n\n";

    while (true) {
        if (signal_received.load()) {
            std::cout << "\n--- SIGNAL RECEIVED ---" << std::endl;
            std::cout << "From PID: " << last_info.pid << "\n";
            std::cout << "From UID: " << last_info.uid << " (" << get_username_by_uid(last_info.uid) << ")\n";
            printf("CPU Context: RIP = 0x%lx, RAX = 0x%lx, RBX = 0x%lx\n", 
                   last_info.rip, last_info.rax, last_info.rbx);
            std::cout << "-----------------------" << std::endl;
            
            signal_received.store(false);
        }

        usleep(500000); 
    }

    return 0; 
}