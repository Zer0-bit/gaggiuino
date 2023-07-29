import React, { useEffect, useState } from 'react';
import {
  AppBar, Box, Dialog, IconButton, SxProps, Theme, Toolbar, useTheme,

} from '@mui/material';
import CloseIcon from '@mui/icons-material/Close';
import Grid from '@mui/material/Unstable_Grid2';
import ShotChart from '../../components/chart/ShotChart';
import {
  PressureStatBox, PumpFlowStatBox, TemperatureStatBox, TimeStatBox, WeightFlowStatBox, WeightStatBox,
} from '../../components/chart/StatBox';
import useShotDataStore from '../../state/ShotDataStore';
import { Shot, ShotSnapshot } from '../../models/models';
import ShotSpeedDial from '../../components/inputs/ShotSpeedDial';

interface ShotDialogProps {
  open?: boolean;
  setOpen: (value: boolean) => void;
  historyShot?: Shot;
}

export default function ShotDialog({ open = false, setOpen, historyShot = undefined }: ShotDialogProps) {
  const {
    latestShotDatapoint, currentShot, shotRunning, addShotToHistory, removeShotFromHistory,
  } = useShotDataStore();
  const theme = useTheme();
  const fullHeightGraph = `calc(100vh - 64px - ${theme.spacing(1)})`;
  const lessHeightGraph = `calc(75vh - 64px - ${theme.spacing(1)})`;
  const [activeDatapoint, setActiveDatapoint] = useState<ShotSnapshot | undefined>(undefined);

  useEffect(() => {
    if (!historyShot || historyShot.datapoints.length === 0) return;
    setActiveDatapoint(historyShot.datapoints[historyShot.datapoints.length - 1]);
  }, [historyShot, setActiveDatapoint]);

  useEffect(() => {
    if (historyShot) return;
    setActiveDatapoint(latestShotDatapoint);
  }, [latestShotDatapoint, historyShot, setActiveDatapoint]);

  return (
    <Dialog
      fullScreen
      open={open || false}
      onClose={() => setOpen(false)}
      PaperProps={{ elevation: 0 }}
    >
      <AppBar sx={{ position: 'relative', height: '64px' }}>
        <Toolbar sx={{ display: 'flex', justifyContent: 'space-between' }}>
          {historyShot ? 'Reviewing shot from history' : 'Shot in progress'}
          <IconButton
            edge="end"
            color="inherit"
            onClick={() => setOpen(false)}
            aria-label="close"
          >
            <CloseIcon />
          </IconButton>
        </Toolbar>
      </AppBar>
      <Grid container spacing={1} sx={{ mx: 0, mt: theme.spacing(1) }}>
        <Grid xs={12} sm={9} sx={{ height: { xs: lessHeightGraph, sm: fullHeightGraph } }}>
          <Box sx={{ position: 'relative', width: '100%', height: '100%' }}>
            {historyShot && <ShotChart data={historyShot.datapoints} onHover={setActiveDatapoint} />}
            {!historyShot && <ShotChart newDataPoint={latestShotDatapoint} onHover={setActiveDatapoint} />}
            {!shotRunning && (
              <Box sx={{ position: 'absolute', top: theme.spacing(2), right: theme.spacing(4) }}>
                <ShotSpeedDial
                  shot={historyShot || currentShot}
                  onSave={addShotToHistory}
                  onDelete={removeShotFromHistory}
                />
              </Box>
            )}
          </Box>
        </Grid>
        <Grid xs={12} sm={3} sx={{ height: { xs: '25vh', sm: fullHeightGraph }, overflow: 'scroll' }}>
          <StatBoxes activeDatapoint={activeDatapoint} />
        </Grid>
      </Grid>
    </Dialog>
  );
}

function StatBoxes({ activeDatapoint = undefined }: { activeDatapoint?: ShotSnapshot}) {
  const boxSx:SxProps<Theme> = { height: { xs: '100%', sm: 'auto' } };

  return (
    <Grid container spacing={1} sx={{ height: { xs: '100%', sm: 'auto' } }}>
      <Grid xs={4} sm={12}>
        <TimeStatBox
          timeInShot={activeDatapoint?.timeInShot || 0}
          sx={boxSx}
        />
      </Grid>
      <Grid xs={4} sm={12}>
        <WeightStatBox
          shotWeight={activeDatapoint?.shotWeight || 0}
          sx={boxSx}
        />
      </Grid>
      <Grid xs={4} sm={12}>
        <PressureStatBox
          pressure={activeDatapoint?.pressure || 0}
          target={activeDatapoint?.targetPressure || 0}
          sx={boxSx}
        />
      </Grid>
      <Grid xs={4} sm={12}>
        <PumpFlowStatBox
          pumpFlow={activeDatapoint?.pumpFlow || 0}
          target={activeDatapoint?.targetPumpFlow || 0}
          sx={boxSx}
        />
      </Grid>
      <Grid xs={4} sm={12}>
        <WeightFlowStatBox
          flow={activeDatapoint?.weightFlow || 0}
          target={activeDatapoint?.targetPumpFlow || 0}
          sx={boxSx}
        />
      </Grid>
      <Grid xs={4} sm={12}>
        <TemperatureStatBox
          temperature={activeDatapoint?.temperature || 0}
          target={activeDatapoint?.targetTemperature || 0}
          sx={boxSx}
        />
      </Grid>
    </Grid>
  );
}
