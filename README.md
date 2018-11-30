# KernelMalwareScanner
2431 Project - Linux kernel module designed to scan and perform functions on running processes

Use with the command kmscan with the following parameters

Project Description:
	This project started with the goal of being a kernel module based malware scanner. It retains much of the same functionality that was initially planned, such as process searching, a system call monitor, and additional functionality, such as the ability to kill a process by name. The project goes by the name “KernelMalwareScanner” on Github, but this is consider more of a Process Manage program, given that this was not tested to work with any known Linux malware programs. It works best with trusted programs running on your host machine. 

Compilation:
	The included Makefile will compile the user.c and kscan.c files for the user program and and kernel module respectively. The default “make” command will compile as well use insmod on the the compiled kscan.ko file afterwards. Be ready to input your password for sudo for the insmod command after compilation completes. The “make clean” command will removed all relevant object and kernel module files, as well as rmmod kscan afterward, which also requires a password for sudo.

Functionality:

Basics:
	After compilation, an executable file called “user” will be used as the basis for sending commands to the kernel. The userspace program and the kernel module communicate through a netlink connection between the two. The kscan module collects a string command from the user and parses it based on the function that is need, including any parameters that are needed to work. This connection is terminated when either user or kernel module programs terminate.

Process Search (ps): 
With the program compiled, using ./user ps followed by a process name will search through running tasks with the “for_each_process” macro from within <linux/sched.h>. “for_each_process” steps through all available task_struct within proc_fs, and the task_struct value comm is used to check the name of the process itself. Using “./user ps ext” will return each running process with the substring “ext”. Using “./user ps” with no parameter will simply return every process currently running.

Kill by Process Name (kn):
	With the program compiled, usering ./user kn followed by a process name will kill the process with that name. This works in a similar way to process search, but when it finds the process, it uses the send_sig_info function to send a SIGKILL signal for the given task_struct task used, killing the process. The name of the process given must match the running process completely. For example, “./user kn firefox” will kill any open Firefox windows, but “./user kn firefo” will not kill any Firefox processes.

Monitor System Calls (sc):
The system call monitor uses the ptrace system call to trace all system calls that a set of processes make. The only time ptrace does not need root access is when the tracing process is tracing its own child. It was designed to trace an existing process, not launch a new one, so root access is needed. The tracer will stop the trace twice for each system call, once when the process enters the system call, and once after the system call is executed. Once the process enters the system call, it will generate the current time, grab the system call number from one of the CPU registers, and output it to the console. It doesn't do anything after the system call is executed, it just stops because of the way ptrace is designed. Before tracing, it reads a file which maps system call numbers to their names. System call numbers differ on different kernel versions and different architectures, so this will not work on all machines. The provided system call map works on x86_64 linux version 4.15. This does not have code inside the kernel, as it needs to fork to allow for multiple processes to be traced at the same time.
Usage example: ./user -sc 1234 6112 5532 9991
Traces system calls made by processes 1234, 6112, 5532, and 9991


Conclusion:
	Most of the basic functionality that was initially planned for this project as been implemented, with the addition of the “kn” command. Additional functionality that could be added to the system call monitor could be getting the parameters or the return values of the calls. There could also be the potential to add CPU and memory information about a given process based off of task_struct fields.
