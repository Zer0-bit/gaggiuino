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
  useTheme,
  IconButton,
  darken,
  useMediaQuery,
} from '@mui/material';
import React, { useEffect } from 'react';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
import AspectRatioBox from '../../components/layout/AspectRatioBox';
import useProfileStore from '../../state/ProfileStore';
import useSensorStateStore from '../../state/SensorStateStore';
import { ProfileReview } from '../../components/profile/ProfilePreview';
import ShotHistory from '../../components/shot/ShotHistory';
import useShotDataStore from '../../state/ShotDataStore';

function Home() {
  const theme = useTheme();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));

  const { sensorState } = useSensorStateStore();
  const { shotHistory } = useShotDataStore();
  const {
    activeProfile, updateActiveProfile, fetchActiveProfile, persistActiveProfile,
  } = useProfileStore();

  useEffect(() => {
    fetchActiveProfile();
  }, [fetchActiveProfile]);

  function handleTempUpdate(value: number) {
    if (!activeProfile) return;
    if (value > 169 || value < 0) return;
    updateActiveProfile({ ...activeProfile, waterTemperature: value });
  }

  function handlePersistActiveProfile() {
    persistActiveProfile();
  }

  const colorScaling = theme.palette.mode === 'light' ? lighten : darken;

  return (
    <Container sx={{ pt: theme.spacing(2), gap: '0px' }}>
      {/* <ShowAlert level="INFO" text="Welcome home motherfucker \_O_/" /> */}
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1), gap: '0px' }}>

        {/* Left size Gauges */}
        {isBiggerScreen && (
        <Grid sm={2} md={2}>
          <Box sx={{ p: theme.spacing(2) }}>
            <GaugeLiquid value={sensorState.waterLevel} />
          </Box>
          <Box sx={{ mt: theme.spacing(1), p: theme.spacing(2) }}>
            <GaugeChart value={sensorState.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />
          </Box>

        </Grid>
        )}

        {/* Center part - profiles */}
        <Grid xs={7} sm={6} sx={{ gap: '8px', position: 'relative' }}>
          <Box>
            <Grid container spacing={1}>
              <Grid xs={12}>
                {activeProfile && (
                <Paper sx={{ padding: theme.spacing(1) }} elevation={1}>
                  <ProfileReview profile={activeProfile} onSave={handlePersistActiveProfile} onEdit={console.log} />
                </Paper>
                ) }
                {!activeProfile && <Skeleton variant="rounded" sx={{ borderRadius: '16px' }} height={190} />}
              </Grid>
              <Grid xs={12}>
                <Paper sx={{ padding: theme.spacing(1) }} elevation={1}>
                  <Typography variant="h6">Shot history</Typography>
                  {shotHistory.length > 0 && <ShotHistory />}
                  {shotHistory.length === 0 && <Typography variant="body2">Pull some shots to see them here.</Typography>}
                </Paper>
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

        {/* Right part - Temperature Gauge and Scale */}
        <Grid xs={5} sm={4}>
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
                  <Typography fontSize={10}>TARGET TEMP</Typography>
                </Grid>
                <Grid xs={4}>
                  <TextField
                    fullWidth
                    size="small"
                    sx={{
                      textAlign: 'center',
                      backgroundColor: colorScaling(theme.palette.background.default, 0.3),
                    }}
                    type="text"
                    value={activeProfile?.waterTemperature || 0}
                    onChange={(e) => handleTempUpdate(parseInt(e.target.value, 10))}
                    InputProps={{
                      sx: {
                        '& input': {
                          textAlign: 'center',
                          paddingLeft: 0,
                          paddingRight: 0,
                        },
                      },
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
                  <Typography fontSize={10}>SCALE</Typography>
                </Grid>
                <Grid xs={4}>
                  <TextField
                    size="small"
                    fullWidth
                    sx={{
                      textAlign: 'center',
                      backgroundColor: colorScaling(theme.palette.background.default, 0.3),
                    }}
                    disabled
                    type="text"
                    value={sensorState.weight.toFixed(1)}
                    onChange={(e) => handleTempUpdate(parseInt(e.target.value, 10))}
                    InputProps={{
                      sx: {
                        '& input': {
                          textAlign: 'center',
                          paddingLeft: 0,
                          paddingRight: 0,
                        },
                      },
                    }}
                  />
                </Grid>
                <Grid xs={4}>
                  <Box sx={{
                    width: '100%', display: 'flex', flexDirection: 'row', justifyContent: 'space-evenly',
                  }}
                  >
                    <Button sx={{ fontSize: { xs: 12, sm: 14 } }} size="small" onClick={() => false}>
                      <ScaleIcon fontSize="inherit" sx={{ mr: theme.spacing(1) }} />
                      Tare
                    </Button>
                  </Box>
                </Grid>
              </Grid>
              {/* End of scale input line */}
              {!isBiggerScreen
                && (
                <Grid xs={12} sx={{ display: 'flex', alignItems: 'stretch' }}>
                  <Box sx={{ width: '50%', p: theme.spacing(1) }}>
                    <GaugeLiquid value={sensorState.waterLevel} />
                  </Box>
                  <Box sx={{ width: '50%', p: theme.spacing(1) }}>
                    <GaugeChart value={sensorState.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />
                  </Box>
                </Grid>
                )}
            </Grid>
          </Box>
        </Grid>
      </Grid>
    </Container>
  );
}

export default Home;
