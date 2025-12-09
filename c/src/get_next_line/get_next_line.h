/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hbreeze <hbreeze@student.42london.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 08:45:14 by hbreeze           #+#    #+#             */
/*   Updated: 2025/12/09 11:26:43 by hbreeze          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

/*
Allowed functions: read, malloc, free
*/
# include <unistd.h>
# include <stdlib.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 1000
# endif

struct s_buffer
{
	/// @brief Temp buffer for reads
	char		tmp[BUFFER_SIZE + 1];
	/// @brief Buffer for return value
	char		*buff;
	/// @brief End of file flag
	short int	eof;
	/// @brief Length of Return Buffer
	size_t		len;
	/// @brief Bytes read into the tmp buffer
	ssize_t		bytes;
};

char	*get_next_line(int fd);

#endif
