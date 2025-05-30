#pragma once

#include <map>
#include <stdexcept>
#include <string>

namespace HTTP {

	class CgiHandler {

		private:
			// Configuration
			std::string m_scriptPath;
			std::string m_scriptName;
			std::map<std::string, std::string> m_env; // Environment variables

			// Request data
			std::string m_method;
			std::string m_queryString;
			std::string m_requestBody;
			size_t m_contentLength;
			std::string m_contentType;

			// Pipe handling
			int m_inputPipe[2];
			int m_outputPipe[2];
			pid_t m_pid;

			// Private methods
			void setupEnvironment();
			void createPipes();
			void executeCGI();
			void cleanupPipes();
			std::string readCGIOutput();
			void writeCGIInput();

		public:
			// Default Constructor
			CgiHandler(const std::string &scriptPath);
			~CgiHandler();

			// Setters
			void setMethod(const std::string &method);
			void setQueryString(const std::string &queryString);
			void setRequestBody(const std::string &requestBody);
			void setContentType(const std::string &contentType);
			void addEnvironmentVariable(const std::string &key,
										const std::string &value);

			// Main execution method
			std::string execute();

			// Exception class for CGI
			class CgiException : public std::runtime_error {
				public:
					CgiException(const std::string &message)
						: std::runtime_error(message) {
					}
			};
	};

} // namespace HTTP