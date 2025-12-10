/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_stdin.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 21:14:40 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/10 11:31:16 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "janus.h"



int	unlock_display(struct s_janus_data *janus, void *data)
{
	(void)data;
	// This will unlock the display after a message hsa been displayed
	if (janus->message_scheduled > 0)
		janus->message_scheduled--;
	eventfd_write(janus->event_fd, 1);
	return (0);
}

int	handle_stdin_event(struct s_janus_data *janus, void *data)
{
 // clear screen, write the data to the screen
 // unset the value of message scheduled
 // 		unset will mean the message lasts until the next refresh
 //			we will 
# ifndef JANUS_TERMINAL_MODE
	Paint_Clear(WHITE);
	Paint_DrawString_EN(0, 0, data, &Font16, WHITE, BLACK);
	EPD_2in13_V4_Display(janus->image_buffer);
# else
	printf("Message from stdin: %s\n", (char *)data);
# endif
	schedule_event(janus, 10, unlock_display, NULL, NULL);
	return (0);
}

int	resize_scratch_buffer(struct s_janus_data *data, size_t new_size)
{
	char	*new_buffer;

	if (!data)
		return (perror("No data to resize scratch buffer"), 1);
	new_buffer = realloc(data->scratch_buffer, new_size);
	if (new_buffer == NULL)
		return (perror("realloc"), 1);
	data->scratch_buffer = new_buffer;
	data->scratch_buffer_size = new_size;
	return (0);
}

// TODO: this is not pretty, could use a refactor
int	handle_stdin(struct s_janus_data *data, struct epoll_event *event)
{
	char	*str;
	int		flag;
	size_t	offset;
	char	*nlpos;

	if (!data || !event)
		return (-1);
	if (data->scratch_buffer == NULL)
		return (fprintf(stderr, "Scratch buffer is NULL\n"), -1);
	if ((nlpos = strchr(data->scratch_buffer, '\n')) != NULL)
	{
		*nlpos = '\0';
		offset = nlpos - data->scratch_buffer + 1;
		str = strdup(data->scratch_buffer);
		if (!str)
			return (perror("strdup"), -1);
		memmove(data->scratch_buffer, data->scratch_buffer + offset, data->scratch_buffer_used - offset + 1);
		data->scratch_buffer_used -= offset;
		schedule_event(data, 1, handle_stdin_event, str, free);
		data->message_scheduled += 1;
		return (0);
	}
	while (1)
	{
		if (data->scratch_buffer_used >= data->scratch_buffer_size - 5)
		{
			if (resize_scratch_buffer(data, data->scratch_buffer_size * 2))
				return (-1);
		}
		flag = read(
			STDIN_FILENO,
			data->scratch_buffer + data->scratch_buffer_used,
			data->scratch_buffer_size - data->scratch_buffer_used - 1
		);
		if (flag > 0)
		{
			data->scratch_buffer_used += flag;
			continue;
		}
		if (flag == 0)
			break;
		// flag < 0
		if (errno == EINTR)
			continue; // interrupted, retry
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			break; // Non-blocking and no more data available now
		perror("read");
		return (-1);
	}
	data->scratch_buffer[data->scratch_buffer_used] = '\0';
	if ((nlpos = strchr(data->scratch_buffer, '\n')) != NULL)
	{
		*nlpos = '\0';
		offset = nlpos - data->scratch_buffer + 1;
		str = strdup(data->scratch_buffer);
		if (!str)
			return (perror("strdup"), -1);
		memmove(data->scratch_buffer, data->scratch_buffer + offset, data->scratch_buffer_used - offset + 1);
		data->scratch_buffer_used -= offset;
	}
	else if (flag == 0)
	{
		// EOF on FIFO: keep FD registered so future writers can send data
		// Only schedule if we actually have content
		if (data->scratch_buffer_used > 0)
		{
			str = strdup(data->scratch_buffer);
			if (!str)
				return (perror("strdup"), -1);
		}
		else
			return (0);
		data->scratch_buffer[0] = '\0';
		data->scratch_buffer_used = 0;
	}
	else
		return (0);
	schedule_event(data, 1, handle_stdin_event, str, free);
	data->message_scheduled += 1;
	return (0);
}
