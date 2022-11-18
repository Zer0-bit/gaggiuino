import { CssBaseline, ThemeProvider, useMediaQuery } from '@mui/material';
import React, { useMemo, useState } from 'react';
import PropTypes from 'prop-types';
import getAppTheme from './AppTheme';
import { ThemeModeContext } from './ThemeModeToggle';

const SAVED_THEME_MODE_KEY = 'savedTheme';

export default function ThemeWrapper({ children }) {
  const themeModeBrowserPreference = useMediaQuery('(prefers-color-scheme: dark)') ? 'dark' : 'light';
  const savedThemeMode = localStorage.getItem(SAVED_THEME_MODE_KEY);

  const [themeMode, setThemeMode] = useState(savedThemeMode || themeModeBrowserPreference);

  const changeThemeMode = (newMode) => {
    localStorage.setItem(SAVED_THEME_MODE_KEY, newMode);
    setThemeMode(newMode);
  };

  const modeContext = useMemo(() => ({ themeMode, changeThemeMode }), [themeMode]);
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
