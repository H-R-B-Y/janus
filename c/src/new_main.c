/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   new_main.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 15:18:29 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 15:26:52 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/signal.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/epoll.h>

// volatile int exit = 0;


int main()
{
	int epoll_fd;
	int nl_socket;
	int signal_fd;
	sigset_t mask;
	int		exit_loop = 0;

	// Block SIGINT so it doesn't get delivered normally
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1)
	{
		perror("pthread_sigmask");
		return (1);
	}
	
	// Now create signal fd
	signal_fd = signalfd(-1, &mask, 0);
	if (signal_fd == -1)
	{
		perror("signalfd");
		return (1);
	}

	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1");
		return (1);
	}
	nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (nl_socket == -1)
	{
		perror("socket");
		close(epoll_fd);
		return (1);
	}
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = nl_socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, nl_socket, &event) == -1)
	{
		perror("epoll_ctl");
		close(nl_socket);
		close(epoll_fd);
		return (1);
	}
	event.events = EPOLLIN;
	event.data.fd = signal_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, signal_fd, &event) == -1)
	{
		perror("epoll_ctl");
		close(nl_socket);
		close(epoll_fd);
		close(signal_fd);
		return (1);
	}
	while (!exit_loop)
	{
		struct epoll_event events[10];
		int n = epoll_wait(epoll_fd, events, 10, -1);
		if (n == -1)
		{
			perror("epoll_wait");
			break;
		}
		for (int i = 0; i < n; i++)
		{
			if (events[i].data.fd == nl_socket)
			{
				char buffer[4096];
				ssize_t len = recv(nl_socket, buffer, sizeof(buffer), 0);
				if (len == -1)
				{
					perror("recv");
					continue;
				}
				printf("Received netlink message of length %zd\n", len);
			}
			if (events[i].data.fd == signal_fd)
			{
				struct signalfd_siginfo fdsi;
				ssize_t s = read(signal_fd, &fdsi, sizeof(fdsi));
				if (s != sizeof(fdsi))
				{
					perror("read");
					continue;
				}
				if (fdsi.ssi_signo == SIGINT)
				{
					printf("Received SIGINT, exiting...\n");
					exit_loop = 1;
					break;
				}
			}
		}
	}
	close(nl_socket);
	close(signal_fd);
	close(epoll_fd);
	return (0);
}

