/* argparse.cpp - Copyright (C) 2024 a dinosaur (zlib, see COPYING.txt) */

#include "argparse.hpp"
#include <iomanip>
#include <filesystem>
#include <optional>
#include <iostream>


ArgParse::ArgParser::ArgParser(
	const std::string_view argv0,
	Options options,
	HandleOption&& handler
) noexcept :
	name(std::filesystem::path(argv0).filename().string()),
	options(options),
	handler(std::forward<HandleOption>(handler)) {}


void ArgParse::Options::ShowShortUsage(const std::string_view name, std::ostream& out) const
{
	out << "Usage: " << name;
	for (const auto& it : options)
	{
		if (it.argumentName)
		{
			// Option with argument
			it.required
				? out << " <-" << it.flag << ' ' << it.argumentName << '>'
				: out << " [-" << it.flag << ' ' << it.argumentName << ']';
		}
		else
		{
			// Argument-less flag
			it.required
				? out << " <-" << it.flag << '>'
				: out << " [-" << it.flag << ']';
		}
	}
	out << std::endl;
}

void ArgParse::Options::ShowHelpUsage(const std::string_view name, std::ostream& out) const
{
	// Base usage
	out << "Usage: " << name << " [-";
	for (const auto& it : options)
		if (!it.required)
			out << it.flag;
	out << "] <-";
	for (const auto& it : options)
		if (it.required)
			out << it.flag;
	out << ">" << std::endl;

	// Determine the alignment width from the longest argument
	auto paramLength = [](const Option& p) -> int { return p.argumentName
		? static_cast<int>(std::strlen(p.argumentName) + 3)
		: 1; };
	auto longestParam = std::max_element(options.begin(), options.end(),
		[=](auto a, auto b) -> bool { return paramLength(a) < paramLength(b); });
	auto alignWidth = paramLength(*longestParam) + 3;

	// print argument descriptions
	for (const auto& it : options)
	{
		auto decorateArgument = [=] { return " <" + std::string(it.argumentName) + "> "; };
		out << "  -" << it.flag
			<< std::left << std::setw(alignWidth) << std::setfill('-') << (it.argumentName ? decorateArgument() : " ")
			<< " " << it.helpString << std::endl;
	}
	out << std::flush;
}


ArgParse::ParseCtrl ArgParse::ParserState::Next(const std::string_view token)
{
	auto getFlag = [](const std::string_view s) { return s[0] == '-' && s[1] ? std::optional<int>(s[1]) : std::nullopt; };
	auto getOption = [&](int flag) -> std::optional<std::reference_wrapper<const Option>>
	{
		for (auto& opt : options.options)
			if (opt.flag == flag)
				return std::optional(std::cref(opt));
		return {};
	};

	if (expectArg)
	{
		expectArg = false;
		return handler(flagChar, token);
	}
	else
	{
		auto flag = getFlag(token);
		if (flag.has_value())
		{
			flagChar = flag.value();
			const auto opt = getOption(flagChar);
			if (opt.has_value())
			{
				bool expect = opt.value().get().argumentName != nullptr;
				if (token.length() <= 2)
				{
					expectArg = expect;
					if (!expectArg)
						return handler(flagChar, "");
				}
				else
				{
					return handler(flagChar, expect ? token.substr(2) : "");
				}
			}
		}
		else if (!token.empty())
		{
			return ParseCtrl::QUIT_ERR_UNEXPECTED;
		}
	}

	return ParseCtrl::CONTINUE;
}


void ArgParse::ArgParser::DisplayError(const std::string_view message, bool helpPrompt) const
{
	std::cerr << GetName() << ": " << message << std::endl;
	options.ShowShortUsage(GetName(), std::cerr);
	if (helpPrompt)
		std::cerr << "Run '" << GetName() << " -h' to view all available options." << std::endl;
}

bool ArgParse::ArgParser::CheckParse(ArgParse::ParseErr err) const
{
	switch (err)
	{
	case ParseErr::OK:
		return true;
	case ParseErr::OPT_UNKNOWN:
		DisplayError("Unrecognised option.");
		return false;
	case ParseErr::UNEXPECTED:
		DisplayError("Unexpected token.");
		return false;
	case ParseErr::ARG_EXPECTED:
		DisplayError("Requires an argument.");
		return false;
	case ParseErr::ARG_INVALID:
		DisplayError("Invalid argument.", false);
		return false;
	case ParseErr::ARG_RANGE:
		DisplayError("Argument out of range.", false);
		return false;
	}
}


bool ArgParse::ReadParamFile(std::vector<std::string>& tokens, std::istream& file)
{
	bool inQuote = false;
	std::string quoteStr;
	const auto store = [&](const std::string_view token, bool quote)
	{
		if (quote)
			quoteStr = token;
		else
			tokens.emplace_back(token);
	};

	while (!file.eof())
	{
		if (!inQuote)
		{
			std::string token;
			file >> token;
			if (!token.empty())
			{
				std::string::size_type beg = 0, end;
				while ((end = token.find_first_of('"', beg)) != std::string::npos)
				{
					auto size = end - beg;
					if (size > 0)
						store(token.substr(beg, size), !inQuote);
					inQuote = !inQuote;
					beg = end + 1;
				}
				if (beg > 0)
				{
					auto size = token.length() - beg;
					if (size > 0)
						token = token.substr(beg);
					else
						token.clear();
				}
				if (!token.empty())
					store(token, inQuote);
			}
		}
		else
		{
			const int c = file.get();
			if (c == '"')
			{
				tokens.emplace_back(quoteStr);
				quoteStr.clear();
				inQuote = false;
			}
			else
			{
				quoteStr.push_back(c);
			}
		}
	}

	return !inQuote;
}
