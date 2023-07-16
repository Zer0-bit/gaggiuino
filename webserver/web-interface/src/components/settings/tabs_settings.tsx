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
  SettingsInputInlineLabel,
  SettingsInputWrapper,
  SettingsNumberInput,
  SettingsToggleInput,
} from '../inputs/settings_inputs';
import { constrain } from '../../models/utils';

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
  sx?: SxProps<Theme>;
}

function TabPanel({
  children = undefined, value, index, ...other
}: TabPanelProps) {
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
    mt: 2,
    ml: { xs: 0, sm: 2 },
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
      width: '100%',
      bgcolor: 'background.paper',
      height: '100%',
      p: theme.spacing(2),
      gap: theme.spacing(1),
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
      display: 'flex', flexDirection: 'column', alignItems: 'stretch', gap: 1.5, width: '100%',
    }}
    >
      <SettingsNumberInput
        label="Steam Temperature"
        value={boiler.steamSetPoint}
        maxDecimals={0}
        onChange={(value) => onChange({ ...boiler, steamSetPoint: constrain(value, 0, 165) })}
      />
      <SettingsNumberInput
        label="Temperature Offset"
        value={boiler.offsetTemp}
        maxDecimals={0}
        onChange={(value) => onChange({ ...boiler, offsetTemp: constrain(value, 0, 20) })}
      />
      <SettingsNumberInput
        label="HPWR"
        value={boiler.hpwr}
        maxDecimals={0}
        onChange={(value) => onChange({ ...boiler, hpwr: value })}
      />
      <SettingsNumberInput
        label="Main Divider"
        value={boiler.mainDivider}
        maxDecimals={0}
        onChange={(value) => onChange({ ...boiler, mainDivider: value })}
      />
      <SettingsNumberInput
        label="Brew Divider"
        value={boiler.brewDivider}
        maxDecimals={0}
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
      display: 'flex', flexDirection: 'column', alignItems: 'stretch', gap: 1.5, width: '100%',
    }}
    >
      <SettingsNumberInput
        label="LCD sleep"
        value={system.lcdSleep}
        onChange={(value) => onChange({ ...system, lcdSleep: value })}
        maxDecimals={0}
      />
      <SettingsNumberInput
        label="Pump Zero (PZ)"
        value={system.pumpFlowAtZero}
        onChange={(value) => onChange({ ...system, pumpFlowAtZero: value })}
        maxDecimals={3}
        buttonIncrements={0.001}
      />
      <SettingsNumberInput
        label="Scales F1"
        value={system.scalesF1}
        onChange={(value) => onChange({ ...system, scalesF1: value })}
        maxDecimals={0}
      />
      <SettingsNumberInput
        label="Scales F2"
        value={system.scalesF2}
        onChange={(value) => onChange({ ...system, scalesF2: value })}
        maxDecimals={0}
      />
      <SettingsInputWrapper>
        <SettingsInputInlineLabel>Dark/Light toggle</SettingsInputInlineLabel>
        <SettingsInputActions><Box display="flex" paddingY={1}><ThemeModeToggle /></Box></SettingsInputActions>
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
      display: 'flex', flexDirection: 'column', alignItems: 'stretch', gap: 1.5, width: '100%',
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
