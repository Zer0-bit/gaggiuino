import React, { useEffect, useMemo, useState } from 'react';
import { useLocation, useNavigate } from 'react-router-dom';
import Tabs from '@mui/material/Tabs';
import Tab from '@mui/material/Tab';
import CoffeeIcon from '@mui/icons-material/Coffee';
import TuneIcon from '@mui/icons-material/Tune';
import SettingsIcon from '@mui/icons-material/Settings';
import Box from '@mui/material/Box';
import {
  useTheme,
  Stack,
  AppBar,
  Toolbar,
  Fab,
  useMediaQuery,
  MenuItem,
  Button,
  Menu,
} from '@mui/material';
import Logo from '../icons/Logo';
import ShotDialog from '../../pages/home/ShotDialog';
import useShotDataStore from '../../state/ShotDataStore';
import useSystemStateStore from '../../state/SystemStateStore';
import formatTime from '../../models/time_format';
import SnackNotification from '../alert/SnackMessage';
import useNotificationStore from '../../state/NotificationDataStore';

const menuItems: { [key: string]: { label: string; icon: JSX.Element } } = {
  '/': { label: 'Home', icon: <CoffeeIcon /> },
  '/profiles': { label: 'Profiles', icon: <TuneIcon /> },
  '/settings': { label: 'Settings', icon: <SettingsIcon /> },
};

interface LinkTabProps {
  value: string;
}

function LinkTab(props: LinkTabProps) {
  const { value: path } = props;
  const navigate = useNavigate();
  const location = useLocation();
  const theme = useTheme();

  const textColor = theme.palette.text.primary;
  const activeColor = theme.palette.mode === 'light'
    ? theme.palette.primary.contrastText
    : theme.palette.primary.main;
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
    />
  );
}

interface TabMenuProps {
  activeItem: string;
  onChange: (value: string) => void;
  activeColor: string;
}

function TabMenu({ activeItem, onChange, activeColor }: TabMenuProps) {
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
      {Object.keys(menuItems).map((item) => (
        <LinkTab key={item} value={item} />
      ))}
    </Tabs>
  );
}

interface NavMenuProps {
  activeItem: string;
  onChange: (value: string) => void;
}

function NavMenu({ activeItem, onChange }: NavMenuProps) {
  const navigate = useNavigate();
  const [anchorEl, setAnchorEl] = useState<null | HTMLElement>(null);
  const theme = useTheme();

  const handleOpenMenu = (event: React.MouseEvent<HTMLButtonElement>) => {
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
  const [activeTab, setActiveTab] = useState<string>(location.pathname || '/');
  const [shotDialogOpen, setShotDialogOpen] = useState<boolean>(false);
  const isBiggerScreen = useMediaQuery(theme.breakpoints.up('sm'));
  const { latestShotDatapoint } = useShotDataStore();
  const { timeAlive } = useSystemStateStore().systemState;
  const { latestNotification } = useNotificationStore();

  useEffect(() => {
    if (latestShotDatapoint.timeInShot > 0) setShotDialogOpen(true);
  }, [latestShotDatapoint]);

  const activeColor = useMemo(() => (theme.palette.mode === 'light'
    ? theme.palette.primary.contrastText
    : theme.palette.primary.main), [theme]);

  return (
    <AppBar sx={{ position: 'static' }} elevation={1}>
      <Toolbar>
        <Stack
          sx={{ display: 'flex', flexGrow: 1 }}
          direction="row"
          spacing={2}
          justifyContent="space-between"
          alignItems="center"
        >
          <Box
            sx={{
              color: activeColor,
              alignContent: 'center',
            }}
          >
            <Fab
              onClick={() => setShotDialogOpen(true)}
              size="medium"
              sx={{
                color: 'primary.main',
                backgroundColor: 'background.paper',
                boxShadow: 0,
                '&:hover': {
                  boxShadow: 0,
                  backgroundColor: 'background.paper',
                },
              }}
            >
              <Box height={30} sx={{ ml: '-4px' }}>
                <Logo size={30} />
              </Box>
            </Fab>
          </Box>
          {isBiggerScreen && (
            <TabMenu activeItem={activeTab} activeColor={activeColor} onChange={setActiveTab} />
          )}
          {!isBiggerScreen && <NavMenu activeItem={activeTab} onChange={setActiveTab} />}
          <div
            style={{
              fontSize: '1rem',
              width: '100px',
              height: '40px',
              borderRadius: '16px',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              border: '1px solid #ccc',
              // Add any other styling you want for the container
            }}
          >
            {formatTime({ time: timeAlive * 1000 })}
          </div>
        </Stack>
      </Toolbar>
      <Box />
      {shotDialogOpen && <ShotDialog open={shotDialogOpen} setOpen={setShotDialogOpen} />}
      <SnackNotification notification={latestNotification} />
    </AppBar>
  );
}

export default MainAppBar;
