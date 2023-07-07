import { PaletteMode, createTheme } from '@mui/material';

const lightTheme = createTheme({
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
    background: {
      default: '#E0E0E0',
      paper: '#FFF',
    },
    temperature: {
      main: '#ef4040',
    },
    flow: {
      main: '#ff9d00',
    },
    weightFlow: {
      main: '#1d7835',
    },
    pressure: {
      main: '#6296C5',
    },
    weight: {
      main: '#844B48',
    },
    water: {
      main: '#178bca',
    },
  },
});

const darkTheme = createTheme({
  palette: {
    mode: 'dark',
    primary: {
      main: '#ef4e2b',
    },
    secondary: {
      main: '#0288d1',
    },
    temperature: {
      main: '#ef4040',
    },
    flow: {
      main: '#ffb53e',
    },
    weightFlow: {
      main: '#1d7835',
    },
    pressure: {
      main: '#6296C5',
    },
    weight: {
      main: '#a75c58',
    },
    background: {
      default: '#222',
      paper: '#232323',
    },
    water: {
      main: '#178bca',
    },
  },
});

export default function getAppTheme(mode: PaletteMode) {
  return (mode === 'light') ? lightTheme : darkTheme;
}
