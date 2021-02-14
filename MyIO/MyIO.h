#pragma once
#include <string>
#include <iostream>
#include <thread>
#include <Windows.h>

std::wstring utf8_decode(const std::string& str);

namespace my {
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

	// ----------------------------------------------------------------------------------------------------------------------------

	class io {
		std::thread th, thw, thr;
		bool th_end = false, ret = true, wait_w = false, wait_r = false;
	public:
		HANDLE inPipe, outPipe;
		std::string Name;

		io(std::string pipename);

		bool join();

		bool isWaiting();

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
		thr = std::thread([this] {
			if (inPipe == INVALID_HANDLE_VALUE) {
				std::cout << Name + " : ì¬‚Å‚«‚È‚©‚Á‚½@><	(in)";
				ret = false;
				th_end = true;
				return;
			}
			wait_r = true;
			if (!ConnectNamedPipe(inPipe, NULL)) {
				std::cout << Name + " : Ú‘±‚Å‚«‚È‚¢!	(in)";
				ret = false;
				th_end = true;
				return;
			}
			});
		thw = std::thread([this] {
			if (outPipe == INVALID_HANDLE_VALUE) {
				std::cout << Name + " : ì¬‚Å‚«‚È‚©‚Á‚½@><	(out)";
				ret = false;
				th_end = true;
				return;
			}
			wait_w = true;
			if (!ConnectNamedPipe(outPipe, NULL)) {
				std::cout << Name + " : Ú‘±‚Å‚«‚È‚¢!	(out)";
				ret = false;
				th_end = true;
				return;
			}
			});

		th = std::thread([this] {
			while (!th_end && !isWaiting())
			{
				Sleep(100);
			}
			if (isWaiting())
				return;

			thr.detach();
			thw.detach();

			if (!ret) {
				CloseHandle(inPipe);
				CloseHandle(outPipe);
			}
			}
		);
	}

	inline bool io::join()
	{
		th.join();

		return ret;
	}

	inline bool io::isWaiting()
	{
		return wait_r && wait_w;
	}

	inline std::string io::operator<<(std::string str)
	{
		str += "\n";
		DWORD dw;
		if (!WriteFile(outPipe, str.c_str(), str.length(), &dw, NULL)) {
			std::cout << Name + " : ‘‚«ž‚ß‚È‚¢!";
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
		if (this == nullptr) {
			std::cout << "NullCout";
			return "";
		}
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
		bool Disconnected = false;
		std::thread dummy_join([this, &Disconnected] {
			while (!Disconnected)
			{
				sub::Client a(Name);
			}
			});
		DisconnectNamedPipe(outPipe);
		Disconnected = true;
		CloseHandle(outPipe);
		FlushFileBuffers(inPipe);
		DisconnectNamedPipe(inPipe);
		CloseHandle(inPipe);

		dummy_join.join();
		th.detach();
		thw.detach();
		thr.detach();
	}
}