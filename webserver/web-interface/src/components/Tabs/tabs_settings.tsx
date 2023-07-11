import CoffeeMakerIcon from '@mui/icons-material/CoffeeMaker';
import TemperatureIcon from '@mui/icons-material/DeviceThermostat';
import FlareIcon from '@mui/icons-material/Flare';
import LogoDevIcon from '@mui/icons-material/LogoDev';
import ScaleIcon from '@mui/icons-material/Scale';
import {
  Box,
  SxProps,
  Tab,
  Tabs,
  Theme,
  Typography,
  debounce,
  useMediaQuery,
  useTheme,
} from '@mui/material';
import React, { useCallback, useEffect, useState } from 'react';
import {
  BoilerSettings, GaggiaSettings, LedSettings, SystemSettings, areGaggiaSettingsEqual,
} from '../../models/models';
import LogContainer from '../log/LogContainer';
import ThemeModeToggle from '../theme/ThemeModeToggle';
import {
  LedColorPickerInput,
  SettingsInputActions,
  SettingsInputField,
  SettingsInputLabel,
  SettingsInputWrapper,
  SettingsNumberInput,
  SettingsToggleInput,
} from './settings_inputs';

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
  sx?: SxProps<Theme>;
}

function TabPanel(props: TabPanelProps) {
  const {
    children, value, index, ...other
  } = props;
  return (
    <Box
      role="tabpanel"
      hidden={value !== index}
      id={`vertical-tabpanel-${index}`}
      aria-labelledby={`vertical-tab-${index}`}
      sx={{ width: '100%' }}
      {...other}
    >
      {value === index && children}
    </Box>
  );
}

TabPanel.defaultProps = {
  children: null,
  sx: {},
};

function a11yProps(index: number) {
  return {
    id: `vertical-tab-${index}`,
    'aria-controls': `vertical-tabpanel-${index}`,
  };
}

export interface TabbedSettingsProps {
  settings: GaggiaSettings;
  onChange: (settings:GaggiaSettings) => void;
}

export default function TabbedSettings({ settings, onChange }: TabbedSettingsProps) {
  const [selectedTab, setSelectedTab] = React.useState(0);
  const [settingsInternal, setSettingsInternal] = useState(settings);

  const handleChange = (event: React.SyntheticEvent, newValue: number) => {
    setSelectedTab(newValue);
  };

  const theme = useTheme();
  const smallScreen = useMediaQuery(theme.breakpoints.only('xs'));
  const tabOrientation = smallScreen ? 'horizontal' : 'vertical';
  const tabPanelStyling = {
    width: '100%',
    maxWidth: '500px',
    mt: { xs: theme.spacing(2), sm: 0 },
    ml: { xs: 0, sm: theme.spacing(2) },
  };

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const updateSettingsDebounced = useCallback(
    debounce((newSettings: GaggiaSettings) => {
      if (areGaggiaSettingsEqual(newSettings, settings)) return;
      onChange(newSettings);
    }, 1000),
    [onChange, settings],
  );

  useEffect(() => {
    updateSettingsDebounced(settingsInternal);
  }, [settingsInternal, updateSettingsDebounced]);

  useEffect(() => setSettingsInternal(settings), [settings]);

  return (
    <Box sx={{
      display: 'flex',
      flexGrow: 1,
      bgcolor: 'background.paper',
      height: '100%',
      p: theme.spacing(2),
      borderRadius: theme.spacing(2),
      flexDirection: { xs: 'column', sm: 'row' },
    }}
    >
      <Tabs
        orientation={tabOrientation}
        variant="scrollable"
        value={selectedTab}
        onChange={handleChange}
      >
        <Tab icon={<TemperatureIcon />} label="Boiler" {...a11yProps(0)} />
        <Tab icon={<CoffeeMakerIcon />} label="System" {...a11yProps(1)} />
        <Tab icon={<FlareIcon />} label="Led" {...a11yProps(2)} />
        <Tab icon={<ScaleIcon />} label="Scales" {...a11yProps(3)} />
        <Tab icon={<LogoDevIcon />} label="SysLog" {...a11yProps(4)} />
      </Tabs>
      <TabPanel value={selectedTab} index={0} sx={tabPanelStyling}>
        <BoilerSettingsPanel
          boiler={settingsInternal.boiler}
          onChange={(boiler) => setSettingsInternal({ ...settingsInternal, boiler })}
        />
      </TabPanel>
      <TabPanel value={selectedTab} index={1} sx={tabPanelStyling}>
        <SystemSettingsPanel
          system={settingsInternal.system}
          onChange={(system) => setSettingsInternal({ ...settingsInternal, system })}
        />
      </TabPanel>
      <TabPanel value={selectedTab} index={2} sx={tabPanelStyling}>
        <LedSettingsPanel
          led={settingsInternal.led}
          onChange={((led) => setSettingsInternal({ ...settingsInternal, led }))}
        />
      </TabPanel>
      <TabPanel value={selectedTab} index={3} sx={tabPanelStyling}>
        PLACEHOLDER
      </TabPanel>
      <TabPanel value={selectedTab} index={4} sx={tabPanelStyling}>
        <LogContainer />
      </TabPanel>
    </Box>
  );
}

interface BoilerSettingsPanelProps {
  boiler: BoilerSettings;
  onChange: (settings: BoilerSettings) => void;
}

function BoilerSettingsPanel({ boiler, onChange }: BoilerSettingsPanelProps) {
  return (
    <Box sx={{
      display: 'flex', flexDirection: 'column', alignItems: 'stretch', gap: 1, width: '100%',
    }}
    >
      <SettingsNumberInput
        label="Steam Temperature"
        value={boiler.steamSetPoint}
        onChange={(value) => onChange({ ...boiler, steamSetPoint: value })}
      />
      <SettingsNumberInput
        label="Temperature Offset"
        value={boiler.offsetTemp}
        onChange={(value) => onChange({ ...boiler, offsetTemp: value })}
      />
      <SettingsNumberInput
        label="HPWR"
        value={boiler.hpwr}
        onChange={(value) => onChange({ ...boiler, hpwr: value })}
      />
      <SettingsNumberInput
        label="Main Divider"
        value={boiler.mainDivider}
        onChange={(value) => onChange({ ...boiler, mainDivider: value })}
      />
      <SettingsNumberInput
        label="Brew Divider"
        value={boiler.brewDivider}
        onChange={(value) => onChange({ ...boiler, brewDivider: value })}
      />
    </Box>
  );
}

interface SystemSettingsPanelProps {
  system: SystemSettings;
  onChange: (settings: SystemSettings) => void;
}

function SystemSettingsPanel({ system, onChange }: SystemSettingsPanelProps) {
  return (
    <Box sx={{
      display: 'flex', flexDirection: 'column', alignItems: 'stretch', gap: 1, width: '100%',
    }}
    >
      <SettingsNumberInput
        label="LCD sleep"
        value={system.lcdSleep}
        onChange={(value) => onChange({ ...system, lcdSleep: value })}
      />
      <SettingsNumberInput
        label="Pump Zero (PZ)"
        value={system.pumpFlowAtZero}
        onChange={(value) => onChange({ ...system, pumpFlowAtZero: value })}
        fractionDigits={3}
      />
      <SettingsNumberInput
        label="Scales F1"
        value={system.scalesF1}
        onChange={(value) => onChange({ ...system, scalesF1: value })}
      />
      <SettingsNumberInput
        label="Scales F2"
        value={system.scalesF2}
        onChange={(value) => onChange({ ...system, scalesF2: value })}
      />
      <SettingsInputWrapper>
        <SettingsInputLabel><Typography>Dark/Light toggle:</Typography></SettingsInputLabel>
        <SettingsInputField></SettingsInputField>
        <SettingsInputActions><ThemeModeToggle /></SettingsInputActions>
      </SettingsInputWrapper>
      <SettingsToggleInput
        label="Warmup on startup"
        value={system.warmupState}
        onChange={(value) => onChange({ ...system, warmupState: value })}
      />
    </Box>
  );
}

interface LedSettingsPanelProps {
  led: LedSettings;
  onChange: (settings: LedSettings) => void;
}

function LedSettingsPanel({ led, onChange }: LedSettingsPanelProps) {
  return (
    <Box sx={{
      display: 'flex', flexDirection: 'column', alignItems: 'stretch', gap: 1, width: '100%',
    }}
    >
      <SettingsToggleInput
        label="Led enabled"
        value={led.state}
        onChange={(state) => onChange({ ...led, state })}
      />
      <SettingsToggleInput
        label="Disco mode"
        value={led.disco}
        onChange={(disco) => onChange({ ...led, disco })}
      />
      <LedColorPickerInput
        label="LED Color"
        value={led.color}
        onChange={(color) => onChange({ ...led, color })}
      />
    </Box>
  );
}
