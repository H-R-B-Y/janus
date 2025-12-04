/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/04 13:55:15 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/04 14:26:09 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "janus.h"

int	init_process(void)
{
	if (DEV_Module_Init() != 0)
	{
		dprintf(STDERR_FILENO, "Failed to initialize the hardware module\n");
		return (1);
	}
	EPD_2in13_V4_Init();
	EPD_2in13_V4_Clear();
	return (0);
}

int main()
{
	UBYTE	*image;

	if (init_process() != 0)
	{
		dprintf(STDERR_FILENO, "Failed to initialize e-paper display\n");
		return (1);
	}
	image = calloc(EPD_2in13_V4_WIDTH / 8 * EPD_2in13_V4_HEIGHT, sizeof(UBYTE));
	if (image == NULL)
	{
		dprintf(STDERR_FILENO, "Failed to allocate memory for image buffer\n");
		return (1);
	}
	Paint_NewImage(image, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, ROTATE_180, WHITE);
	Paint_Clear(WHITE);
	Paint_DrawChar(10, 10, 'A', &Font24, BLACK, WHITE);
	EPD_2in13_V4_Display(image);
	free(image);
	EPD_2in13_V4_Sleep();
	DEV_Module_Exit();
	return (0);
}
