import React, { useState } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import CoffeeIcon from '@mui/icons-material/Coffee';
import TuneIcon from '@mui/icons-material/Tune';
import SettingsIcon from '@mui/icons-material/Settings';
import Box from '@mui/material/Box';
import {
  useTheme, Stack, AppBar, Toolbar, Fab, useMediaQuery, MenuItem, Button, Menu,
} from '@mui/material';
import PropTypes from 'prop-types';
import Logo from '../icons/Logo';
import ThemeModeToggle from '../theme/ThemeModeToggle';
import ShotDialog from '../../pages/home/ShotDialog';

const menuItems = {
  '/': { label: 'Home', icon: <CoffeeIcon /> },
  '/profiles': { label: 'Profiles', icon: <TuneIcon /> },
  '/settings': { label: 'Settings', icon: <SettingsIcon /> },
};

function LinkTab(props) {
  const { value: path } = props;
  const navigate = useNavigate();
  const location = useLocation();
  const theme = useTheme();

  const textColor = theme.palette.text.secondary;
  const activeColor = theme.palette.mode === 'light' ? theme.palette.primary.contrastText : theme.palette.primary.main;
  const id = path.replace('/', '');
  const { label, icon } = menuItems[path];

  return (
    <Tab
      {...props}
      iconPosition="start"
      id={`appbar-tab-${id}`}
      sx={{ color: location.pathname === path ? activeColor : textColor, opacity: 1 }}
      onClick={() => navigate(path)}
      label={label}
      icon={icon}
    >
    </Tab>
  );
}

LinkTab.propTypes = {
  value: PropTypes.string.isRequired,
};

function TabMenu({ activeItem, onChange, activeColor }) {
  return (
    <Tabs
      value={activeItem}
      onChange={(event, value) => onChange(value)}
      aria-label="Navigation tabs"
      textColor="inherit"
      TabIndicatorProps={{
        style: {
          backgroundColor: activeColor,
        },
      }}
    >
      {Object.keys(menuItems).map((item) => <LinkTab key={item} value={item} />)}
    </Tabs>
  );
}

function NavMenu({ activeItem, onChange }) {
  const navigate = useNavigate();
  const [anchorEl, setAnchorEl] = useState(null);
  const theme = useTheme();

  const handleOpenMenu = (event) => {
    setAnchorEl(event.currentTarget);
  };

  const handleCloseMenu = () => {
    setAnchorEl(null);
  };

  return (
    <Box sx={{ flexGrow: 0 }}>
      <Button
        onClick={(event) => handleOpenMenu(event)}
        sx={{
          color: theme.palette.primary.contrastText,
          display: 'flex',
          alignItems: 'center',
          gap: theme.spacing(2),
        }}
      >
        {menuItems[activeItem].icon}
        {menuItems[activeItem].label}
      </Button>
      <Menu
        id="menu-appbar"
        anchorEl={anchorEl}
        anchorOrigin={{
          vertical: 'bottom',
          horizontal: 'right',
        }}
        keepMounted
        transformOrigin={{
          vertical: 'top',
          horizontal: 'right',
        }}
        open={Boolean(anchorEl)}
        onClose={handleCloseMenu}
      >
        {Object.keys(menuItems).map((item) => (
          <MenuItem
            key={item}
            sx={{
              display: 'flex',
              alignItems: 'center',
              gap: theme.spacing(2),
            }}
            onClick={() => {
              handleCloseMenu();
              onChange(item);
              navigate(item);
            }}
          >
            {menuItems[item].icon}
            {menuItems[item].label}
          </MenuItem>
        ))}
      </Menu>
    </Box>
  );
}

function MainAppBar() {
  const theme = useTheme();
  const location = useLocation();
  const [activeTab, setActiveTab] = useState(location.pathname || '/');
  const [shotDialogOpen, setShotDialogOpen] = useState(false);
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));

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
            <Fab
              onClick={() => setShotDialogOpen(true)}
              size="medium"
              sx={{
                color: 'primary.main',
                backgroundColor: 'background.default',
                boxShadow: 0,
                '&:hover': { boxShadow: 0, backgroundColor: 'background.default' },
              }}
            >
              <Box height={30} sx={{ ml: '-4px' }}><Logo size={30} sx={{}} /></Box>
            </Fab>
          </Box>
          {isBiggerScreen && (
            <TabMenu activeItem={activeTab} activeColor={activeColor} onChange={setActiveTab} />
          )}
          {!isBiggerScreen && (<NavMenu activeItem={activeTab} onChange={setActiveTab} />)}
          <ThemeModeToggle />
        </Stack>
      </Toolbar>
      <Box />
      {shotDialogOpen && <ShotDialog open={shotDialogOpen} setOpen={setShotDialogOpen} />}
    </AppBar>
  );
}

export default MainAppBar;
