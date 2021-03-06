#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <chrono>
#include <iomanip>


namespace vis
{
	/**
	 * @brief The Logger class is a singleton that is used to print any information regarding application state.
	 * The output stream can be changed to print into a file instead of cout.
	 */
	class Logger
	{
	public:
		/**
		 * @brief The Severity enum represents the severity of the following messages..
		 */
		enum class Severity
		{
			ERROR,
			WARNING,
			DEBUG
		};

		/**
		 * @brief instance Returns a reference to the single static instance of the Logger.
		 */
		static Logger& instance();

		/**
		 * @brief log Returns a reference to the single static instance of the Logger and sets the severity.
		 */
		static Logger& log(const Severity& severity);
		/**
		 * @brief Returns a reference to the single static instance of the Logger and sets the severity to ERROR.
		 */
		static Logger& error();
		/**
		 * @brief debug Returns a reference to the single static instance of the Logger and sets the severity to DEBUG.
		 */
		static Logger& debug();
		/**
		 * @brief warning Returns a reference to the single static instance of the Logger and sets the severity to WARNING.
		 */
		static Logger& warning();

		/**
		 * @brief setStream Sets the stream (default is std::cout) to which all messages will be sent.
		 */
		void set_stream(std::ostream* stream);

		/**
		 * @brief operator << Sets _severity to @param severity and returns a reference to this logger.
		 */
		Logger& operator<<(const Severity& severity);

		template<typename T>
		/**
		 * @brief operator << prints @param message to _stream.
		 * @return Returns a reference to the stream.
		 */
		std::ostream& operator<<(const T& message)
		{
			auto timestamp = std::chrono::system_clock::now();
			auto timestamp_clock = std::chrono::system_clock::to_time_t(timestamp);
			if(_stream)
				*_stream << std::put_time(std::localtime(&timestamp_clock), "%F %T ")
						 << severity_string(_severity)
						 << " "
						 << message;
			return *_stream;
		}

	private:
		explicit Logger();
		~Logger();

		/**
		 * @brief Log Deleted.
		 */
		explicit Logger(const Logger&) = delete;

		/**
		 * @brief Log Deleted.
		 */
		explicit Logger(Logger&&) = delete;

		/**
		 * @brief operator = Deleted.
		 */
		Logger& operator=(const Logger&) = delete;

		/**
		 * @brief operator = Deleted.
		 */
		Logger& operator=(Logger&&) = delete;

		/**
		 * @brief severityString returns a string representation of @param severity.
		 * Throws std::out_of_range if @param severity does not exist.
		 */
		static std::string severity_string(Severity severity);


		/// Current severity.
		Severity _severity{Severity::DEBUG};
		/// Not owning pointer to output stream.
		/// All messages will be sent to this stream.
		std::ostream* _stream{&std::cout};
	};
}
#endif // LOGGER_H
