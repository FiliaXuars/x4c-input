#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>

void usage()
{
	printf("program_name\t<destination>\n\t\t<127.0.0.1>\n");
}

struct NewTcpSocket
{
	int file;
	struct sockaddr_in socket_address_in;
};

struct NewTcpSocket create_socket(char *argument[])
{
	struct in_addr address_in;
	int address_number = inet_aton(argument[1], &address_in);
	if (address_number == 0)
	{
		printf("bad address\n");
		exit(1);
	}

	struct sockaddr_in socket_address_in = 
	{
		AF_INET,
		htons(5863),
		address_in
	};

	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket == -1)
	{
		printf("socket creation failed");
		exit(errno);
	}

	struct NewTcpSocket tcp_socket_struct =
	{
		tcp_socket,
		socket_address_in
	};
	return tcp_socket_struct;
}

void start_connection(struct NewTcpSocket tcp_socket)
{
	int connection = connect(tcp_socket.file, (struct sockaddr *)&tcp_socket.socket_address_in, sizeof(tcp_socket.socket_address_in));
	if (connection == -1)
	{
		printf("connection failed\n");
		exit(errno);
	}
}

void close_connection(int tcp_socket, int files_count)
{
	if (close(tcp_socket) == -1)
	{
		printf("failed to shutdown tcp connection\n");
		exit(errno);
	}
}

enum terminal_settings
{
	ECHOSETTING,
	CANONICALSETTING
};

void terminal_control(unsigned setting, unsigned value)
{
	struct termios terminal;

	tcgetattr(STDIN_FILENO, &terminal);
	switch (setting)
	{
		case ECHOSETTING:
			terminal.c_lflag ^= ((-(terminal.c_lflag+1)) & ECHO) ^ (value * ECHO);
			break;
		case CANONICALSETTING:
			terminal.c_lflag ^= ((-(terminal.c_lflag+1)) & ICANON) ^ (value * ICANON);
			break;
		default:
			break;
	}
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal);
}

void terminal_control_raw(int state)
{
	if (state == 0)
	{
		terminal_control(ECHOSETTING, 0);
		terminal_control(CANONICALSETTING, 0);
	}
	else
	{
		terminal_control(ECHOSETTING, 1);
		terminal_control(CANONICALSETTING, 1);
	}
}

static volatile unsigned exit_value = 0;
static volatile unsigned force_exit = 0;

void interrupt_handler(int _)
{
	printf("press any button to continue\n");
	exit_value = 130;
	force_exit++;
	if (force_exit >= 3)
	{
		printf("forcibly exiting\n");
		exit(1);
	}
}

void handle_exit()
{
	terminal_control_raw(0);
	exit(errno);
}

void main(int argument_count, char *arguments[])
{
	signal(SIGINT, interrupt_handler);
	atexit(handle_exit);

	if (argument_count != 2)
	{
		usage();
		exit(2);
	}

	terminal_control_raw(1);
	while (exit_value == 0)
	{
		char key;
		struct NewTcpSocket tcp_socket = create_socket(arguments);
		start_connection(tcp_socket);
		read(STDIN_FILENO, &key, 1);
		//printf("%u\n", key);
		if (send(tcp_socket.file, &key, sizeof(char), 0) == -1)
		{
			exit(errno);
		}
		close_connection(tcp_socket.file, 2);
	}

	exit(exit_value & 65534);
}
