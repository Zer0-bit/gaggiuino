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
    mode: 'dark',
    primary: {
      main: '#ef4e2b',
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
