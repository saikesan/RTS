#include <Windows.h>
#include <stdio.h>
#include <iostream>

// Размер памяти буфера разделяемой памяти.
#define MAPPED_MEMORY_SIZE 1024

// Имя создаваемого объекта семафора, для работы с ним из других процессов.
LPCWSTR semaphoreName = TEXT("Global\\mySemaphore");

// Имя создаваемого объекта разделяемой памяти, для работы с ней из других процессов.
LPCWSTR fileMapName = TEXT("Global\\myFileMap");



int main()
{
	HANDLE semaphore = NULL;
	HANDLE fileMap = NULL;
	LPCTSTR fileMemory = NULL;
	std::string msg;

	// Открытие уже созданного обекта семафора.
	semaphore = OpenSemaphore
	(
		SEMAPHORE_ALL_ACCESS, // описатель структуры безопасности.
		NULL,				  // начальное количество открытых каналов.
		semaphoreName		  // имя семафора.
	);

	if (semaphore == NULL) // проверка на то, что создали объект.
	{
		printf("Semaphore creating error: %d", GetLastError());

		return -1;
	}

	// Открытие уже созданного объекта разделяемой памяти.
	fileMap = OpenFileMapping
	(
		FILE_MAP_ALL_ACCESS, // режимы доступа (мы можем читать и писать).
		NULL,				 // если TRUE, то дочерние для этого процесса процессы могут наследовать этот описатель.
		fileMapName			 // имя объекта.
	);

	if (fileMap == NULL) // проверка на то, что создали объект.
	{
		printf("Create file mapping error: %d", GetLastError());

		return -1;
	}

	std::cout << "Enter message:\n";
	std::cin >> msg; // вводим сообщения которое запишем в разделяемую память.

	// получаем указатель на область памяти нашей разделяемой памяти, чтобы мы могли в нее что то писать.
	fileMemory = (LPTSTR)MapViewOfFile
	(
		fileMap,			 // описатель разделяемой памяти на которую будем получать указатель.
		FILE_MAP_ALL_ACCESS, // режимы доступа к памяти (мы можем писать и читать).
		0,					 // смещение внутри разделяемой памяти (по сути выбираем блок разделяемой памяти в который будем писать или читать(если еще понятнее - с какого места в раздеяемой памяти от ее начала будем писать)) старшее слово.
		0,					 // смещение внутри разделяемой памяти (по сути выбираем блок разделяемой памяти в который будем писать или читать(если еще понятнее - с какого места в раздеяемой памяти от ее начала будем писать)) младшее слово.
		MAPPED_MEMORY_SIZE	 // размер области
	);
		
	if (fileMemory == NULL) // проверяем получили ли указатель.
	{
		printf("Could not map view of file %d.\n",	GetLastError());

		CloseHandle(fileMap);

		return 1;
	}

	// Копируем память в которой находтся текст введенного нами сообщения в область памяти объекта разделяемой памяти. 
	// По сути записываем данные в нашу разделяемую память.
	memcpy((void*)fileMemory, msg.c_str(), msg.length());

	UnmapViewOfFile(fileMemory); // освобождаем получаенный указатель.

	// Увеличиваем кол-во объектов семафора для того, чтобы сработала функция WaitForSingleObject.
	if (!ReleaseSemaphore(semaphore, 1, NULL))
	{
		printf("Release semaphore error: %d", GetLastError());

		return -1;
	}

	// Освобождаем описатели.
	CloseHandle(fileMap);
	CloseHandle(semaphore);

	return 0;
}