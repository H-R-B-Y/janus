/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_net_event.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 15:28:40 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 18:19:38 by hbreeze          ###   ########.fr       */
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

				printf("Janus: WLAN0 interface up with IP: %s\n", ip_str);

				eventfd_write(data->event_fd, 1);
			}
			else if (strcmp(ifname, "eth0") == 0)
			{
				strncpy(data->eth0_interface, ip_str, INET_ADDRSTRLEN - 1);
				data->eth0_interface[INET_ADDRSTRLEN - 1] = '\0';
				mark_interface_up(&data->interface_status, JAN_ETH0);

				printf("Janus: ETH0 interface up with IP: %s\n", ip_str);

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

		printf("Janus: WLAN0 interface down\n");

		eventfd_write(data->event_fd, 1);
	}
	else if (strcmp(ifname, "eth0") == 0)
	{
		memset(data->eth0_interface, 0, INET_ADDRSTRLEN);
		mark_interface_down(&data->interface_status, JAN_ETH0);

		printf("Janus: ETH0 interface down\n");

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
				printf("Received RTM_NEWADDR message\n");
				new_address_handler(data, nlh);
				break;
			case RTM_DELADDR:
				printf("Received RTM_DELADDR message\n");
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
	static char		buffer[8192];  // Static buffer to avoid malloc/free
	ssize_t			len;
	struct nlmsghdr	*nlh;
	
	// Read available data directly into static buffer
	len = recv(event->data.fd, buffer, sizeof(buffer), 0);
	if (len == -1)
		return (perror("recv"), 1);
	else if (len == 0)
	{
		fprintf(stderr, "Netlink socket closed unexpectedly\n");
		return (1);
	}
	else if (len < (ssize_t)sizeof(struct nlmsghdr))
	{
		fprintf(stderr, "Received incomplete netlink message (%zd bytes)\n", len);
		return (1);
	}
	
	// Basic validation of the first message header
	nlh = (struct nlmsghdr *)buffer;
	if (nlh->nlmsg_len > (size_t)len || nlh->nlmsg_len < sizeof(struct nlmsghdr))
	{
		fprintf(stderr, "Invalid netlink message: len=%u, received=%zd\n", 
				nlh->nlmsg_len, len);
		return (1);
	}
	
	// Process the messages
	if (process_netlink_messages(data, buffer, len))
		return (1);
	return (0);
}
