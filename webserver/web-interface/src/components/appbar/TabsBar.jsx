import * as React from 'react';
import PropTypes from 'prop-types';
import { useNavigate } from 'react-router-dom';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import CoffeeIcon from '@mui/icons-material/Coffee';
import TuneIcon from '@mui/icons-material/Tune';
import SettingsIcon from '@mui/icons-material/Settings';
import Typography from '@mui/material/Typography';
import Box from '@mui/material/Box';
import { useTheme, Stack } from '@mui/material';
import Logo from '../icons/Logo';
import ThemeModeToggle from '../theme/ThemeModeToggle';

function TabPanel(props) {
  const {
    children, value, index, ...other
  } = props;

  return (
    <div
      role="tabpanel"
      hidden={value !== index}
      id={`simple-tabpanel-${index}`}
      aria-labelledby={`simple-tab-${index}`}
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

TabPanel.propTypes = {
  children: PropTypes.node.isRequired,
  index: PropTypes.number.isRequired,
  value: PropTypes.number.isRequired,
};

function handleProps(index) {
  return {
    id: `simple-tab-${index}`,
    'aria-controls': `simple-tabpanel-${index}`,
  };
}

function TabsBar() {
  const navigate = useNavigate();
  const theme = useTheme();
  const [value, setValue] = React.useState(0);

  const iconColor = theme.palette.mode === 'light' ? theme.palette.primary.main : theme.palette.primary.main;

  const handleChange = (event, newValue) => {
    setValue(newValue);
  };

  return (
    <Box sx={{ width: '100%', mt: theme.spacing(2) }}>
      <Stack sx={{ flexGrow: 1, display: 'flex' }} direction="row" spacing={2} justifyContent="space-between" alignContent="flex-start">
        <Box sx={{ display: 'flex', color: iconColor, alignContent: 'flex-start' }}>
          <Logo size={40} />
        </Box>
        <Box sx={{ borderBottom: 1, borderColor: 'divider' }}>
          <Tabs value={value} onChange={handleChange} aria-label="basic tabs example">
            <Tab icon={<CoffeeIcon />} iconPosition="start" label="Home" {...handleProps(0)} onClick={() => navigate('/')} />
            <Tab icon={<TuneIcon />} iconPosition="start" label="Profiles" {...handleProps(1)} onClick={() => navigate('/profiles')} />
            <Tab icon={<SettingsIcon />} iconPosition="start" label="Settings" {...handleProps(2)} onClick={() => navigate('/settings')} />
          </Tabs>
        </Box>
        <Box sx={{ flexgrow: 0, display: 'flex' }}>
          <ThemeModeToggle />
        </Box>
      </Stack>
      <TabPanel value={value} index={0}></TabPanel>
      <TabPanel value={value} index={1}></TabPanel>
      <TabPanel value={value} index={2}></TabPanel>
    </Box>
  );
}

export default TabsBar;
