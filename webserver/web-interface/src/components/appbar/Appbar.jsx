import React from 'react';
import AppBar from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import Toolbar from '@mui/material/Toolbar';
import Container from '@mui/material/Container';
import Button from '@mui/material/Button';
import { useNavigate } from 'react-router-dom';
import { useTheme } from '@mui/material';
import logo from '../../logo.png';
import ThemeModeToggle from '../theme/ThemeModeToggle';

function ResponsiveAppBar() {
  const navigate = useNavigate();
  const theme = useTheme();

  return (
    <AppBar position="static" color={theme.palette.mode === 'light' ? 'appbar' : 'primary'}>
      <Container maxWidth="xl">
        <Toolbar disableGutters>
          <Box sx={{ display: 'flex' }}>
            <img src={logo} alt="logo" style={{ maxWidth: '60px' }} />
          </Box>
          <Box sx={{ flexGrow: 1, display: 'flex' }}>
            <Button onClick={() => navigate('/')} sx={{ my: 2, color: 'white', display: 'block' }}>Home</Button>
            <Button onClick={() => navigate('/settings')} sx={{ my: 2, color: 'white', display: 'block' }}>Settings</Button>
          </Box>
          <Box sx={{ flexgrow: 0, display: 'flex' }}>
            <ThemeModeToggle />
          </Box>
        </Toolbar>
      </Container>
    </AppBar>
  );
}
export default ResponsiveAppBar;
