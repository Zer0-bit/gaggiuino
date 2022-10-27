import React from 'react';
import SignalWifi3BarIcon from '@mui/icons-material/SignalWifi3Bar';
import SignalWifiOffIcon from '@mui/icons-material/SignalWifiOff';
import { Button } from '@mui/material';
import PropTypes from 'prop-types';
import { WifiStatusPropType } from './NetworkPropTypes';
import { disconnectFromWifi } from '../client/WifiClient';

export default function WifiStatus({ status, onDisconnected }) {
  function isConnected() {
    return status.status === 'connected';
  }

  async function disconnect() {
    await disconnectFromWifi();
    onDisconnected();
  }

  function wifiActions() {
    return (
      <div className="WiFiActions">
        {isConnected() && <Button variant="outlined" onClick={() => disconnect()}>Disconnect</Button>}
      </div>
    );
  }

  return (
    <div className="Wifi">
      {!isConnected() && (
      <div className="Inline-Icon">
        <SignalWifiOffIcon />
        Not connected
      </div>
      )}
      {isConnected() && (
      <div className="Inline-Icon">
        <SignalWifi3BarIcon />
        {`${status.ssid} (${status.ip})`}
      </div>
      )}
      {wifiActions()}
    </div>
  );
}

WifiStatus.propTypes = {
  status: WifiStatusPropType.isRequired,
  onDisconnected: PropTypes.func,
};

WifiStatus.defaultProps = {
  onDisconnected: () => {},
};
