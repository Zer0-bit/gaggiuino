import React, { useEffect, useState } from 'react';
import {
  AppBar, Dialog, IconButton, Stack, Toolbar,
} from '@mui/material';
import PropTypes from 'prop-types';
import useWebSocket from 'react-use-websocket';
import CloseIcon from '@mui/icons-material/Close';
import Chart from '../../components/chart/ShotChart';

export default function ShotDialog({ open, setOpen }) {
  const { lastJsonMessage } = useWebSocket(`ws://${window.location.host}/ws`, {
    share: true,
  });

  const [sensorData, setSensorData] = useState([]);

  useEffect(() => {
    if (lastJsonMessage !== null && lastJsonMessage.action === 'sensor_data_update') {
      setSensorData((prev) => {
        if (prev.length >= 400) {
          prev.shift();
        }
        return prev.concat(lastJsonMessage.data);
      });
    }
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
          <Chart data={sensorData} />
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
