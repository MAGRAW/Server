//  ServerCpp.cpp: определяет точку входа для приложения.
//

#include "Server.h"

using namespace std;

enum Packet {
	P_ChatMessage,
	P_Test
};

bool ProcessPacket(int index, Packet packettype) {
	switch (packettype) {
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1]; //динамическое выделение памяти
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		//отправим полученное сообщение всем клиентам,
		for (int i = 0; i < connCounter; i++) {
			/* кроме того, кто его отправил изначально*/
			if (i == index) {
				continue;
			}
			Packet msgtype = P_ChatMessage;
			send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);
		}

		//так-же напечатаем сообщение от клиента на сервере
		cout << msg << endl;

		delete[] msg;
		break;
	}
	default:
		cout << "Unrecognized packet: " << packettype << endl;
		break;
	}
	return true;
}

////Функция принимающая индекс соединения в сокет-массиве
//Далее она будет отвечать за принятие пакетов и вызов ProcessPacket
void ClientHandler(int index) {
	////char msg[256]; //переменная для хранения переданного клиентом сообщения
	//перетащили эту реализацию в ProcessPacket
	//int msg_size;
	Packet packettype;
	while (1) {
		////функция для принятия сообщения от клиента
		////recv(Connections[index], msg, sizeof(msg), NULL);

		//перетащили эту реализацию в ProcessPacket
		//recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		//char* msg = new char[msg_size + 1]; //динамическое выделение памяти
		//msg[msg_size] = '\0';
		//recv(Connections[index], msg, msg_size, NULL);
		////отправим полученное сообщение всем клиентам,
		//for (int i = 0; i < connCounter; i++) {
		//	/* кроме того, кто его отправил изначально*/
		//	if (i == index) {
		//		continue;
		//	}
		//	//send(Connections[i], msg, sizeof(msg), NULL);
		//	send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
		//	send(Connections[i], msg, msg_size, NULL);
		//}

		////так-же напечатаем сообщение от клиента на сервере
		//cout << msg << endl;

		//delete[] msg;

		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);

		if (!ProcessPacket(index, packettype)) {
			break;
		}
	}
	closesocket(Connections[index]);
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru");

	//WSAStartup подготовим переменные
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);

	//Проверяем загрузку библиотеки
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		cout << "Error: The library WSAStartup isn't available." << endl;
		exit(1);
	}

	//Заполняем информацию об адресе сокета
	SOCKADDR_IN addr; //SOCKADDR_IN предназначена для хранения адреса
	int sizeofaddr = sizeof(addr); //сервисная переменная необходимая для дальнейшей передачи
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //sin_addr предназначена для хранения ip-адреса. Задали localhost
	addr.sin_port = htons(1111); //sin_port порт идентификации программы поступающими данными. Выбирается любой не зарезервированный порт
	addr.sin_family = AF_INET; //sin_family семейство протоколов. Для интернет-протоколов указывается константа AF_INET

	//Чтобы ПК могли установить соединение, один из них должен запустить прослушивание на определённом порту
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL); //создали сокет

	//Привязываем адрес сокета
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));//привязали локальный адрес и порт к сокету

	//Запускаем прослушивание порта в ожидании соединения со стороны клиента:
	//   по sListen функция определить по какому порту нужно запустить прослушивание
	//   SOMAXCONN - максимально допустимое число запросов ожидающих обработки
	listen(sListen, SOMAXCONN);

	//Принимаем соединение. Объявляем новый сокет, чтобы удерживать соединение с клиентом:
	SOCKET newConnection;
	for (int i = 0; i < CONN_COUNT; i++) {
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr); /*после выполнения функции
					accept,  второй параметр addr,  будет держать сведения об ip-адресе клиента,
					который произвёл подключение. Эти данные можно использовать для контроля
					доступа к серверу по ip-адресу. accept - возвращает указатель на новый сокет,
					который используется для общения с клиентом. Если ф-я возвращает 0, значит
					клиент не смог подключиться к серверу.*/
		if (newConnection == 0) {
			cout << "Error: Client can't to get connection from server." << endl;
		}
		else {
			cout << "Client #" << connCounter << " is connected!\n";
			//char msg[256] = "It's the test message of network program for client #";
			//send(newConnection, msg, sizeof(msg), NULL);

			string msg = "It's the test message of network program for client #" + to_string(connCounter);
			int msg_size = msg.size();
			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), msg_size, NULL);

			Connections[i] = newConnection;
			connCounter++;

			//создадим поток для выполнения функции рассылки сообщений ClientHandler
			//в 6-й параметр пишется tread_id созданного потока 
			//в случае успеха процедура возвратит handle созданного потока. или 0.
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(i), NULL, NULL);

			Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
		}
	}

	cout << "Hello CMake." << endl;
	system("pause");
	return 0;
}
