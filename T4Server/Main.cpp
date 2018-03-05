#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <list>

#define PEERS_NUMBER 4

//function to send the new Nodeinfo to all peers connected
void sendNodeInfoToAllPeers(std::list<sf::TcpSocket*>* peerSockets, sf::SocketSelector* selector, std::string ip, std::string port) {
	std::string msj = ip + "-" + port;
	for (std::list<sf::TcpSocket*>::iterator it = peerSockets->begin(); it != peerSockets->end(); it++) {
		sf::TcpSocket* socket = *it;
		sf::Socket::Status status = socket->send(msj.c_str(), msj.length());
		if (status != sf::Socket::Done) {
			std::cout << "Error al enviar\n";
		}
	}
}

void sendString(sf::TcpSocket* socket, std::string msj) {
	sf::Socket::Status status = socket->send(msj.c_str(), msj.length());
	if (status != sf::Socket::Done) {
		std::cout << "Error al enviar\n";
	}
}

//void disconnectSocket(sf::TcpSocket* socket, sf::SocketSelector* selector, std::list<sf::TcpSocket*>* clients) {
//	selector->remove(*socket);
//	socket->disconnect();
//	std::cout << "Cliente desconectado\n";
//}


int main() {
	
	sf::TcpListener listener;
	sf::SocketSelector selector;
	
	//char buffer[MAX_MSJ_SIZE];
	//std::size_t bytesReceived;

	//creamos una lista para guardar los sockets
	std::list<sf::TcpSocket*> peers;

	//escuchamos si el cliente se quiere conectar
	sf::Socket::Status status = listener.listen(5000);
	if (status != sf::Socket::Done) {

	}

	selector.add(listener);

	bool isActive = true;
	int count = 0;
	while (isActive && count < 4)
	{
		//recibir datos
		if (selector.wait()) {
			//miramos el listener
			if (selector.isReady(listener)) {
				//creamos un socket para el nuevo peer
				sf::TcpSocket* peer = new sf::TcpSocket;
				//si el accept se hace bien
				sf::Socket::Status status = listener.accept(*peer);
				if (status == sf::Socket::Done) {
					std::string ip = peer->getRemoteAddress().toString();
					std::string port = std::to_string(peer->getRemotePort());
					//añadimos al nuevo cliente a la lista
					peers.push_back(peer);
					//añadimos el cliente al selector
					selector.add(*peer);
					//avisamos de que se ha conectado y enviamos la info del nuevo nodo
					count++;
					sendString(peer, std::to_string(count));
					sendNodeInfoToAllPeers(&peers, &selector, ip, port);
					std::cout << "Cliente aceptado\n";
				}
				else {
					std::cout << "problema al conectar el cliente\n";
				}
				/*else if (status == sf::Socket::Disconnected) {
					disconnectSock(peer, &selector, &peers);
				}*/
			}
			//else {
			//	//comprovamos los sockets de los clientes
			//	for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); it++) {
			//		sf::TcpSocket* client = *it;
			//		if (selector.isReady(*client)) {
			//			//recivimos los datos del cliente
			//			if (client->receive(buffer, MAX_MSJ_SIZE, bytesReceived) == sf::Socket::Done) {
			//				buffer[bytesReceived] = '\0';
			//				receive(buffer, client, &clients, &selector);
			//			}
			//		}
			//	}
			//}
		}
	}
	
	for (std::list<sf::TcpSocket*>::iterator it = peers.begin(); it != peers.end(); it++) {
		sf::TcpSocket* client = *it;
		client->disconnect();
		selector.remove(*client);
	}
	listener.close();
}