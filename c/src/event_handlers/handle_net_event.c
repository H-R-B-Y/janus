/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_net_event.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 15:28:40 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 18:06:47 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"
# include <arpa/inet.h>

/*
Here we have been pinged from the netlink socket.
We want to extract the message and data from it to see if
there is anything we care about, then back populate the main
data struct as needed.
*/

int	new_address_handler(struct s_janus_data *data, struct nlmsghdr *nlh)
{
	struct ifaddrmsg	*ifa;
	struct rtattr		*rta;
	int					rtalen;
	char				ifname[IFNAMSIZ];
	char				ip_str[INET_ADDRSTRLEN];

	ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
	rta = (struct rtattr *)IFA_RTA(ifa);
	rtalen = IFA_PAYLOAD(nlh);
	
	if (if_indextoname(ifa->ifa_index, ifname) == NULL)
	{
		fprintf(stderr, "Could not get interface name for index %d\n", ifa->ifa_index);
		return (1);
	}
	while (RTA_OK(rta, rtalen))
	{
		if (rta->rta_type == IFA_LOCAL)
		{
			
			inet_ntop(AF_INET, RTA_DATA(rta), ip_str, sizeof(ip_str));
			if (strcmp(ifname, "wlan0") == 0)
			{
				strncpy(data->wlan0_interface, ip_str, INET_ADDRSTRLEN - 1);
				data->wlan0_interface[INET_ADDRSTRLEN - 1] = '\0';
				mark_interface_up(&data->interface_status, JAN_WLAN0);
#ifdef JANUS_TERMINAL_MODE
				printf("Janus: WLAN0 interface up with IP: %s\n", ip_str);
#endif
				eventfd_write(data->event_fd, 1);
			}
			else if (strcmp(ifname, "eth0") == 0)
			{
				strncpy(data->eth0_interface, ip_str, INET_ADDRSTRLEN - 1);
				data->eth0_interface[INET_ADDRSTRLEN - 1] = '\0';
				mark_interface_up(&data->interface_status, JAN_ETH0);
#ifdef JANUS_TERMINAL_MODE
				printf("Janus: ETH0 interface up with IP: %s\n", ip_str);
#endif
				eventfd_write(data->event_fd, 1);
			}
		}
		rta = RTA_NEXT(rta, rtalen);
	}
	return (0);
}

int	del_address_handler(struct s_janus_data *data, struct nlmsghdr *nlh)
{
	struct ifaddrmsg	*ifa;
	char				ifname[IFNAMSIZ];
	
	ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
	if (if_indextoname(ifa->ifa_index, ifname) == NULL)
	{
		fprintf(stderr, "Could not get interface name for index %d\n", ifa->ifa_index);
		return (1);
	}
	if (strcmp(ifname, "wlan0") == 0)
	{
		memset(data->wlan0_interface, 0, INET_ADDRSTRLEN);
		mark_interface_down(&data->interface_status, JAN_WLAN0);
#ifdef JANUS_TERMINAL_MODE
		printf("Janus: WLAN0 interface down\n");
#endif
		eventfd_write(data->event_fd, 1);
	}
	else if (strcmp(ifname, "eth0") == 0)
	{
		memset(data->eth0_interface, 0, INET_ADDRSTRLEN);
		mark_interface_down(&data->interface_status, JAN_ETH0);
#ifdef JANUS_TERMINAL_MODE
		printf("Janus: ETH0 interface down\n");
#endif
		eventfd_write(data->event_fd, 1);
	}
	return (0);
}

int	process_netlink_messages(struct s_janus_data *data, char *msg_buffer, ssize_t len)
{
	struct nlmsghdr	*nlh;

	for (nlh = (struct nlmsghdr *)msg_buffer; 
		 NLMSG_OK(nlh, len); 
		 nlh = NLMSG_NEXT(nlh, len))
	{
		// Ok so the events we care about are:
		// newaddr - New IP address assigned (RTM_NEWADDR)
		// deladdr - IP address removed (RTM_DELADDR)
		switch (nlh->nlmsg_type)
		{
			case RTM_NEWADDR:
				new_address_handler(data, nlh);
				break;
			case RTM_DELADDR:
				del_address_handler(data, nlh);
				break;
			default:
				// Ignore other message types
				break;
		}
	}
	return (0);
}

int	handle_net_event(struct s_janus_data *data, struct epoll_event *event)
{
	char			initial_buffer[sizeof(struct nlmsghdr)];
	char			*dynamic_buffer;
	ssize_t			len;
	struct nlmsghdr	*nlh;
	size_t			msg_len;

	len = recv(event->data.fd, initial_buffer, sizeof(initial_buffer), MSG_PEEK);
	if (len == -1)
		return (perror("recv peek"), 1);
	else if (len < (ssize_t)sizeof(struct nlmsghdr))
	{
		fprintf(stderr, "Received incomplete netlink message header\n");
		return (1);
	}
	nlh = (struct nlmsghdr *)initial_buffer;
	
	// Validate message length to prevent invalid malloc
	msg_len = nlh->nlmsg_len;
	if (msg_len < sizeof(struct nlmsghdr) || msg_len > 65536)
	{
		fprintf(stderr, "Invalid netlink message length: %zu\n", msg_len);
		// Consume the invalid message to prevent infinite loop
		char discard[4096];
		recv(event->data.fd, discard, sizeof(discard), 0);
		return (1);
	}
	
	dynamic_buffer = malloc(msg_len);
	if (dynamic_buffer == NULL)
		return (perror("malloc"), 1);
	len = recv(event->data.fd, dynamic_buffer, msg_len, 0);
	if (len == -1)
		return (perror("recv"),free(dynamic_buffer),1);
	else if (len < (ssize_t)msg_len)
	{
		fprintf(stderr, "Received incomplete netlink message (%zd of %zu bytes)\n", 
				len, msg_len);
		return (free(dynamic_buffer), 1);
	}
	if (process_netlink_messages(data, dynamic_buffer, len))
		return (free(dynamic_buffer), 1);
	return (free(dynamic_buffer), 0);
}
