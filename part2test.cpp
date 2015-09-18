#include <sys/syscall.h>
#include <iostream>
#include <iomanip>
#include "processinfo_usr.h"
#include <stdlib.h>
#include <sys/wait.h>

using namespace std;

#define __NR_cs3013_syscall2 356

/**
 * Print a stat returned from the syscall.
 **/
void printStat(char const *stat, long long val)
{
    cout << left << setw(20) << stat << right << setw(20) << val << endl;
}

int main()
{
    int pid1, pid2;
    if ((pid1 = fork()) < 0)
    {
        cerr << "Fork failed!" << endl;
    }
    else if (pid1 == 0) //is child
    {
        sleep(3);
        cout << "I am a child process. I slept for 3 second." << endl;
        struct processinfo info;
        long retVal = (long) syscall(__NR_cs3013_syscall2, &info);
        cout << "Got a return value from syscall: " << retVal << endl;
        if (retVal == 0)
        {
            printStat("State:", info.state);
            printStat("PID:", info.pid);
            printStat("Parent PID:", info.parent_pid);
            printStat("Youngest Child:", info.youngest_child);
            printStat("Younger Sibling:", info.younger_sibling);
            printStat("Older Sibling:", info.older_sibling);
            printStat("UID:", info.uid);
            printStat("Start Time:", info.start_time);
            printStat("User Time:", info.user_time);
            printStat("System Time:", info.sys_time);
            printStat("Child User Time:", info.cutime);
            printStat("Child Sys Time:", info.cstime);
        }
        exit(0);
    }
    else //is parent
    {
        if ((pid2 = fork()) < 0)
        {
            cerr << "Fork failed!" << endl;
        }
        else if (pid2 == 0) //is child
        {
            sleep(2);
            cout << "I am a child process. I slept for 2 second." << endl;
            struct processinfo info;
            long retVal = (long) syscall(__NR_cs3013_syscall2, &info);
            cout << "Got a return value from syscall: " << retVal << endl;
            if (retVal == 0)
            {
                printStat("State:", info.state);
                printStat("PID:", info.pid);
                printStat("Parent PID:", info.parent_pid);
                printStat("Youngest Child:", info.youngest_child);
                printStat("Younger Sibling:", info.younger_sibling);
                printStat("Older Sibling:", info.older_sibling);
                printStat("UID:", info.uid);
                printStat("Start Time:", info.start_time);
                printStat("User Time:", info.user_time);
                printStat("System Time:", info.sys_time);
                printStat("Child User Time:", info.cutime);
                printStat("Child Sys Time:", info.cstime);
            }
            exit(0);
        } else
        {
            cout << "I am a parent process. I am printing stats before my children wake up." << endl;
            struct processinfo info;
            long retVal = (long) syscall(__NR_cs3013_syscall2, &info);
            cout << "Got a return value from syscall: " << retVal << endl;
            if (retVal == 0)
            {
                printStat("State:", info.state);
                printStat("PID:", info.pid);
                printStat("Parent PID:", info.parent_pid);
                printStat("Youngest Child:", info.youngest_child);
                printStat("Younger Sibling:", info.younger_sibling);
                printStat("Older Sibling:", info.older_sibling);
                printStat("UID:", info.uid);
                printStat("Start Time:", info.start_time);
                printStat("User Time:", info.user_time);
                printStat("System Time:", info.sys_time);
                printStat("Child User Time:", info.cutime);
                printStat("Child Sys Time:", info.cstime);
            }
            int status;
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
            cout << "I am a parent process. I am printing stats after my child has woken." << endl;
            retVal = (long) syscall(__NR_cs3013_syscall2, &info);
            cout << "Got a return value from syscall: " << retVal << endl;
            if (retVal == 0)
            {
                printStat("State:", info.state);
                printStat("PID:", info.pid);
                printStat("Parent PID:", info.parent_pid);
                printStat("Youngest Child:", info.youngest_child);
                printStat("Younger Sibling:", info.younger_sibling);
                printStat("Older Sibling:", info.older_sibling);
                printStat("UID:", info.uid);
                printStat("Start Time:", info.start_time);
                printStat("User Time:", info.user_time);
                printStat("System Time:", info.sys_time);
                printStat("Child User Time:", info.cutime);
                printStat("Child Sys Time:", info.cstime);
            }
            exit(0);
        }
    }
    return 0;
}

