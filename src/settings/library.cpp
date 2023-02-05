#include "library.h"
#include <fstream>



using namespace ae;

Game::Game() :
	_title(""),
	_console(""),
	_cover(""),
	_coverId(0)
{}

Game::Game(const json& data) : _settings(data) {
	data.at("title").get_to(_title);
	data.at("console").get_to(_console);
	if (data.contains("cover")) {
		data.at("cover").get_to(_cover);
	}
	_coverId = 0;
}

void Library::load(const string& filename) {
	std::ifstream f(filename);
	json data = json::parse(f);

	for (auto e : data) {
		string console = e.at("console");
		if (_library.find(console) == _library.end()) {
			_library.insert(std::pair<string&, std::vector<Game>>(console, std::vector<Game>()));
		}
		_library[console].push_back(Game(e));
	}
}

std::vector<Game>& Library::games(const string& console) {
	return _library[console];
}
