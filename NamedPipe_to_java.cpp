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
	list << "set" = share(new Set_Command("pvp"));
	list << "survival" = share(new Server_Command("java -server -Xmx4G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "C:/Ahaha_world/world", "survival"));
	list << "flat" = share(new Server_Command("java -server -Xmx4G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "C:/Ahaha_world/flat", "flat"));
	list << "skyblock" = share(new Server_Command("java -server -Xmx8G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "C:/Ahaha_world/skyblock", "skyblock"));
	list << "hub" = share(new Server_Command("java -server -Xmx1G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "C:/Ahaha_world/Hub", "hub"));
	list << "pvp" = share(new Server_Command("java -server -Xmx4G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "C:/Ahaha_world/pvp", "pvp"));
	list << "pvp_snow" = share(new Server_Command("java -server -Xmx2G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "C:/Ahaha_world/pvp/SnowBattle", "pvp_snow"));
	list << "pvp_murder" = share(new Server_Command("java -server -Xmx2G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar spigot.jar nogui", "C:/Ahaha_world/pvp/murder", "pvp_murder"));
	list << "server" = share(new Server_Command("java -server -Xmx1G -Xms1M -XX:PermSize=256M -XX:MaxPermSize=256M -XX:+UseConcMarkSweepGC -XX:+UseParNewGC -XX:+CMSIncrementalPacing -XX:ParallelGCThreads=3 -XX:+AggressiveOpts -jar BungeeCord.jar nogui", "C:/Ahaha_world/server", "server"));
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