import * as React from 'react';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import Typography from '@mui/material/Typography';
import Box from '@mui/material/Box';
import TemperatureIcon from '@mui/icons-material/DeviceThermostat';
import CoffeeMakerIcon from '@mui/icons-material/CoffeeMaker';
import FlareIcon from '@mui/icons-material/Flare';
import ScaleIcon from '@mui/icons-material/Scale';
import LogoDevIcon from '@mui/icons-material/LogoDev';
import LogContainer from '../log/LogContainer';

interface TabPanelProps {
  children?: React.ReactNode;
  index: number;
  value: number;
}

function TabPanel(props: TabPanelProps) {
  const {
    children, value, index, ...other
  } = props;

  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`vertical-tabpanel-${index}`}
      aria-labelledby={`vertical-tab-${index}`}
      {...other}
    >
      {value === index && (
        <Box sx={{ p: 3 }}>
          <Typography>{children}</Typography>
        </Box>
      )}
    </div>
  );
}

function a11yProps(index: number) {
  return {
    id: `vertical-tab-${index}`,
    'aria-controls': `vertical-tabpanel-${index}`,
  };
}

export default function VerticalTabs() {
  const [value, setValue] = React.useState(0);

  const handleChange = (event: React.SyntheticEvent, newValue: number) => {
    setValue(newValue);
  };

  return (
    <Box sx={{
      flexGrow: 1, bgcolor: 'background.paper', display: 'flex', height: '100%', width: '100%', borderRadius: '16px',
    }}
    >
      <Tabs orientation="vertical" variant="standard" value={value} onChange={handleChange} sx={{ borderRight: 1, borderColor: 'divider' }}>
        <Tab icon={<TemperatureIcon />} label="Boiler" {...a11yProps(0)} />
        <Tab icon={<CoffeeMakerIcon />} label="System" {...a11yProps(1)} />
        <Tab icon={<FlareIcon />} label="Led" {...a11yProps(2)} />
        <Tab icon={<ScaleIcon />} label="Scales" {...a11yProps(3)} />
        <Tab icon={<LogoDevIcon />} label="SysLog" {...a11yProps(4)} />
      </Tabs>
      <TabPanel value={value} index={0}>
        PLACEHOLDER
      </TabPanel>
      <TabPanel value={value} index={1}>
        PLACEHOLDER
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
