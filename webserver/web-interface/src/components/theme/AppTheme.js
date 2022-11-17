import { createTheme } from '@mui/material';

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
      default: '#F5F5F5',
    },
    temperature: {
      main: '#ef4040',
    },
    flow: {
      main: '#844B48',
    },
    pressure: {
      main: '#6296C5',
    },
    weight: {
      main: '#ffb53e',
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
      main: '#844B48',
    },
    pressure: {
      main: '#6296C5',
    },
    weight: {
      main: '#ffb53e',
    },
  },
});

export default function getAppTheme(mode) {
  return (mode === 'light') ? lightTheme : darkTheme;
}
