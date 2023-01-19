import React, { useEffect, useState } from 'react';
import {
  AppBar, Dialog, IconButton, Stack, Toolbar,
} from '@mui/material';
import PropTypes from 'prop-types';
import useWebSocket from 'react-use-websocket';
import CloseIcon from '@mui/icons-material/Close';
import { filterSocketMessage, MSG_TYPE_SHOT_DATA } from '../../models/api';
import Chart from '../../components/chart/ShotChart';

export default function ShotDialog({ open, setOpen }) {
  const { lastJsonMessage } = useWebSocket(`ws://${window.location.host}/ws`, {
    share: true,
    retryOnError: true,
    shouldReconnect: () => true,
    reconnectAttempts: 1000,
    filter: (message) => filterSocketMessage(message, MSG_TYPE_SHOT_DATA),
  });

  const [sensorData, setSensorData] = useState([]);
  const [latestSensorData, setLatestSensorData] = useState(undefined);

  useEffect(() => {
    if (lastJsonMessage === null) {
      return;
    }
    setLatestSensorData(lastJsonMessage.data);
    setSensorData((prev) => {
      if (prev.length >= 400) {
        prev.shift();
      }
      return prev.concat(lastJsonMessage.data);
    });
  }, [lastJsonMessage]);

  return (
    <Dialog
      fullScreen
      open={open}
      onClose={() => setOpen(false)}
      PaperProps={{ elevation: 0 }}
    >
      <Stack direction="column" alignItems="stretch" justifyContent="flex-start" height="100vh" spacing={2}>
        <AppBar sx={{ position: 'relative', display: 'flex', flexGrow: 0 }}>
          <Toolbar>
            <IconButton
              edge="start"
              color="inherit"
              onClick={() => setOpen(false)}
              aria-label="close"
            >
              <CloseIcon />
            </IconButton>
          </Toolbar>
        </AppBar>
        <div style={{ position: 'relative', flexGrow: 1 }}>
          <Chart data={sensorData} newDataPoint={latestSensorData} maxLength={1200} />
        </div>
      </Stack>
    </Dialog>
  );
}

ShotDialog.propTypes = {
  open: PropTypes.bool,
  setOpen: PropTypes.func.isRequired,
};

ShotDialog.defaultProps = {
  open: false,
};
