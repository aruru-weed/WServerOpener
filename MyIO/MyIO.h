#pragma once
#include <string>
#include <iostream>
#include <thread>
#include <Windows.h>

std::wstring utf8_decode(const std::string& str);

namespace my {
	class io {
	public:
		HANDLE inPipe, outPipe;
		std::string Name;

		io(std::string pipename);

		bool join();

		//input
		std::string operator <<(std::string);
		//input
		std::istream& operator <<(std::istream&);
		//output
		std::string operator >>(std::string&);
		//output
		std::string operator >>(std::ostream&);
		~io();
	};

	inline io::io(std::string pipename) :Name(pipename)
	{
		outPipe = INVALID_HANDLE_VALUE;
		outPipe = CreateNamedPipe(utf8_decode(R"(\\.\pipe\)" + pipename + ".c2p").c_str(), //lpName
			PIPE_ACCESS_DUPLEX,            // dwOpenMode
			PIPE_TYPE_BYTE | PIPE_WAIT,     // dwPipeMode
			10,                              // nMaxInstances
			0,                              // nOutBufferSize
			0,                              // nInBufferSize
			100,                            // nDefaultTimeOut
			NULL);                          // lpSecurityAttributes
		inPipe = INVALID_HANDLE_VALUE;
		inPipe = CreateNamedPipe(utf8_decode(R"(\\.\pipe\)" + pipename + ".p2c").c_str(), //lpName
			PIPE_ACCESS_DUPLEX,            // dwOpenMode
			PIPE_TYPE_BYTE | PIPE_WAIT,     // dwPipeMode
			10,                              // nMaxInstances
			0,                              // nOutBufferSize
			0,                              // nInBufferSize
			100,                            // nDefaultTimeOut
			NULL);                          // lpSecurityAttributes
	}

	inline bool io::join()
	{
		bool ret = true;
		std::thread th([this, &ret] {
			if (inPipe == INVALID_HANDLE_VALUE) {
				std::cout << "ì¬‚Å‚«‚È‚©‚Á‚½@><	(in)";
			}
			if (!ConnectNamedPipe(inPipe, NULL)) {
				std::cout << "Ú‘±‚Å‚«‚È‚¢!	(in)";
				ret = false;
			}

			if (outPipe == INVALID_HANDLE_VALUE) {
				std::cout << "ì¬‚Å‚«‚È‚©‚Á‚½@><	(out)";
			}
			if (!ConnectNamedPipe(outPipe, NULL)) {
				std::cout << "Ú‘±‚Å‚«‚È‚¢!	(out)";
				ret = false;
			}

			if (!ret) {
				CloseHandle(inPipe);
				CloseHandle(outPipe);
			}
			}
		);
		return ret;
	}

	inline std::string io::operator<<(std::string str)
	{
		str += "\n";
		DWORD dw;
		if (!WriteFile(outPipe, str.c_str(), str.length(), &dw, NULL)) {
			std::cout << "‘‚«ž‚ß‚È‚¢!";
			return str;
		}
		FlushFileBuffers(outPipe);
		return str;
	}

	inline std::istream& io::operator<<(std::istream& out)
	{
		std::string str;
		out >> str;
		*this << str;
		return out;
	}

	inline std::string io::operator>>(std::string& str) {
		char buffer[256] = {};
		DWORD dw;
		if (!ReadFile(inPipe, buffer, sizeof(buffer), &dw, NULL)) {
			throw std::exception("“Ç‚Ýž‚ß‚È‚¢!");
		}
		str = Name + " : " + buffer;
		return (Name + " : " + buffer);
	}

	inline std::string io::operator>>(std::ostream& out)
	{
		std::string str;
		*this >> str;
		out << str;
		return std::string();
	}

	std::ostream& operator<<(std::ostream& stream, io& io);

	inline io::~io()
	{
		FlushFileBuffers(outPipe);
		DisconnectNamedPipe(outPipe);
		CloseHandle(outPipe);
		FlushFileBuffers(inPipe);
		DisconnectNamedPipe(inPipe);
		CloseHandle(inPipe);
	}


	//--------------------------------------------------------------------------------------------------------------------

	namespace sub {
		class Client {
		public:
			HANDLE hPipe;

			Client(std::string pipename);
			~Client();
		};

		inline Client::Client(std::string pipename)
		{
			hPipe = INVALID_HANDLE_VALUE;
			hPipe = CreateFile(utf8_decode(R"(\\.\pipe\)" + pipename).c_str(), //lpName
				GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		}
		inline Client::~Client()
		{
			CloseHandle(hPipe);
		}
	}
}