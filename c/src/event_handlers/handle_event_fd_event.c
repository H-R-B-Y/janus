/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_event_fd_event.c                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 16:03:59 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 18:20:15 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int	handle_event_fd_event(struct s_janus_data *data, struct epoll_event *event)
{
	uint64_t	event_count;
	ssize_t		s;
#ifndef JANUS_TERMINAL_MODE
	uint16_t	offset;
	sFONT		*font_select = &Font12;
#endif

	s = read(event->data.fd, &event_count, sizeof(event_count));
	if (s != sizeof(event_count))
		return (perror("read event_fd"), 1);
#ifndef JANUS_TERMINAL_MODE
	offset = 0;
	if (data->image_buffer == NULL)
		return (fprintf(stderr, "Image buffer is NULL\n"), 1);
	Paint_Clear(WHITE);
#endif
	if (data->interface_status == 0)
	{
#ifndef JANUS_TERMINAL_MODE
		;
#else
		printf("Janus: No network interfaces are up\n");
#endif
	}
	else
	{
#ifndef JANUS_TERMINAL_MODE
		
		if (is_interface_up(data->interface_status, JAN_ETH0))
		{
			char eth0_str[64];
			snprintf(eth0_str, sizeof(eth0_str), "ETH0: %s", data->eth0_interface);
			Paint_DrawString_EN(0, 10 + offset, eth0_str, font_select, BLACK, WHITE);
			offset += font_select->Height;
		}
		if (is_interface_up(data->interface_status, JAN_WLAN0))
		{
			char wlan0_str[64];
			snprintf(wlan0_str, sizeof(wlan0_str), "WLAN0: %s", data->wlan0_interface);
			Paint_DrawString_EN(0, 10 + offset, wlan0_str, font_select, BLACK, WHITE);
			offset += font_select->Height;
		}
#else
		printf("Janus: Network interfaces status:\n");
		if (is_interface_up(data->interface_status, JAN_ETH0))
		{
			printf("  ETH0: %s\n", data->eth0_interface);
		}
		if (is_interface_up(data->interface_status, JAN_WLAN0))
		{
			printf("  WLAN0: %s\n", data->wlan0_interface);
		}
#endif
	}
# ifndef JANUS_TERMINAL_MODE
	EPD_2in13_V4_Display(data->image_buffer);
# endif
	return (0);
}
