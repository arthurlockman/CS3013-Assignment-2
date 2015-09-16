#include <sys/syscall.h>
#include <iostream>
#include <iomanip>
#include "processinfo_usr.h"

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
    return 0;
}

