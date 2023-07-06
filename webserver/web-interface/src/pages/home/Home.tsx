import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import ScaleIcon from '@mui/icons-material/Scale';
import {
  Box,
  Button,
  Container,
  Unstable_Grid2 as Grid, Paper,
  Skeleton,
  TextField,
  Typography,
  lighten,
  Stack,
  useTheme,
  IconButton,
  darken,
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

  function handleTempUpdate(value: number) {
    if (!activeProfile) return;
    if (value > 169 || value < 0) return;
    activeProfile.waterTemperature = value;
    updateActiveProfile(activeProfile);
  }

  const colorScaling = theme.palette.mode === 'light' ? lighten : darken;

  return (
    <Container sx={{ pt: theme.spacing(2), gap: '0px' }}>
      {/* <ShowAlert level="INFO" text="Welcome home motherfucker \_O_/" /> */}
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1), gap: '0px' }}>

        {/* Left size Gauges */}
        <Grid xs={2}>
          <Box sx={{ p: theme.spacing(2) }}>
            <GaugeLiquid value={sensorState.waterLevel} />
          </Box>
          <Box sx={{ mt: theme.spacing(1), p: theme.spacing(2) }}>
            <GaugeChart value={sensorState.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />
          </Box>
        </Grid>

        {/* Center part - profiles */}
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

        {/* Left part - Temperature Gauge and Scale */}
        <Grid xs={4}>
          <Box sx={{
            pb: 1, pl: 0.5, pr: 0.5, borderRadius: '50%', backgroundColor: colorScaling(theme.palette.background.default, 0.4),
          }}
          >
            <AspectRatioBox ratio={1}>
              <GaugeChart value={sensorState.temperature} primaryColor={theme.palette.temperature.main} unit="°C" />
            </AspectRatioBox>
          </Box>

          <Box sx={{ mt: theme.spacing(3) }}>
            <Grid container spacing={2}>
              {/* Target temp input line */}
              <Grid container xs={12} alignItems="center">
                <Grid xs={4}>
                  <Typography variant="caption" color={theme.palette.text.secondary}>TARGET TEMP</Typography>
                </Grid>
                <Grid xs={4}>
                  <TextField
                    fullWidth
                    sx={{
                      backgroundColor: colorScaling(theme.palette.background.default, 0.3),
                      border: 0,
                    }}
                    type="text"
                    value={activeProfile?.waterTemperature || 0}
                    onChange={(e) => handleTempUpdate(parseInt(e.target.value, 10))}
                    InputProps={{
                      disableUnderline: true,
                    }}
                  />
                </Grid>
                <Grid xs={4}>
                  <Box sx={{
                    width: '100%', display: 'flex', flexDirection: 'row', justifyContent: 'space-evenly',
                  }}
                  >
                    <IconButton size="small" color="primary" onClick={() => handleTempUpdate((activeProfile?.waterTemperature || 0) - 1)}>
                      <RemoveIcon />
                    </IconButton>
                    <IconButton size="small" color="primary" onClick={() => handleTempUpdate((activeProfile?.waterTemperature || 0) + 1)}>
                      <AddIcon />
                    </IconButton>
                  </Box>
                </Grid>
              </Grid>

              {/* Scale input line */}
              <Grid container xs={12} alignItems="center">
                <Grid xs={4}>
                  <Typography variant="caption" color={theme.palette.text.secondary}>SCALE</Typography>
                </Grid>
                <Grid xs={4}>
                  <TextField
                    fullWidth
                    sx={{
                      backgroundColor: colorScaling(theme.palette.background.default, 0.3),
                      border: 0,
                    }}
                    disabled
                    type="text"
                    value={sensorState.weight}
                    onChange={(e) => handleTempUpdate(parseInt(e.target.value, 10))}
                    InputProps={{
                      disableUnderline: true,
                    }}
                  />
                </Grid>
                <Grid xs={4}>
                  <Box sx={{
                    width: '100%', display: 'flex', flexDirection: 'row', justifyContent: 'space-evenly',
                  }}
                  >
                    <Button size="small" onClick={() => false}>
                      <ScaleIcon sx={{ mr: theme.spacing(1) }} />
                      Tare
                    </Button>
                  </Box>
                </Grid>
              </Grid>
            </Grid>
          </Box>
        </Grid>
      </Grid>
    </Container>
  );
}

export default Home;
