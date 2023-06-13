import React, { useState } from 'react';
import RefreshIcon from '@mui/icons-material/Refresh';
import {
  Box,
  Button, Drawer, Stack, Typography, useTheme,
} from '@mui/material';
import PropTypes from 'prop-types';
import { refrehNetworks } from '../../client/WifiClient';
import Loader from '../../loader/Loader';
import AvailableNetworks from './AvailableNetworks';

export default function AvailableNetworksDrawer({ open, onOpenChanged, onConnected }) {
  const theme = useTheme();
  const [networksRefreshing, setNetworksRefreshing] = useState(false);
  const [wifiDrawerRefreshKey, setWifiDrawerRefreshKey] = useState(0);

  async function refreshNetworksAction() {
    setNetworksRefreshing(true);
    try {
      await refrehNetworks();
      setWifiDrawerRefreshKey((oldKey) => oldKey + 1);
    } finally {
      setNetworksRefreshing(false);
    }
  }

  const toggleDrawer = (isOpen) => (event) => {
    if (event.type === 'keydown' && (event.key === 'Tab' || event.key === 'Shift')) {
      return;
    }
    onOpenChanged(isOpen);
  };

  return open ? (
    <Drawer
      anchor="right"
      open={open}
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
      {!networksRefreshing && <AvailableNetworks key={wifiDrawerRefreshKey} onConnected={onConnected} />}
    </Drawer>
  ) : <span />;
}

AvailableNetworksDrawer.propTypes = {
  open: PropTypes.bool.isRequired,
  onOpenChanged: PropTypes.func.isRequired,
  onConnected: PropTypes.func,
};

AvailableNetworksDrawer.defaultProps = {
  onConnected: () => {},
};
