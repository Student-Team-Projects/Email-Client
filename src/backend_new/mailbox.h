#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

struct MessageId {
  std::string uid, folder;
};
struct MessageMeta {
  std::string from, to, subject;
};
struct InboundMessageMeta : MessageId, MessageMeta {
  bool read; 
};
struct Message {
  MessageMeta meta;
  std::string data;
};
struct MailFolder {
  std::string name;
  std::vector<InboundMessageMeta> messages;
};
struct MailReceiver {
  virtual void sync_mail() = 0;
  virtual std::vector<MailFolder> list_mail() = 0;
  virtual Message fetch_mail(const MessageId& id) = 0;
};
struct MailSender {
  virtual void send_mail(const Message& msg) = 0;
};

struct MailAccount {
  template<typename Sender, typename Receiver,
    typename = std::enable_if_t<std::is_base_of_v<MailSender, Sender>>,
    typename = std::enable_if_t<std::is_base_of_v<MailReceiver, Receiver>>>
  MailAccount(std::shared_ptr<Sender> sender,
             std::shared_ptr<Receiver> receiver)
    : sender(std::static_pointer_cast(sender))
    , receiver(std::static_pointer_cast(receiver)) {}
  std::shared_ptr<MailSender> sender;
  std::shared_ptr<MailReceiver> receiver;
};

