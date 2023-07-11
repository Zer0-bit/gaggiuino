import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import {
  Box,
  IconButton, Switch, TextField, Typography, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2';
import React, { useCallback } from 'react';
import { Colorful, RgbaColor, rgbaToHex } from '@uiw/react-color';
import { LedColor } from '../../models/models';

export interface SettingsToggleInputProps {
  label: string;
  value: boolean;
  onChange: (value: boolean) => void;
}

export function SettingsToggleInput({ label, value, onChange }: SettingsToggleInputProps) {
  return (
    <SettingsInputWrapper>
      <SettingsInputLabel><Typography>{label}</Typography></SettingsInputLabel>
      <SettingsInputField />
      <SettingsInputActions>
        <Switch checked={value} onChange={(e) => onChange(e.target.checked)} />
      </SettingsInputActions>
    </SettingsInputWrapper>
  );
}

export interface SettingsNumberInputProps {
  label: string;
  value: number;
  onChange: (value: number) => void;
  fractionDigits?: number;
}

export function SettingsNumberInput({
  label, value, onChange, fractionDigits,
}: SettingsNumberInputProps) {
  const theme = useTheme();
  const scale = 10.0 ** (fractionDigits || 0);

  return (
    <SettingsInputWrapper>
      <SettingsInputLabel><Typography>{label}</Typography></SettingsInputLabel>
      <SettingsInputField>
        <TextField
          value={value.toFixed(fractionDigits)}
          onChange={(e) => onChange(parseInt(e.target.value, 10))}
          variant="outlined"
          sx={{
            width: '7ch',
            '& fieldset': { border: 'none' },
            '& input': { textAlign: 'center', p: theme.spacing(0.5) },
          }}
          InputProps={{ readOnly: true }}
        />
      </SettingsInputField>
      <SettingsInputActions>
        <IconButton size="small" color="primary" onClick={() => onChange(value - (1.0 / scale))}>
          <RemoveIcon />
        </IconButton>
        <IconButton size="small" color="primary" onClick={() => onChange(value + (1.0 / scale))}>
          <AddIcon />
        </IconButton>
      </SettingsInputActions>
    </SettingsInputWrapper>
  );
}

SettingsNumberInput.defaultProps = { fractionDigits: 0 };

export interface LedColorInputProps {
  label: string;
  value: LedColor;
  onChange: (value: LedColor) => void;
}

export function LedColorPickerInput({ label, value, onChange }: LedColorInputProps) {
  const theme = useTheme();

  const color = {
    r: value.R, g: value.G, b: value.B, a: 1,
  };

  const updateLedColor = useCallback(
    (inputColor: RgbaColor) => {
      onChange({ R: inputColor.r, G: inputColor.g, B: inputColor.b });
    },
    [onChange],
  );

  // useEffect(() => updateLedColor(color), [updateLedColor, color]);

  return (
    <SettingsInputWrapper>
      <SettingsInputLabel><Typography>{label}</Typography></SettingsInputLabel>
      <Colorful
        style={{ width: '100%', marginTop: theme.spacing(1) }}
        color={rgbaToHex(color)}
        disableAlpha
        onChange={(newColor) => {
          updateLedColor(newColor.rgba);
        }}
      />
    </SettingsInputWrapper>
  );
}

export function SettingsInputWrapper({ children }: {children: React.ReactNode}) {
  const theme = useTheme();
  return (
    <Box
      sx={{
        p: theme.spacing(1),
        border: `1px solid ${theme.palette.divider}`,
        borderRadius: theme.spacing(1),
        width: '100%',
        gap: theme.spacing(2),
      }}
    >
      <Grid container width="100%">
        {children}
      </Grid>
    </Box>
  );
}

export interface GridItemProps {
  children?: React.ReactNode;
}

export function SettingsInputLabel({ children }: GridItemProps) {
  return (
    <Grid xs={5} sx={{ display: 'flex', alignItems: 'center' }}>
      {children}
    </Grid>
  );
}
SettingsInputLabel.defaultProps = { children: undefined };

export function SettingsInputField({ children }: GridItemProps) {
  return (
    <Grid xs={3} alignItems="center" display="flex">
      {children}
    </Grid>
  );
}
SettingsInputField.defaultProps = { children: undefined };

export function SettingsInputActions({ children }: GridItemProps) {
  return (
    <Grid xs={4} alignItems="center" display="flex" justifyContent="end">
      {children}
    </Grid>
  );
}
SettingsInputActions.defaultProps = { children: undefined };
