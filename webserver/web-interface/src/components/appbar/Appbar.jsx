import React from 'react';
import AppBar from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import Toolbar from '@mui/material/Toolbar';
import Container from '@mui/material/Container';
import Button from '@mui/material/Button';
import { useNavigate } from 'react-router-dom';
import logo from '../../logo.png';

function ResponsiveAppBar() {
  const navigate = useNavigate();

  return (
    <AppBar position="static">
      <Container maxWidth="xl">
        <Toolbar disableGutters>
          <Box sx={{ flexGrow: 1, display: { xs: 'flex', md: 'flex' } }}>
            <img src={logo} alt="logo" style={{ maxWidth: '60px' }} />
          </Box>
          <Box sx={{ flexGrow: 1, display: { xs: 'flex', md: 'flex' } }}>
            <Button onClick={() => navigate('/')} sx={{ my: 2, color: 'white', display: 'block' }}>Home</Button>
            <Button onClick={() => navigate('/settings')} sx={{ my: 2, color: 'white', display: 'block' }}>Settings</Button>
          </Box>
        </Toolbar>
      </Container>
    </AppBar>
  );
}
export default ResponsiveAppBar;
