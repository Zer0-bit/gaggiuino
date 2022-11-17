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
import TabsBar from './components/appbar/TabsBar';
import ThemeWrapper from './components/theme/ThemeWrapper';

function Layout() {
  return (
    <ThemeWrapper>
      <TabsBar />
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
  return <RouterProvider router={router} />;
}

export default App;
