/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 13:55:15 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/09 12:31:22 by hbreeze          ###   ########.fr       */
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

int	setup_timerfd(struct s_janus_data *data)
{
	if (!data)
		return (dprintf(STDERR_FILENO, "No data to intialise timerfd\n"), 1);
	data->timerfd = timerfd_create(CLOCK_REALTIME, 0);
	if (data->timerfd < 0)
		return (perror("timerfd"), 1);
	return (0);
}

int	setup_netlink_socket(struct s_janus_data *data)
{
	struct sockaddr_nl addr;

	//								protocol NETLINK_ROUTE see man netlink.7
	data->netlink_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (data->netlink_socket == -1)
		return (perror("socket"), 1);
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	// Subscribe to both IPv4 address changes and link changes
	// For the life of me I cannot find the fucking man page
	// that defines the groups
	// they should be in man rtnetlink.7 but they arent
	// so i am not sure what groups are available to me?
	// it does say in netlink that this is a bitfield of up to 32 groups to subscribe too 
	// but rtnetlink does not say which flags to set for what group of messages
	addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_LINK;
	if (bind(data->netlink_socket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		return (perror("bind netlink socket"), 1);
	printf("Janus: Netlink socket set up, subscribed to groups: %u\n", addr.nl_groups);
	return (0);
}

int	setup_signal_fd(struct s_janus_data *data)
{
	sigset_t mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGUSR2);
	/*
	Reason for pthread mask:
	The signalfd() function creates a file descriptor that can be used to receive signals.
	However, for the signals to be delivered to this file descriptor, they must be blocked
	from being delivered in the traditional way (i.e., to the process or thread). By using pthread_sigmask(),
	we ensure that these signals are blocked for the entire process (or thread), allowing them to be received via the signalfd.
	*/
	if (pthread_sigmask(SIG_BLOCK, &mask, NULL) == -1)
		return (perror("pthread_sigmask"), 1);
	data->signal_fd = signalfd(-1, &mask, 0);
	if (data->signal_fd == -1)
		return (perror("signalfd"), 1);
	return (0);
}

int	setup_event_fd(struct s_janus_data *data)
{
	data->event_fd = eventfd(1, EFD_NONBLOCK);
	if (data->event_fd == -1)
		return (perror("eventfd"), 1);
	return (0);
}

int	setup_epoll(struct s_janus_data *data)
{
	data->epoll_fd = epoll_create1(0);
	if (data->epoll_fd == -1)
		return (perror("epoll_create1"), 1);
	return (0);
}

int	register_epoll_events(struct s_janus_data *data)
{
	struct epoll_event event;

	event.events = EPOLLIN;
	event.data.fd = data->netlink_socket;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->netlink_socket, &event) == -1)
		return (perror("epoll_ctl: netlink_socket"), 1);
	event.events = EPOLLIN;
	event.data.fd = data->signal_fd;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->signal_fd, &event) == -1)
		return (perror("epoll_ctl: signal_fd"), 1);
	event.events = EPOLLIN;
	event.data.fd = data->event_fd;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->event_fd, &event) == -1)
		return (perror("epoll_ctl: event_fd"), 1);
	event.events = EPOLLIN;
	event.data.fd = data->timerfd;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, data->timerfd, &event) == -1)
		return (perror("epoll_ctl: timer_fd"), 1);
	event.events = EPOLLIN;
	event.data.fd = STDIN_FILENO;
	if (epoll_ctl(data->epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event) == -1)
		return (perror("epoll_ctl: stdin"), 1);
	return (0);
}

int	init_process(struct s_janus_data *data)
{
	memset(data, 0, sizeof(struct s_janus_data));
#ifndef JANUS_TERMINAL_MODE
	if (DEV_Module_Init() != 0)
	{
		dprintf(STDERR_FILENO, "Failed to initialize hardware module\n");
		return (1);
	}
	EPD_2in13_V4_Init();
	// Clear the display
	EPD_2in13_V4_Clear();
	data->image_buffer = (UBYTE *)malloc(EPD_2in13_V4_WIDTH * EPD_2in13_V4_HEIGHT / 8);
	if (data->image_buffer == NULL)
		return (dprintf(STDERR_FILENO, "Failed to allocate memory for image buffer\n"), 1);
	Paint_NewImage(data->image_buffer, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, ROTATE_270, WHITE);
	Paint_DrawString_EN(0, 0, "Janus: starting", &Font16, BLACK, WHITE);
	EPD_2in13_V4_Display(data->image_buffer);
#else
	printf("Janus: Starting in terminal mode\n");
#endif
	if (setup_netlink_socket(data))
		return (1);
	if (setup_signal_fd(data))
		return (1);
	if (setup_event_fd(data))
		return (1);
	if (setup_epoll(data))
		return (1);
	if (setup_timerfd(data))
		return (1);
	if (register_epoll_events(data))
		return (1);
	if (scan_existing_interfaces(data))
		return (1);
	return (0);
}

int main()
{
	struct s_janus_data	janus_data;
	int					stat;

	memset(&janus_data, 0, sizeof(janus_data));
	if ((stat = init_process(&janus_data)) == 0)
		janus_run(&janus_data);
	deinit_process(&janus_data);
	return (0);
}
