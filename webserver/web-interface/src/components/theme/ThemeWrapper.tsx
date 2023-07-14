import {
  CssBaseline, PaletteMode, ThemeProvider, useMediaQuery,
} from '@mui/material';
import React, { ReactNode, useMemo, useState } from 'react';
import getAppTheme from './AppTheme';
import { ThemeModeContext } from './ThemeModeToggle';

const SAVED_THEME_MODE_KEY = 'savedTheme';

export default function ThemeWrapper({ children = undefined }: { children: ReactNode}) {
  const themeModeBrowserPreference = useMediaQuery('(prefers-color-scheme: dark)') ? 'dark' : 'light';
  const savedThemeMode = localStorage.getItem(SAVED_THEME_MODE_KEY) as (PaletteMode | null);

  const [themeMode, setThemeMode] = useState(savedThemeMode || themeModeBrowserPreference);

  const changeThemeMode = (newMode: PaletteMode) => {
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
