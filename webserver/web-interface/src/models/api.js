export const apiHost = (process.env.NODE_ENV === 'development')
  ? '192.168.0.21'
  : window.location.host;

export function formatTimeInShot(timeInShot) {
  const milliseconds = timeInShot % 1000;
  const seconds = Math.floor(timeInShot / 1000) % 60;
  const minutes = Math.floor(timeInShot / 60000);
  return `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}.${milliseconds.toString().padStart(3, '0')}`;
}
