#include "console.h"

using namespace ae;

Console::Console():
	_id(""),
	_name("")
{}

Console::Console(const json& data) {
	data.at("id").get_to(_id);
	data.at("name").get_to(_name);
}

void Consoles::load(const string& filename) {
	std::ifstream f(filename);
	json data = json::parse(f);

	for (auto& e : data) {
		string id = e.at("id");
		_consoles.insert(std::pair<string&, Console>(id, Console(e)));
	}
}