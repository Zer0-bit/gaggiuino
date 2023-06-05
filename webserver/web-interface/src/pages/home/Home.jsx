import React, { useState, useEffect } from 'react';
import useWebSocket from 'react-use-websocket';
import {
  Box, Container, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2';
import GaugeChart from '../../components/chart/GaugeChart';
import {
  apiHost,
  filterJsonMessage, filterSocketMessage, MSG_TYPE_SENSOR_DATA, MSG_TYPE_SHOT_DATA,
} from '../../models/api';
import ProfilesTable from '../../components/table/table';
import ShotDialog from './ShotDialog';

function Home() {
  const { lastJsonMessage } = useWebSocket(`ws://${apiHost}/ws`, {
    share: true,
    retryOnError: true,
    shouldReconnect: () => true,
    reconnectAttempts: 1000,
    filter: (message) => filterSocketMessage(message, MSG_TYPE_SHOT_DATA, MSG_TYPE_SENSOR_DATA),
  });
  const theme = useTheme();

  const [shotDialogOpen, setShotDialogOpen] = useState(false);
  const [scalesPresent, setScalesPresent] = useState(false);

  const [lastSensorData, setLastSensorData] = useState({
    temperature: 0, pressure: 0, pumpFlow: 0, weight: 0, scalesPresent: true,
  });

  useEffect(() => {
    setScalesPresent(lastSensorData.scalesPresent);
  }, [lastSensorData]);

  useEffect(() => {
    if (lastJsonMessage === null) {
      return;
    }
    if (filterJsonMessage(lastJsonMessage, MSG_TYPE_SENSOR_DATA)) {
      setLastSensorData(lastJsonMessage.data);
    }
    if (filterJsonMessage(lastJsonMessage, MSG_TYPE_SHOT_DATA)) {
      setShotDialogOpen(true);
    }
  }, [lastJsonMessage]);

  function boxedComponent(component) {
    return (
      <Box sx={{
        border: `2px solid ${theme.palette.divider}`,
        position: 'relative',
        borderRadius: '16px',
        width: '100%',
        padding: '8px',
      }}
      >
        {component}
      </Box>
    );
  }

  return (
    <Container sx={{ pt: theme.spacing(2) }}>
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1) }}>
        <Grid item xs={scalesPresent ? 4 : 6}>
          {boxedComponent(<GaugeChart value={lastSensorData.temperature} primaryColor={theme.palette.temperature.main} title="Temperature" unit="°C" />)}
        </Grid>
        <Grid item xs={scalesPresent ? 4 : 6}>
          {boxedComponent(<GaugeChart value={lastSensorData.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />)}
        </Grid>
        {scalesPresent && (
        <Grid item xs={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.weight} primaryColor={theme.palette.weight.main} title="Weight" unit="gr" maxValue={100} />)}
        </Grid>
        )}
        <Grid item xs={12}>
          <ProfilesTable />
        </Grid>
      </Grid>
      {shotDialogOpen && <ShotDialog open={shotDialogOpen.toString()} setOpen={setShotDialogOpen} />}
    </Container>
  );
}

export default Home;
