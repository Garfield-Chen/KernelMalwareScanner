#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <time.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/

struct iovec iov;
int socket_l;
struct msghdr msg;
struct sockaddr_nl s_addr, d_addr;
struct nlmsghdr *nlh = NULL;


void socket_init(int argc, char *argv[])
{
	socket_l = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	
	// Check to make sure socket_l initialized correctly.
	if(socket_l<0) { return; }

	// Initialize socket address parameters for kernel
	memset(&d_addr, 0, sizeof(d_addr));
	d_addr.nl_family = AF_NETLINK;
	d_addr.nl_pad = 0; // no padding
	d_addr.nl_pid = 0; // kernel pid
	d_addr.nl_groups = 0; // set to unicast

	// Initialize socket address parameters for user
	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.nl_family = AF_NETLINK;
	s_addr.nl_pad = 0; // no padding
	s_addr.nl_pid = getpid(); // set pid of source to this process's pid
	bind(socket_l, (struct sockaddr*)&s_addr, sizeof(s_addr));

	// setup message header
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	// Before using NLMSG_DATA, copy all of argv into nlh data
	// Starts at 1 to ignore program name
	for (int i = 1; i < argc; i++)
	{
		strcat(NLMSG_DATA(nlh), argv[i]);
		strcat(NLMSG_DATA(nlh), " ");
	}

	// Finish IOV settup so message can copied into nlh
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&d_addr;
	msg.msg_namelen = sizeof(d_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

}

int monitor_syscalls(int argc, char* argv[])
{
    if (argc < 3) {
	printf("usage: procman -sc <PID_1> <PID_2> ... <PID_N>\n");
	return 1;
    }

    if (getuid() != 0) {
	printf("system call monitoring requires root access!\n");
	return 2;
    }
    
    // put the system call name map into memory
    
    FILE* syscall_map;
    syscall_map = fopen("./syscall.map", "r");

    if (syscall_map == NULL) {
	printf("error: could not read system call map");
	return 3;
    }
    
    int size;
    fscanf(syscall_map, "%i", &size);

    int index;
    char name[20]; 
    char syscall_name[size][20];

    while(fscanf(syscall_map, "%i %s", &index, name) != EOF)	
	strcpy(syscall_name[index], name);
    
    // create additional tracing processes if needed
    
    pid_t pid;   
    int i = 2;
    while (i < argc) {

	pid_t child;
	pid = atoi(argv[i]);

	if (i != (argc - 1))
	    child = fork();
	
	if (child == 0)		
	    i = argc;		
	else	
	    i++;
    }
	    
    int status;
    struct user_regs_struct regs;
    
    ptrace(PTRACE_ATTACH, pid, 0, 0);
    
    while (1) {

	ptrace(PTRACE_SYSCALL, pid, 0, 0);
	waitpid(pid, &status, 0);
	
	ptrace(PTRACE_GETREGS, pid, 0, &regs);

	time_t unix_time;
	struct tm *local_time;
	
	time(&unix_time);
	local_time = localtime(&unix_time);
	
	char time_str[20];
	strftime(time_str, 20, "%F %T", local_time);		
	
	int syscall = regs.orig_rax;	
	printf("[%s] PID %i called %s (%i) \n", time_str, pid, syscall_name[syscall], syscall);

	ptrace(PTRACE_SYSCALL, pid, 0, 0);
	waitpid(pid, &status, 0);

    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    
	// Check for valid input
	//if (argc < 2 || argc > 4)
    if (argc < 2)
	{
		printf("\nInvalid number of arguments\n.");
		exit(0);
	}

	if (strcmp(argv[1], "-sc") == 0)
	{
	    monitor_syscalls(argc, argv);
	}

	else if (strcmp(argv[1], "ps") == 0 || strcmp(argv[1], "kn") == 0)
	{
		// Initializes socket_l with raw datagram type using netlink standard
		socket_init(argc, argv);
		sendmsg(socket_l,&msg,0);

		// Read kernel message for process search
		recvmsg(socket_l, &msg, 0);
		printf("%s\n", (char *)NLMSG_DATA(nlh));
		close(socket_l);
	}
}

