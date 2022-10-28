import React, { useMemo, useState } from 'react';
import {
  createBrowserRouter,
  RouterProvider,
  createRoutesFromElements,
  Route,
  Outlet,
} from 'react-router-dom';
import { CssBaseline, ThemeProvider, useMediaQuery } from '@mui/material';
import Home from './pages/home/Home';
import WifiPage from './components/wifi/WifiPage';
import AppBar from './components/appbar/Appbar';
import getAppTheme from './components/theme/AppTheme';
import { ThemeModeContext } from './components/theme/ThemeModeToggle';

function Layout() {
  const prefersDarkMode = useMediaQuery('(prefers-color-scheme: dark)');
  const [themeMode, setThemeMode] = useState(prefersDarkMode ? 'dark' : 'light');
  const modeContext = useMemo(
    () => ({
      themeMode,
      changeThemeMode: (mode) => {
        setThemeMode(mode);
      },
    }),
    [themeMode],
  );

  const theme = useMemo(() => getAppTheme(themeMode), [themeMode]);

  return (
    <ThemeModeContext.Provider value={modeContext}>
      <ThemeProvider theme={theme}>
        <CssBaseline />
        <AppBar />
        <Outlet />
      </ThemeProvider>
    </ThemeModeContext.Provider>
  );
}

const router = createBrowserRouter(
  createRoutesFromElements(
    <Route path="/" element={<Layout />}>
      <Route index element={<Home />} />
      <Route path="/settings" element={<WifiPage />} />
    </Route>,
  ),
);

function App() {
  return <RouterProvider router={router} />;
}

export default App;
