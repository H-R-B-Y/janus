/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_signal_event.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:13:11 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 17:13:45 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int handle_signal_event(struct s_janus_data *data, struct epoll_event *event)
{
	struct signalfd_siginfo	fdsi;
	ssize_t					s;

	(void)event; // Silence unused parameter warning

	s = read(data->signal_fd, &fdsi, sizeof(fdsi));
	if (s != sizeof(fdsi))
	{
		perror("read");
		return (1);
	}
	if (fdsi.ssi_signo == SIGINT)
	{
		printf("Received SIGINT, exiting...\n");
		data->exit_loop = 1;
	}
	if (fdsi.ssi_signo == SIGUSR1)
	{
		printf("Received SIGUSR1\n");
		eventfd_write(data->event_fd, 1); // Signal redraw
		// Handle SIGUSR1
	}
	if (fdsi.ssi_signo == SIGUSR2)
	{
		printf("Received SIGUSR2\n");
		// Handle SIGUSR2
	}
	return (0);
}
