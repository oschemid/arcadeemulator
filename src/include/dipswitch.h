#pragma once

#include "types.h"
#include <map>
#include "machine.h"

using ae::uint8_t;
using ae::string;


namespace ae
{
	namespace machine
	{
		template <int N> class DIPSwitch : public ae::IParameter {
		protected:
			string _name;
			uint8_t _value;
			std::map<const uint8_t, const string> _aliases;

		public:
			DIPSwitch(const string name) :
				_name(name),
				_value(0) {}
			~DIPSwitch() = default;
			const string getName() const override { return _name; }
			const uint8_t getValue() const override { return _value; }
			const uint8_t getMaxValue() const override { return (1 << N) - 1; }
			const string getAlias() const override {
				std::map<const uint8_t, const string>::const_iterator pos = _aliases.find(_value);
				if (pos == _aliases.end()) {
					return "";
				}
				else {
					return pos->second;
				}
			}
			bool setValue(const uint8_t v) override { if (v < 1 << N) _value = v; return _value == v; }

			bool addAlias(const uint8_t v, const string a) {
				if (v >= 1 << N)
					return false;
				_aliases.insert(std::pair<const uint8_t, const string>(v, a));
				return true;
			}
		};
	}
}