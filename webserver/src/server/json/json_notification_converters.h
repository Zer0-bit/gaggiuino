#ifndef JSON_NOTIFICATION_CONVERTERS_H
#define JSON_NOTIFICATION_CONVERTERS_H

#include "ArduinoJson.h"
#include "notification_message.h"
#include "string"

namespace json {
  // ------------------------------------------------------------------------------------------
  // ------------------------------------- Serializers ----------------------------------------
  // ------------------------------------------------------------------------------------------
  std::string mapNotificationTypeToJsonValue(const NotificationType& type);
  void mapNotificationToJson(const Notification& notification, JsonObject& taret);
}
#endif
