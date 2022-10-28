import React, { useState, useEffect } from 'react';
import useWebSocket from 'react-use-websocket';
import {
  Box, Container, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2';
import Chart from '../../components/chart/ShotChart';
import GaugeChart from '../../components/chart/GaugeChart';

function Home() {
  const [socketUrl] = useState(`ws://${window.location.host}/ws`);
  const { lastJsonMessage } = useWebSocket(socketUrl, {
    share: true,
  });
  const theme = useTheme();

  const [sensorData, setSensorData] = useState([]);
  const [lastSensorData, setLastSensorData] = useState({
    temp: 0, pressure: 0, timeInShot: 0, flow: 0, weight: 0,
  });

  useEffect(() => {
    if (lastJsonMessage !== null && lastJsonMessage.action === 'sensor_data_update') {
      setSensorData((prev) => {
        if (prev.length >= 400) {
          prev.shift();
        }
        return prev.concat(lastJsonMessage.data);
      });
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
    <Container className="Home" sx={{ mt: '16px' }}>
      <Grid container columns={16} spacing={2} sx={{ mb: '16px' }}>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.temp} primaryColor={theme.palette.temperature.main} title="Temperature" unit="°C" />)}
        </Grid>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" />)}
        </Grid>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.flow} primaryColor={theme.palette.flow.main} title="Flow" unit="ml/s" />)}
        </Grid>
        <Grid item xs={8} sm={4}>
          {boxedComponent(<GaugeChart value={lastSensorData.weight} primaryColor={theme.palette.weight.main} title="Weight" unit="g" />)}
        </Grid>
      </Grid>
      {boxedComponent(<div><Chart data={sensorData} /></div>)}
    </Container>
  );
}

export default Home;
