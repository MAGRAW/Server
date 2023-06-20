// Server.h : включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта.

#pragma once

#include <iostream>
#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <string>

const short CONN_COUNT = 10;
SOCKET Connections[CONN_COUNT]; //массив для хранения соединений
int connCounter = 0; //индекс соединения
//char chCounter[2];

// TODO: установите здесь ссылки на дополнительные заголовки, требующиеся для программы.
