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
export const defaultShotData = {
  timeInShot: 0,
  temperature: 0,
  pressure: 0,
  pumpFlow: 0,
  shotWeight: 0,
  targetTemperature: 0,
  targetPumpFlow: 0,
  targetPressure: 0,
};

export const apiHost = (process.env.NODE_ENV === 'development')
  ? '192.168.0.14'
  : window.location.host;

export function formatTimeInShot(timeInShot) {
  const milliseconds = timeInShot % 1000;
  const seconds = Math.floor(timeInShot / 1000) % 60;
  const minutes = Math.floor(timeInShot / 60000);
  return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}.${milliseconds.toString().padStart(3, '0')}`;
}
