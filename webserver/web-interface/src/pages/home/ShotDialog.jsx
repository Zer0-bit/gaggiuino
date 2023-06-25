import React, { useEffect, useState } from 'react';
import {
  AppBar, Box, Dialog, IconButton, Stack, Toolbar,
} from '@mui/material';
import PropTypes from 'prop-types';
import useWebSocket from 'react-use-websocket';
import CloseIcon from '@mui/icons-material/Close';
import Grid from '@mui/material/Grid';
import {
  apiHost, filterSocketMessage, MSG_TYPE_SHOT_DATA,
} from '../../models/api';
import ShotChart from '../../components/chart/ShotChart';
import {
  PressureStatBox, PumpFlowStatBox, TemperatureStatBox, TimeStatBox, WeightFlowStatBox, WeightStatBox,
} from '../../components/chart/StatBox';

export default function ShotDialog({ open, setOpen }) {
  const { lastJsonMessage } = useWebSocket(`ws://${apiHost}/ws`, {
    share: true,
    retryOnError: true,
    shouldReconnect: () => true,
    reconnectAttempts: 1000,
    filter: (message) => filterSocketMessage(message, MSG_TYPE_SHOT_DATA),
  });

  const [latestShotSnapshot, setLatestShotSnapshot] = useState(null);

  useEffect(() => {
    if (lastJsonMessage === null) {
      return;
    }
    setLatestShotSnapshot(lastJsonMessage.data);
  }, [lastJsonMessage]);

  return (
    <Dialog
      fullScreen
      open={open}
      onClose={() => setOpen(false)}
      PaperProps={{ elevation: 0 }}
    >
      <Stack direction="column" display="flex" alignItems="stretch" justifyContent="flex-start" height="100%" spacing={2}>
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

        <Grid container columns={4} spacing={1} sx={{ flexGrow: 1 }}>
          <Grid xs={4} sm={3} display="flex" alignContent="stretch" flexGrow={1}>
            <Box sx={{ position: 'relative', width: '100%' }}>
              <ShotChart newDataPoint={latestShotSnapshot} />
            </Box>
          </Grid>
          <Grid xs={4} sm={1}>
            {latestShotSnapshot && (
            <Grid container columns={3} spacing={1}>
              <Grid xs={1} sm={3}>
                <TimeStatBox
                  timeInShot={latestShotSnapshot.timeInShot}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <WeightStatBox
                  shotWeight={latestShotSnapshot.shotWeight}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <PressureStatBox
                  pressure={latestShotSnapshot.pressure}
                  target={latestShotSnapshot.targetPressure}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <PumpFlowStatBox
                  pumpFlow={latestShotSnapshot.pumpFlow}
                  target={latestShotSnapshot.targetPumpFlow}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <WeightFlowStatBox
                  flow={latestShotSnapshot.weightFlow}
                  target={latestShotSnapshot.targetPumpFlow}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <TemperatureStatBox
                  temperature={latestShotSnapshot.temperature}
                  target={latestShotSnapshot.targetTemperature}
                  sx={{ height: '100%' }}
                />
              </Grid>
            </Grid>
            )}
          </Grid>
        </Grid>
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
