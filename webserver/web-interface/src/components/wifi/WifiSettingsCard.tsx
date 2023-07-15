import {
  Button,
  Card, CardActions, CardContent, Typography, useTheme,
} from '@mui/material';
import React, { useEffect, useState } from 'react';
import { disconnectFromWifi, getWifiStatus } from '../client/WifiClient';
import Loader from '../loader/Loader';
import WifiStatusDisplay from './WifiStatus';
import AvailableNetworksDrawer from './available-networks/AvailableNetworksDrawer';
import { WifiStatus } from './NetworkTypes';

export default function WifiSettingsCard() {
  const [wifiStatus, setWifiStatus] = useState<WifiStatus | undefined>(undefined);
  const [wifiStatusLoading, setWifiStatusLoading] = useState(true);
  const [wifiDrawerOpen, setWiFiDrawerOpen] = useState(false);
  const theme = useTheme();

  function isConnected() {
    return wifiStatus && wifiStatus.status === 'connected';
  }

  async function loadWiFiStatus() {
    try {
      setWifiStatusLoading(true);
      const status = await getWifiStatus();
      setWifiStatus(status);
    } catch (e) {
      setWifiStatus(undefined);
    } finally {
      setWifiStatusLoading(false);
    }
  }

  async function disconnect() {
    await disconnectFromWifi();
    return loadWiFiStatus();
  }

  useEffect(() => {
    loadWiFiStatus();
  }, []);

  return (
    <div style={{ height: '100%' }}>
      <Card sx={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
        <CardContent sx={{ flex: '1 0 auto' }}>
          <Typography variant="h5" sx={{ mb: theme.spacing(2) }}>
            WiFi Status
          </Typography>
          {wifiStatusLoading ? <Loader /> : <WifiStatusDisplay status={wifiStatus} />}
        </CardContent>
        <CardActions>
          {isConnected() && <Button variant="outlined" size="small" color="secondary" onClick={() => disconnect()}>Disconnect</Button>}
          {isConnected() && <Button variant="outlined" size="small" color="secondary" onClick={() => setWiFiDrawerOpen(true)}>Change</Button>}
          {!isConnected() && <Button variant="outlined" size="small" onClick={() => setWiFiDrawerOpen(true)}>Connect</Button>}
          <Button variant="outlined" size="small" color="secondary" onClick={() => loadWiFiStatus()}>Refresh</Button>
        </CardActions>
      </Card>
      <AvailableNetworksDrawer open={wifiDrawerOpen} onOpenChanged={setWiFiDrawerOpen} onConnected={loadWiFiStatus} />
    </div>
  );
}
