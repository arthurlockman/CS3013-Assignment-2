Authors: Arthur Lockman (ajlockman) & Tucker Haydon (tchaydon)

To compile the files, run make with the Makefile in the present directory. The Makefile will create two .o files (1 for each part of the assignment). Execute the files to create a .ko file. Run "sudo insmod [.ko file]" to insert the system call interception. To remove it, call "sudo rmmod [.ko file]".

Part 1
--------------------------------------
For part 1 of the project, we used a Loadable Kernel Module to intercept the system open and close files. In our intereption function, we included system logging to each function. The program writes to the logs everytime a user opens and closes a file, making note of which user and which file. The program ensures that it only logs user activity by checking the uid of the open/close call. If the uid of the call is greater than or equal to 1000, a user called the function, otherwise the OS or kernel called the function. 

To our knowledge, the function works exactly as described and there are no bugs. 

Part 2
---------------------------------------
For part 2 of the project, we intercepted the "cs3013_syscall2" system call compiled into our linux kernel. While the old system call took no input parameters, we modified that to include a pointer to a processInfo struct. The new intercepted function returned a number of statistics regarding the calling process, including state, cpu usage, id, and sibling/children process info. The function is successfully able to collect all of this data through a "task_struct" struct, provided by a "current". The function fills out the data in the parameter struct and then returns 0. Since the input is a pointer to the struct, the data is conserved even through one function is in the kernel space and the calling function is in the user space. 

To our knowledge, everything works as expected with one exception. To gather information on sibling and child processes, we had to iterate through a list located in task_struct and find the older and younger sibling, and the youngest child. If the process had none of these, the PID returns -1. To check to ensure that it has any children, we check to see if the list is empty. If not, re return the PID of the last element in the list. We adopted a similar approach for the sibling processes but encountered some problems. The sibling list wraps around to the original element so we could not simply check the length of the list. To see if it had a younger child, we checked the PID of the next element in the list. If its ID was higher than the current element, then that element was younger. This worked great until we tried to apply the same test on the older sibling. For some reason, the list always had an element with a PID lower than the current, indicating an older child. Though, when we printed the PID out, we got 1. We weren't sure if that is correct, seeing as the process with a PID of 1 is the starup process. We tried testing by age created but that returned the same result. That is the only known error with part 2. 

Tests and Comments
----------------------------------------
We ran our program through a number of tests to see if the output made sense. After each instance or running the system call, the PID incremented upwards, indicating it was calculating the PID currectly. The PID of the parent did not change, as was expected. Since our test file was run from the shell, the parent PID should be the shell PID and should not change. At the end of each run, the state of the process was 0, indicating the process was running at the time it logged its state. It makes sense that all would log running when running.
