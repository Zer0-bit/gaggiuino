import React, { useState, useEffect, useRef } from 'react';
import {
  Box, Container, useTheme, Fab, TextField, Button, Skeleton, Typography, Unstable_Grid2 as Grid, Paper,
} from '@mui/material';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import ScaleIcon from '@mui/icons-material/Scale';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
import useSensorStateStore from '../../state/SensorStateStore';
import useProfileStore from '../../state/ProfileStore';
import ProfileChart from '../../components/chart/ProfileChart';

function Home() {
  const theme = useTheme();

  const { sensorState } = useSensorStateStore();
  const { activeProfile, updateActiveProfile, fetchActiveProfile } = useProfileStore();

  function boxedComponent(component: React.ReactNode) {
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

  useEffect(() => {
    fetchActiveProfile();
  }, [fetchActiveProfile]);

  const gaugeRef = useRef<HTMLDivElement>(null);
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

  function handleAdd() {
    if (!activeProfile) return;
    activeProfile.waterTemperature += 1;
    updateActiveProfile(activeProfile);
  }

  function handleRemove() {
    if (!activeProfile) return;
    activeProfile.waterTemperature -= 1;
    updateActiveProfile(activeProfile);
  }

  return (
    <Container sx={{ pt: theme.spacing(2), gap: '0px' }}>
      {/* <ShowAlert level="INFO" text="Welcome home motherfucker \_O_/" /> */}
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1), gap: '0px' }}>
        <Grid xs={2}>
          <Box sx={{
            border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '0px', gap: '0px',
          }}
          >
            {boxedComponent(<GaugeLiquid value={sensorState.waterLevel} radius={gaugeSize.width} />)}
            {boxedComponent(<GaugeChart value={sensorState.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />)}
            {boxedComponent(<GaugeChart value={sensorState.weight} primaryColor={theme.palette.weight.main} title="Weight" unit="gr" maxValue={100} />)}
          </Box>
        </Grid>
        <Grid xs={6} sx={{ gap: '8px', position: 'relative' }}>
          <Box>
            <Grid container spacing={1}>
              <Grid xs={12}>
                {activeProfile && (
                <Paper sx={{ padding: theme.spacing(1) }} elevation={1}>
                  <Typography variant="h6" sx={{ color: theme.palette.text.secondary, mb: theme.spacing(2) }}>
                    {activeProfile.name}
                  </Typography>
                  <Box position="relative"><ProfileChart profile={activeProfile} /></Box>
                </Paper>
                )}
                {!activeProfile && <Skeleton variant="rounded" sx={{ borderRadius: '16px' }} height={190} />}
              </Grid>
              <Grid xs={6}>
                <Skeleton variant="rounded" height={150} sx={{ borderRadius: '16px' }} />
              </Grid>
              <Grid xs={6}>
                <Skeleton variant="rounded" height={150} sx={{ fontSize: '1rem', borderRadius: '16px' }} />
              </Grid>
            </Grid>
          </Box>
        </Grid>
        <Grid xs={4}>
          <Box sx={{
            border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '0px',
          }}
          >
            <Box sx={{
              justifyContent: 'space-evenly', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '180px', width: '100%', padding: '0px', backgroundColor: '#292929',
            }}
            >
              {boxedComponent(<GaugeChart value={sensorState.temperature} primaryColor={theme.palette.temperature.main} unit="°C" />)}
            </Box>
            <Box sx={{
              justifyContent: 'center', alignItems: 'center', display: 'flex', border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '25px',
            }}
            >
              <TextField variant="standard" sx={{ width: '10ch' }} id="targetBox" label="Target (°C)" value={activeProfile?.waterTemperature || 0} InputProps={{ readOnly: true }} />
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
              <TextField variant="standard" sx={{ width: '10ch' }} id="outlined-read-only-input" label="Scales (g)" value={sensorState.weight} InputProps={{ readOnly: true }} />
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
