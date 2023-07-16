import React from 'react';
import SignalWifi3BarIcon from '@mui/icons-material/SignalWifi3Bar';
import SignalWifiOffIcon from '@mui/icons-material/SignalWifiOff';
import { Stack, Typography } from '@mui/material';
import { WifiStatus } from './NetworkTypes';

type WifiStatusProps = {
  status?: WifiStatus
}

export default function WifiStatusDisplay({ status = undefined }: WifiStatusProps) {
  function isConnected() {
    return status && status.status === 'connected';
  }

  return isConnected()
    ? (
      <Stack spacing={1} direction="row" alignItems="center">
        <SignalWifi3BarIcon color="success" />
        <Typography>{`${status?.ssid} (${status?.ip})`}</Typography>
      </Stack>
    )
    : (
      <Stack spacing={1} direction="row" alignItems="center">
        <SignalWifiOffIcon />
        <Typography>Not connected</Typography>
      </Stack>
    );
}
