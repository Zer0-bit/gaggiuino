import React from 'react';
import {
  createBrowserRouter,
  RouterProvider,
  createRoutesFromElements,
  Route,
  Outlet,
} from 'react-router-dom';
import Home from './pages/home/Home';
import Profiles from './pages/home/Profiles';
import Settings from './pages/home/Settings';
import MainAppBar from './components/appbar/MainAppBar';
import ThemeWrapper from './components/theme/ThemeWrapper';
import useWebSocket from './api/websocket';

function Layout() {
  return (
    <ThemeWrapper>
      <MainAppBar />
      <Outlet />
    </ThemeWrapper>
  );
}

const router = createBrowserRouter(
  createRoutesFromElements(
    <Route path="/" element={<Layout />}>
      <Route index element={<Home />} />
      <Route path="/profiles" element={<Profiles />} />
      <Route path="/settings" element={<Settings />} />
    </Route>,
  ),
);

function App() {
  return (
    <>
      <WebSocket />
      <RouterProvider router={router} />
    </>
  );
}

function WebSocket() {
  useWebSocket(`ws://${window.location.host}/ws`);
  return null;
}

export default App;
