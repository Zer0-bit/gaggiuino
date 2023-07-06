import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import ScaleIcon from '@mui/icons-material/Scale';
import {
  Box,
  Button,
  Container,
  Fab,
  Unstable_Grid2 as Grid, Paper,
  Skeleton,
  TextField,
  Typography,
  lighten,
  useTheme,
} from '@mui/material';
import React, { useEffect } from 'react';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
import ProfileChart from '../../components/chart/ProfileChart';
import AspectRatioBox from '../../components/layout/AspectRatioBox';
import useProfileStore from '../../state/ProfileStore';
import useSensorStateStore from '../../state/SensorStateStore';

function Home() {
  const theme = useTheme();

  const { sensorState } = useSensorStateStore();
  const { activeProfile, updateActiveProfile, fetchActiveProfile } = useProfileStore();

  useEffect(() => {
    fetchActiveProfile();
  }, [fetchActiveProfile]);

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
          <Box sx={{ p: theme.spacing(2) }}>
            <GaugeLiquid value={sensorState.waterLevel} />
          </Box>
          <Box sx={{ mt: theme.spacing(1), p: theme.spacing(2) }}>
            <GaugeChart value={sensorState.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />
          </Box>
          <Box sx={{ mt: theme.spacing(1), p: theme.spacing(2) }}>
            <GaugeChart value={sensorState.weight} primaryColor={theme.palette.weight.main} title="Weight" unit="gr" maxValue={100} />
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
          <AspectRatioBox ratio={1} sx={{ borderRadius: '50%', backgroundColor: lighten(theme.palette.background.default, 0.2) }}>
            <GaugeChart value={sensorState.temperature} primaryColor={theme.palette.temperature.main} unit="°C" />
          </AspectRatioBox>
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
        </Grid>
      </Grid>
    </Container>
  );
}

export default Home;
