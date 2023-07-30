import LocalCarWashIcon from '@mui/icons-material/LocalCarWash';
import ScaleIcon from '@mui/icons-material/Scale';
import ShowerIcon from '@mui/icons-material/Shower';
import {
  Box,
  Button,
  ButtonBase,
  Container,
  Unstable_Grid2 as Grid,
  Paper,
  Skeleton,
  Tab,
  Tabs,
  TextField,
  Theme,
  Typography,
  alpha,
  darken,
  debounce,
  lighten,
  useMediaQuery,
  useTheme,
} from '@mui/material';
import React, {
  ReactNode, useCallback, useMemo, useState,
} from 'react';
import GaugeChart from '../../components/chart/GaugeChart';
import GaugeLiquid from '../../components/chart/GaugeLiquid';
import { selectActiveProfile } from '../../components/client/ProfileClient';
import { getOperationMode, updateOperationMode } from '../../components/client/SystemStateClient';
import { SettingsNumberIncrementButtons } from '../../components/inputs/settings_inputs';
import AspectRatioBox from '../../components/layout/AspectRatioBox';
import AvailableProfileSelector from '../../components/profile/AvailableProfileSelector';
import { ProfileReview } from '../../components/profile/ProfilePreview';
import ShotHistory from '../../components/shot/ShotHistory';
import {
  GaggiaSettings, NotificationType, OperationMode, SensorState,
} from '../../models/models';
import { Profile } from '../../models/profile';
import useProfileStore from '../../state/ProfileStore';
import useSensorStateStore from '../../state/SensorStateStore';
import useSettingsStore from '../../state/SettingsStore';
import useShotDataStore from '../../state/ShotDataStore';
import useSystemStateStore from '../../state/SystemStateStore';
import useNotificationStore from '../../state/NotificationDataStore';

const colorScaling = (theme: Theme) => (theme.palette.mode === 'light' ? lighten : darken);

function Home() {
  const theme = useTheme();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));

  const { sensorState } = useSensorStateStore();
  const { updateLatestNotification } = useNotificationStore();
  const { updateLocalOperationMode } = useSystemStateStore();
  const { settings, updateLocalSettings, updateSettingsAndSync } = useSettingsStore();
  const {
    activeProfile,
    updateActiveProfileAndSync, persistActiveProfile, updateLocalActiveProfile,
    fetchAvailableProfiles, fetchActiveProfile,
  } = useProfileStore();

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const handleProfileUpdate = useCallback(debounce(async (newProfile: Profile) => {
    try {
      await updateActiveProfileAndSync(newProfile);
      updateLatestNotification({ message: 'Updated active profile.', type: NotificationType.SUCCESS });
    } catch (e) {
      updateLatestNotification({ message: 'Failed to update active profile.', type: NotificationType.ERROR });
    }
  }, 1000), [updateActiveProfileAndSync]);

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const handlePersistActiveProfile = useCallback(debounce(async () => {
    try {
      await persistActiveProfile();
      fetchAvailableProfiles(); // Fetch profiles to update list names

      updateLatestNotification({ message: 'Persisted active profile.', type: NotificationType.SUCCESS });
    } catch (e) {
      updateLatestNotification({ message: 'Failed to persist active profile.', type: NotificationType.ERROR });
    }
  }, 1000), [persistActiveProfile]);

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const updateSettingsDebounced = useCallback(
    debounce((newSettings: GaggiaSettings) => {
      updateSettingsAndSync(newSettings);
    }, 1000),
    [updateSettingsAndSync],
  );

  const handleBrewTempUpdate = useCallback((value: number) => {
    if (!activeProfile) return;
    if (value > 120 || value < 0) return;
    updateLocalActiveProfile({ ...activeProfile, waterTemperature: value });
    handleProfileUpdate({ ...activeProfile, waterTemperature: value });
  }, [activeProfile, handleProfileUpdate, updateLocalActiveProfile]);

  const handleSteamTempUpdate = useCallback((value: number) => {
    if (!settings) return;
    if (value > 169 || value < 120) return;
    const newSettings = { ...settings, boiler: { ...settings.boiler, steamSetPoint: value } };
    updateLocalSettings(newSettings);
    updateSettingsDebounced(newSettings);
  }, [settings, updateLocalSettings, updateSettingsDebounced]);

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
      updateLocalOperationMode(mode);
    } catch (e) {
      updateLatestNotification({ message: 'Failed to change operation mode', type: NotificationType.ERROR });
    }
  }, 500), []);

  return (
    <Container sx={{ pt: theme.spacing(2), px: { xs: 0.5, sm: 1 }, gap: 0 }}>
      {/* <ShowAlert level="INFO" text="Welcome home motherfucker \_O_/" /> */}
      <Grid container columns={12} spacing={1} sx={{ mb: theme.spacing(1), gap: '0px' }}>

        {/* Left size Gauges */}
        {isBiggerScreen && (
        <Grid sm={2} md={2}>
          <LeftSection sensorState={sensorState} />
        </Grid>
        )}

        {/* Center part - profiles */}
        <Grid xs={7} sm={6} sx={{ gap: '8px', position: 'relative' }}>
          <MiddleSection
            activeProfile={activeProfile}
            handlePersistActiveProfile={handlePersistActiveProfile}
            handleProfileUpdate={handleProfileUpdate}
            handleNewProfileSelected={handleNewProfileSelected}
          />
        </Grid>

        {/* Right part - Temperature Gauge and Scale */}
        <Grid xs={5} sm={4}>
          <RightSection
            sensorState={sensorState}
            activeProfile={activeProfile}
            handleBrewTempUpdate={handleBrewTempUpdate}
            handleSteamTempUpdate={handleSteamTempUpdate}
            handleOpmodeChange={handleOpmodeChange}
          />
        </Grid>
      </Grid>
    </Container>
  );
}

export default Home;

function LeftSection({ sensorState }: {sensorState: SensorState}): React.ReactNode {
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

interface MiddleSectionProps {
  activeProfile: Profile | null,
  handlePersistActiveProfile: () => void,
  handleProfileUpdate: (profile: Profile) => void,
  handleNewProfileSelected: (id: number) => void,
}

function MiddleSection({
  activeProfile, handlePersistActiveProfile, handleProfileUpdate, handleNewProfileSelected,
}: MiddleSectionProps) {
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

interface RightSectionProps{
  sensorState: SensorState;
  activeProfile: Profile | null;
  handleBrewTempUpdate: (value: number) => void;
  handleSteamTempUpdate: (value: number) => void;
  handleOpmodeChange: (opMode: OperationMode) => void;
}

function RightSection({
  sensorState,
  activeProfile,
  handleBrewTempUpdate,
  handleSteamTempUpdate,
  handleOpmodeChange,
}: RightSectionProps) {
  const theme = useTheme();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));
  const { settings } = useSettingsStore();
  const targetTemp = useMemo(() => (sensorState.steamActive
    ? settings?.boiler.steamSetPoint
    : activeProfile?.waterTemperature || 0), [settings, activeProfile, sensorState]);
  const tempUpdateHandler = useMemo(
    () => (sensorState.steamActive ? handleSteamTempUpdate : handleBrewTempUpdate),
    [sensorState, handleSteamTempUpdate, handleBrewTempUpdate],
  );

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
            maxValue={targetTemp}
            flashAfterValue={settings && settings.boiler ? settings.boiler.steamSetPoint - 20 : undefined}
            unit="°C"
          />
        </AspectRatioBox>
      </Box>

      <Grid container spacing={2} sx={{ px: 1, mt: 3 }}>
        <Grid xs={12}>
          <TargetTempInput
            targetTemp={targetTemp || 0}
            handleTempUpdate={tempUpdateHandler}
          />
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
      background: (theme.palette.mode === 'light') ? 'rgba(0, 0, 0, 0.4)' : 'rgba(25, 25, 25, 0.2)',
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
            color: 'rgba(255, 255, 255, 0.8)',
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
        <SettingsNumberIncrementButtons fontSize="22px" value={targetTemp} onChange={handleTempUpdate} />
      </Grid>
    </Grid>
  );
}

function ScalesInput() {
  const theme = useTheme();
  const { sensorState } = useSensorStateStore();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));

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
          width: '100%', display: 'flex', flexDirection: 'row', justifyContent: 'end',
        }}
        >
          <Button
            sx={{
              fontSize: '14px',
              minWidth: 30,
            }}
            size="small"
            onClick={() => false}
          >
            <ScaleIcon fontSize="inherit" sx={{ mr: theme.spacing(1) }} />
            {isBiggerScreen ? 'TARE' : 'TR'}
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
