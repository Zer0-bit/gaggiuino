export const MSG_TYPE_SHOT_DATA = 'shot_data_update';
export const MSG_TYPE_SENSOR_DATA = 'sensor_data_update';

export function filterSocketMessage(message, ...types) {
  if (!message || !message.data) {
    return false;
  }
  return types.some((type) => message.data.indexOf(type) >= 0);
}

export function filterJsonMessage(message, ...types) {
  if (!message || !message.action) {
    return false;
  }
  return types.some((type) => message.action === type);
}
