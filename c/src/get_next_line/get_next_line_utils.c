/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 13:48:29 by hbreeze           #+#    #+#             */
/*   Updated: 2025/10/02 14:01:26 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

void	*ft_memchr(const char *src, int c, size_t len)
{
	size_t	index;

	if (!src)
		return (0);
	index = 0;
	while (index < len)
	{
		if (((unsigned char *)src)[index] == (unsigned char)c)
			return (&(((unsigned char *)src)[index]));
		index++;
	}
	return (0);
}

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	unsigned char	*temp_dest;
	unsigned char	*temp_src;
	size_t			i;

	if (!src)
		return (0);
	if (!dest)
		return (0);
	temp_src = (unsigned char *)src;
	temp_dest = (unsigned char *)dest;
	i = 0;
	if (src > dest)
	{
		while (i < n)
		{
			temp_dest[i] = temp_src[i];
			i++;
		}
	}
	if (src < dest)
		while (n--)
			temp_dest[n] = temp_src[n];
	return (dest);
}

void	*ft_realloc(void *ptr, size_t old_size, size_t new_size)
{
	void	*new_ptr;

	if (new_size == 0)
	{
		free(ptr);
		return (NULL);
	}
	if (!ptr)
		return (malloc(1 * new_size));
	if (new_size <= old_size)
		return (ptr);
	new_ptr = malloc(1 * new_size);
	if (!new_ptr)
		return (NULL);
	ft_memmove(new_ptr, ptr, old_size);
	free(ptr);
	return (new_ptr);
}
