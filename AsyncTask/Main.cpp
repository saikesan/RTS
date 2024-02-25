#include <stdio.h>
#include <windows.h>

#define BLOCK_SIZE 65536
#define D0X_LAST_SYMBOL 1

/*
 * Todo:
 * 
 * 10. ����������� ���������, �������������� ������� ���������� �������� 
 * �������� �������� � �����. ������ ����� ������ �������������� ����������� 
 * ��������. ������� ��������� ������ �������� �� ����� ����.
 * 
 * 19. ������� 2 ������� � 1 ������. ���������� ������������ �������� ������ 
 * �� �������� � ������� ����� ���� � ��� �� ����� � ����������� ������. 
 * ������������� � �������������� ���������.
 */


/*
 * ����� �������� ��� �����, ������� �� 299 ������ � ������ ���_����� �� ��� ������ �����,
 * � ����������_����� �� ���������� ������ �����.
 * 
 * TEXT("../TestFiles/���_�����.����������_�����"), <- ��� �������� �� ������.
 * 
 * � ����� ������� TestFiles ������������� �������� �����, �� ����� ��� ���������� �������� �������� �������� � ���.
 */

/*
 * ������� ������������ ���������� �������� �������� �������� � ����������� ����� �����.
 * 
 * params:
 *  pointer   - ��������� �� ��������� ������.
 *  blockSize - ������ ���� ������.
 * 
 * returns:
 *  ������ �����.
 */
LONGLONG CountingRuSymbols(PBYTE pointer, DWORD blockSize);

/*
 * ������� ������� ��������� ������� ��� ����������� ������.
 *
 * params:
 *  e  - ��������� �������.
 *  sa - ��������� �� ��������� ���������� �������������.
 *
 * returns:
 *  0 - ���� ������� ��������� �������, ����� -1.
 */
int CreatingEvent(PHANDLE e, PSECURITY_ATTRIBUTES sa);

/*
 * ������� ������� ��������� ����� ��� ������������ ������ ������.
 * ����� ������� �������� ������ ���������� �����.
 *
 * params:
 *  file	 - ��������� �����.
 *  fileSize - ������ ���������� �����.
 *  sa		 - ��������� �� ��������� ���������� �������������.
 *
 * returns:
 *  0 - ���� ������� ��������� �������, ����� -1.
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

	// ������ ������ ����� �����. �� �������� (65537), ��� ��� ������� �������� 
	// �������� ���������� 2 �������. ����� ���������� ����� �������� ����� ����� 
	// ������ ������ ���� ���� �� ���� ����������� ������, ��� ����� �� ������ 
	// ������ �� 1 ������ ������ �� ��������� �����. �� �������� ���� ��� ����� 
	// �������� ������ � 65537 ����� (� �� � 65538).
	DWORD blockSize = BLOCK_SIZE + D0X_LAST_SYMBOL;

	PBYTE pointer1 = new BYTE[blockSize];
	PBYTE pointer2 = new BYTE[blockSize];
	PBYTE readingBuffer = pointer1;
	PBYTE countingBuffer = pointer2;

	// ��������� ���������� ������ ������.
	ZeroMemory(pointer1, blockSize);
	ZeroMemory(pointer2, blockSize);
	
	// ��������� ��������� ��������� ������������.
	// Event � file �� ������ ���� ������������ ��������� ����������, ��� ��� sa.bInheritHandle = FALSE.
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = FALSE;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);


	// �������� ������� ��� ����������� ������ � ������.
	if (CreatingEvent(&e, &sa) != 0)
	{
		return -1;
	}


	// ��������� ��������� OVERLAPPED ���������� ����������
	// ��� ��������� ������� ��� ������������ ������ �� �����.
	ol.hEvent = e;

	// ��� 2 ��������� �������� �� �������� � �����. ��� ��� �������� � ������ �����, �� �������� ��.
	ol.Offset = 0;
	ol.OffsetHigh = 0;


	// �������� ��������� �����.
	if (CreatingFileAndGettgingFileSize(&fileSize, &file, &sa) != 0)
	{
		return -1;
	}


	DWORD currentRead = 0;	  // ���������� ������� ����������� ������ � ������ ReadFile.
	DWORD ruSymbolsCount = 0; // ���������� �������� �������� ��������.

	// ���� ������ ����� ������ 65537, �� � ������ ����� �� ������� �����, � �� ���� � 65337 ����.
	if (fileSize.QuadPart < static_cast<LONGLONG>(blockSize))
		blockSize = static_cast<DWORD>(fileSize.QuadPart);
	
	// ���������� ������ ����� ������ ���� �����.
	if (!ReadFile(file, readingBuffer, blockSize, NULL, &ol))
	{
		if (auto errorCode = GetLastError() != ERROR_IO_PENDING)
		{
			printf("File reading error: %d\n", errorCode);

			return -1;
		}
	}

	// �������� ��������� ������������ ������. � currentRead ����� ��������� ��, ������� ���� ������� ���� �������.
	if (!GetOverlappedResult(file, &ol, &currentRead, TRUE))
	{
		printf("GetOverlappedResult error: %d\n", GetLastError());

		return -1;
	}

	// ���� ������ ����� ������ ������� �����, �� ������ ��������� ���-�� �������� ��������.
	if (fileSize.QuadPart < BLOCK_SIZE + D0X_LAST_SYMBOL)
	{
		// ������� �������� �������� ��������.
		ruSymbolsCount = CountingRuSymbols(readingBuffer, currentRead);
		printf("Count of russian symbols: %d\n", ruSymbolsCount);

		return 0;
	}

	// ��� ��� �� �� ��� ��������� 65537 ����, �� 
	// � ������� ��������� � ��� ����� 65537 ����.
	// �� ��� ��� �������� ���� ������ �� ����� ��������� � 65537 �����,
	// �� ��������� ���������� ������� ��������� ������ �� 1.
	if (currentRead == BLOCK_SIZE + D0X_LAST_SYMBOL)
		currentRead -= D0X_LAST_SYMBOL;

	// ����������� ��������� ��� �������� �������� ������.
	bytesReaded.QuadPart += currentRead;
	ol.Offset = bytesReaded.LowPart;
	ol.OffsetHigh = bytesReaded.HighPart;

	// ���������� ������ ����, ���� �� ������ �� �����.
	while (bytesReaded.QuadPart <= fileSize.QuadPart)
	{
		// ���������� 2 ��������� �� ������.
		// ��� ����, ����� ���� �� ���������� ������ � ���� � ������� ReadFile,
		// �� ������ �� ������������ ���������� ������ ��������.
		PBYTE temp = readingBuffer;
		readingBuffer = countingBuffer;
		countingBuffer = temp;

		// ���� �������� ��������� ������ ��� 65537, �� ��������� ������� ������� ��������.
		if ((bytesReaded.QuadPart + static_cast<LONGLONG>(blockSize)) > fileSize.QuadPart)
			blockSize = static_cast<DWORD>(fileSize.QuadPart - bytesReaded.QuadPart);

		if (blockSize > 0)
		{
			// ���������� ������ ����� ������ ���� �����.
			if (!ReadFile(file, readingBuffer, blockSize, NULL, &ol))
			{
				if (auto errorCode = GetLastError() != ERROR_IO_PENDING)
				{
					printf("File reading error: %d\n", errorCode);

					return -1;
				}
			}
		}

		// ������� �������� �������� ��������.
		ruSymbolsCount += CountingRuSymbols(countingBuffer, currentRead);

		if (blockSize > 0)
		{
			// �������� ��������� ������������ ������. � currentRead ����� ��������� ��, ������� ���� ������� ���� �������.
			if (!GetOverlappedResult(file, &ol, &currentRead, TRUE))
			{
				printf("GetOverlappedResult error: %d\n", GetLastError());

				return -1;
			}
		}
		
		// ��� ��� �� �� ��� ��������� 65537 ����, �� 
		// � ������� ��������� � ��� ����� 65537 ����.
		// �� ��� ��� �������� ���� ������ �� ����� ��������� � 65537 �����,
		// �� ��������� ���������� ������� ��������� ������ �� 1.
		if (currentRead == BLOCK_SIZE + D0X_LAST_SYMBOL)
			currentRead -= D0X_LAST_SYMBOL;

		// ����������� ��������� ��� �������� �������� ������.
		bytesReaded.QuadPart += currentRead;
		ol.Offset = bytesReaded.LowPart;
		ol.OffsetHigh = bytesReaded.HighPart;
	}

	printf("Count of russian symbols: %d\n", ruSymbolsCount);

	// �������� ������ � �������� ����������.
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

				// ����� ����� �� ������������� pointer[i + 1], � ����� ���� � ��������� ������� ����������� i �� 1. 
				// ��� ��� ����� �������, ��� ��� ��� ����� ������� �� �������, �� ������ �������� ��������.
				i++; 
			}
		}
		else if (pointer[i] == 0xD1)
		{
			if (((pointer[i + 1] >= 128) && (pointer[i + 1] <= 143)) || (pointer[i + 1] == 145))
			{
				ruSymbolsCount++;

				// ����� ����� �� ������������� pointer[i + 1], � ����� ���� � ��������� ������� ����������� i �� 1. 
				// ��� ��� ����� �������, ��� ��� ��� ����� ������� �� �������, �� ������ �������� ��������.
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
		sa,		// �������� �� ��������� ������������, ������� ����������, ����� �� �������� �������� ����������� ��������� ����� �������.
		FALSE,	// ��� ������ ��������� �������, FALSE - �������������.
		TRUE,	// ��������� ���������, ������ ���� TRUE.
		NULL
	);

	if (e == NULL) // ���� ������� �� ���� �������.
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
		TEXT("../TestFiles/196601.txt"),			  // ��� �����.
		GENERIC_READ | GENERIC_WRITE,				  // ������ ������� � �����.
		FILE_SHARE_READ | FILE_SHARE_WRITE,			  // ����� ����������� ������� � �����.
		sa,											  // ��������� �� ��������� ������������, ������� ����������, ����� �� �������� �������� ����������� ��������� ����� �����.
		OPEN_EXISTING,								  // ����� �������� �����, � ������ ������ ����� ������� ������ ������������ ����, ����� ������.
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // �������� � ����� �����.
		NULL										  // ��������� ������� ��� ������������ �����, �� NULL ������ ��� �������� ������ �����.
	);

	if (*file == INVALID_HANDLE_VALUE) // ���� ���� �� ��� ������.
	{
		printf("Creating file error: %d\n", GetLastError());

		return -1;
	}

	printf("File created successfully!\n");


	if (GetFileSizeEx(*file, fileSize) == 0) // �� ���� ������� �������� 0 ������ ��� ������.
	{
		printf("Getting file size error: %d\n", GetLastError());

		return -1;
	}

	printf("File size: %I64u\n", fileSize->QuadPart);

	return 0;
}