#pragma once

#include "types.h"
#include <map>
#include <vector>
#include "imgui.h"


namespace ae {
	class Game {
	protected:
		string _title;
		string _console;
		json _settings;
		string _cover;
		ImTextureID _coverId;

	public:
		Game();
		Game(const json&);

		const string& title() const { return _title; }
		const json& settings() const { return _settings; }
		const string& cover() const { return _cover; }
		ImTextureID coverId() const { return _coverId; }
		void setCoverId(const ImTextureID& id) { _coverId = id; }
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