/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scan_existing_interfaces.c                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:45:00 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/06 11:09:58 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "janus.h"
#include <ifaddrs.h>
#include <arpa/inet.h>

/*
Sort of monolithic, would be good to split this into
smaller chunks later.
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
		if (strcmp(ifa->ifa_name, INTERFACE_2) == 0)
		{
			strncpy(data->interface2_addr, ip_str, INET_ADDRSTRLEN - 1);
			data->interface2_addr[INET_ADDRSTRLEN - 1] = '\0';
			mark_interface_up(&data->interface_status, INTERFACE_2_NAME);
			printf("Found existing %s interface with IP: %s\n", INTERFACE_2, ip_str);
			found_interfaces++;
		}
		else if (strcmp(ifa->ifa_name, INTERFACE_1) == 0)
		{
			strncpy(data->interface1_addr, ip_str, INET_ADDRSTRLEN - 1);
			data->interface1_addr[INET_ADDRSTRLEN - 1] = '\0';
			mark_interface_up(&data->interface_status, INTERFACE_1_NAME);
			printf("Found existing %s interface with IP: %s\n", INTERFACE_1, ip_str);
			found_interfaces++;
		}
	}
	freeifaddrs(ifaddrs_list);
	if (found_interfaces > 0)
		eventfd_write(data->event_fd, 1);
	return (0);
}
