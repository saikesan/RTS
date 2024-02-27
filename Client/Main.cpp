#include <Windows.h>
#include <stdio.h>
#include <iostream>

// ������ ������ ������ ����������� ������.
#define MAPPED_MEMORY_SIZE 1024

// ��� ������������ ������� ��������, ��� ������ � ��� �� ������ ���������.
LPCWSTR semaphoreName = TEXT("Global\\mySemaphore");

// ��� ������������ ������� ����������� ������, ��� ������ � ��� �� ������ ���������.
LPCWSTR fileMapName = TEXT("Global\\myFileMap");



int main()
{
	HANDLE semaphore = NULL;
	HANDLE fileMap = NULL;
	LPCTSTR fileMemory = NULL;
	std::string msg;

	// �������� ��� ���������� ������ ��������.
	semaphore = OpenSemaphore
	(
		SEMAPHORE_ALL_ACCESS, // ��������� ��������� ������������.
		NULL,				  // ��������� ���������� �������� �������.
		semaphoreName		  // ��� ��������.
	);

	if (semaphore == NULL) // �������� �� ��, ��� ������� ������.
	{
		printf("Semaphore creating error: %d", GetLastError());

		return -1;
	}

	// �������� ��� ���������� ������� ����������� ������.
	fileMap = OpenFileMapping
	(
		FILE_MAP_ALL_ACCESS, // ������ ������� (�� ����� ������ � ������).
		NULL,				 // ���� TRUE, �� �������� ��� ����� �������� �������� ����� ����������� ���� ���������.
		fileMapName			 // ��� �������.
	);

	if (fileMap == NULL) // �������� �� ��, ��� ������� ������.
	{
		printf("Create file mapping error: %d", GetLastError());

		return -1;
	}

	std::cout << "Enter message:\n";
	std::cin >> msg; // ������ ��������� ������� ������� � ����������� ������.

	// �������� ��������� �� ������� ������ ����� ����������� ������, ����� �� ����� � ��� ��� �� ������.
	fileMemory = (LPTSTR)MapViewOfFile
	(
		fileMap,			 // ��������� ����������� ������ �� ������� ����� �������� ���������.
		FILE_MAP_ALL_ACCESS, // ������ ������� � ������ (�� ����� ������ � ������).
		0,					 // �������� ������ ����������� ������ (�� ���� �������� ���� ����������� ������ � ������� ����� ������ ��� ������(���� ��� �������� - � ������ ����� � ���������� ������ �� �� ������ ����� ������)) ������� �����.
		0,					 // �������� ������ ����������� ������ (�� ���� �������� ���� ����������� ������ � ������� ����� ������ ��� ������(���� ��� �������� - � ������ ����� � ���������� ������ �� �� ������ ����� ������)) ������� �����.
		MAPPED_MEMORY_SIZE	 // ������ �������
	);
		
	if (fileMemory == NULL) // ��������� �������� �� ���������.
	{
		printf("Could not map view of file %d.\n",	GetLastError());

		CloseHandle(fileMap);

		return 1;
	}

	// �������� ������ � ������� �������� ����� ���������� ���� ��������� � ������� ������ ������� ����������� ������. 
	// �� ���� ���������� ������ � ���� ����������� ������.
	memcpy((void*)fileMemory, msg.c_str(), msg.length());

	UnmapViewOfFile(fileMemory); // ����������� ����������� ���������.

	// ����������� ���-�� �������� �������� ��� ����, ����� ��������� ������� WaitForSingleObject.
	if (!ReleaseSemaphore(semaphore, 1, NULL))
	{
		printf("Release semaphore error: %d", GetLastError());

		return -1;
	}

	// ����������� ���������.
	CloseHandle(fileMap);
	CloseHandle(semaphore);

	return 0;
}