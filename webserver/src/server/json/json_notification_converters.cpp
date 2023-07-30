#include "json_notification_converters.h"

namespace json {
  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  std::string mapNotificationTypeToJsonValue(const NotificationType& type) {
    switch (type) {
    case NotificationType::INFO : return "INFO";
    case NotificationType::WARN: return "WARN";
    case NotificationType::ERROR: return "ERROR";
    case NotificationType::SUCCESS: return "SUCCESS";
    default: return "INFO";
    }
  }

  void mapNotificationToJson(const Notification& notification, JsonObject& target) {
    target["type"] = mapNotificationTypeToJsonValue(notification.type);
    target["message"] = notification.message;
  }
}
