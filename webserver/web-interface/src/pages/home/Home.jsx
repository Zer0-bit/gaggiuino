import React, { useState, useEffect } from 'react';
import useWebSocket from 'react-use-websocket';
import {
  Box, Container, useTheme, Button,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2';
import GaugeChart from '../../components/chart/GaugeChart';
import ShotDialog from './ShotDialog';

function Home() {
  const { lastJsonMessage } = useWebSocket(`ws://${window.location.host}/ws`, {
    share: true,
  });
  const theme = useTheme();

  const [lastSensorData, setLastSensorData] = useState({
    temp: 0, pressure: 0, timeInShot: 0, flow: 0, weight: 0,
  });

  const [shotDialogOpen, setShotDialogOpen] = useState(false);

  useEffect(() => {
    if (lastJsonMessage !== null && lastJsonMessage.action === 'sensor_data_update') {
      setLastSensorData(lastJsonMessage.data);
    }
  }, [lastJsonMessage]);

  function boxedComponent(component) {
    return (
      <Box sx={{
        border: `2px solid ${theme.palette.divider}`,
        borderRadius: '16px',
        padding: '8px',
      }}
      >
        {component}
      </Box>
    );
  }

  return (
    <Container sx={{ pt: theme.spacing(2) }}>
      <Grid container columns={16} spacing={1} sx={{ mb: theme.spacing(2) }}>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.temp} primaryColor={theme.palette.temperature.main} title="Temperature" unit="°C" />)}
        </Grid>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />)}
        </Grid>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.flow} primaryColor={theme.palette.flow.main} title="Flow" unit="ml/s" maxValue={15} />)}
        </Grid>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.weight} primaryColor={theme.palette.weight.main} title="Weight" unit="g" />)}
        </Grid>
      </Grid>
      <Button variant="contained" onClick={() => setShotDialogOpen(true)}>
        Start Brew
      </Button>
      <ShotDialog open={shotDialogOpen} setOpen={setShotDialogOpen} />

    </Container>
  );
}

export default Home;
