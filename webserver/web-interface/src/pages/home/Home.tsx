import ScaleIcon from '@mui/icons-material/Scale';
import ShowerIcon from '@mui/icons-material/Shower';
import LocalCarWashIcon from '@mui/icons-material/LocalCarWash';
import {
  Box,
  Button,
  Container,
  Unstable_Grid2 as Grid,
  Paper,
  Skeleton,
  TextField,
  Typography,
  darken,
  debounce,
  lighten,
  useMediaQuery,
  useTheme,
  Tab,
  Tabs,
  alpha,
  Theme,
  ButtonBase,
} from '@mui/material';
import React, { ReactNode, useCallback, useState } from 'react';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
import AspectRatioBox from '../../components/layout/AspectRatioBox';
import { ProfileReview } from '../../components/profile/ProfilePreview';
import ShotHistory from '../../components/shot/ShotHistory';
import useProfileStore from '../../state/ProfileStore';
import useSensorStateStore from '../../state/SensorStateStore';
import useShotDataStore from '../../state/ShotDataStore';
import SnackNotification, { SnackMessage } from '../../components/alert/SnackMessage';
import { Profile } from '../../models/profile';
import AvailableProfileSelector from '../../components/profile/AvailableProfileSelector';
import { selectActiveProfile } from '../../components/client/ProfileClient';
import { OperationMode, SensorState } from '../../models/models';
import useSystemStateStore from '../../state/SystemStateStore';
import { getOperationMode, updateOperationMode } from '../../components/client/SystemStateClient';
import { SettingsNumberIncrementButtons } from '../../components/inputs/settings_inputs';

const colorScaling = (theme: Theme) => (theme.palette.mode === 'light' ? lighten : darken);

function Home() {
  const theme = useTheme();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));
  const [alertMessage, setAlertMessage] = useState<SnackMessage>();

  const { sensorState } = useSensorStateStore();

  const { systemState, updateSystemState } = useSystemStateStore();

  const {
    activeProfile,
    updateActiveProfile, persistActiveProfile, setLocalActiveProfile,
    fetchAvailableProfiles, fetchActiveProfile,
  } = useProfileStore();

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const handleProfileUpdate = useCallback(debounce(async (newProfile: Profile) => {
    try {
      await updateActiveProfile(newProfile);
      setAlertMessage({ content: 'Updated active profile.', level: 'success' });
    } catch (e) {
      setAlertMessage({ content: 'Failed to update active profile', level: 'error' });
    }
  }, 1000), [updateActiveProfile]);

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const handlePersistActiveProfile = useCallback(debounce(async () => {
    try {
      await persistActiveProfile();
      fetchAvailableProfiles(); // Fetch profiles to update list names

      setAlertMessage({ content: 'Persisted active profile', level: 'success' });
    } catch (e) {
      setAlertMessage({ content: 'Failed to persist active profile', level: 'error' });
    }
  }, 1000), [persistActiveProfile]);

  const handleTempUpdate = useCallback((value: number) => {
    if (!activeProfile) return;
    if (value > 169 || value < 0) return;
    setLocalActiveProfile({ ...activeProfile, waterTemperature: value });
    handleProfileUpdate({ ...activeProfile, waterTemperature: value });
  }, [activeProfile, handleProfileUpdate, setLocalActiveProfile]);

  const handleNewProfileSelected = useCallback((id: number) => {
    if (activeProfile?.id !== id) {
      selectActiveProfile(id);
      fetchActiveProfile();
    }
  }, [activeProfile, fetchActiveProfile]);

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const handleOpmodeChange = useCallback(debounce(async (newMode: OperationMode) => {
    try {
      await updateOperationMode(newMode);
      const mode = await getOperationMode();
      updateSystemState({ ...systemState, operationMode: mode });
    } catch (e) {
      setAlertMessage({ content: 'Failed to persist active profile', level: 'error' });
    }
  }, 500), []);

  return (
    <Container sx={{ pt: theme.spacing(2), gap: '0px' }}>
      {/* <ShowAlert level="INFO" text="Welcome home motherfucker \_O_/" /> */}
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1), gap: '0px' }}>

        {/* Left size Gauges */}
        {isBiggerScreen && (
        <Grid sm={2} md={2}>
          {LeftSection(sensorState)}
        </Grid>
        )}

        {/* Center part - profiles */}
        <Grid xs={7} sm={6} sx={{ gap: '8px', position: 'relative' }}>
          {MiddleSection(
            activeProfile,
            handlePersistActiveProfile,
            handleProfileUpdate,
            handleNewProfileSelected,
          )}
        </Grid>

        {/* Right part - Temperature Gauge and Scale */}
        <Grid xs={5} sm={4}>
          {RightSection(sensorState, activeProfile, handleTempUpdate, handleOpmodeChange)}
        </Grid>
      </Grid>
      <SnackNotification message={alertMessage} />
    </Container>
  );
}

export default Home;

function LeftSection(sensorState: SensorState): React.ReactNode {
  const theme = useTheme();
  return (
    <>
      <Box sx={{ p: theme.spacing(1) }}>
        <GaugeLiquid value={sensorState.waterLevel} />
      </Box>
      <Box sx={{ mt: theme.spacing(1), p: theme.spacing(1) }}>
        <GaugeChart value={sensorState.pressure} primaryColor={theme.palette.pressure.main} title="Pressure" unit="bar" maxValue={14} />
      </Box>
    </>
  );
}

function MiddleSection(
  activeProfile: Profile | null,
  handlePersistActiveProfile: () => void,
  handleProfileUpdate: (profile: Profile) => void,
  handleNewProfileSelected: (id: number) => void,
) {
  const theme = useTheme();

  return (
    <Box>
      <Grid container spacing={1}>
        <Grid xs={12}>
          {activeProfile && (
          <Paper sx={{ padding: theme.spacing(1), position: 'relative' }} elevation={1}>
            <ProfileReview
              profile={activeProfile}
              onSave={handlePersistActiveProfile}
              onChange={handleProfileUpdate}
            />
          </Paper>
          )}
          {!activeProfile && <Skeleton variant="rounded" sx={{ borderRadius: '16px' }} height={190} />}
        </Grid>
        <Grid xs={12}>
          <Paper sx={{ padding: theme.spacing(1) }} elevation={1}>
            {ProfileAndHistoryTabs(activeProfile, handleNewProfileSelected)}
          </Paper>
        </Grid>
      </Grid>
    </Box>
  );
}

function RightSection(
  sensorState: SensorState,
  activeProfile: Profile | null,
  handleTempUpdate: (value: number) => void,
  handleOpmodeChange: (opMode: OperationMode) => void,
) {
  const theme = useTheme();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));

  return (
    <>
      <Box sx={{
        pb: 1, pl: 0.5, pr: 0.5, borderRadius: '50%', backgroundColor: colorScaling(theme)(theme.palette.background.default, 0.4),
      }}
      >
        <AspectRatioBox ratio={1}>
          <GaugeChart
            value={sensorState.temperature}
            primaryColor={theme.palette.temperature.main}
            flashAfterValue={120}
            unit="°C"
          />
        </AspectRatioBox>
      </Box>

      <Grid container spacing={2} sx={{ px: 1, mt: 3 }}>
        <Grid xs={12}>
          <TargetTempInput targetTemp={activeProfile?.waterTemperature || 0} handleTempUpdate={handleTempUpdate} />
        </Grid>
        <Grid xs={12}>
          <ScalesInput />
        </Grid>
        <Grid xs={12}>
          <OpModeButtons onChange={handleOpmodeChange} />
        </Grid>

        {/* Gauges for small screens */}
        {!isBiggerScreen && (
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
    </>
  );
}

function OpModeButtons({ onChange }: {onChange: (opMode: OperationMode) => void}) {
  const { operationMode } = useSystemStateStore().systemState;

  function handleFlushStateChange(newState: boolean) {
    onChange(newState ? OperationMode.FLUSH : OperationMode.BREW_AUTO);
  }
  function handleDescaleStateChange(newState: boolean) {
    onChange(newState ? OperationMode.DESCALE : OperationMode.BREW_AUTO);
  }

  return (
    <Box width="100%" display="flex" justifyContent="space-evenly" gap={2}>
      <OpmodeButton
        state={operationMode === OperationMode.FLUSH}
        onChange={handleFlushStateChange}
        icon={<ShowerIcon fontSize="inherit" />}
        label="FLUSH"
      />
      <OpmodeButton
        state={operationMode === OperationMode.DESCALE}
        onChange={handleDescaleStateChange}
        icon={<LocalCarWashIcon fontSize="inherit" />}
        label="DESCALE"
      />
    </Box>
  );
}

interface OpModeButtonProps {
  state: boolean;
  icon?: ReactNode;
  label?: ReactNode;
  onChange?: (state: boolean) => void;
}

function OpmodeButton({
  state = false,
  icon = undefined,
  label = undefined,
  onChange = undefined,
}: OpModeButtonProps) {
  const theme = useTheme();
  const background = (state)
    ? 'linear-gradient(to top, rgba(127, 127, 127, 0.1) 0%, rgba(0, 0, 0, 0.1) 100%)'
    : 'linear-gradient(to top, rgba(255, 255, 255, 0.1) 0%, rgba(80, 80, 80, 0.1) 100%)';
  const boxShadow = '0px 1px 2px -1px rgba(0,0,0,0.2), 0px 3px 3px 0px rgba(0,0,0,0.14), 0px 1px 5px 0px rgba(0,0,0,0.12)';
  const ledColorOn = '#ef4e2b';
  const ledColorOff = '#822714';

  return (
    <Box sx={{
      p: { xs: 0.5, sm: 1 },
      borderRadius: theme.spacing(1),
      width: '100%',
      maxWidth: '120px',
      border: `1px solid ${alpha(theme.palette.divider, 0.05)}`,
      boxShadow,
    }}
    >
      <AspectRatioBox ratio={1 / 1.4}>
        <ButtonBase
          sx={{
            borderRadius: theme.spacing(1),
            boxShadow,
            background,
            border: `1px solid ${alpha(theme.palette.divider, 0.05)}`,
            ':hover': {
              border: `1px solid ${alpha(theme.palette.divider, 0.2)}`,
            },
          }}
          onClick={() => (onChange && onChange(!state))}
        >
          <Box sx={{
            height: '100%',
            width: '100%',
            display: 'flex',
            flexDirection: 'column',
            justifyContent: 'space-around',
            alignItems: 'center',
            py: { xs: 1, sm: 2 },
          }}
          >
            <Box sx={{ fontSize: { xs: theme.typography.body2.fontSize, sm: theme.typography.body1.fontSize } }}>
              {icon}
            </Box>
            <Box sx={{ overflowX: 'hidden', textOverflow: 'ellipsis', width: '100%' }}>{label}</Box>
            <Box sx={{
              width: '15px',
              height: '10px',
              backgroundColor: state ? ledColorOn : ledColorOff,
              boxShadow: state ? '0 0 5px 3px rgba(239,78,43,0.5)' : '', // Glow effect
            }}
            />
          </Box>
        </ButtonBase>
      </AspectRatioBox>
    </Box>
  );
}

function TargetTempInput(
  { targetTemp, handleTempUpdate }: { targetTemp: number, handleTempUpdate: (value: number) => void},
) {
  const theme = useTheme();
  return (
    <Grid container alignItems="center">
      <Grid xs={4}>
        <Typography fontSize={12}>TARGET TEMP</Typography>
      </Grid>
      <Grid xs={4}>
        <TextField
          fullWidth
          size="small"
          sx={{
            textAlign: 'center',
            backgroundColor: colorScaling(theme)(theme.palette.background.default, 0.3),
          }}
          type="text"
          value={targetTemp}
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
        <SettingsNumberIncrementButtons value={targetTemp} onChange={handleTempUpdate} />
      </Grid>
    </Grid>
  );
}

function ScalesInput() {
  const theme = useTheme();
  const { sensorState } = useSensorStateStore();

  return (
    <Grid container alignItems="center">
      <Grid xs={4}>
        <Typography fontSize={12}>SCALE</Typography>
      </Grid>
      <Grid xs={4}>
        <TextField
          size="small"
          fullWidth
          sx={{
            textAlign: 'center',
            backgroundColor: colorScaling(theme)(theme.palette.background.default, 0.3),
          }}
          disabled
          type="text"
          value={sensorState.weight.toFixed(1)}
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
  );
}

function ProfileAndHistoryTabs(
  activeProfile: Profile | null,
  handleNewProfileSelected: (id: number) => void,
) {
  const theme = useTheme();
  const [tabValue, setTabValue] = useState(0);
  const { shotHistory } = useShotDataStore();

  return (
    <>
      <Tabs value={tabValue} variant="fullWidth" onChange={(e, newValue) => setTabValue(newValue)}>
        <Tab label="Available Profiles" {...a11yProps(0)} />
        <Tab label="Shot History" {...a11yProps(1)} />
      </Tabs>
      <TabPanel value={tabValue} index={0}>
        <Box sx={{
          p: { xs: 0, sm: theme.spacing(1) },
          height: '45vh',
          overflow: 'auto', // Makes the box scrollable when contents overflow
        }}
        >
          <AvailableProfileSelector
            selectedProfileId={activeProfile?.id}
            onSelected={handleNewProfileSelected}
          />
        </Box>
      </TabPanel>
      <TabPanel value={tabValue} index={1}>
        {shotHistory.length > 0 && (
        <Box sx={{
          p: { xs: 0, sm: theme.spacing(1) },
          height: '45vh',
          overflow: 'auto', // Makes the box scrollable when contents overflow
        }}
        >
          <ShotHistory />
          {shotHistory.length === 0 && <Typography variant="body2">Pull some shots to see them here.</Typography>}
        </Box>
        )}
      </TabPanel>
    </>
  );
}

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
}

function TabPanel({
  children = 'tabpanel',
  value,
  index,
  ...other
}: TabPanelProps) {
  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`simple-tabpanel-${index}`}
      aria-labelledby={`simple-tab-${index}`}
      {...other}
    >
      {value === index && children}
    </div>
  );
}

function a11yProps(index: number) {
  return {
    id: `full-width-tab-${index}`,
    'aria-controls': `full-width-tabpanel-${index}`,
  };
}
