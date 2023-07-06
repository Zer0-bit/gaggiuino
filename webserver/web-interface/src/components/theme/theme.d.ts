/* eslint-disable @typescript-eslint/no-unused-vars */
/* eslint-disable @typescript-eslint/no-empty-interface */
import {
  PaletteColor, SimplePaletteColorOptions,
} from '@mui/material/styles/createPalette';

/** add more custom colors here */
type CustomPalette = {
  appbar?: PaletteColor;
  temperature: PaletteColor;
  flow: PaletteColor;
  weightFlow: PaletteColor;
  pressure: PaletteColor;
  weight: PaletteColor;
  water: PaletteColor;
}

type CustomPaletteOptions = {
  appbar?: SimplePaletteColorOptions;
  temperature: SimplePaletteColorOptions;
  flow: SimplePaletteColorOptions;
  weightFlow: SimplePaletteColorOptions;
  pressure: SimplePaletteColorOptions;
  weight: SimplePaletteColorOptions;
  water: SimplePaletteColorOptions;
}

declare module '@mui/material/styles/createPalette' {
  interface Palette extends CustomPalette {}

  interface PaletteOptions extends CustomPaletteOptions {}
}
