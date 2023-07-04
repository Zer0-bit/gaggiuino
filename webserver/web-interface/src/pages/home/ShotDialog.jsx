import React from 'react';
import {
  AppBar, Box, Dialog, IconButton, Stack, Toolbar,
} from '@mui/material';
import PropTypes from 'prop-types';
import CloseIcon from '@mui/icons-material/Close';
import Grid from '@mui/material/Grid';
import ShotChart from '../../components/chart/ShotChart';
import {
  PressureStatBox, PumpFlowStatBox, TemperatureStatBox, TimeStatBox, WeightFlowStatBox, WeightStatBox,
} from '../../components/chart/StatBox';
import useShotDataStore from '../../state/ShotDataStore';

export default function ShotDialog({ open, setOpen }) {
  const { latestShotDatapoint } = useShotDataStore();

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
              <ShotChart newDataPoint={latestShotDatapoint} />
            </Box>
          </Grid>
          <Grid xs={4} sm={1}>
            {latestShotDatapoint && (
            <Grid container columns={3} spacing={1}>
              <Grid xs={1} sm={3}>
                <TimeStatBox
                  timeInShot={latestShotDatapoint.timeInShot}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <WeightStatBox
                  shotWeight={latestShotDatapoint.shotWeight}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <PressureStatBox
                  pressure={latestShotDatapoint.pressure}
                  target={latestShotDatapoint.targetPressure}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <PumpFlowStatBox
                  pumpFlow={latestShotDatapoint.pumpFlow}
                  target={latestShotDatapoint.targetPumpFlow}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <WeightFlowStatBox
                  flow={latestShotDatapoint.weightFlow}
                  target={latestShotDatapoint.targetPumpFlow}
                  sx={{ height: '100%' }}
                />
              </Grid>
              <Grid xs={1} sm={3}>
                <TemperatureStatBox
                  temperature={latestShotDatapoint.temperature}
                  target={latestShotDatapoint.targetTemperature}
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
