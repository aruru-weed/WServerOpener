#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <memory>

namespace MyCommand {

	using Args = std::vector<std::string>;

	struct command_sturct {
		command_sturct() {};
		command_sturct(std::string str);

		std::string label;
		Args args;
	};

	std::string getLineUppedArgs(Args args);

	command_sturct to_command_struct(std::string);

	class Command {
	public:
		virtual void run(command_sturct&) = 0;
		virtual ~Command() {};
	};

	using share = std::shared_ptr<Command>;

	template<typename FunctionObject>
	class Commands {
		std::map<std::string, std::shared_ptr<Command>> commands;
		class NotFoundCommand : public Command {
		public:
			void run(MyCommand::command_sturct& cs);
		};
	public:
		std::shared_ptr<Command>& operator<<(std::string Name);

		std::shared_ptr<Command> operator[](std::string Name);

		std::map<std::string, std::shared_ptr<Command>>& getMap();

		void UnRegister(std::string Name);

		bool contains(std::string Name);
	};

	template<typename FunctionObject>
	inline std::shared_ptr<Command>& Commands<FunctionObject>::operator<<(std::string Name) {
		return commands[Name];
	}

	template<typename FunctionObject>
	inline std::shared_ptr<Command> Commands<FunctionObject>::operator[](std::string Name) {
		std::shared_ptr<NotFoundCommand> nfc(new NotFoundCommand());
		return (commands.count(Name) == 0 ? nfc : commands[Name]);
	}

	template<typename FunctionObject>
	inline std::map<std::string, std::shared_ptr<Command>>& Commands<FunctionObject>::getMap()
	{
		return commands;
	}

	template<typename FunctionObject>
	inline void Commands<FunctionObject>::UnRegister(std::string Name) {
		commands.erase(Name);
	}

	template<typename FunctionObject>
	inline bool Commands<FunctionObject>::contains(std::string Name)
	{
		return (commands.count(Name) == 1 ? true : false);
	}

	template<typename FunctionObject>
	inline void Commands<FunctionObject>::NotFoundCommand::run(MyCommand::command_sturct& cs) {
		FunctionObject obj(cs);
	}
}