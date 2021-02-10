#include "commands.h"

#include <iostream>
/*
	-- 語録 --
	contains -> 含まれているか

	-- 関数 --
	MyCommad::getLineUppedArgs(Args) -> Args (*1) から一行分の文字列作成		*1 Args = vector<string>
	str_lower(std::string) -> 引数の小文字を返す
	commands.h
*/

//----------------------------------------------------------------------------------------------------

//globals
bool end = false;

Commands<NotFoundFunc> list;

sc::time_point<sc::system_clock> past = sc::system_clock::now() - sc::seconds(10);
std::mutex mut;

//----------------------------------------------------------------------------------------------------

int main()
{
	// メタプログラミングみたいなものなので　こんなもんだと思って深い考察しないでw
	// list << $コマンドの名前$ = share(new $作ったコマンド$);
	list << "set" = share(new Set_Command("1.12"));
	// list << "1.15" = share(new Server_Command("java -server -Xmx2G -Xms1G -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "D:/Minecraft/server1.15.2/", "1.15"));
	list << "1.12" = share(new Server_Command("java -server -Xmx2G -Xms1G -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "D:/Minecraft/server/", "1.12"));
	list << "cmd" = share(new Server_Command("cmd", "", "cmd"));
	list << "end" = share(new End_Command);
	list << "echo" = share(new Echo_Command);
	list << "list" = share(new List_Command);

	//mainloop
	while (!end)
	{
		std::string str = "";
		std::getline(std::cin, str);
		if (str == "")
			continue;
		command_sturct cs(str);

		list[cs.label]->run(cs);
	}
}