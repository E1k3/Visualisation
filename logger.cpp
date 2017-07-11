#include "logger.h"

#include <map>

namespace vis
{
	Logger::Logger()
	{

	}

	Logger::~Logger()
	{
		if(_stream)
			*_stream << '\n';
	}

	Logger& Logger::instance()
	{
		static Logger instance{};
		if(*instance._stream)
			*instance._stream << '\n';
		return instance;
	}

	void Logger::set_stream(std::ostream* stream)
	{
		_stream = stream;
	}

	std::string Logger::severity_string(Logger::Severity severity)
	{
		static const std::map<Logger::Severity, std::string> sevMap = {
			{Logger::Severity::ERROR, "ERROR"},
			{Logger::Severity::WARNING, "WARNING"},
			{Logger::Severity::DEBUG, "DEBUG"}};
		return sevMap.at(severity);
	}

	Logger& Logger::operator<<(const Severity& severity)
	{
		_severity = severity;
		return *this;
	}
}
