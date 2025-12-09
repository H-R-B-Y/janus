/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 08:46:30 by hbreeze           #+#    #+#             */
/*   Updated: 2025/10/10 17:21:52 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

void	*ft_memchr(const char *src, int c, size_t len);
void	*ft_memmove(void *dest, const void *src, size_t n);
void	*ft_realloc(void *ptr, size_t old_size, size_t new_size);

ssize_t	read_into_buffer(int fd, struct s_buffer *buf, ssize_t *bytes)
{
	*bytes = read(fd, buf->tmp, BUFFER_SIZE);
	if (*bytes < 0)
		return (-1);
	if (*bytes == 0)
		buf->eof = 1;
	buf->tmp[*bytes] = '\0';
	return (*bytes);
}

char	*read_until_complete(
	int fd,
	struct s_buffer *buf,
	ssize_t *bytes,
	char **res
)
{
	char	*newline;
	size_t	offset;

	newline = 0;
	newline = (char *)ft_memchr(buf->tmp, '\n', *bytes);
	while (!buf->eof && !newline)
	{
		buf->buff = ft_realloc(buf->buff, buf->len, buf->len + *bytes + 1);
		((char *)ft_memmove(buf->buff + buf->len, buf->tmp, *bytes))
			[*bytes] = '\0';
		buf->len += *bytes;
		read_into_buffer(fd, buf, bytes);
		newline = (char *)ft_memchr(buf->tmp, '\n', *bytes);
	}
	offset = (*bytes) * (!newline) + (newline - buf->tmp + 1) * (!!newline);
	buf->buff = ft_realloc(buf->buff, buf->len, buf->len + offset + 1);
	((char *)ft_memmove(buf->buff + buf->len, buf->tmp, offset))[offset] = '\0';
	buf->len += offset;
	((char *)ft_memmove(buf->tmp, buf->tmp + offset, *bytes - offset + 1))
		[*bytes - offset] = '\0';
	*res = buf->buff;
	*bytes = *bytes - offset;
	return (buf->buff = 0, buf->len = 0, *res);
}

char	*get_next_line(int fd)
{
	static struct s_buffer	buf = {0};
	char					*result;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	if (buf.bytes > 0)
		return (read_until_complete(fd, &buf, &buf.bytes, &result));
	if (buf.eof)
		return (buf.eof = 0, NULL);
	if (read_into_buffer(fd, &buf, &buf.bytes) <= 0)
	{
		if (buf.buff)
			free(buf.buff);
		return (buf.buff = NULL, buf.len = 0, buf.tmp[0] = '\0', NULL);
	}
	return (read_until_complete(fd, &buf, &buf.bytes, &result));
}
