// ConsoleApplication1.cpp: определяет точку входа для консольного приложения.
//
#include "stdafx.h"
#include <boost\asio.hpp>
#include <iostream>
#include <fstream>
#include "windows.h"
#include <vector>
#include "omp.h"

const int k = 5;
int kf = 1;
bool semaphor[k];
HANDLE Threads[k];
std::vector <std::string> url;
std::vector <std::string> httpquery;
int times[k];

DWORD WINAPI Proc(LPVOID arg) {
	int i = *(int *)arg;
	int number = kf;
	std::string name = std::to_string(number)+".txt";
	std::cout << "thread = " << i << std::endl;

	boost::asio::ip::tcp::iostream stream(url[number-1], "http");
	stream << httpquery[number-1];

	std::ofstream f(name, std::ios::out);
	f << url[number - 1] << std::endl;
	f << httpquery[number - 1];
	f << stream.rdbuf();
	
	semaphor[i] = true;
	times[i] = 0;
	ExitThread(0);
}

int FreeThread()
{
	while (true)
	{
		for (int i = 0; i < k; i++)
		{
			if (semaphor[i])
				return i;
			if ((times[i]) && (omp_get_wtime() - times[i] > 300))
			{
				TerminateThread(Threads[i], 0);
				semaphor[i] = true;
			}
		}
		Sleep(1);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "Russian");
	const int len = 256;
	char line[256];
	std::string s = "";

	for (int i = 0; i < k; i++)
	{
		semaphor[i] = true;
		Threads[i] = 0;
		times[i] = 0;
	}
	std::ifstream f("input.txt", std::ios::in);
	if (!f)
	{
		std::cout << "Ошибка открытия файла" << std::endl;
		system("PAUSE");
		return -1;
	}
	
	while (f.getline(line, len))
	{
		if (s == "")
		{
			url.push_back(line);
			f.getline(line, len);
		}

		if ((strspn(line, " \t\r\n") == strlen(line)))
			{
				s = s + "\n\r\n";
				httpquery.push_back(s);
				s = "";

				int i = FreeThread();
				semaphor[i] = false;
				times[i] = omp_get_wtime();
				Threads[i] = CreateThread(NULL, 0, Proc, (LPVOID)&i, 0, NULL);
				Sleep(1);
				kf++;
			}
			else
				if (s == "")
					s = s + line;
				else
				    s = s + "\r\n" + line;
	}
	
	// Wait for all threads to finish.
	WaitForMultipleObjects(k, Threads, TRUE, INFINITE);
	
	// Close all thread handles.
	for (int i = 0; i < k; i++) 
		if (Threads[i])
			CloseHandle(Threads[i]);
	system("pause");
	return 0;
}

