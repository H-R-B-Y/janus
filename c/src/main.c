/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 13:55:15 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 17:14:14 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int	deinit_process(struct s_janus_data *data)
{
	if (data == NULL)
		return (1);
	if (data->netlink_socket && data->netlink_socket != -1)
		close(data->netlink_socket);
	if (data->epoll_fd && data->epoll_fd != -1)
		close(data->epoll_fd);
	if (data->signal_fd && data->signal_fd != -1)
		close(data->signal_fd);
	if (data->event_fd && data->event_fd != -1)
		close(data->event_fd);
#ifndef JANUS_TERMINAL_MODE
	EPD_2in13_V4_Sleep();
	DEV_Module_Exit();
#else
	printf("Janus: Cleaning up (terminal mode)\n");
#endif
	return (0);
}

int	setup_netlink_socket(struct s_janus_data *data)
{
	struct sockaddr_nl addr;

	data->netlink_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (data->netlink_socket == -1)
	{
		perror("socket");
		return (1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTMGRP_IPV4_IFADDR;

	if (bind(data->netlink_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		perror("bind netlink socket");
		close(data->netlink_socket);
		return (1);
	}

	return (0);
}

int	setup_signal_fd(struct s_janus_data *data)
{
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1)
	{
		perror("pthread_sigmask");
		return (1);
	}
	data->signal_fd = signalfd(-1, &mask, 0);
	if (data->signal_fd == -1)
	{
		perror("signalfd");
		return (1);
	}
	return (0);
}

int	setup_event_fd(struct s_janus_data *data)
{
	data->event_fd = eventfd(1, EFD_NONBLOCK);
	return (0);
}

int	setup_epoll(struct s_janus_data *data)
{
	data->epoll_fd = epoll_create1(0);
	if (data->epoll_fd == -1)
	{
		perror("epoll_create1");
		return (1);
	}
	return (0);
}

int	register_epoll_events(struct s_janus_data *data)
{
	struct epoll_event event;

	event.events = EPOLLIN;
	event.data.fd = data->netlink_socket;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->netlink_socket, &event) == -1)
	{
		perror("epoll_ctl: netlink_socket");
		return (1);
	}
	event.events = EPOLLIN;
	event.data.fd = data->signal_fd;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->signal_fd, &event) == -1)
	{
		perror("epoll_ctl: signal_fd");
		return (1);
	}
	event.events = EPOLLIN;
	event.data.fd = data->event_fd;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->event_fd, &event) == -1)
	{
		perror("epoll_ctl: event_fd");
		return (1);
	}
	return (0);
}

int	init_process(struct s_janus_data *data)
{
#ifndef JANUS_TERMINAL_MODE
	// Init the hardware
	if (DEV_Module_Init() != 0)
	{
		dprintf(STDERR_FILENO, "Failed to initialize hardware module\n");
		return (1);
	}
	// Init the e-paper display
	EPD_2in13_V4_Init();
	// Clear the display
	EPD_2in13_V4_Clear();
#else
	printf("Janus: Starting in terminal mode\n");
#endif
	memset(data, 0, sizeof(struct s_janus_data));
	if (setup_netlink_socket(data))
		return (1);
	if (setup_signal_fd(data))
		return (1);
	if (setup_event_fd(data))
		return (1);
	if (setup_epoll(data))
		return (1);
	if (register_epoll_events(data))
		return (1);
	// Scan for existing interface addresses before starting event loop
	if (scan_existing_interfaces(data))
		return (1);
	return (0);
}

int main()
{
	struct s_janus_data	janus_data;

	memset(&janus_data, 0, sizeof(janus_data));
	if (init_process(&janus_data) != 0)
	{
		deinit_process(&janus_data);
		return (1);
	}
	janus_run(&janus_data);
	deinit_process(&janus_data);
	return (0);
}
