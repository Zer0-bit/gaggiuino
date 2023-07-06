import { PaletteMode, createTheme } from '@mui/material';

declare module '@mui/material/styles' {

  interface Palette {
    appbar?: Palette['primary'],
    temperature: Palette['primary'],
    flow: Palette['primary'],
    weightFlow: Palette['primary'],
    pressure: Palette['primary'],
    weight: Palette['primary'],
    water: Palette['primary'],
  }

  interface PaletteOptions {
    appbar?: PaletteOptions['primary'],
    temperature: PaletteOptions['primary'],
    flow: PaletteOptions['primary'],
    weightFlow: PaletteOptions['primary'],
    pressure: PaletteOptions['primary'],
    weight: PaletteOptions['primary'],
    water: Palette['primary'],
  }
}

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
      default: '#111',
      paper: '#111',
    },
    water: {
      main: '#178bca',
    },
  },
});

export default function getAppTheme(mode: PaletteMode) {
  return (mode === 'light') ? lightTheme : darkTheme;
}
