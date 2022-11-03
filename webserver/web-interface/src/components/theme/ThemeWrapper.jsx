import { CssBaseline, ThemeProvider, useMediaQuery } from '@mui/material';
import React, { useMemo, useState } from 'react';
import PropTypes from 'prop-types';
import getAppTheme from './AppTheme';
import { ThemeModeContext } from './ThemeModeToggle';

export default function ThemeWrapper({ children }) {
  const themeSelection = localStorage.getItem('savedTheme');
  const prefersDarkMode = useMediaQuery(`(prefers-color-scheme: ${themeSelection})`);
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
  localStorage.setItem('savedTheme', themeMode);
  const theme = useMemo(() => getAppTheme(themeMode), [themeMode]);

  return (
    <ThemeModeContext.Provider value={modeContext}>
      <ThemeProvider theme={theme}>
        <CssBaseline />
        {children}
      </ThemeProvider>
    </ThemeModeContext.Provider>
  );
}

ThemeWrapper.propTypes = {
  children: PropTypes.node,
};

ThemeWrapper.defaultProps = {
  children: undefined,
};
