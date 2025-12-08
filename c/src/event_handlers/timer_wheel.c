/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   timer_wheel.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/07 11:05:40 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/08 13:32:22 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int	handle_timer_wheel(
	struct s_janus_data *data,
	struct epoll_event *event
)
{
	uint64_t	tick;
	int			bytes;
	register uint16_t	calls;

	if (!data || !event)
		return (1);
	bytes = read(event->data.fd, &tick, sizeof(uint64_t));
	if (bytes < 0)
		return (perror("Failed to read timerfd"), 1);
# ifdef JANUS_TERMINAL_MODE
	printf("recieved %zu timer events\n", tick);
#endif
	calls = 0;
	while (calls < tick)
	{
		data->wheel_idx = (data->wheel_idx + 1) % MAX_TIMER_EVENT;
		if (data->timerwheel[data->wheel_idx].valid)
		{
			if (data->timerwheel[data->wheel_idx].fn(data,
				data->timerwheel[data->wheel_idx].data))
				return (1);
			data->timerwheel[data->wheel_idx].valid = 0;
			data->events_queued--;
		}
		calls++;
	}
	if (data->events_queued == 0)
	{
		struct itimerspec spec = {0};
		spec.it_interval.tv_sec = 0;
		spec.it_value.tv_sec = 0;
		if (timerfd_settime(data->timerfd, 0, &spec, NULL) < 0)
			return (perror("timerfd set time"), 1);
	}
	return (0);
}
