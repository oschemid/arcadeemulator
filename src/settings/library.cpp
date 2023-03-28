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
	if (data.at("console").is_object()) {
		_console = data.at("console").at("name");
		_hardware = data.at("console").at("hardware");
	}
	else {
		_console = data.at("console");
		_hardware = "";
	}
	if (data.contains("cover")) {
		data.at("cover").get_to(_cover);
	}
	if (data.contains("versions")) {
		auto versions = data.at("versions");
		for (auto version : versions) {
			if (version.contains("settings"))
				_versions[version.at("version")] = GameVersion{ ._version = version.at("version"), ._romspath = version.at("roms"), .settings = version.at("settings")};
			else
				_versions[version.at("version")] = GameVersion{ ._version = version.at("version"), ._romspath = version.at("roms"), .settings = json()};
		}
	}

	_coverId = 0;
}

void Library::load(const string& filename) {
	std::ifstream f(filename);
	json data = json::parse(f);

	for (auto e : data) {
		string console;
		if (e.at("console").is_object()) {
			console = e.at("console").at("name");
		}
		else {
			console = e.at("console");
		}
		if (_library.find(console) == _library.end()) {
			_library.insert(std::pair<string&, std::vector<Game>>(console, std::vector<Game>()));
		}
		_library[console].push_back(Game(e));
	}
}

std::vector<Game>& Library::games(const string& console) {
	return _library[console];
}
