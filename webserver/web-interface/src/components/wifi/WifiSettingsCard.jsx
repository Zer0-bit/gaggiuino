import RefreshIcon from '@mui/icons-material/Refresh';
import {
  Box,
  Button,
  Card, CardActions, CardContent, Drawer, Stack, Typography, useTheme,
} from '@mui/material';
import React, { useEffect, useState } from 'react';
import { disconnectFromWifi, getWifiStatus, refrehNetworks } from '../client/WifiClient';
import Loader from '../loader/Loader';
import WifiStatus from './WifiStatus';
import AvailableNetworks from './available-networks/AvailableNetworks';

export default function WifiSettingsCard() {
  const [wifiStatus, setWifiStatus] = useState(null);
  const [wifiStatusLoading, setWifiStatusLoading] = useState(true);
  const [wifiDrawerOpen, setWiFiDrawerOpen] = useState(false);
  const [wifiDrawerRefreshKey, setWifiDrawerRefreshKey] = useState(0);
  const [networksRefreshing, setNetworksRefreshing] = useState(false);
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

  async function refreshNetworksAction() {
    setNetworksRefreshing(true);
    try {
      await refrehNetworks();
      setWifiDrawerRefreshKey((oldKey) => oldKey + 1);
    } finally {
      setNetworksRefreshing(false);
    }
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
    <div style={{ height: '100%' }}>
      <Card sx={{ display: 'flex', flexDirection: 'column', height: '100%' }}>
        <CardContent sx={{ flex: '1 0 auto' }}>
          <Typography gutterBottom variant="h5" component="div">
            WiFi Status
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

        <Stack spacing={1} direction="row" alignItems="center" justifyContent="space-between">
          <Typography variant="h5" sx={{ m: theme.spacing(2) }}>
            Available networks
          </Typography>
          <Typography variant="h5" sx={{ m: theme.spacing(2) }}>
            <Button onClick={refreshNetworksAction}><RefreshIcon /></Button>
          </Typography>
        </Stack>
        {networksRefreshing && <Box display="flex" justifyContent="center"><Loader /></Box>}
        {!networksRefreshing && <AvailableNetworks key={wifiDrawerRefreshKey} onConnected={() => loadWiFiStatus()} />}
      </Drawer>
    </div>
  );
}
