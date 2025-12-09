/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handle_stdin.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 21:14:40 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/09 11:28:25 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "janus.h"



int	unlock_display(struct s_janus_data *janus, void *data)
{
	(void)data;
	// This will unlock the display after a message hsa been displayed
	janus->message_scheduled = 0;
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

int	handle_stdin(struct s_janus_data *data, struct epoll_event *event)
{
	char	*str;

	if (!data || !event)
		return (-1);
	// Is get next line going to work here?
	// it should because stdin is buffered by default on terminal device
	// but we should probably write a specific reading funciton for this
	// because we cannot garantee that the stdin filedes is going to be line buffered
	// and if it isnt this is going to block the program.
	str = get_next_line(STDIN_FILENO);
	// What we should do is tokenise the string
	// first two tokens should be font select and color?
	// then after that there is a string, but we can do that later.
	schedule_event(data, 1, handle_stdin_event, str, free);
	data->message_scheduled += 1;
	return (0);
}
