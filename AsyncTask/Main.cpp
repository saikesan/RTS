#include <stdio.h>
#include <windows.h>

#define BLOCK_SIZE 65536
#define D0X_LAST_SYMBOL 1

/*
 * Todo:
 * 
 * 10. Разработать программу, осуществляющую подсчет количества символов 
 * русского алфавита в файле. Чтение файла должно осуществляться параллельно 
 * подсчету. Процесс программы должен состоять из одной нити.
 * 
 * 19. Имеется 2 клиента и 1 сервер. Необходимо осуществлять передачу данных 
 * от клиентов к серверу через один и тот же буфер в разделяемой памяти. 
 * Синхронизация с использованием семафоров.
 */


/*
 * Чтобы изменить имя файла, перейди на 299 строку и замени ИМЯ_ФАЙЛА на имя твоего файла,
 * а РАСШИРЕНИЕ_ФАЙЛА на расширение твоего файла.
 * 
 * TEXT("../TestFiles/ИМЯ_ФАЙЛА.РАСШИРЕНИЕ_ФАЙЛА"), <- так выглядит та строка.
 * 
 * В папке проекта TestFiles располагаются тестовые файлы, их имена это количество символов русского алфавита в них.
 */

/*
 * Функция подсчитывает количество символов русского алфавита в прочитанном блоке файла.
 * 
 * params:
 *  pointer   - указатель на считанную память.
 *  blockSize - размер этой памяти.
 * 
 * returns:
 *  размер файла.
 */
LONGLONG CountingRuSymbols(PBYTE pointer, DWORD blockSize);

/*
 * Функция создает описатель события для асинхронной работы.
 *
 * params:
 *  e  - описатель события.
 *  sa - указатель на структуру аттрибутов безопастности.
 *
 * returns:
 *  0 - если функция выполнена успешно, иначе -1.
 */
int CreatingEvent(PHANDLE e, PSECURITY_ATTRIBUTES sa);

/*
 * Функция создает описатель файла для асинхронного чтения данных.
 * Также функция получает размер созданного файла.
 *
 * params:
 *  file	 - описатель файла.
 *  fileSize - размер созданного файла.
 *  sa		 - указатель на структуру аттрибутов безопастности.
 *
 * returns:
 *  0 - если функция выполнена успешно, иначе -1.
 */
int CreatingFileAndGettgingFileSize(PLARGE_INTEGER fileSize, PHANDLE file, PSECURITY_ATTRIBUTES sa);



int main()
{
	HANDLE e = NULL;
	OVERLAPPED ol = { 0 };
	LARGE_INTEGER bytesReaded = { 0 };

	HANDLE file = NULL;
	LARGE_INTEGER fileSize = { 0 };
	SECURITY_ATTRIBUTES sa = { 0 };

	// Задаем размер блока файла. Он нечетный (65537), так как символы русского 
	// алфавита кодируются 2 байтами. Может возникнуть такая ситуация когда будет 
	// считан только один байт из двух описывающих символ, для этого мы всегда 
	// читаем на 1 символ больше из следущего блока. Но следущий блок все равно 
	// начинаем читать с 65537 байта (а не с 65538).
	DWORD blockSize = BLOCK_SIZE + D0X_LAST_SYMBOL;

	PBYTE pointer1 = new BYTE[blockSize];
	PBYTE pointer2 = new BYTE[blockSize];
	PBYTE readingBuffer = pointer1;
	PBYTE countingBuffer = pointer2;

	// Заполняем выделенную память нулями.
	ZeroMemory(pointer1, blockSize);
	ZeroMemory(pointer2, blockSize);
	
	// Заполняем структуру атрибутов бесопасности.
	// Event и file не смогут быть унаследованы дочерними процессами, так как sa.bInheritHandle = FALSE.
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);


	// Создание события для асинхронной работы с файлом.
	if (CreatingEvent(&e, &sa) != 0)
	{
		return -1;
	}


	// Заполняем структуру OVERLAPPED начальными значениями
	// Это описатель события для асинхронного чтения из файла.
	ol.hEvent = e;

	// Эти 2 параметра отвечают за смещение в файле. Так как начинаем с начала файла, то обнуляем их.
	ol.Offset = 0;
	ol.OffsetHigh = 0;


	// Создание описателя файла.
	if (CreatingFileAndGettgingFileSize(&fileSize, &file, &sa) != 0)
	{
		return -1;
	}


	DWORD currentRead = 0;	  // Количество реально прочитанных байтов в функии ReadFile.
	DWORD ruSymbolsCount = 0; // Количество символов русского алфавита.

	// Если размер файла меньше 65537, то и читать будем по размеру файла, а не блок в 65337 байт.
	if (fileSize.QuadPart < static_cast<LONGLONG>(blockSize))
		blockSize = static_cast<DWORD>(fileSize.QuadPart);
	
	// Асинхронно читаем самый первой блок файла.
	if (!ReadFile(file, readingBuffer, blockSize, NULL, &ol))
	{
		if (auto errorCode = GetLastError() != ERROR_IO_PENDING)
		{
			printf("File reading error: %d\n", errorCode);

			return -1;
		}
	}

	// Получаем результат асинхронного чтения. В currentRead будет заненсено то, сколько байт реально было считано.
	if (!GetOverlappedResult(file, &ol, &currentRead, TRUE))
	{
		printf("GetOverlappedResult error: %d\n", GetLastError());

		return -1;
	}

	// Если размер файла меньше размера блока, то просто считываем кол-во русскких символов.
	if (fileSize.QuadPart < BLOCK_SIZE + D0X_LAST_SYMBOL)
	{
		// Подсчет символов русского алфавита.
		ruSymbolsCount = CountingRuSymbols(readingBuffer, currentRead);
		printf("Count of russian symbols: %d\n", ruSymbolsCount);

		return 0;
	}

	// Так как мы за раз считываем 65537 байт, то 
	// и реально считанных у нас будет 65537 байт.
	// Но так как следущий блок данных мы будем считывать с 65537 байта,
	// То уменьшаем количество реально считанных байтов на 1.
	if (currentRead == BLOCK_SIZE + D0X_LAST_SYMBOL)
		currentRead -= D0X_LAST_SYMBOL;

	// Увеличиваем смещещние для следущей операции чтения.
	bytesReaded.QuadPart += currentRead;
	ol.Offset = bytesReaded.LowPart;
	ol.OffsetHigh = bytesReaded.HighPart;

	// Асинхронно читаем файл, пока не дойдем до конца.
	while (bytesReaded.QuadPart <= fileSize.QuadPart)
	{
		// Используем 2 указателя на память.
		// Для того, чтобы пока мы асинхронно читаем в один в функции ReadFile,
		// во втором мы подсчитываем количество нужных символов.
		PBYTE temp = readingBuffer;
		readingBuffer = countingBuffer;
		countingBuffer = temp;

		// Если осталось прочитать меньше чем 65537, то прочитаем столько сколько осталось.
		if ((bytesReaded.QuadPart + static_cast<LONGLONG>(blockSize)) > fileSize.QuadPart)
			blockSize = static_cast<DWORD>(fileSize.QuadPart - bytesReaded.QuadPart);

		if (blockSize > 0)
		{
			// Асинхронно читаем самый первой блок файла.
			if (!ReadFile(file, readingBuffer, blockSize, NULL, &ol))
			{
				if (auto errorCode = GetLastError() != ERROR_IO_PENDING)
				{
					printf("File reading error: %d\n", errorCode);

					return -1;
				}
			}
		}

		// Подсчет символов русского алфавита.
		ruSymbolsCount += CountingRuSymbols(countingBuffer, currentRead);

		if (blockSize > 0)
		{
			// Получаем результат асинхронного чтения. В currentRead будет заненсено то, сколько байт реально было считано.
			if (!GetOverlappedResult(file, &ol, &currentRead, TRUE))
			{
				printf("GetOverlappedResult error: %d\n", GetLastError());

				return -1;
			}
		}
		
		// Так как мы за раз считываем 65537 байт, то 
		// и реально считанных у нас будет 65537 байт.
		// Но так как следущий блок данных мы будем считывать с 65537 байта,
		// То уменьшаем количество реально считанных байтов на 1.
		if (currentRead == BLOCK_SIZE + D0X_LAST_SYMBOL)
			currentRead -= D0X_LAST_SYMBOL;

		// Увеличиваем смещещние для следущей операции чтения.
		bytesReaded.QuadPart += currentRead;
		ol.Offset = bytesReaded.LowPart;
		ol.OffsetHigh = bytesReaded.HighPart;
	}

	printf("Count of russian symbols: %d\n", ruSymbolsCount);

	// Очищение памяти и закрытие описателей.
	delete[] pointer1;
	delete[] pointer2;
	CloseHandle(e);
	CloseHandle(file);

	return 0;
}


LONGLONG CountingRuSymbols(PBYTE pointer, DWORD blockSize)
{
	LONGLONG ruSymbolsCount = 0;

	for (DWORD i = 0; i < blockSize; i++)
	{
		if (pointer[i] == 0xD0)
		{
			if (((pointer[i + 1] >= 144) && (pointer[i + 1] <= 191)) || (pointer[i + 1] == 001))
			{
				ruSymbolsCount++;

				// Чтобы потом не просматривать pointer[i + 1], а сразу идти к следущему СИМВОЛУ увеличиваем i на 1. 
				// Так как сразу понятно, что раз оба байта подошли по условию, то символ русского алфавита.
				i++; 
			}
		}
		else if (pointer[i] == 0xD1)
		{
			if (((pointer[i + 1] >= 128) && (pointer[i + 1] <= 143)) || (pointer[i + 1] == 145))
			{
				ruSymbolsCount++;

				// Чтобы потом не просматривать pointer[i + 1], а сразу идти к следущему СИМВОЛУ увеличиваем i на 1. 
				// Так как сразу понятно, что раз оба байта подошли по условию, то символ русского алфавита.
				i++;
			}
		}
	}

	return ruSymbolsCount;
}

int CreatingEvent(PHANDLE e, PSECURITY_ATTRIBUTES sa)
{
	*e = CreateEvent
	(
		sa,		// Указтель на структуру безопасности, которая определяет, могут ли дочерние процессы наследовать описатель этого события.
		FALSE,	// Вид сборса состояния события, FALSE - автоматически.
		TRUE,	// Начальное состояние, должно быть TRUE.
		NULL
	);

	if (e == NULL) // Если событие не было создано.
	{
		printf("Creating event error: %d\n", GetLastError());

		return -1;
	}

	printf("Event created successfully!\n");

	return 0;
}

int CreatingFileAndGettgingFileSize(PLARGE_INTEGER fileSize, PHANDLE file, PSECURITY_ATTRIBUTES sa)
{
	*file = CreateFile
	(
		TEXT("../TestFiles/196601.txt"),			  // Имя файла.
		GENERIC_READ | GENERIC_WRITE,				  // Режимы доступа к файлу.
		FILE_SHARE_READ | FILE_SHARE_WRITE,			  // Режим совместного доступа к файлу.
		sa,											  // Указатель на структуру безопасности, которая определяет, могут ли дочерние процессы наследовать описатель этого файла.
		OPEN_EXISTING,								  // Режим открытия файла, в данном случае может открыть только существующий файл, иначе ошибка.
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // Атрибуты и флаги файла.
		NULL										  // Описатель шаблона для создаваемого файла, не NULL только при создании НОВОГО файла.
	);

	if (*file == INVALID_HANDLE_VALUE) // Если файл не был создан.
	{
		printf("Creating file error: %d\n", GetLastError());

		return -1;
	}

	printf("File created successfully!\n");


	if (GetFileSizeEx(*file, fileSize) == 0) // Из этой функции вернется 0 только при ошибке.
	{
		printf("Getting file size error: %d\n", GetLastError());

		return -1;
	}

	printf("File size: %I64u\n", fileSize->QuadPart);

	return 0;
}