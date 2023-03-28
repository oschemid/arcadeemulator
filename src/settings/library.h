#pragma once

#include "types.h"
#include <map>
#include <vector>
//#include <emulator.h>
#include "imgui.h"


namespace ae {
	class Game {
	public:
		struct GameVersion
		{
			string _version;
			string _romspath;
			json settings;
		};

		Game();
		Game(const json&);

		const string& title() const { return _title; }
		const string& hardware() const { return _hardware; }
		const string& cover() const { return _cover; }
		std::vector<string> versions() const { std::vector<string> result; std::transform(_versions.begin(), _versions.end(), std::back_inserter(result), [](const std::map<string, GameVersion>::value_type pair) { return pair.first; }); return result; }
		GameVersion* currentVersion() const { return _currentversion; }
		void setCurrentVersion(const string& v) { auto c = _versions.find(v); _currentversion = &(c->second); }

	protected:
		string _title;
		string _console;
		string _hardware;
		json _settings;
		string _cover;
		std::map<string, GameVersion> _versions;
		GameVersion* _currentversion{ nullptr };
		ImTextureID _coverId;


		const json& settings() const { return _settings; }
	};
	class Library {
	protected:
		std::map<const string, std::vector<Game>> _library;

	public:
		Library() = default;

		void load(const string&);
		std::vector<Game>& games(const string&);
	};
}