/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_event_fd_event.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:03:59 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/08 13:28:12 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int	refresh_callback(
	struct s_janus_data *data,
	void *event_data
)
{
#ifndef JANUS_TERMINAL_MODE
	uint16_t	offset;
	sFONT		*font_select = &Font24;
#endif
	(void)event_data;
	
	if (data->next_interface_status != data->interface_status)
		data->interface_status = data->next_interface_status;
# ifndef JANUS_TERMINAL_MODE
	if (data->image_buffer == NULL)
		return (fprintf(stderr, "Image buffer is NULL\n"), 1);
	Paint_Clear(WHITE);
	offset = 0;
	if (data->interface_status == 0)
		Paint_DrawString_EN(0, 10, "No network interfaces are up", font_select, BLACK, WHITE);
	else
	{
		if (is_interface_up(data->interface_status, INTERFACE_1_NAME))
		{
			Paint_DrawString_EN(0, 2 + offset, INTERFACE_1, font_select, BLACK, WHITE);
			offset += font_select->Height + 1;
			char eth0_str[64];
			snprintf(eth0_str, sizeof(eth0_str), "%s", data->interface1_addr);
			Paint_DrawString_EN(0, 2 + offset, eth0_str, font_select, BLACK, WHITE);
			offset += font_select->Height + 1;
		}
		if (is_interface_up(data->interface_status, INTERFACE_2_NAME))
		{
			Paint_DrawString_EN(0, 2 + offset, INTERFACE_2, font_select, BLACK, WHITE);
			offset += font_select->Height + 1;
			char wlan0_str[64];
			snprintf(wlan0_str, sizeof(wlan0_str), "%s", data->interface2_addr);
			Paint_DrawString_EN(0, 2 + offset, wlan0_str, font_select, BLACK, WHITE);
			offset += font_select->Height + 1;
		}
	}
	EPD_2in13_V4_Display(data->image_buffer);
# else
	if (data->interface_status == 0)
		printf("Janus: No network interfaces are up\n");
	else
	{
		printf("Janus: Network interfaces status:\n");
		if (is_interface_up(data->interface_status, INTERFACE_1_NAME))
			printf("  %s: %s\n", INTERFACE_1, data->interface1_addr);
		if (is_interface_up(data->interface_status, INTERFACE_2_NAME))
			printf("  %s: %s\n", INTERFACE_2, data->interface2_addr);
	}
# endif
	data->refresh_scheduled = 0;
	return (0);
}

int	handle_event_fd_event(struct s_janus_data *data, struct epoll_event *event)
{
	uint64_t	event_count;
	ssize_t		s;

	(void)data;
	s = read(event->data.fd, &event_count, sizeof(event_count));
	if (s != sizeof(event_count))
		return (perror("read event_fd"), 1);
	if (!data->refresh_scheduled)
	{
		schedule_event(data, 5, refresh_callback, NULL, NULL);
		data->refresh_scheduled = 1;
	}
	return (0);
}
