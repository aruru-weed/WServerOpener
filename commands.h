#pragma once
#include "Command/command.h"
#include "MyIO/MyIO.h"
#include "sync.h"

#include <functional>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <exception>
#include <algorithm>
#include <chrono>

#include <filesystem>

#include <Windows.h>

//-----------------------------------------------------------------------------------------------
// globals
using namespace MyCommand;
using MyCommand::command_sturct;
namespace fs = std::filesystem;
namespace sc = std::chrono;

//-----------------------------------------------------------------------------------------------

extern bool end;
//	コマンドがなかった時参照される関数		(の代用)
struct NotFoundFunc {
	NotFoundFunc(MyCommand::command_sturct& cs);
};
extern Commands<NotFoundFunc> list;
extern sc::time_point<sc::system_clock> past;
extern std::mutex mut;

//-----------------------------------------------------------------------------------------------

//	小文字化関数
std::string str_lower(std::string str) {
	using namespace std;
	transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

// SetCurrentDelectryの排他的処理
void SetCD(std::string path, std::function<void()> f) {
	synchronized(mut) {
		sc::time_point now = sc::system_clock::now();
		past += sc::seconds(2);
		if (past < now) {
			SetCurrentDirectory(utf8_decode(path).c_str());
		}
		else {
			std::this_thread::sleep_for(past - now);
			SetCurrentDirectory(utf8_decode(path).c_str());
		}
		f();
		past = now;
	}
}

//Commands
class Pipes_Command : public Command {
	friend class Server_Command;
	std::unique_ptr<my::io> io;
	std::atomic<bool> setupped = false;
	std::string pipeName;
	std::thread WaitTh;
	std::thread ReadTh;
public:
	operator bool() {
		return setupped;
	}

	Pipes_Command(std::string PipeName) : pipeName(PipeName) {
		using namespace std;
		io.release();
		io = make_unique<my::io>(pipeName);
		WaitTh = std::thread([this]() {
			while (true)
			{
				if ((*io).join()) {
					//	ioの削除
					io.reset();
					io = make_unique<my::io>(pipeName);
					continue;
				}
				setupped = true;
				cout << pipeName << ": ready!" << endl;
				ReadTh = thread([this] {
					while (setupped)
					{
						try {
							*io >> cout;
						}
						catch (std::exception& e) {
							cout << e.what() << ": 3秒後に再読み込みします。" << endl;
							Sleep(3000);
						}
					}
					});
			}
			});
	}

	void run(command_sturct& cs)
	{
		using namespace std;
		try {
			if (setupped) {
				// stop　だったら io の中身を消す
				if (str_lower(cs.args.at(0)) == "stop") {
					//	サーバーに送信
					*io << getLineUppedArgs(cs.args);
					//	ioの削除
					io.reset();
					setupped = false;
				}
				else {
					//	サーバーに送信
					*io << getLineUppedArgs(cs.args);
				}
			}
			else {
				// 接続してなくて stop だったら
				if (str_lower(cs.args.at(0)) == "stop") {
					if (io != NULL) {
						my::sub::Client a(io->Name);
						std::cout << "Connect dummy client" << std::endl;
					}
					io.reset();
				}
				// 起動させる
				else if (str_lower(cs.args.at(0)) == "start") {
					io = make_unique<my::io>(pipeName);
					WaitTh.join();
					WaitTh = std::thread([this]() {
						while (true)
						{
							if ((*io).join()) {
								//	ioの削除
								io.reset();
								io = make_unique<my::io>(pipeName);
								continue;
							}
							setupped = true;
							cout << pipeName << ": ready!" << endl;
							ReadTh.join();
							ReadTh = thread([this] {
								while (setupped)
								{
									try {
										*io >> cout;
									}
									catch (std::exception& e) {
										cout << pipeName + ":" + e.what() << ": 3秒後に再読み込みします。" << endl;
										Sleep(3000);
									}
								}
								});
						}
						});
				}
			}
		}
		catch (std::exception& e) {
			cout << e.what() << endl;
		}
	}

	~Pipes_Command() {
		if (!setupped) {
			if (io != NULL) {
				my::sub::Client a(io->Name);
				std::cout << "Connect dummy client" << std::endl;
			}
		}
		WaitTh.join();
		ReadTh.join();
		io.get();
	}
};

class Server_Command : public Pipes_Command {
	bool settuped = false;
	std::string cmdLine;
	fs::path JarPath;
	std::thread JoinWaitTh;
	std::thread Process;
public:
	Server_Command(std::string _cmdLine, fs::path _JarPath, std::string PipeName) : Pipes_Command(PipeName), cmdLine(_cmdLine), JarPath(_JarPath) {
		JoinWaitTh = std::thread([this] {
			Sleep(500);
			Process = std::thread([this] {
				try {
					const std::string p2cstr(R"( > \\.\pipe\)");
					const std::string c2pstr(R"( < \\.\pipe\)");
					settuped = true;
					SetCD(JarPath.string(), [this, &p2cstr, &c2pstr] {
						system((cmdLine + p2cstr + pipeName + ".p2c" + c2pstr + pipeName + ".c2p").c_str());
						});
					settuped = false;
				}
				catch (std::exception& e) {
					std::cout << e.what() << std::endl;
				}
				});
			});
		settuped = true;
	}

	void run(command_sturct& cs) {
		// 接続してなくて stop だったら
		if (str_lower(cs.args.at(0)) == "stop") {
			if (settuped) {
				settuped = false;
				(*io) << "stop\n";
				Process.join();
			}
			Pipes_Command::run(cs);
		}
		// 起動させる
		else if (str_lower(cs.args.at(0)) == "start") {
			if (!settuped) {
				Pipes_Command::run(cs);
				JoinWaitTh.join();
				JoinWaitTh = std::thread([this] {
					Sleep(500);
					Process = std::thread([this] {
						try {
							const std::string p2cstr(R"( > \\.\pipe\)");
							const std::string c2pstr(R"( < \\.\pipe\)");
							settuped = true;
							SetCD(JarPath.string(), [this, &p2cstr, &c2pstr] {
								system((cmdLine + p2cstr + pipeName + ".p2c" + c2pstr + pipeName + ".c2p").c_str());
								});
							settuped = false;
						}
						catch (std::exception& e) {
							std::cout << e.what() << std::endl;
						}
						});
					});
			}
		}
		// restart
		else if (str_lower(cs.args.at(0)) == "restart") {
			JoinWaitTh.join();
			JoinWaitTh = std::thread([&cs, this] {
				if (settuped) {
					settuped = false;
					(*io) << "stop\n";
					Process.join();
					command_sturct a;
					a.label = cs.label;
					a.args.push_back("stop");
					Pipes_Command::run(a);
					Sleep(500);
					a.args.clear();
					a.args.push_back("start");
					Pipes_Command::run(a);
					Sleep(500);
					Process = std::thread([this] {
						try {
							const std::string p2cstr(R"( > \\.\pipe\)");
							const std::string c2pstr(R"( < \\.\pipe\)");
							settuped = true;
							SetCD(JarPath.string(), [this, &p2cstr, &c2pstr] {
								system((cmdLine + p2cstr + pipeName + ".p2c" + c2pstr + pipeName + ".c2p").c_str());
								});
							settuped = false;
						}
						catch (std::exception& e) {
							std::cout << e.what() << std::endl;
						}
						});
				}
				});
		}
		else {
			Pipes_Command::run(cs);
		}
	}
	~Server_Command() {
		if (settuped) {
			*Pipes_Command::io << "stop";
		}
		setupped = false;
		Process.detach();
		Pipes_Command::~Pipes_Command();
	}
};

//	コマンドがなかった時呼び出されるコマンドを設定する
class Set_Command : public Command {
public:
	static std::string key;

	Set_Command() {};
	Set_Command(std::string label) {
		key = label;
	}

	void run(command_sturct& cs)
	{
		if (list.contains(cs.args[0])) {
			Set_Command::key = cs.args[0];
		}
	}
};
//start up
std::string Set_Command::key = "";

//	さっき言ってた関数の実装
NotFoundFunc::NotFoundFunc(command_sturct& cs)
{
	if (list.contains(Set_Command::key)) {
		//	ラベルを引数の先頭に追加してから
		cs.args.insert(cs.args.begin(), cs.label);
		cs.label = Set_Command::key;
		//	setされているラベルをrun
		list[Set_Command::key]->run(cs);
	}
}

// 終了させるコマンド
class End_Command : public Command {
	void run(command_sturct& cs) {
		end = true;
	}
};

//	echo
class Echo_Command : public Command {
	void run(command_sturct& cs) {
		std::cout << MyCommand::getLineUppedArgs(cs.args) << std::endl;
	}
};

//	list
class List_Command : public Command {
	void run(command_sturct& cs) {
		for (auto& i : list.getMap())
			std::cout << i.first << std::endl;
	}
};