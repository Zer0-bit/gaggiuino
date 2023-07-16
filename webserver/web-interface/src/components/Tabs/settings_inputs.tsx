import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import {
  Box,
  IconButton, Switch, TextField, Typography, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2';
import React, {
  useCallback, useEffect, useMemo, useState,
} from 'react';
import { Colorful, RgbaColor, rgbaToHex } from '@uiw/react-color';
import { LedColor } from '../../models/models';
import { same, sanitizeNumberString } from '../../models/utils';

export interface SettingsToggleInputProps {
  label: string;
  value: boolean;
  onChange: (value: boolean) => void;
}

export function SettingsToggleInput({ label, value, onChange }: SettingsToggleInputProps) {
  return (
    <SettingsInputWrapper>
      <SettingsInputInlineLabel>{label}</SettingsInputInlineLabel>
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
  buttonIncrements?: number;
  maxDecimals?: number;
}

export function SettingsNumberInput({
  label, value, onChange, maxDecimals = undefined, buttonIncrements = 1,
}: SettingsNumberInputProps) {
  const theme = useTheme();
  const round = useMemo(() => (x: number) => {
    if (maxDecimals === undefined) return x;
    const scale = 10 ** maxDecimals;
    return Math.round(x * scale) / scale;
  }, [maxDecimals]);

  const [inputValue, setInputValue] = useState<string>(sanitizeNumberString(round(value).toString()));

  const handleValueChange = useCallback((newNumericValue: number) => {
    const roundedNumber = round(newNumericValue);

    // Ignore update if the value hasn't changed
    if (!same(value, roundedNumber)) {
      onChange(roundedNumber);
    }
  }, [onChange, value, round]);

  useEffect(() => setInputValue(round(value).toString()), [value, round]);

  useEffect(
    () => {
      const number = parseFloat(inputValue || '');
      if (Number.isNaN(number) || same(number, value)) return;
      handleValueChange(number);
    },
    // eslint-disable-next-line react-hooks/exhaustive-deps
    [inputValue], // Remove handleValueChange dependency. Important to avoid infinite loop.
  );

  const handleFocusLost = useCallback(() => {
    setInputValue(sanitizeNumberString(round(value).toString()));
  }, [value, round]);

  const handleInputValueChanged = useCallback((newValue: string) => {
    setInputValue(sanitizeNumberString(newValue, maxDecimals));
  }, [setInputValue, maxDecimals]);

  return (
    <SettingsInputWrapper>
      <SettingsInputBorderLabel><Typography variant="caption" color="text.secondary">{label}</Typography></SettingsInputBorderLabel>
      <SettingsInputField>
        <TextField
          fullWidth
          value={inputValue}
          onChange={(e) => handleInputValueChanged(e.target.value)}
          onBlur={handleFocusLost}
          variant="outlined"
          sx={{
            '& fieldset': { border: 'none' },
            '& input': { textAlign: 'left', py: theme.spacing(0.5), px: 0 },
          }}
          InputProps={{ readOnly: false }}
        />
      </SettingsInputField>
      <SettingsInputActions>
        <IconButton size="small" color="primary" onClick={() => handleValueChange(value - buttonIncrements)}>
          <RemoveIcon />
        </IconButton>
        <IconButton size="small" color="primary" onClick={() => handleValueChange(value + buttonIncrements)}>
          <AddIcon />
        </IconButton>
      </SettingsInputActions>
    </SettingsInputWrapper>
  );
}

export interface LedColorInputProps {
  label: string;
  value: LedColor;
  onChange: (value: LedColor) => void;
}

export function LedColorPickerInput({ label, value, onChange }: LedColorInputProps) {
  const color = {
    r: value.R, g: value.G, b: value.B, a: 1,
  };

  const updateLedColor = useCallback(
    (inputColor: RgbaColor) => {
      onChange({ R: inputColor.r, G: inputColor.g, B: inputColor.b });
    },
    [onChange],
  );

  return (
    <SettingsInputWrapper>
      <SettingsInputBorderLabel>{label}</SettingsInputBorderLabel>
      <Grid xs={12} sx={{ mt: 1 }}>
        <Colorful
          style={{ width: '100%' }}
          color={rgbaToHex(color)}
          disableAlpha
          onChange={(newColor) => {
            updateLedColor(newColor.rgba);
          }}
        />
      </Grid>
    </SettingsInputWrapper>
  );
}

export function SettingsInputWrapper({ children }: {children: React.ReactNode}) {
  const childrenArray = React.Children.toArray(children);

  // Check if child is a valid React element and its type is SettingsInputLabel
  const borderLabel = childrenArray.find((c) => React.isValidElement(c) && c.type === SettingsInputBorderLabel);
  const inlineLabel = childrenArray.find((c) => React.isValidElement(c) && c.type === SettingsInputInlineLabel);
  const field = childrenArray.find((c) => React.isValidElement(c) && c.type === SettingsInputField);
  const actions = childrenArray.find((c) => React.isValidElement(c) && c.type === SettingsInputActions);
  const otherChildern = childrenArray.filter((c) => [borderLabel, inlineLabel, field, actions].indexOf(c) === -1);

  const theme = useTheme();

  // See https://css-tricks.com/snippets/css/complete-guide-grid for CSS Grid
  const gridTemplateColunns = `${inlineLabel ? '1fr ' : ''}${field ? '1fr ' : ''}${actions ? '50px ' : ''}`;

  return (
    <Box
      sx={{
        position: 'relative',
        width: '100%',
        p: theme.spacing(1),
      }}
    >
      {borderLabel && <Box sx={{ position: 'absolute', top: -14, left: theme.spacing(1) }}>{borderLabel}</Box>}
      <Box sx={{
        display: 'grid',
        width: '100%',
        px: theme.spacing(0.5),
        gridTemplateColumns: gridTemplateColunns,
      }}
      >
        {inlineLabel}
        {field}
        {actions}
      </Box>
      {otherChildern}
      <fieldset style={{
        position: 'absolute',
        margin: 0,
        padding: 0,
        top: borderLabel ? -12 : 0,
        right: 0,
        bottom: 0,
        left: 0,
        border: `1px solid ${theme.palette.divider}`,
        borderRadius: theme.spacing(1),
        pointerEvents: 'none',
      }}
      >
        {borderLabel && (
        <legend style={{
          visibility: 'hidden',
          padding: 0,
          float: 'none',
          margin: 0,
          marginLeft: theme.spacing(1),
          textOverflow: 'ellipsis',
          whiteSpace: 'nowrap',
          overflow: 'hidden',
          maxWidth: '50%',
        }}
        >
          {borderLabel}
        </legend>
        )}
      </fieldset>
    </Box>
  );
}

export interface GridItemProps {
  children?: React.ReactNode;
}

export function SettingsInputInlineLabel({ children = undefined }: GridItemProps) {
  return (
    <Box alignItems="center" display="flex" width="100%">
      <Typography noWrap variant="body2">{ children }</Typography>
    </Box>
  );
}

export function SettingsInputBorderLabel({ children = undefined }: GridItemProps) {
  const theme = useTheme();
  return (
    <Typography
      variant="caption"
      sx={{
        px: theme.spacing(0.5),
        textOverflow: 'ellipsis',
        whiteSpace: 'nowrap',
        overflow: 'hidden',
        maxWidth: '50%',
      }}
    >
      { children }

    </Typography>
  );
}

export function SettingsInputField({ children = undefined }: GridItemProps) {
  return <Box alignItems="center" display="flex" width="100%">{ children }</Box>;
}

export function SettingsInputActions({ children = undefined }: GridItemProps) {
  return <Box alignItems="center" display="flex" justifyContent="end">{ children }</Box>;
}
