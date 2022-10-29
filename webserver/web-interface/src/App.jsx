import React from 'react';
import {
  createBrowserRouter,
  RouterProvider,
  createRoutesFromElements,
  Route,
  Outlet,
} from 'react-router-dom';
import Home from './pages/home/Home';
import Settings from './pages/home/Settings';
import AppBar from './components/appbar/Appbar';
import ThemeWrapper from './components/theme/ThemeWrapper';

function Layout() {
  return (
    <ThemeWrapper>
      <AppBar />
      <Outlet />
    </ThemeWrapper>
  );
}

const router = createBrowserRouter(
  createRoutesFromElements(
    <Route path="/" element={<Layout />}>
      <Route index element={<Home />} />
      <Route path="/settings" element={<Settings />} />
    </Route>,
  ),
);

function App() {
  return <RouterProvider router={router} />;
}

export default App;
