#pragma once

#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <unistd.h>

struct httpRequest;
struct Location;

namespace http {

	enum ValidCgi {
		PY,
	};

	class CgiHandler {

		private:
			// Valid CGI
			static const std::set<std::string> validCgiExtensions;

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

			// Valid CGI
			static bool isValidCgiExtension(const std::string &ext);
			static std::set<std::string> createValidCgiExtensions() {
				std::set<std::string> s;
				s.insert(".py");
				s.insert(".cgi");
				return s;
			}
			static bool isCgiRequest(httpRequest request);

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

} // namespace http