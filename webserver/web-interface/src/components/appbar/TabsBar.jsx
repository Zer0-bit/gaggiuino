import React, { useState } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import CoffeeIcon from '@mui/icons-material/Coffee';
import TuneIcon from '@mui/icons-material/Tune';
import SettingsIcon from '@mui/icons-material/Settings';
import Box from '@mui/material/Box';
import {
  useTheme, Stack, AppBar, Toolbar, Fab,
} from '@mui/material';
import PropTypes from 'prop-types';
import Logo from '../icons/Logo';
import ThemeModeToggle from '../theme/ThemeModeToggle';
import ShotDialog from '../../pages/home/ShotDialog';

function LinkTab(props) {
  const { value: path } = props;
  const navigate = useNavigate();
  const location = useLocation();
  const theme = useTheme();

  const textColor = theme.palette.text.secondary;
  const activeColor = theme.palette.mode === 'light' ? theme.palette.primary.contrastText : theme.palette.primary.main;
  const id = path.replace('/', '');

  return (
    <Tab
      {...props}
      iconPosition="start"
      id={`appbar-tab-${id}`}
      sx={{ color: location.pathname === path ? activeColor : textColor, opacity: 1 }}
      onClick={() => navigate(path)}
    >
    </Tab>
  );
}

LinkTab.propTypes = {
  value: PropTypes.string.isRequired,
};

function TabsBar() {
  const theme = useTheme();
  const location = useLocation();
  const [activeTab, setActiveTab] = useState(location.pathname || '/');
  const [shotDialogOpen, setShotDialogOpen] = useState(false);

  const activeColor = theme.palette.mode === 'light' ? theme.palette.primary.contrastText : theme.palette.primary.main;

  return (
    <AppBar sx={{ position: 'static' }} elevation={1}>
      <Toolbar>
        <Stack sx={{ display: 'flex', flexGrow: 1 }} direction="row" spacing={2} justifyContent="space-between" alignItems="center">
          <Box
            sx={{
              color: activeColor, alignContent: 'center',
            }}
          >
            <Fab onClick={() => setShotDialogOpen(true)} sx={{ color: ['primary.main'], backgroundColor: ['theme.main'] }}>
              <Logo size={40} />
            </Fab>
            {shotDialogOpen && <ShotDialog open={shotDialogOpen} setOpen={setShotDialogOpen} />}
          </Box>
          <Tabs
            value={activeTab}
            onChange={(event, value) => setActiveTab(value)}
            aria-label="Navigation tabs"
            textColor="inherit"
            TabIndicatorProps={{
              style: {
                backgroundColor: activeColor,
              },
            }}
          >
            <LinkTab icon={<CoffeeIcon />} label="Home" value="/" />
            <LinkTab icon={<TuneIcon />} label="Profiles" value="/profiles" />
            <LinkTab icon={<SettingsIcon />} label="Settings" value="/settings" />
          </Tabs>
          <ThemeModeToggle />
        </Stack>
      </Toolbar>
      <Box />
    </AppBar>
  );
}

export default TabsBar;
