/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   janus.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 13:55:27 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/06 11:10:24 by hbreeze          ###   ########.fr       */
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

/*
Should really double check that all of these are needed,
but for now include them all.
*/
# include <stdio.h>
# include <stdlib.h>
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
# include <ifaddrs.h>
# include <arpa/inet.h>

/* Default interfaces; can be overridden via compiler -DINTERFACE_1/2 */
# ifndef INTERFACE_1
/// @brief Name of the first network interface to monitor (e.g., "eth0")
#  define INTERFACE_1 "eth0"
# endif
# ifndef INTERFACE_2
/// @brief Name of the second network interface to monitor (e.g., "wlan0")
#  define INTERFACE_2 "wlan0"
# endif

/// @brief Enumeration for interface status flags
enum e_interface_status
{
	/// @brief No interfaces are up
	JAN_NO_INTERFACE = 0,
	/// @brief First interface is up
	JAN_INTERFACE_1 = (1 << 0),
	/// @brief Second interface is up
	JAN_INTERFACE_2 = (1 << 1)
};


# define INTERFACE_1_NAME JAN_INTERFACE_1
# define INTERFACE_2_NAME JAN_INTERFACE_2

/**
 * @brief Test if an interface is marked as up in the status
 * 
 * @param status The status bitfield
 * @param interface The interface to check
 * @return int 1 if up, 0 if down
 */
static inline int	is_interface_up(int status, enum e_interface_status interface)
{
	return ((status & interface) != 0);
}

/**
 * @brief Mark an interface as up in the status bitfield
 * 
 * @param status The status bitfield
 * @param interface The interface to mark as up
 */
static inline void	mark_interface_up(int *status, enum e_interface_status interface)
{
	if (status)
		*status |= interface;
}

/**
 * @brief Mark an interface as down in the status bitfield
 * 
 * @param status The status bitfield
 * @param interface The interface to mark as down
 */
static inline void	mark_interface_down(int *status, enum e_interface_status interface)
{
	if (status)
		*status &= ~interface;
}
/// @brief Janus internal data
struct s_janus_data
{
	/// @brief Bitfield representing the status of monitored interfaces
	int		interface_status;
	
	/// @brief IP address for the first interface
	char	interface1_addr[INET_ADDRSTRLEN];

	/// @brief IP address for the second interface
	char	interface2_addr[INET_ADDRSTRLEN];

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
	/// @brief Buffer for e-paper display image
	/// @note Only used if JANUS_TERMINAL_MODE is not defined
	/// @warning DO NOT FREE THIS POINTER it will be managed by the display library
	UBYTE	*image_buffer;
# endif

};

/**
 * @brief Handler for new netlink events
 * 
 * This function processes netlink messages received on the netlink socket.
 * There are two main types of messages we care about:
 * - RTM_NEWADDR: A new IP address has been assigned to an interface.
 * - RTM_DELADDR: An IP address has been removed from an interface.
 * 
 * The events we subscribe to are:
 * - RTMGRP_IPV4_IFADDR: IPv4 address changes
 * - RTMGRP_LINK: Link status changes (interface up/down)
 * 
 * Other possible events (not currently handled):
 * - RTM_NEWLINK: A network interface has changed state (e.g., up/down)
 * - RTM_DELLINK: A network interface has been removed
 * 
 * @param data Janus data structure
 * @param event Epoll event for the netlink socket
 * @return int non-zero on error
 */
int	handle_net_event(struct s_janus_data *data, struct epoll_event *event);

/**
 * @brief Signal event handler
 * 
 * Currently there are three signals we setup for the signalfd:
 * - SIGINT: Used to gracefully exit the program
 * - SIGUSR1: Used to trigger an immediate update of the display
 * - SIGUSR2: Currently does nothing, later could be used for debugging or something
 * 
 * @param data Janus data structure
 * @param event Epoll event for the signal fd
 * @return int non-zero on error
 */
int	handle_signal_event(struct s_janus_data *data, struct epoll_event *event);

/**
 * @brief Event fd handler
 * 
 * @note I would like to rename this because we could later introduce new event fds
 * for other purposes.
 * 
 * This handler is called when the event fd is signaled, indicating that
 * a change has occurred and the display needs to be updated.
 * Depending on the current status of the network interfaces, it will update
 * the e-paper display (or terminal output) to reflect the current state.
 * 
 * @param data Janus data structure
 * @param event Epoll event for the event fd
 * @return int non-zero on error
 */
int	handle_event_fd_event(struct s_janus_data *data, struct epoll_event *event);

/**
 * @brief Main event loop for Janus
 * 
 * This function just runs the epoll loop.
 * 
 * @param data Janus data structure
 * @return int Non-zero on error
 */
int	janus_run(struct s_janus_data *data);

/**
 * @brief Part of the startup routine to scan existing interfaces
 * 
 * Scanning for existing interfaces at startup allows Janus to 
 * recognize and display the status of network interfaces that
 * were already active before the program started. This ensures that
 * the initial state is accurately reflected on the e-paper display.
 * 
 * @param data Janus data structure
 * @return int non-zero on error
 */
int	scan_existing_interfaces(struct s_janus_data *data);

#endif /* JANUS_H */
