import React, { useState } from 'react';
import SignalWifi3BarIcon from '@mui/icons-material/SignalWifi3Bar';
import {
  Accordion, AccordionSummary, Alert, Button, TextField, Typography, AccordionDetails, Stack,
} from '@mui/material';
import PropTypes from 'prop-types';
import { Network } from '../NetworkPropTypes';
import { connectToWifi } from '../../client/WifiClient';
import Loader from '../../loader/Loader';

export default function AvailableNetwork({
  network,
  onClick,
  expanded,
  onConnected,
}) {
  const [password, setPassword] = useState('');
  const [connecting, setConnecting] = useState(false);
  const [connectionError, setConnectionError] = useState(false);

  async function handleSubmit() {
    setConnecting(true);
    setConnectionError(false);
    try {
      await connectToWifi({
        ssid: network.ssid,
        pass: password,
      });
      onConnected(network);
    } catch (e) {
      setConnectionError(true);
    } finally {
      setConnecting(false);
    }
  }

  return (
    <Accordion expanded={expanded}>
      <AccordionSummary
        onClick={onClick}
        aria-controls="panel1a-content"
        id="panel1a-header"
      >
        <Stack direction="row" spacing={1}>
          <SignalWifi3BarIcon />
          <Typography>{network.ssid}</Typography>
        </Stack>
      </AccordionSummary>
      <AccordionDetails>
        {connectionError && <Alert severity="error">Failed to connect to WiFi</Alert>}
        <form>
          <Stack spacing={1} direction={{ xs: 'column', sm: 'row' }} alignItems={{ xs: 'stretch', sm: 'center' }}>
            <TextField
              id={`pwd-${network.ssid}`}
              size="small"
              type="password"
              label="Password"
              variant="outlined"
              autoComplete="on"
              value={password}
              onChange={(event) => setPassword(event.target.value)}
            />
            <Button type="submit" size="large" variant="outlined" onClick={() => handleSubmit()} disabled={connecting}>
              {connecting && <Loader />}
              Connect
            </Button>
          </Stack>
        </form>
      </AccordionDetails>
    </Accordion>
  );
}

AvailableNetwork.propTypes = {
  network: Network.isRequired,
  onClick: PropTypes.func,
  onConnected: PropTypes.func,
  expanded: PropTypes.bool,
};

AvailableNetwork.defaultProps = {
  onClick: () => {},
  onConnected: () => {},
  expanded: false,
};
