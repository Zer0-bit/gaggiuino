import * as React from 'react';
import {
  Box, TextField, Button, Tabs, Tab, Typography, useTheme, Grid, StandardTextFieldProps,
} from '@mui/material';
import TemperatureIcon from '@mui/icons-material/DeviceThermostat';
import CoffeeMakerIcon from '@mui/icons-material/CoffeeMaker';
import FlareIcon from '@mui/icons-material/Flare';
import ScaleIcon from '@mui/icons-material/Scale';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import LogoDevIcon from '@mui/icons-material/LogoDev';
import LogContainer from '../log/LogContainer';
import ThemeModeToggle from '../theme/ThemeModeToggle';

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
}

function TabPanel(props: TabPanelProps) {
  const {
    children, value, index, ...other
  } = props;
  const theme = useTheme();
  const styles = {
    [theme.breakpoints.up('lg')]: {
      width: '50%',
    },
    [theme.breakpoints.down('lg')]: {
      width: '60%',
    },
    [theme.breakpoints.down('md')]: {
      width: '100%',
    },
  };

  return (
    <Box
      role="tabpanel"
      hidden={value !== index}
      id={`vertical-tabpanel-${index}`}
      aria-labelledby={`vertical-tab-${index}`}
      sx={styles}
      {...other}
    >
      {value === index && (
        <Box sx={{ p: 3 }}>
          {children}
        </Box>
      )}
    </Box>
  );
}

function a11yProps(index: number) {
  return {
    id: `vertical-tab-${index}`,
    'aria-controls': `vertical-tabpanel-${index}`,
  };
}

const textFieldStyles = { width: '5ch', '& input': { textAlign: 'center' } };

const sharedTextFieldProps: StandardTextFieldProps = {
  sx: textFieldStyles, id: 'contained-read-only-input', defaultValue: '0', InputProps: { readOnly: true, inputMode: 'numeric' },
};
const sharedGridProps = {
  alignItems: 'center', justifyContent: 'space-between',
};

const iconButtonStyles = { flex: '0 0 5%', '& .MuiButton-startIcon': { marginRight: '0px' } };

interface NumericStateBoxProps {
  fieldDisplay: string;
  value: string;
  onChange?: (e: React.ChangeEvent<HTMLInputElement>) => void;
  onAddBtnClick?: () => void;
  onSubBtnClick?: () => void;
}

function NumericStepInput({
  fieldDisplay, onChange, onAddBtnClick, onSubBtnClick, value,
}: NumericStateBoxProps): JSX.Element {
  return (
    <Grid container spacing={2} {...sharedGridProps}>
      <Grid item md={5} xs={3}>
        <Typography>
          {`${fieldDisplay}:`}
        </Typography>
      </Grid>
      <Grid item xs={2}>
        <TextField variant="standard" value={value} {...sharedTextFieldProps} onChange={onChange} />
      </Grid>
      <Grid container spacing={2} item xs={5}>
        <Grid item order={{ sm: 1, xs: 2 }}>
          <Button variant="contained" startIcon={<RemoveIcon />} sx={iconButtonStyles} onClick={onSubBtnClick} />
        </Grid>
        <Grid item order={{ sm: 2, xs: 1 }}>
          <Button variant="contained" startIcon={<AddIcon />} sx={iconButtonStyles} onClick={onAddBtnClick} />
        </Grid>
      </Grid>
    </Grid>
  );
}

export default function VerticalTabs() {
  const [value, setValue] = React.useState(0);

  const defaultBoilerState = {
    waterTemp: 93, steamTemp: 155, tempOffset: 0, hpwr: 0, mainDivider: 0, brewDivider: 0,
  };
  const [boilerState, setBoilerState] = React.useState(defaultBoilerState);

  const handleTabsChange = (event: React.SyntheticEvent, newValue: number) => {
    setValue(newValue);
  };

  const theme = useTheme();
  const boxStyles = {
    border: `1px solid ${theme.palette.divider}`, borderRadius: '16px', padding: '10px',
  };

  const onAddBtnClick = (stateKey: keyof typeof defaultBoilerState) => {
    setBoilerState((prev) => ({ ...prev, [stateKey]: prev[stateKey] + 1 }));
  };

  const onSubBtnClick = (stateKey: keyof typeof defaultBoilerState) => {
    setBoilerState((prev) => ({ ...prev, [stateKey]: prev[stateKey] - 1 }));
  };

  const onNumericInputChange = (stateKey: keyof typeof defaultBoilerState, eventValue: string) => {
    setBoilerState((prev) => ({ ...prev, [stateKey]: eventValue }));
  };

  return (
    <Box sx={{
      flexGrow: 1, bgcolor: 'background.paper', display: 'flex', height: '100%', borderRadius: '16px',
    }}
    >
      <Tabs orientation="vertical" variant="standard" value={value} onChange={handleTabsChange} sx={{ borderRight: 1, borderColor: 'divider' }}>
        <Tab icon={<TemperatureIcon />} label="Boiler" {...a11yProps(0)} />
        <Tab icon={<CoffeeMakerIcon />} label="System" {...a11yProps(1)} />
        <Tab icon={<FlareIcon />} label="Led" {...a11yProps(2)} />
        <Tab icon={<ScaleIcon />} label="Scales" {...a11yProps(3)} />
        <Tab icon={<LogoDevIcon />} label="SysLog" {...a11yProps(4)} />
      </Tabs>
      <TabPanel value={value} index={0}>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Water Temperature" value={boilerState.waterTemp.toString()} onChange={(e) => onNumericInputChange('waterTemp', e.target.value)} onAddBtnClick={() => onAddBtnClick('waterTemp')} onSubBtnClick={() => onSubBtnClick('waterTemp')} />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Steam Temperature" value={boilerState.steamTemp.toString()} onChange={(e) => onNumericInputChange('steamTemp', e.target.value)} onAddBtnClick={() => onAddBtnClick('steamTemp')} onSubBtnClick={() => onSubBtnClick('steamTemp')} />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Temperature Offset" value={boilerState.tempOffset.toString()} onChange={(e) => onNumericInputChange('tempOffset', e.target.value)} onAddBtnClick={() => onAddBtnClick('tempOffset')} onSubBtnClick={() => onSubBtnClick('tempOffset')} />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="HPWR" value={boilerState.hpwr.toString()} onChange={(e) => onNumericInputChange('hpwr', e.target.value)} onAddBtnClick={() => onAddBtnClick('hpwr')} onSubBtnClick={() => onSubBtnClick('hpwr')} />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Main Divider" value={boilerState.mainDivider.toString()} onChange={(e) => onNumericInputChange('mainDivider', e.target.value)} onAddBtnClick={() => onAddBtnClick('mainDivider')} onSubBtnClick={() => onSubBtnClick('mainDivider')} />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Brew Divider" value={boilerState.brewDivider.toString()} onChange={(e) => onNumericInputChange('brewDivider', e.target.value)} onAddBtnClick={() => onAddBtnClick('brewDivider')} onSubBtnClick={() => onSubBtnClick('brewDivider')} />
        </Box>
      </TabPanel>
      <TabPanel value={value} index={1}>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Scales Factor 1" value="0" />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Scales Factor 2" value="0" />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="LCD Sleep" value="0" />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="System Update Rate" value="0" />
        </Box>
        <Box sx={boxStyles}>
          <NumericStepInput fieldDisplay="Pump Zero" value="23.0" />
        </Box>

        <Box sx={{
          justifyContent: 'center', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Dark/Light toggle:</Typography>
          <ThemeModeToggle />
        </Box>
        <Box sx={{
          justifyContent: 'center', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Button>Reset to defaults</Button>
        </Box>
      </TabPanel>
      <TabPanel value={value} index={2}>
        PLACEHOLDER
      </TabPanel>
      <TabPanel value={value} index={3}>
        PLACEHOLDER
      </TabPanel>
      <TabPanel value={value} index={4}>
        <LogContainer />
      </TabPanel>
    </Box>
  );
}
