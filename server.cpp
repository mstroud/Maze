#include <iostream>
#include "Server.hpp"

Server::Server() {

}

Server::Server(sf::Uint16 port) {
  this->m_sock.bind(port);
}

void Server::Listen() {
  std::cout << "Listening on port" << this->m_sock.getLocalPort() << std::endl ; 
  // Receive a message from anyone
  char buffer[1024];
  std::size_t received = 0;
  sf::IpAddress sender;
  unsigned short port;
  (this->m_sock).receive(buffer, sizeof(buffer), received, sender, port);
  std::cout << sender.toString() << " said: " << buffer << std::endl;

}