import React, { useState, useEffect, useRef } from 'react';
import {
  Box, Container, useTheme, Fab, TextField, Grid, Button, Skeleton, Stack,
} from '@mui/material';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import ScaleIcon from '@mui/icons-material/Scale';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
import ShowAlert from '../../components/alert/alert';
import useSensorStateStore from '../../state/SensorStateStore';
import ShowAlert from '../../components/alert/alert';

function Home() {
  const theme = useTheme();

  const { sensorState } = useSensorStateStore();

  function boxedComponent(component) {
    return (
      <Box
        ref={gaugeRef}
        sx={{
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

  const gaugeRef = useRef(null);
  const [gaugeSize, setGaugeSize] = useState({ width: 0, height: 0 });
  const calculateGaugeSize = () => {
    if (gaugeRef.current) {
      const { width, height } = gaugeRef.current.getBoundingClientRect();
      setGaugeSize({ width, height });
    }
  };

  useEffect(() => {
    calculateGaugeSize();
    window.addEventListener('resize', calculateGaugeSize);

    return () => {
      window.removeEventListener('resize', calculateGaugeSize);
    };
  }, []);

  function handleRemove() {
    const targetBox = document.getElementById('targetBox');
    const currentValue = parseInt(targetBox.value);
    const newValue = currentValue - 1;
    targetBox.value = newValue.toString();
  }

  function handleAdd() {
    const targetBox = document.getElementById('targetBox');
    const currentValue = parseInt(targetBox.value);
    const newValue = currentValue + 1;
    targetBox.value = newValue.toString();

  }

  return (
    <Container sx={{ pt: theme.spacing(2), gap: '0px' }}>
      <ShowAlert level='INFO' text='Welcome home motherfucker \_O_/' />
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1), gap: '0px' }}>
        <Grid item xs={2}>
          <Box sx={{
            border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '0px', gap: '0px',
          }}
          >
            {boxedComponent(<GaugeLiquid value={sensorState.waterLevel} radius={gaugeSize.width} />)}
            {boxedComponent(<GaugeChart value={sensorState.pressure} maintainAspectRatio={false} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />)}
            {boxedComponent(<GaugeChart value={sensorState.weight} maintainAspectRatio={false} primaryColor={theme.palette.weight.main} title="Weight" unit="gr" maxValue={100} />)}
          </Box>
        </Grid>
        <Grid item xs={6} sx={{ gap: '8px' }}>
          {/* <ProfilesTable /> */}
          <Box sx={{
            border: `0.1px solid ${theme.palette.divider}`, justifyContent: 'center', alignItems: 'center', display: 'flex', position: 'relative', borderRadius: '16px', width: '100%', height: '100%', padding: '0px', backgroundColor: '#292929',
          }}
          >
            <Stack spacing={1} width="98%">
              {/* For variant="text", adjust the height via font-size */}
              <Skeleton variant="rounded" sx={{ fontSize: '1rem', borderRadius: '16px' }} width="100%" height={190} />
              <Skeleton variant="rounded" width="50%" height={150} sx={{ fontSize: '1rem', borderRadius: '16px' }} />
              <Skeleton variant="rounded" width="50%" height={150} sx={{ fontSize: '1rem', borderRadius: '16px' }} />
            </Stack>
          </Box>
        </Grid>
        <Grid item xs={4}>
          <Box sx={{
            border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '0px',
          }}
          >
            <Box sx={{
              justifyContent: 'space-evenly', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '180px', width: '100%', padding: '0px', backgroundColor: '#292929',
            }}
            >
              {boxedComponent(<GaugeChart value={sensorState.temperature} maintainAspectRatio primaryColor={theme.palette.temperature.main} unit="°C" />)}
            </Box>
            <Box sx={{
              justifyContent: 'center', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '25px',
            }}
            >
              <TextField variant="standard" sx={{ width: '10ch' }} id="targetBox" label="Target (°C)" value="93" InputProps={{ readOnly: true }} />
              <Fab color="primary" aria-label="add" onClick={handleRemove}>
                <RemoveIcon />
              </Fab>
              <Fab color="primary" aria-label="rem" onClick={handleAdd}>
                <AddIcon />
              </Fab>
            </Box>
            <Box sx={{
              justifyContent: 'center', alignItems: 'center', display: 'flex', border: `2px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '2px', gap: '0px', backgroundColor: '#292929',
            }}
            >
            </Box>
            <Box sx={{
              justifyContent: 'center', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '25px',
            }}
            >
              <TextField variant="standard" sx={{ width: '10ch' }} id="outlined-read-only-input" label="Scales (g)" value={lastSensorData.weight} InputProps={{ readOnly: true }} />
              <Button variant="contained" startIcon={<ScaleIcon />} sx={{ width: '40%' }}>
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
