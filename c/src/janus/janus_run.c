/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   janus_run.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 15:56:05 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/08 13:31:21 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int	janus_run(struct s_janus_data *data)
{
	struct epoll_event	events[10];
	int					n;

	while (!data->exit_loop)
	{
		n = epoll_wait(data->epoll_fd, events, 10, -1);
		if (n == -1)
			return (perror("epoll_wait"), 1);
		for (int i = 0; i < n; i++)
		{
			if (events[i].data.fd == data->netlink_socket)
			{
				if (handle_net_event(data, &events[i]) != 0)
					return (1);
			}
			else if (events[i].data.fd == data->signal_fd)
			{
				if (handle_signal_event(data, &events[i]) != 0)
					return (1);
			}
			else if (events[i].data.fd == data->event_fd)
			{
				if (handle_event_fd_event(data, &events[i]) != 0)
					return (1);
			}
			else if (events[i].data.fd == data->timerfd)
			{
				if (handle_timer_wheel(data, &events[i]) != 0)
					return (1);
			}
		}
	}
	return (0);
}
