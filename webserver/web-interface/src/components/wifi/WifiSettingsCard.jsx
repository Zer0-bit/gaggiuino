import React, { useState, useEffect } from 'react';
import {
  Button,
  Card, CardActions, CardContent, Drawer, Typography, useTheme,
} from '@mui/material';
import WifiStatus from './WifiStatus';
import { getWifiStatus, disconnectFromWifi } from '../client/WifiClient';
import AvailableNetworks from './available-networks/AvailableNetworks';
import Loader from '../loader/Loader';

export default function WifiSettingsCard() {
  const [wifiStatus, setWifiStatus] = useState(null);
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
      setWifiStatus(null);
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

  const toggleDrawer = (open) => (event) => {
    if (event.type === 'keydown' && (event.key === 'Tab' || event.key === 'Shift')) {
      return;
    }

    setWiFiDrawerOpen(open);
  };

  return (
    <div>
      <Card>
        <CardContent>
          <Typography gutterBottom variant="h5" component="div">
            WiFi status
          </Typography>
          {wifiStatusLoading ? <Loader /> : <WifiStatus status={wifiStatus} />}
        </CardContent>
        <CardActions>
          {isConnected() && <Button variant="outlined" size="small" color="secondary" onClick={() => disconnect()}>Disconnect</Button>}
          {isConnected() && <Button variant="outlined" size="small" color="secondary" onClick={() => setWiFiDrawerOpen(true)}>Change</Button>}
          {!isConnected() && <Button variant="outlined" size="small" onClick={() => setWiFiDrawerOpen(true)}>Connect</Button>}
          <Button variant="outlined" size="small" color="secondary" onClick={() => loadWiFiStatus()}>Refresh</Button>
        </CardActions>
      </Card>
      <Drawer
        anchor="right"
        open={wifiDrawerOpen}
        onClose={toggleDrawer(false)}
      >
        <Typography variant="h5" sx={{ m: theme.spacing(2) }}>
          Available networks
        </Typography>
        <AvailableNetworks onConnected={() => loadWiFiStatus()} />
      </Drawer>
    </div>
  );
}
