#include <iostream>

#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>

#include <vmime/vmime.hpp>

#include <fstream>
#include <cstdlib>
#include <nlohmann/json.hpp>  // JSON library header

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Button, Horizontal, Renderer
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"

using namespace ftxui;

class certificateVerifier : public vmime::security::cert::defaultCertificateVerifier {
public:
    void verify(const vmime::shared_ptr<vmime::security::cert::certificateChain>& chain,
                const vmime::string& hostname) {
        try {
            setX509TrustedCerts(m_trustedCerts);
            defaultCertificateVerifier::verify(chain, hostname);
        } catch (vmime::security::cert::certificateException&) {
            vmime::shared_ptr<vmime::security::cert::certificate> cert = chain->getAt(0);
            if (cert->getType() == "X.509") {
                m_trustedCerts.push_back(vmime::dynamicCast<vmime::security::cert::X509Certificate>(cert));
                setX509TrustedCerts(m_trustedCerts);
                defaultCertificateVerifier::verify(chain, hostname);
            }
        }
    }

    void loadRootCertificates(const std::string& path) {
        std::ifstream certFile(path, std::ios::binary);
        if (!certFile) throw std::runtime_error("Failed to open certificate file");
        vmime::utility::inputStreamAdapter is(certFile);
        vmime::security::cert::X509Certificate::import(is, m_trustedCerts);
    }

private:
    std::vector<vmime::shared_ptr<vmime::security::cert::X509Certificate>> m_trustedCerts;
};


ButtonOption Style() {
  auto option = ButtonOption::Animated();
  option.transform = [](const EntryState& s) {
    auto element = text(s.label);
    if (s.focused) {
      element |= bold;
    }
    return element | center | borderEmpty | flex;
  };
  return option;
}
 
enum class email_state{
    WELCOME,
    NEW_EMAIL,
    INBOX,
    SENT
};

std::string to_string(email_state state){
    switch(state){
        case email_state::WELCOME:      return "WELCOME!";
        case email_state::NEW_EMAIL:    return "Write the email here.";
        case email_state::INBOX:        return "Here are emails sent to you.";
        case email_state::SENT:         return "Here are emails sent by you.";
    }
    return "UNREACHABLE!";
}

int main(){
    email_state state = email_state::WELCOME;

    auto bottom_buttons = Container::Horizontal({
        Button("New email", [&](){state = email_state::NEW_EMAIL;}),
        Button("Inbox", [&](){state = email_state::INBOX;}),
        Button("Sent", [&](){state = email_state::SENT;})
    });
    auto main_component = Renderer(bottom_buttons, [&](){
        return vbox({
            text(to_string(state)) | flex,
            separator(),
            bottom_buttons->Render()
        });
    });
    auto screen = ScreenInteractive::Fullscreen();
    screen.Loop(main_component);
}