export const apiHost = (process.env.NODE_ENV === 'development')
  ? '192.168.0.21'
  : window.location.host;

export function formatTime({
  time,
  includeMillis = false,
  truncateLeadingZeroUnits = true,
}: {
  time: number;
    includeMillis?: boolean;
    truncateLeadingZeroUnits?: boolean;
  }) {
  const hours = Math.floor(time / 3600000);
  const minutes = Math.floor((time - hours * 3600000) / 60000);
  const seconds = Math.floor((time - hours * 3600000 - minutes * 60000) / 1000);
  const milliseconds = time % 1000;

  // Format hours, minutes, and seconds as 2 digits
  let hourString = (`0${hours}`).slice(-2);
  const minuteString = (`0${minutes}`).slice(-2);
  const secondString = (`0${seconds}`).slice(-2);

  // Format milliseconds as 3 digits
  const millisecondString = (`00${milliseconds}`).slice(-3);

  // If truncateLeadingZeroUnits, remove the zero hours/minutes/seconds
  if (truncateLeadingZeroUnits) {
    if (hours === 0) {
      hourString = '';
    }
  }

  // Construct the final string
  const formattedTime = `${hourString ? `${hourString}:` : ''}`
      + `${minuteString ? `${minuteString}:` : ''}`
      + `${secondString || '00'}`
      + `${includeMillis ? `:${millisecondString}` : ''}`;

  return formattedTime;
}
