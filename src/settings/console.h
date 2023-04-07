#pragma once

#include "types.h"
#include <map>
#include <vector>


namespace ae {
	class Console {
	protected:
		string _id;
		string _name;

	public:
		Console();
		Console(const json&);

		const string& id() const { return _id; }
		const string& name() const { return _name; }
	};

	class Consoles {
	protected:
		std::map<string, Console> _consoles;

	public:
		typedef std::map<string, Console>::iterator iterator;
		Consoles() = default;

		void load(const string&);
		iterator begin() { return _consoles.begin(); }
		iterator end() { return _consoles.end(); }
	};
}