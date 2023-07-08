import * as React from 'react';
import {
  Box, TextField, Button, Tabs, Tab, Typography, useTheme, Switch,
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

TabPanel.defaultProps = {
  children: null,
};

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

  const theme = useTheme();

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
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Water Temperature:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Steam Temperature:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Temperature Offset:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>HPWR:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Main Divider:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Brew Divider:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
      </TabPanel>
      <TabPanel value={value} index={1}>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Scales Factor 1:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Scales Factor 2:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>LCD Sleep:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>System Update rate:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Pump Zero:</Typography>
          <TextField variant="standard" sx={{ width: '5ch', '& input': { textAlign: 'center' } }} id="contained-read-only-input" defaultValue="0" InputProps={{ readOnly: true }} />
          <Button variant="contained" startIcon={<RemoveIcon />} sx={{ flex: '0 0 5%' }} />
          <Button variant="contained" startIcon={<AddIcon />} sx={{ flex: '0 0 5%' }} />
        </Box>
        <Box sx={{
          justifyContent: 'space-between', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Reset to defaults:</Typography>
          <Switch defaultChecked />
        </Box>
        <Box sx={{
          justifyContent: 'center', alignItems: 'center', display: 'flex', border: `1px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '100%', padding: '10px', gap: '10px',
        }}
        >
          <Typography>Dark/Light toggle:</Typography>
          <ThemeModeToggle />
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
