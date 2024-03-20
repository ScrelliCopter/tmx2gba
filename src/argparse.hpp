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
		bool required;
		const char* argumentName;
		const char* helpString;

		static constexpr Option Optional(char flag, const char* name, const char* help)
		{
			return { flag, false, name, help };
		}
		static constexpr Option Required(char flag, const char* name, const char* help)
		{
			return { flag, true, name, help };
		}
	};

	struct Options
	{
		const std::vector<Option> options;

		inline Options(const std::initializer_list<Option>&& rhs) : options(rhs) {}

		void ShowShortUsage(const std::string_view name, std::ostream& out) const;
		void ShowHelpUsage(const std::string_view name, std::ostream& out) const;
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
		const Options& options;

	public:
		ParserState(HandleOption handler, const Options& options) noexcept
			: handler(handler), options(options) {}
		[[nodiscard]] bool ExpectingArg() const { return expectArg; }
		[[nodiscard]] ParseCtrl Next(const std::string_view token);
	};

	class ArgParser
	{
		const std::string name;
		Options options;
		HandleOption handler;

		[[nodiscard]] bool CheckParse(ArgParse::ParseErr err) const;

	public:
		explicit ArgParser(const std::string_view argv0, Options options, HandleOption&& handler) noexcept;

		[[nodiscard]] const std::string_view GetName() const noexcept { return name; }
		void DisplayError(const std::string_view message, bool helpPrompt = true) const;

		template <typename V>
		[[nodiscard]] bool Parse(V args)
		{
			ParserState state(handler, options);
			for (auto arg : args)
			{
				ParseErr err = ParseErr::UNEXPECTED;
				switch (state.Next(arg))
				{
				case ParseCtrl::CONTINUE:            continue;
				case ParseCtrl::QUIT_EARLY:          err = ParseErr::OK; break;
				case ParseCtrl::QUIT_ERR_UNKNOWN:    err = ParseErr::OPT_UNKNOWN; break;
				case ParseCtrl::QUIT_ERR_UNEXPECTED: err = ParseErr::UNEXPECTED; break;
				case ParseCtrl::QUIT_ERR_EXPECTARG:  err = ParseErr::ARG_EXPECTED; break;
				case ParseCtrl::QUIT_ERR_INVALID:    err = ParseErr::ARG_INVALID; break;
				case ParseCtrl::QUIT_ERR_RANGE:      err = ParseErr::ARG_RANGE; break;
				}
				if (!CheckParse(err))
					return false;
			}
			return CheckParse(state.ExpectingArg() ? ParseErr::ARG_EXPECTED : ParseErr::OK);
		}

		[[nodiscard]] inline bool Parse(std::initializer_list<std::string_view> args)
		{
			return Parse<std::initializer_list<std::string_view>>(args);
		}

		[[nodiscard]] inline bool Parse(std::span<char*> args)
		{
			return Parse(args | std::views::transform([](char const* v){ return std::string_view(v); }));
		}
	};

	[[nodiscard]] extern bool ReadParamFile(std::vector<std::string>& tokens, std::istream& file);
}

#endif//ARGPARSE_HPP
