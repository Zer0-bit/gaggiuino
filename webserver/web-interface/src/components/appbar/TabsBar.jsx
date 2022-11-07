import * as React from 'react';
import { useNavigate } from 'react-router-dom';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import CoffeeIcon from '@mui/icons-material/Coffee';
import TuneIcon from '@mui/icons-material/Tune';
import SettingsIcon from '@mui/icons-material/Settings';
import Box from '@mui/material/Box';
import {
  useTheme, Stack, AppBar, Toolbar,
} from '@mui/material';
import Logo from '../icons/Logo';
import ThemeModeToggle from '../theme/ThemeModeToggle';

function TabsBar() {
  const navigate = useNavigate();
  const theme = useTheme();
  const [value, setValue] = React.useState(0);

  const textColor = theme.palette.text.secondary;
  const activeColor = theme.palette.mode === 'light' ? theme.palette.primary.contrastText : theme.palette.primary.main;

  function handleProps(index) {
    return {
      id: `appbar-tab-${index}`,
      'aria-controls': `appbar-tabpanel-${index}`,
      sx: { color: value === index ? activeColor : textColor, opacity: 1 },
    };
  }
  const handleChange = (event, newValue) => {
    setValue(newValue);
  };

  return (
    <AppBar sx={{ position: 'static' }} elevation={1}>
      <Toolbar>
        <Stack sx={{ display: 'flex', flexGrow: 1 }} direction="row" spacing={2} justifyContent="space-between" alignItems="center">
          <Box sx={{
            display: 'flex', color: activeColor, alignContent: 'center',
          }}
          >
            <Logo size={40} />
          </Box>
          <Tabs
            value={value}
            onChange={handleChange}
            aria-label="Navigation tabs"
            textColor="inherit"
            TabIndicatorProps={{
              style: {
                backgroundColor: activeColor,
              },
            }}
          >
            <Tab icon={<CoffeeIcon />} iconPosition="start" label="Home" {...handleProps(0)} onClick={() => navigate('/')} />
            <Tab icon={<TuneIcon />} iconPosition="start" label="Profiles" {...handleProps(1)} onClick={() => navigate('/profiles')} />
            <Tab icon={<SettingsIcon />} iconPosition="start" label="Settings" {...handleProps(2)} onClick={() => navigate('/settings')} />
          </Tabs>
          <ThemeModeToggle />
        </Stack>
      </Toolbar>
      <Box />
    </AppBar>
  );
}

export default TabsBar;
