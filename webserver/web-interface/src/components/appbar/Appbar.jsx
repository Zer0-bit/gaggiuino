import React from 'react';
import AppBar from '@mui/material/AppBar';
import Box from '@mui/material/Box';
import Toolbar from '@mui/material/Toolbar';
import Container from '@mui/material/Container';
import Button from '@mui/material/Button';
import { useNavigate } from 'react-router-dom';
import { Stack, useTheme } from '@mui/material';
import ThemeModeToggle from '../theme/ThemeModeToggle';
import Logo from '../icons/Logo';

function ResponsiveAppBar() {
  const navigate = useNavigate();
  const theme = useTheme();

  const iconColor = theme.palette.mode === 'light' ? theme.palette.primary.contrastText : theme.palette.primary.main;

  return (
    <AppBar position="static">
      <Container maxWidth="xl">
        <Toolbar disableGutters>
          <Stack sx={{ flexGrow: 1, display: 'flex' }} direction="row" spacing={2} justifyItems="center" alignContent="flex-start">
            <Box sx={{ display: 'flex', color: iconColor }}>
              <Logo size={40} />
            </Box>
            <Button onClick={() => navigate('/')} sx={{ my: 2, color: 'white', display: 'block' }}>Home</Button>
            <Button onClick={() => navigate('/settings')} sx={{ my: 2, color: 'white', display: 'block' }}>Settings</Button>
          </Stack>
          <Box sx={{ flexgrow: 0, display: 'flex' }}>
            <ThemeModeToggle />
          </Box>
        </Toolbar>
      </Container>
    </AppBar>
  );
}
export default ResponsiveAppBar;
