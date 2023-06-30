import  React, { useState, useEffect } from 'react';
import useWebSocket from 'react-use-websocket';
import {
  Box, Container, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Grid';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
import {
  apiHost,
  filterJsonMessage, filterSocketMessage, MSG_TYPE_SENSOR_DATA,
} from '../../models/api';
import ProfilesTable from '../../components/table/table';

function Home() {
  const { lastJsonMessage } = useWebSocket(`ws://${apiHost}/ws`, {
    share: true,
    retryOnError: true,
    shouldReconnect: () => true,
    reconnectAttempts: 1000,
    filter: (message) => filterSocketMessage(message, MSG_TYPE_SENSOR_DATA),
  });
  const theme = useTheme();

  const [scalesPresent, setScalesPresent] = useState(false);

  const [lastSensorData, setLastSensorData] = useState({
    temperature: 0, pressure: 0, pumpFlow: 0, weight: 0, scalesPresent: false, waterLvl: 0,
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
        <Grid item xs={6}>
          {boxedComponent(<GaugeChart value={lastSensorData.temperature} primaryColor={theme.palette.temperature.main} title="Temperature" unit="°C" />)}
        </Grid>
        <Grid item xs={6}>
          {boxedComponent(<GaugeChart value={lastSensorData.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />)}
        </Grid>
        <Grid item xs={6}>
          {boxedComponent(<GaugeLiquid initialValue={lastSensorData.waterLvl}/>)}
          {/* {boxedComponent(<GaugeChart value={lastSensorData.waterLvl} primaryColor={theme.palette.pressure.main} title="Water Level" unit="%" maxValue={100} />)} */}
        </Grid>
        {scalesPresent && (
        <Grid item xs={6}>
          {boxedComponent(<GaugeChart value={lastSensorData.weight} primaryColor={theme.palette.weight.main} title="Weight" unit="gr" maxValue={100} />)}
        </Grid>
        )}
        <Grid item xs={12}>
          <ProfilesTable />
        </Grid>
      </Grid>
    </Container>
  );
}

export default Home;
