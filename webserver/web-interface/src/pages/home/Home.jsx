import  React, { useState, useEffect } from 'react';
import useWebSocket from 'react-use-websocket';
import {
  Box, Container, useTheme, Fab, TextField
} from '@mui/material';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import {
  apiHost,
  filterJsonMessage, filterSocketMessage, MSG_TYPE_SENSOR_DATA,
} from '../../models/api';
import Grid from '@mui/material/Grid';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
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
        border: `0px solid ${theme.palette.divider}`,
        position: 'relative',
        borderRadius: '20px',
        width: '100%',
        padding: '10px',
      }}
      >
        {component}
      </Box>
    );
  }

  return (
    <Container sx={{ pt: theme.spacing(2) }}>
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1) }}>
        <Grid item xs={2}>
          <Box sx={{ border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '1px' }}>
            {boxedComponent(<GaugeLiquid value={lastSensorData.waterLvl} radius={50}/>)}
            {boxedComponent(<GaugeChart value={lastSensorData.pressure} maintainAspectRatio={false}  primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />)}
            {boxedComponent(<GaugeChart value={lastSensorData.weight} maintainAspectRatio={false} primaryColor={theme.palette.weight.main} title="Weight" unit="gr" maxValue={100} />)}
          </Box>
        </Grid>
        <Grid item xs={6}>
          <ProfilesTable />
        </Grid>
        <Grid item xs={4}>
          <Box sx={{ border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '1px' }}>
            {boxedComponent(<GaugeChart value={lastSensorData.temperature} maintainAspectRatio={true} primaryColor={theme.palette.temperature.main} title="Temperature" unit="°C"/>)}
            <Box sx={{ justifyContent: 'center', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '5px', gap: '8px', }} >
              <Fab color="primary" aria-label="add">
                <RemoveIcon />
              </Fab>
              <TextField sx={{ width: '10ch', }} id="outlined-read-only-input" label="Target"  defaultValue="93C" InputProps={{readOnly: true,}} />
              <Fab color="primary" aria-label="rem">
                <AddIcon />
              </Fab>
            </Box>
          </Box>
        </Grid>
      </Grid>
    </Container>
  );
}

export default Home;
