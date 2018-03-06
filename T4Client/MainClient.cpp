#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <vector>
#include <list>

#define LOCALHOST "127.0.0.1"

#define MAX_MENSAJES_SIZE 100
std::mutex mu;
bool connected = false;

void sendAll(std::list<sf::TcpSocket*>* clients, sf::SocketSelector* selector, std::string msj) {
	for (std::list<sf::TcpSocket*>::iterator it = clients->begin(); it != clients->end(); it++) {
		sf::TcpSocket* client = *it;
		sf::Socket::Status status = client->send(msj.c_str(), msj.length());
		if (status == sf::Socket::Disconnected) {
			//disconnectSock(client, selector, clients);
		}
	}
}

void disconnectSock(sf::TcpSocket* socket, sf::SocketSelector* selector, std::list<sf::TcpSocket*>* clients) {
	selector->remove(*socket);
	socket->disconnect();
	sendAll(clients, selector, "Un cliente se ha desconectado\n");
	std::cout << "Cliente desconectado\n";
}

void sendString(sf::TcpSocket* socket, std::string msj) {
	sf::Socket::Status status = socket->send(msj.c_str(), msj.length());
	if (status != sf::Socket::Done) {
		std::cout << "Error al enviar\n";
	}
}

void receive(sf::TcpSocket* socket, std::vector<std::string>* aMensajes) {
	bool open = true;
	char buffer[MAX_MENSAJES_SIZE];
	std::size_t bytesReceived;
	while (open) {
		sf::Socket::Status status = socket->receive(&buffer, MAX_MENSAJES_SIZE, bytesReceived);
		/*if (status == sf::Socket::Status::Disconnected) {
			open = false;
			mu.lock();
			aMensajes->push_back("Desconectado del servidor");
			mu.unlock();
		}*/
		if (status == sf::Socket::Status::Done) {
			buffer[bytesReceived] = '\0';
			mu.lock();
			aMensajes->push_back(std::string(buffer));
			mu.unlock();
		}

		if (aMensajes->size() > 25) {
			mu.lock();
			aMensajes->erase(aMensajes->begin(), aMensajes->begin() + 1);
			mu.unlock();
		}
	}
}

void receiveBySelector(sf::SocketSelector* selector, sf::TcpSocket* socket, std::vector<std::string>* aMensajes) {
	bool open = true;
	char buffer[100];
	std::size_t bytesReceived;
	while (open) {
		//esperamos hasta que haya datos entrantes en alguno de los sockets dentro del selector
		if (selector->wait()) {
			//como sabemos que solo hay una conexion no necesitamos mirar el listener ni tener una lista de sockets
			if (selector->isReady(*socket)) {
				sf::Socket::Status status = socket->receive(&buffer, 100, bytesReceived);
				if (status == sf::Socket::Status::Disconnected) {
					open = false;
				}
				else if (status == sf::Socket::Status::Done) {
					buffer[bytesReceived] = '\0';
					mu.lock();
					aMensajes->push_back(std::string(buffer));
					mu.unlock();
				}
			}
		}
	}
}

struct NodeInfo {
	std::string ip, port;
};

int main()
{
	sf::SocketSelector selector;
	std::list<sf::TcpSocket*> sockets;
	NodeInfo nodesInfo[4];
	sf::TcpListener listener;

	//std::thread t1;
	std::vector<std::string> aMensajes;

	//establecimiento de conexion
	sf::TcpSocket socket;
	sf::Socket::Status status = socket.connect(LOCALHOST, 5000, sf::seconds(5.f));

	if (status != sf::Socket::Status::Done) {
		std::cout << "Problema al establecer conexión.\n";
	}
	else {
		std::cout << "Conectado con el bootstrap server\n";
		
		//inicializamos el thread
		//t1 = std::thread(&receive, &socket, &aMensajes);

		//recieve con el numero de peers
		char buffer[MAX_MENSAJES_SIZE];
		std::size_t bytesReceived;
		sf::Socket::Status status = socket.receive(&buffer, MAX_MENSAJES_SIZE, bytesReceived);
		if (status == sf::Socket::Status::Done) {
			buffer[bytesReceived] = '\0';
			mu.lock();
			aMensajes.push_back("Hay " + std::string(buffer) + " peers conectados.");
			if (aMensajes.size() > 25) {
				aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
			}
			mu.unlock();
			std::cout << "Hay " + std::string(buffer) + " peers conectados.\n";
			//crear y conectar nuevos sockets a cada peer
			for (int i = 1; i < atoi(buffer); ++i) {
				std::cout << "aqui nos conectariamos con otro peer\n";
				//hacer recieve de ip+puerto de un socket desde servidor
				char buffer[MAX_MENSAJES_SIZE];
				std::size_t bytesReceived;
				sf::Socket::Status status = socket.receive(&buffer, MAX_MENSAJES_SIZE, bytesReceived);
				if (status != sf::Socket::Done) std::cout << "Error al recibir ip-port desde el servidor\n";
				else {
					std::string data = std::string(buffer);
					std::cout << data << std::endl;
					//crear socket y añadirlo a la lista
					//conectar el socket al puerto recibido
				}
				
			}

			//abrir el listener para que los futuros peers se puedan conectar
			if (atoi(buffer)<4) {
				
				sf::Socket::Status status = listener.listen(socket.getLocalPort());
				if (status == sf::Socket::Done) {
					std::cout << "Puerto local escuchando: " << socket.getLocalPort() << std::endl;
				}
				else {
					std::cout << "Error a listen\n";
				}
				//cuando alguien se quiera conectar creamos un socket y accept
				
			}
			
			
		}
		else {
			std::cout << "Problema al recibir el numero de peers conectados.\n";
		}
		

	}
	

	//Codigo de display y enviar/recivir datos
	/*
	sf::Vector2i screenDimensions(800, 600);

	sf::RenderWindow window;
	window.create(sf::VideoMode(screenDimensions.x, screenDimensions.y), "Chat");

	sf::Font font;
	if (!font.loadFromFile("calibril.ttf"))
	{
		std::cout << "Can't load the font file" << std::endl;
	}

	sf::String mensaje = ">";

	sf::Text chattingText(mensaje, font, 14);
	chattingText.setFillColor(sf::Color(0, 160, 0));
	chattingText.setStyle(sf::Text::Bold);

	sf::Text text(mensaje, font, 14);
	text.setFillColor(sf::Color(0, 160, 0));
	text.setStyle(sf::Text::Bold);
	text.setPosition(0, 560);

	sf::RectangleShape separator(sf::Vector2f(800, 5));
	separator.setFillColor(sf::Color(200, 200, 200, 255));
	separator.setPosition(0, 550);

	while (window.isOpen())
	{
		sf::Event evento;
		while (window.pollEvent(evento))
		{
			switch (evento.type)
			{
			case sf::Event::Closed:
				sendString(&socket, ">/exit");
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (evento.key.code == sf::Keyboard::Escape)
					window.close();
				else if (evento.key.code == sf::Keyboard::Return)
				{
					if (mensaje == ">/exit") {
						sendString(&socket, mensaje);
						//window.close();
						std::cout << "sale\n";
						//std::string msj = "Cliente desconectado\n";
						//sendString(&socket, msj);
					}
					else {
						//SEND
						sendString(&socket, mensaje);

						if (aMensajes.size() > 25)
						{
							aMensajes.erase(aMensajes.begin(), aMensajes.begin() + 1);
						}

						mensaje = ">";
					}
				}
				break;
			case sf::Event::TextEntered:
				if (evento.text.unicode >= 32 && evento.text.unicode <= 126)
					mensaje += (char)evento.text.unicode;
				else if (evento.text.unicode == 8 && mensaje.getSize() > 0)
					mensaje.erase(mensaje.getSize() - 1, mensaje.getSize());
				break;
			}
		}

		window.draw(separator);
		for (size_t i = 0; i < aMensajes.size(); i++)
		{
			std::string chatting = aMensajes[i];
			chattingText.setPosition(sf::Vector2f(0, 20 * i));
			chattingText.setString(chatting);
			window.draw(chattingText);
		}
		std::string mensaje_ = mensaje + "_";
		text.setString(mensaje_);
		window.draw(text);

		window.display();
		window.clear();
	}
	
	*/
	socket.disconnect();
	std::cout << "socket disconnected\n";
	//t1.join();
	system("pause");
}