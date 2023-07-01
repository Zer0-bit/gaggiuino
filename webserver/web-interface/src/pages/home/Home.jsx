<<<<<<< HEAD
import  React, { useState, useEffect, useRef } from 'react';
=======
import React, { useState, useEffect } from 'react';
>>>>>>> f364c7c (Move persistence to ESP and communicate data via mcu comms)
import useWebSocket from 'react-use-websocket';
import {
  Box, Container, useTheme, Fab, TextField, Grid, Button, Skeleton, Stack,
} from '@mui/material';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import ScaleIcon from '@mui/icons-material/Scale';
import {
  apiHost,
  filterJsonMessage, filterSocketMessage, MSG_TYPE_SENSOR_DATA,
} from '../../models/api';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';

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
    temperature: 0, pressure: 0, pumpFlow: 0, weight: 0, scalesPresent: false, waterLevel: 0,
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
      <Box ref={boxRef} sx={{
        border: `0px solid ${theme.palette.divider}`,
        position: 'relative',
        justifyContent: 'space-evenly',
        alignItems: 'center',
        display: 'flex',
        borderRadius: '20px',
        width: '100%',
        padding: '10px',
      }}
      style={{ marginTop: '-9px' }}
      >
        {component}
      </Box>
    );
  }

  const boxRef = useRef(null);
  const [boxSize, setBoxSize] = useState({ width: 0, height: 0 });

  useEffect(() => {
    if (boxRef.current) {
      const { width, height } = boxRef.current.getBoundingClientRect();
      setBoxSize({ width, height });
    }
  }, []);

  return (
    <Container sx={{ pt: theme.spacing(2), gap: '0px' }}>
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1), gap: '0px' }}>
        <Grid item xs={2}>
          <Box sx={{ border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '0px', gap: '0px' }}>
            {boxedComponent(<GaugeLiquid value={lastSensorData.waterLevel} radius={boxSize.width}/>)}
            {boxedComponent(<GaugeChart value={lastSensorData.pressure} maintainAspectRatio={false}  primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />)}
            {boxedComponent(<GaugeChart value={lastSensorData.weight} maintainAspectRatio={false} primaryColor={theme.palette.weight.main} title="Weight" unit="gr" maxValue={100} />)}
          </Box>
        </Grid>
        <Grid item xs={6} sx={{gap: '8px'}}>
          {/* <ProfilesTable /> */}
          <Box sx={{ border: `0.1px solid ${theme.palette.divider}`, justifyContent: 'center', alignItems: 'center', display: 'flex', position: 'relative', borderRadius: '16px', width: '100%', height: '100%', padding: '0px', backgroundColor: '#292929' }}>
            <Stack spacing={1} width='98%'>
              {/* For variant="text", adjust the height via font-size */}
              <Skeleton variant="rounded" sx={{ fontSize: '1rem', borderRadius: '16px' }} width='100%' height={190} />
              <Skeleton variant="rounded" width='50%' height={150} sx={{ fontSize: '1rem', borderRadius: '16px' }} />
              <Skeleton variant="rounded" width='50%' height={150} sx={{ fontSize: '1rem', borderRadius: '16px' }} />
            </Stack>
          </Box>
        </Grid>
        <Grid item xs={4}>
          <Box sx={{ border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '0px' }}>
            <Box sx={{ justifyContent: 'space-evenly', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '180px', width: '100%', padding: '0px', backgroundColor: '#292929'}}>
              {boxedComponent(<GaugeChart value={lastSensorData.temperature} maintainAspectRatio={true} primaryColor={theme.palette.temperature.main} unit="°C"/>)}
            </Box>
            <Box sx={{ justifyContent: 'center', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '25px', }} >
              <TextField variant="standard" sx={{ width: '10ch', }} id="outlined-read-only-input" label="Target"  defaultValue="93C" InputProps={{readOnly: true,}} />
              <Fab color="primary" aria-label="add">
                <RemoveIcon />
              </Fab>
              <Fab color="primary" aria-label="rem">
                <AddIcon />
              </Fab>
            </Box>
            <Box sx={{ justifyContent: 'center', alignItems: 'center', display: 'flex', border: `2px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '2px', gap: '0px', backgroundColor: '#292929' }} ></Box>
            <Box sx={{ justifyContent: 'center', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '25px', }} >
              <TextField variant="standard" sx={{ width: '10ch', }} id="outlined-read-only-input" label="Scales"  defaultValue="0.0g" InputProps={{readOnly: true,}} />
              <Button variant="outlined" startIcon={<ScaleIcon />} sx={{ width: '40%' }}>
                Tare
              </Button>
            </Box>
          </Box>
        </Grid>
      </Grid>
    </Container>
  );
}

export default Home;
