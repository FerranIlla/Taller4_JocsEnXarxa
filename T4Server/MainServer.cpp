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
//void sendNodeInfoToAllPeers(std::list<sf::TcpSocket*>* sockets, sf::SocketSelector* selector, std::string ip, std::string port) {
//	std::string msj = ip + "-" + port;
//	for (std::list<sf::TcpSocket*>::iterator it = sockets->begin(); it != sockets->end(); it++) {
//		sf::TcpSocket* socket = *it;
//		sf::Socket::Status status = socket->send(msj.c_str(), msj.length());
//		if (status != sf::Socket::Done) {
//			std::cout << "Error al enviar\n";
//		}
//	}
//}

struct NodeInfo {
	std::string ip, port;
	NodeInfo(std::string _ip, std::string _port) :ip(_ip), port(_port) {}
};

void sendNodesInfo(sf::TcpSocket* socket, std::list<NodeInfo> nodesInfo) {
	int count = 0;
	for (std::list<NodeInfo>::iterator it = nodesInfo.begin(); it != nodesInfo.end(); it++) {
		count++;
		NodeInfo ni = *it;
		sf::Socket::Status status = socket->send(ni.ip.c_str(), ni.ip.length());
		if (status != sf::Socket::Done) std::cout << "Error al enviar la ip del socket " << count << std::endl;
		status = socket->send(ni.port.c_str(), ni.port.length());
		if (status != sf::Socket::Done) std::cout << "Error al enviar el puerto del socket " << count << std::endl;
	}
	std::cout << "Enviada la info de " << count << " peers!\n";
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

	//creamos una lista para guardar el ip:port de cada peer
	std::list<NodeInfo> nodesInfo;

	//escuchamos si el cliente se quiere conectar
	sf::Socket::Status status = listener.listen(5000);
	if (status != sf::Socket::Done) {
		std::cout << "Error a listen(5000)\n";
	}
	else {
		std::cout << "Puerto 5000 escuchando\n";
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
				sf::TcpSocket* socket = new sf::TcpSocket;
				//si el accept se hace bien
				sf::Socket::Status status = listener.accept(*socket);
				if (status == sf::Socket::Done) {
					std::cout << "Cliente aceptado\n";
					//avisamos cuantos peers se han conectado
					count++;
					sendString(socket, std::to_string(count));
					//enviamos la info de los otros peers(si la hay) a la ultima conexion
					sendNodesInfo(socket, nodesInfo);
					std::string ip = socket->getRemoteAddress().toString();
					std::string port = std::to_string(socket->getRemotePort());
					nodesInfo.push_back(NodeInfo(ip, port));
					std::cout << "El cliente tiene ip: " << ip << " y port: " << port << std::endl;
					
					//añadimos el cliente al selector --------------------------hace falta??-----------------------------------
					selector.add(*socket);
					
				}
				else {
					std::cout << "problema al conectar el cliente\n";
				}
				/*else if (status == sf::Socket::Disconnected) {
					disconnectSock(socket, &selector, &sockets);
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
	
	/*for (std::list<sf::TcpSocket*>::iterator it = sockets.begin(); it != sockets.end(); it++) {
		sf::TcpSocket* client = *it;
		client->disconnect();
		selector.remove(*client);
	}*/
	listener.close();
}