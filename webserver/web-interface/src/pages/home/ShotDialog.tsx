import React from 'react';
import {
  AppBar, Box, Dialog, IconButton, Stack, Toolbar, useMediaQuery, useTheme, Container,

} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import Grid from '@mui/material/Unstable_Grid2';
import ShotChart from '../../components/chart/ShotChart';
import {
  PressureStatBox, PumpFlowStatBox, TemperatureStatBox, TimeStatBox, WeightFlowStatBox, WeightStatBox,
} from '../../components/chart/StatBox';
import useShotDataStore from '../../state/ShotDataStore';
import { Shot } from '../../models/models';

interface ShotDialogProps {
  open?: boolean;
  setOpen: (value: boolean) => void;
  historyShot?: Shot;
}

export default function ShotDialog({ open, setOpen, historyShot }: ShotDialogProps) {
  const { latestShotDatapoint } = useShotDataStore();
  const theme = useTheme();
  const fullHeightGraph = `calc(100vh - 64px - ${theme.spacing(2)})`;
  const lessHeightGraph = `calc(75vh - 64px - ${theme.spacing(2)})`;

  return (
    <Dialog
      fullScreen
      open={open || false}
      onClose={() => setOpen(false)}
      PaperProps={{ elevation: 0 }}
    >
      <AppBar sx={{ position: 'relative', height: '64px' }}>
        <Toolbar>
          <IconButton
            edge="start"
            color="inherit"
            onClick={() => setOpen(false)}
            aria-label="close"
          >
            <CloseIcon />
          </IconButton>
          {historyShot ? 'Reviewing shot from history' : 'Shot in progress'}
        </Toolbar>
      </AppBar>
      <Grid container spacing={1} sx={{ mx: 0, mt: theme.spacing(2) }}>
        <Grid xs={12} sm={9} sx={{ height: { xs: lessHeightGraph, sm: fullHeightGraph } }}>
          <Box sx={{ position: 'relative', width: '100%', height: '100%' }}>
            {historyShot && <ShotChart data={historyShot.datapoints} />}
            {!historyShot && <ShotChart newDataPoint={latestShotDatapoint} />}
          </Box>
        </Grid>
        <Grid xs={12} sm={3} sx={{ height: { xs: '25vh', sm: fullHeightGraph } }}>
          <Grid container columns={3} spacing={1} sx={{ height: '100%', overflow: 'scroll' }}>
            <Grid xs={1} sm={3}>
              <TimeStatBox
                timeInShot={latestShotDatapoint?.timeInShot || 0}
                sx={{ height: '100%' }}
              />
            </Grid>
            <Grid xs={1} sm={3}>
              <WeightStatBox
                shotWeight={latestShotDatapoint?.shotWeight || 0}
                sx={{ height: '100%' }}
              />
            </Grid>
            <Grid xs={1} sm={3}>
              <PressureStatBox
                pressure={latestShotDatapoint?.pressure || 0}
                target={latestShotDatapoint?.targetPressure || 0}
                sx={{ height: '100%' }}
              />
            </Grid>
            <Grid xs={1} sm={3}>
              <PumpFlowStatBox
                pumpFlow={latestShotDatapoint?.pumpFlow || 0}
                target={latestShotDatapoint?.targetPumpFlow || 0}
                sx={{ height: '100%' }}
              />
            </Grid>
            <Grid xs={1} sm={3}>
              <WeightFlowStatBox
                flow={latestShotDatapoint?.weightFlow || 0}
                target={latestShotDatapoint?.targetPumpFlow || 0}
                sx={{ height: '100%' }}
              />
            </Grid>
            <Grid xs={1} sm={3}>
              <TemperatureStatBox
                temperature={latestShotDatapoint?.temperature || 0}
                target={latestShotDatapoint?.targetTemperature || 0}
                sx={{ height: '100%' }}
              />
            </Grid>
          </Grid>
        </Grid>
      </Grid>
    </Dialog>
  );
}

ShotDialog.defaultProps = {
  open: false,
  historyShot: undefined,
};
