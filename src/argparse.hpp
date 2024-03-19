/* argparse.hpp - Copyright (C) 2024 a dinosaur (zlib, see COPYING.txt) */

#ifndef ARGPARSE_HPP
#define ARGPARSE_HPP

#include <initializer_list>
#include <functional>
#include <string>
#include <ostream>
#include <span>
#include <ranges>
#include <string_view>
#include <vector>

namespace ArgParse
{
	struct Option
	{
		char flag;
		const char* argumentName;
		bool required;
		const char* helpString;
	};

	enum class ParseCtrl
	{
		CONTINUE,
		QUIT_EARLY,
		QUIT_ERR_UNKNOWN,
		QUIT_ERR_UNEXPECTED,
		QUIT_ERR_EXPECTARG,
		QUIT_ERR_INVALID,
		QUIT_ERR_RANGE
	};

	enum class ParseErr
	{
		OK,
		OPT_UNKNOWN,
		UNEXPECTED,
		ARG_EXPECTED,
		ARG_INVALID,
		ARG_RANGE
	};

	using HandleOption = std::function<ParseCtrl(int, const std::string_view)>;

	class ParserState
	{
		bool expectArg = false;
		int flagChar;
		HandleOption handler;
		const std::initializer_list<Option>& options;

	public:
		ParserState(HandleOption handler, const std::initializer_list<Option>& options) noexcept
			: handler(handler), options(options) {}
		[[nodiscard]] bool ExpectingArg() const { return expectArg; }
		[[nodiscard]] ParseCtrl Next(const std::string_view token);
	};

	class ArgParser
	{
		const std::string name;
		std::initializer_list<Option> options;
		HandleOption handler;

	public:
		explicit ArgParser(const std::string_view argv0, std::initializer_list<Option> options, HandleOption&& handler) noexcept;

		[[nodiscard]] const std::string_view GetName() const { return name; }

		void ShowShortUsage(std::ostream& out) const;
		void ShowHelpUsage(std::ostream& out) const;

		template <typename V>
		ParseErr Parse(V args)
		{
			ParserState state(handler, options);
			for (auto arg : args)
			{
				switch (state.Next(arg))
				{
				case ParseCtrl::CONTINUE:            continue;
				case ParseCtrl::QUIT_EARLY:          return ParseErr::OK;
				case ParseCtrl::QUIT_ERR_UNKNOWN:    return ParseErr::OPT_UNKNOWN;
				case ParseCtrl::QUIT_ERR_UNEXPECTED: return ParseErr::UNEXPECTED;
				case ParseCtrl::QUIT_ERR_EXPECTARG:  return ParseErr::ARG_EXPECTED;
				case ParseCtrl::QUIT_ERR_INVALID:    return ParseErr::ARG_INVALID;
				case ParseCtrl::QUIT_ERR_RANGE:      return ParseErr::ARG_RANGE;
				}
			}
			return state.ExpectingArg() ? ParseErr::ARG_EXPECTED : ParseErr::OK;
		}

		inline ParseErr Parse(std::initializer_list<std::string_view> args)
		{
			return Parse<std::initializer_list<std::string_view>>(args);
		}

		inline ParseErr Parse(std::span<char*> args)
		{
			return Parse(args | std::views::transform([](char const* v){ return std::string_view(v); }));
		}
	};
}

extern bool ReadParamFile(std::vector<std::string>& tokens, std::istream& file);

#endif//ARGPARSE_HPP
