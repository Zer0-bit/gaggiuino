import React from 'react';
import {
  createBrowserRouter,
  RouterProvider,
  createRoutesFromElements,
  Route,
  Outlet,
} from 'react-router-dom';
import { createTheme, CssBaseline, ThemeProvider } from '@mui/material';
import Home from './pages/home/Home';
import WifiPage from './components/wifi/WifiPage';
import AppBar from './components/appbar/Appbar';

const theme = createTheme({
  palette: {
    mode: 'light',
    primary: {
      main: '#ef4e2b',
    },
    secondary: {
      main: '#0288d1',
    },
    appbar: {
      main: '#272727',
    },
  },
});

function Layout() {
  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <AppBar />
      <Outlet />
    </ThemeProvider>
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
