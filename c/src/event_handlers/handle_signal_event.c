/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_signal_event.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:13:11 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/09 12:33:00 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int handle_signal_event(struct s_janus_data *data, struct epoll_event *event)
{
	struct signalfd_siginfo	fdsi;
	ssize_t					s;

	(void)event;
	s = read(data->signal_fd, &fdsi, sizeof(fdsi));
	if (s != sizeof(fdsi))
		return (perror("read"), 1);
	if (fdsi.ssi_signo == SIGINT)
	{
		dprintf(STDERR_FILENO, "Received SIGINT, exiting...\n");
		data->exit_loop = 1;
	}
	if (fdsi.ssi_signo == SIGUSR1)
	{
		dprintf(STDERR_FILENO, "Received SIGUSR1\n");
		eventfd_write(data->event_fd, 1);
	}
	if (fdsi.ssi_signo == SIGUSR2)
		dprintf(STDERR_FILENO, "Received SIGUSR2\n");
	return (0);
}
