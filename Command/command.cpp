#include "command.h"

#include <sstream>

std::string MyCommand::getLineUppedArgs(MyCommand::Args args)
{
	using namespace std;
	stringstream ss;
	string s;
	for (auto& i : args)
		ss << i << " ";
	getline(ss, s);
	return s;
}

MyCommand::command_sturct MyCommand::to_command_struct(std::string str) {
	using namespace std;
	stringstream stream(str);
	MyCommand::command_sturct ret_struct;
	stream >> ret_struct.label;
	string s;

	while (getline(stream, s, ' '))
	{
		ret_struct.args.push_back(s);
	}
	if (ret_struct.args.size() != 0)
		ret_struct.args.erase(ret_struct.args.begin());
	return ret_struct;
}

MyCommand::command_sturct::command_sturct(std::string str)
{
	*this = MyCommand::to_command_struct(str);
}