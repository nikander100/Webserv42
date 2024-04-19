/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: code <code@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/12 15:18:10 by code              #+#    #+#             */
/*   Updated: 2024/04/12 15:49:13 by code             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>

class Config
{
	private:
		int			_listen;
		std::string _serverName;
		char** 		_errorPage;
		int			_clientMaxBodySize;
		
		
};

#endif