import PropTypes from 'prop-types';

export const WifiStatusPropType = PropTypes.shape({
  status: PropTypes.oneOf(['connected', 'disconnected']).isRequired,
  ssid: PropTypes.string.isRequired,
  ip: PropTypes.string.isRequired,
});

export const Network = PropTypes.shape({
  ssid: PropTypes.string.isRequired,
  rssi: PropTypes.number.isRequired,
  secured: PropTypes.bool.isRequired,
});

export const NetworksList = PropTypes.arrayOf(Network);
