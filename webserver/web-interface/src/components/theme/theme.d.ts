/* eslint-disable @typescript-eslint/no-unused-vars */
/* eslint-disable @typescript-eslint/no-empty-interface */
import {
  Palette,
  PaletteOptions,
} from '@mui/material/styles/createPalette';

/** add more custom colors here */
type CustomPalette = {
  appbar?: { main: string };
  temperature: { main: string };
  flow: { main: string };
  weightFlow: { main: string };
  pressure: { main: string };
  weight: { main: string };
}

declare module '@mui/material/styles/createPalette' {
  interface Palette extends CustomPalette {}

  interface PaletteOptions extends CustomPalette {}
}
