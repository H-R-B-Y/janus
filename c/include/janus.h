/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   janus.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 13:55:27 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 17:45:13 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef JANUS_H
# define JANUS_H

# ifndef _GNU_SOURCE
#  define _GNU_SOURCE
# endif

# ifndef __USE_GNU
#  define __USE_GNU
# endif

# ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 200809L
# endif

# ifndef _DEFAULT_SOURCE
#  define _DEFAULT_SOURCE
# endif

# ifndef INET_ADDRSTRLEN
# define INET_ADDRSTRLEN 16
# endif

// Conditional compilation for e-paper display support
#ifndef JANUS_TERMINAL_MODE
# include "DEV_Config.h"      // For hardware initialization
# include "EPD_2in13_V4.h"    // For display functions
# include "GUI_Paint.h"       // For drawing functions
# include "fonts.h"          // For font structures and definitions
#endif

# include <stdio.h>
# include <stdlib.h>

/*
Ok, we need to wait until eiher
WLAN0 or ETH0 is connected
We want to run this under EPOLL, and we want to register events for
SIGUSR1 and SIGUSR2.

*/
# include <unistd.h>
# include <fcntl.h>
# include <string.h>
# include <sys/epoll.h>
# include <signal.h>
# include <sys/socket.h>
# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <sys/signal.h>
# include <sys/signalfd.h>
# include <linux/netlink.h>
# include <linux/rtnetlink.h>
# include <sys/eventfd.h>
# include <net/if.h>

enum e_interface_status
{
	JAN_NO_INTERFACE = 0,
	JAN_ETH0 = (1 << 0),
	JAN_WLAN0 = (1 << 1)
};

static inline int	is_interface_up(int status, enum e_interface_status interface)
{
	return ((status & interface) != 0);
}

static inline void	mark_interface_up(int *status, enum e_interface_status interface)
{
	*status |= interface;
}

static inline void	mark_interface_down(int *status, enum e_interface_status interface)
{
	*status &= ~interface;
}


struct s_janus_data
{
	int	interface_status;
	// We need somewhere to store the IP addresses
	char	wlan0_interface[INET_ADDRSTRLEN];
	char	eth0_interface[INET_ADDRSTRLEN];
	

	/// @brief Netlink socket for monitoring network events
	int		netlink_socket;

	/// @brief Epoll file descriptor for event monitoring
	int		epoll_fd;

	/// @brief For handling signals like SIGINT, SIGUSR1, SIGUSR2
	int		signal_fd;

	/// @brief Event file descriptor for pushing a new image to the display
	int		event_fd;

	/// @brief Flag to control the main event loop
	int		exit_loop;

# ifndef JANUS_TERMINAL_MODE
	UBYTE	*image_buffer; // For e-paper display buffer
# endif

};

int	handle_net_event(struct s_janus_data *data, struct epoll_event *event);
int	handle_signal_event(struct s_janus_data *data, struct epoll_event *event);
int	handle_event_fd_event(struct s_janus_data *data, struct epoll_event *event);
int	janus_run(struct s_janus_data *data);
int	scan_existing_interfaces(struct s_janus_data *data);



#endif /* JANUS_H */
