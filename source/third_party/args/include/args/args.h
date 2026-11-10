// args.h - minimal command line argument parsing
//
// Written for the buz engine, but engine-agnostic on purpose: this header
// lives in third_party because the string parsing below is implementation
// detail nobody should have to read to understand the engine or a game. Use
// it from main() to override buz::engine_settings, e.g.:
//
//   int main(int argc, char** argv)
//   {
//       args::parser arguments(argc, argv);
//       settings.window_width = arguments.get("window_width", settings.window_width);
//       ...
//   }
//
// Every argument has the form -name=value (a leading -- works too). A bare
// -name with no "=" is stored as the value "true", so it doubles as a flag.
// Settings that are not on the command line keep whatever fallback you pass,
// which is how "use whatever we set up in code" falls out for free.

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace args
{
	class parser
	{
	public:
		//--------------------------------------------------------------
		// Parses argv into a name -> value table. argv[0] is the exe path
		// and is skipped.
		parser(int argc, char** argv)
		{
			for (int index = 1; index < argc; ++index)
			{
				std::string token = argv[index];

				// Strip a leading "-" or "--" so "-name" and "--name" both work.
				size_t start = token.find_first_not_of('-');
				if (start == std::string::npos)
				{
					continue; // a lone "-" or "--", nothing to name
				}
				token.erase(0, start);

				// Split on the first "=" into name and value. A token without
				// "=" is a flag; store it as "true".
				const size_t equals = token.find('=');
				if (equals == std::string::npos)
				{
					m_values.emplace(token, "true");
				}
				else
				{
					m_values.emplace(token.substr(0, equals), token.substr(equals + 1));
				}
			}
		}

		//--------------------------------------------------------------
		// True when the given setting was passed on the command line.
		bool has(std::string_view name) const
		{
			return m_values.find(std::string(name)) != m_values.end();
		}

		//--------------------------------------------------------------
		// Returns the value of the setting converted to the type of the
		// fallback, or the fallback when the setting is absent or cannot be
		// converted. The fallback's type decides how the text is read:
		// numbers, bool ("true"/"1"), std::string and std::wstring are all
		// supported.
		template<typename T>
		T get(std::string_view name, T fallback) const
		{
			const std::string* text = find(name);
			if (text == nullptr)
			{
				return fallback;
			}

			try
			{
				return convert<T>(*text);
			}
			catch (const std::exception&)
			{
				mark_invalid(name);
				return fallback;
			}
		}

		//--------------------------------------------------------------
		// Settings that were passed on the command line but never read back
		// (typos like "window_wdith") or that failed to parse. Log these so
		// the mistake is not silently ignored.
		std::vector<std::string> unused_keys() const
		{
			std::vector<std::string> result;
			for (const auto& entry : m_values)
			{
				if (m_queried.find(entry.first) == m_queried.end() || m_invalid.find(entry.first) != m_invalid.end())
				{
					result.push_back(entry.first);
				}
			}
			return result;
		}

	private:
		//--------------------------------------------------------------
		// Finds a setting's raw text and records that it was asked for, or
		// returns nullptr when it is absent.
		const std::string* find(std::string_view name) const
		{
			const std::string key(name);
			m_queried.insert(key);

			const auto entry = m_values.find(key);
			return entry != m_values.end() ? &entry->second : nullptr;
		}

		//--------------------------------------------------------------
		void mark_invalid(std::string_view name) const
		{
			m_invalid.insert(std::string(name));
		}

		//--------------------------------------------------------------
		// Converts raw text to the requested type, picking the right
		// std::sto* / conversion based on the fallback's type. Throws when the
		// text is not a valid number; get() catches that and uses the fallback.
		template<typename T>
		static T convert(const std::string& text)
		{
			if constexpr (std::is_same_v<T, std::string>)
			{
				return text;
			}
			else if constexpr (std::is_same_v<T, std::wstring>)
			{
				return std::wstring(text.begin(), text.end()); // ascii/utf-8 widen, enough for a title
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				return text == "true" || text == "1";
			}
			else if constexpr (std::is_floating_point_v<T>)
			{
				return static_cast<T>(std::stod(text));
			}
			else if constexpr (std::is_unsigned_v<T>)
			{
				return static_cast<T>(std::stoull(text));
			}
			else
			{
				return static_cast<T>(std::stoll(text));
			}
		}

		std::unordered_map<std::string, std::string> m_values;

		// Bookkeeping for unused_keys(); mutable because these are filled by the
		// const getters as a side effect of reading a setting.
		mutable std::unordered_set<std::string> m_queried;
		mutable std::unordered_set<std::string> m_invalid;
	};
}
