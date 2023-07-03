#ifndef NOTIFICATION_MESSAGE_H
#define NOTIFICATION_MESSAGE_H

#include "string"

enum class NotificationType {
  INFO = 0,
  SUCCESS = 1,
  WARN = 2,
  ERROR = 3,
};

struct Notification {
  NotificationType type;
  std::string message;

  static Notification info(std::string message) { return Notification{ NotificationType::INFO, message }; };
  static Notification success(std::string message) { return Notification{ NotificationType::SUCCESS, message }; };
  static Notification error(std::string message) { return Notification{ NotificationType::ERROR, message }; };
  static Notification warn(std::string message) { return Notification{ NotificationType::WARN, message }; };
};

#endif
