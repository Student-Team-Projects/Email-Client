#include <nlohmann/json.hpp>

#include "backend/mailbox.h"

int main() {
	std::ifstream configFile("config.json");
	if (!configFile)
		std::cerr << "Failed to open config.json\n";
	nlohmann::json config;
	configFile >> config;

	std::string senderEmail = config["sender_email"];
	std::string appPassword = config["app_password"];

	std::ifstream recipientsFile("recipients.txt");
	if (!recipientsFile)
		std::cerr << "Failed to open recipients.txt\n";

	std::vector<std::string> recipients;
	std::string recipient;
	while (std::getline(recipientsFile, recipient))
		recipients.push_back(recipient);

	std::string subject = "Test email";
	std::string body = "This is a test email";

	Mailbox mailbox(senderEmail, appPassword);
	Message message(recipients, subject, body);
	mailbox.send(message);
}
