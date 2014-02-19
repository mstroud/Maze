#ifndef SERVER_HPP
#define SERVER_HPP

// Headers
#include <SFML/System.hpp>
#include <SFML/Network/UDPSocket.hpp>
#include <SFML/Network/IpAddress.hpp>

class Server
{
public :

	Server();
	Server::Server(sf::Uint16 port);
	
	void Listen();

	// Member data
	sf::UdpSocket m_sock ;   
};

#endif // SERVER_HPP