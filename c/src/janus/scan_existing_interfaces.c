/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scan_existing_interfaces.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:45:00 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 16:46:25 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "janus.h"
#include <ifaddrs.h>
#include <arpa/inet.h>

/**
 * @brief Scan existing network interfaces and populate IP addresses
 * 
 * This function checks if wlan0 or eth0 already have assigned IP addresses
 * when the program starts, which would be missed by the netlink event monitoring
 * since those events only catch new assignments.
 * 
 * @param data Pointer to the main janus data structure
 * @return 0 on success, 1 on failure
 */
int scan_existing_interfaces(struct s_janus_data *data)
{
	struct ifaddrs		*ifaddrs_list;
	struct ifaddrs		*ifa;
	struct sockaddr_in	*sin;
	char				ip_str[INET_ADDRSTRLEN];
	int					found_interfaces = 0;

	if (getifaddrs(&ifaddrs_list) == -1)
		return (perror("getifaddrs"), 1);
	for (ifa = ifaddrs_list; ifa != NULL; ifa = ifa->ifa_next)
	{
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
			continue;
		if (strcmp(ifa->ifa_name, "lo") == 0)
			continue;
		sin = (struct sockaddr_in *)ifa->ifa_addr;
		if (inet_ntop(AF_INET, &sin->sin_addr, ip_str, sizeof(ip_str)) == NULL)
		{
			perror("inet_ntop");
			continue;
		}
		if (strcmp(ifa->ifa_name, "wlan0") == 0)
		{
			strncpy(data->wlan0_interface, ip_str, INET_ADDRSTRLEN - 1);
			data->wlan0_interface[INET_ADDRSTRLEN - 1] = '\0';
			mark_interface_up(&data->interface_status, JAN_WLAN0);
			printf("Found existing wlan0 interface with IP: %s\n", ip_str);
			found_interfaces++;
		}
		else if (strcmp(ifa->ifa_name, "eth0") == 0)
		{
			strncpy(data->eth0_interface, ip_str, INET_ADDRSTRLEN - 1);
			data->eth0_interface[INET_ADDRSTRLEN - 1] = '\0';
			mark_interface_up(&data->interface_status, JAN_ETH0);
			printf("Found existing eth0 interface with IP: %s\n", ip_str);
			found_interfaces++;
		}
	}
	freeifaddrs(ifaddrs_list);
	if (found_interfaces > 0)
		eventfd_write(data->event_fd, 1);
	return (0);
}
