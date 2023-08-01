import LocalCarWashIcon from '@mui/icons-material/LocalCarWash';
import ScaleIcon from '@mui/icons-material/Scale';
import ShowerIcon from '@mui/icons-material/Shower';
import {
  Box,
  Button,
  Container,
  Unstable_Grid2 as Grid,
  Paper,
  Skeleton,
  Tab,
  Tabs,
  TextField,
  Theme,
  Typography,
  darken,
  debounce,
  lighten,
  useMediaQuery,
  useTheme,
} from '@mui/material';
import React, {
  useCallback, useState,
} from 'react';
import { selectActiveProfile } from '../../components/client/ProfileClient';
import { getOperationMode, updateOperationMode } from '../../components/client/SystemStateClient';
import { SettingsNumberIncrementButtons } from '../../components/inputs/settings_inputs';
import AspectRatioBox from '../../components/layout/AspectRatioBox';
import AvailableProfileSelector from '../../components/profile/AvailableProfileSelector';
import { ProfileReview } from '../../components/profile/ProfilePreview';
import ShotHistory from '../../components/shot/ShotHistory';
import {
  GaggiaSettings, NotificationType, OperationMode,
} from '../../models/models';
import { Profile } from '../../models/profile';
import useProfileStore from '../../state/ProfileStore';
import useSensorStateStore from '../../state/SensorStateStore';
import useSettingsStore from '../../state/SettingsStore';
import useSystemStateStore from '../../state/SystemStateStore';
import useNotificationStore from '../../state/NotificationDataStore';
import { SwitchLedButton, SwitchLedState } from '../../components/inputs/SwitchLedButton';
import PressureGauge from '../../components/gauges/PressureGauge';
import WaterLevelGauge from '../../components/gauges/WaterLevelGauge';
import TemperatureGauge from '../../components/gauges/TemperatureGauge';

const colorScaling = (theme: Theme) => (theme.palette.mode === 'light' ? lighten : darken);

const MemoMiddleSection = React.memo(MiddleSection);
const MemoLeftSection = React.memo(LeftSection);
const MemoRightSection = React.memo(RightSection);

function Home() {
  const theme = useTheme();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));
  const updateLatestNotification = useNotificationStore((state) => state.updateLatestNotification);
  const updateLocalOperationMode = useSystemStateStore((state) => state.updateLocalOperationMode);
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
          <MemoLeftSection />
        </Grid>
        )}

        {/* Center part - profiles */}
        <Grid xs={7} sm={6} sx={{ gap: '8px', position: 'relative' }}>
          <MemoMiddleSection
            activeProfile={activeProfile}
            handlePersistActiveProfile={handlePersistActiveProfile}
            handleProfileUpdate={handleProfileUpdate}
            handleNewProfileSelected={handleNewProfileSelected}
          />
        </Grid>

        {/* Right part - Temperature Gauge and Scale */}
        <Grid xs={5} sm={4}>
          <MemoRightSection
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

function LeftSection() {
  const theme = useTheme();
  return (
    <>
      <Box sx={{ p: theme.spacing(1) }}>
        <WaterLevelGauge />
      </Box>
      <Box sx={{ mt: theme.spacing(1), p: theme.spacing(1) }}>
        <PressureGauge />
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
  handleBrewTempUpdate: (value: number) => void;
  handleSteamTempUpdate: (value: number) => void;
  handleOpmodeChange: (opMode: OperationMode) => void;
}

const MemoOpModeButtons = React.memo(OpModeButtons);
const MemoTargetTempInput = React.memo(TargetTempInput);
const MemoScalesInput = React.memo(ScalesInput);
const MemoTemperatureGauge = React.memo(TemperatureGauge);

function RightSection({
  handleBrewTempUpdate,
  handleSteamTempUpdate,
  handleOpmodeChange,
}: RightSectionProps) {
  const theme = useTheme();
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));
  const steamActive = useSensorStateStore((state) => state.sensorState.steamActive);
  const steamSetPoint = useSettingsStore((state) => state.settings?.boiler.steamSetPoint || 0);
  const brewTemperature = useProfileStore((state) => state.activeProfile?.waterTemperature || 0);
  const targetTemp = steamActive ? steamSetPoint : brewTemperature;
  const tempUpdateHandler = steamActive ? handleSteamTempUpdate : handleBrewTempUpdate;

  return (
    <>
      <Box sx={{
        pb: 1, pl: 0.5, pr: 0.5, borderRadius: '50%', backgroundColor: colorScaling(theme)(theme.palette.background.default, 0.4),
      }}
      >
        <AspectRatioBox ratio={1}>
          <MemoTemperatureGauge targetTemperature={targetTemp} />
        </AspectRatioBox>
      </Box>

      <Grid container spacing={2} sx={{ px: 1, mt: 3 }}>
        <Grid xs={12}>
          <MemoTargetTempInput
            targetTemp={targetTemp || 0}
            handleTempUpdate={tempUpdateHandler}
          />
        </Grid>
        <Grid xs={12}>
          <MemoScalesInput />
        </Grid>
        <Grid xs={12}>
          <MemoOpModeButtons onChange={handleOpmodeChange} />
        </Grid>

        {/* Gauges for small screens */}
        {!isBiggerScreen && (
          <Grid xs={12} sx={{ display: 'flex', alignItems: 'stretch' }}>
            <Box sx={{ width: '50%', p: theme.spacing(1) }}>
              <WaterLevelGauge />
            </Box>
            <Box sx={{ width: '50%', p: theme.spacing(1) }}>
              <PressureGauge />
            </Box>
          </Grid>
        )}
      </Grid>
    </>
  );
}

function getBrewBtnState(operationMode: OperationMode) {
  if (operationMode === OperationMode.FLUSH) return SwitchLedState.ON;
  if (operationMode === OperationMode.FLUSH_AUTO) return SwitchLedState.AUTO;
  return SwitchLedState.OFF;
}

function OpModeButtons({ onChange }: {
  onChange: (opMode: OperationMode) => void
}) {
  const operationMode = useSystemStateStore((state) => state.systemState.operationMode);

  const handleFlushStateChange = useCallback((newState: SwitchLedState) => {
    if (newState === SwitchLedState.OFF) onChange(OperationMode.BREW_AUTO);
    else if (newState === SwitchLedState.ON) onChange(OperationMode.FLUSH);
    else onChange(OperationMode.FLUSH_AUTO);
  }, [onChange]);

  const handleDescaleStateChange = useCallback((newState: SwitchLedState) => {
    onChange(newState === SwitchLedState.ON ? OperationMode.DESCALE : OperationMode.BREW_AUTO);
  }, [onChange]);

  return (
    <Box width="100%" display="flex" justifyContent="space-evenly" gap={2}>
      <SwitchLedButton
        state={getBrewBtnState(operationMode)}
        onChange={handleFlushStateChange}
        icon={<ShowerIcon fontSize="inherit" />}
        supportsAuto
        label="FLUSH"
      />
      <SwitchLedButton
        state={operationMode === OperationMode.DESCALE ? SwitchLedState.ON : SwitchLedState.OFF}
        onChange={handleDescaleStateChange}
        icon={<LocalCarWashIcon fontSize="inherit" />}
        label="DESCALE"
      />
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
  const currentWeight = useSensorStateStore((state) => state.sensorState.weight);
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
          value={currentWeight.toFixed(1)}
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
        <Box sx={{
          p: { xs: 0, sm: theme.spacing(1) },
          height: '45vh',
          overflow: 'auto', // Makes the box scrollable when contents overflow
        }}
        >
          <ShotHistory />
        </Box>
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
