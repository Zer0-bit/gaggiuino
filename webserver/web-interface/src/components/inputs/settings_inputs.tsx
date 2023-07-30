import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import {
  Box,
  IconButton, Switch, SxProps, TextField, Theme, Typography, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2';
import React, {
  forwardRef,
  useCallback, useEffect, useMemo, useRef, useState,
} from 'react';
import { Colorful, RgbaColor, rgbaToHex } from '@uiw/react-color';
import { Variant } from '@mui/material/styles/createTypography';
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

export interface SettingsInputFieldNumberProps {
  value: number;
  onChange: (value: number) => void;
  maxDecimals?: number;
  sx?: SxProps<Theme>;
  readOnly?: boolean;
  textAlign?: string;
  onBlur?: () => void;
  disabled?: boolean
}

export const SettingsInputFieldNumber = forwardRef(({
  value,
  onChange,
  maxDecimals = 1,
  sx = {},
  readOnly = false,
  textAlign = 'left',
  onBlur = undefined,
  disabled = false,
}: SettingsInputFieldNumberProps, ref) => {
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
    onBlur && onBlur();
  }, [value, round, onBlur]);

  const handleInputValueChanged = useCallback((newValue: string) => {
    setInputValue(sanitizeNumberString(newValue, maxDecimals));
  }, [setInputValue, maxDecimals]);

  return (
    <TextField
      inputRef={ref}
      disabled={disabled}
      fullWidth
      value={inputValue}
      onChange={(e) => handleInputValueChanged(e.target.value)}
      onBlur={handleFocusLost}
      variant="outlined"
      sx={{
        '& fieldset': { border: 'none' },
        '& input': { textAlign, py: theme.spacing(0.5), px: 0 },
        ...sx,
      }}
      InputProps={{ readOnly }}
    />
  );
});

SettingsInputFieldNumber.defaultProps = {
  maxDecimals: undefined,
  sx: {},
  readOnly: false,
  textAlign: 'left',
  onBlur: undefined,
  disabled: false,
};

export interface SettingsNumberIncrementButtonsProps {
  value: number;
  onChange: (val: number) => void;
  buttonIncrements?: number;
  fontSize?: string;
}

export function SettingsNumberIncrementButtons(
  {
    value, onChange, buttonIncrements = 1, fontSize = undefined,
  } : SettingsNumberIncrementButtonsProps,
) {
  return (
    <SettingsInputActions>
      <IconButton sx={{ p: '2px', fontSize }} size="small" color="primary" onClick={() => onChange(value - buttonIncrements)}>
        <RemoveIcon fontSize="inherit" />
      </IconButton>
      <IconButton sx={{ p: '2px', fontSize }} size="small" color="primary" onClick={() => onChange(value + buttonIncrements)}>
        <AddIcon fontSize="inherit" />
      </IconButton>
    </SettingsInputActions>
  );
}

export interface SettingsNumberInputProps {
  label: string;
  value: number;
  onChange: (value: number) => void;
  buttonIncrements?: number;
  maxDecimals?: number;
  optional?: boolean;
}

export function SettingsNumberInput({
  label, value, onChange, maxDecimals = undefined, buttonIncrements = 1, optional = false,
}: SettingsNumberInputProps) {
  const [justEnabled, setJustEnabled] = useState(false);
  const inputRef = useRef<HTMLInputElement>(null); // Create a ref
  const handleInputChanged = (newValue: number) => {
    onChange(newValue);
    setJustEnabled(false);
  };
  const handleToggleSwitch = (newValue: boolean) => {
    setJustEnabled(newValue);
    setTimeout(() => inputRef.current && inputRef.current.focus(), 200);
  };
  const handleInputLostFocus = () => {
    if (justEnabled) {
      setTimeout(() => setJustEnabled(false), 200);
    }
  };

  return (
    <SettingsInputWrapper>
      <SettingsInputBorderLabel><Typography variant="caption" color="text.secondary">{label}</Typography></SettingsInputBorderLabel>
      <SettingsInputField>
        <SettingsInputFieldNumber
          ref={inputRef}
          value={value}
          disabled={optional && value === 0 && !justEnabled}
          onChange={handleInputChanged}
          maxDecimals={maxDecimals}
          onBlur={handleInputLostFocus}
        />
      </SettingsInputField>
      <SettingsInputActions>
        {(optional && value === 0 && !justEnabled)
          && (
            <DelayedSwitch
              checked={justEnabled}
              onChange={handleToggleSwitch}
            />
          )}
        {(!optional || value !== 0 || justEnabled) && (
          <SettingsNumberIncrementButtons
            value={value}
            onChange={handleInputChanged}
            buttonIncrements={buttonIncrements}
          />
        )}
      </SettingsInputActions>
    </SettingsInputWrapper>
  );
}

interface DelayedSwitchProps {
  checked: boolean;
  onChange: (checked: boolean) => void;
}

export function DelayedSwitch({ checked, onChange }: DelayedSwitchProps) {
  const [checkedInternal, setCheckedInternal] = useState(checked);
  useEffect(() => setCheckedInternal(checked), [checked]);

  const handleToggle = useCallback((value: boolean) => {
    setCheckedInternal(value);
    const timeoutId = setTimeout(() => {
      onChange(value);
    }, 150); // approximate duration of the MUI switch animation

    return () => {
      clearTimeout(timeoutId);
    };
  }, [onChange]);

  return (
    <Box>
      <Switch
        color="primary"
        size="small"
        checked={checkedInternal}
        onChange={(e) => handleToggle(e.target.checked)}
      />
    </Box>
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
  const gridTemplateColunns = `${inlineLabel ? '1fr ' : ''}${field ? '1fr ' : ''}${actions ? 'auto' : ''}`;

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

export interface SettingsInputInlineLabelProps {
  children?: React.ReactNode;
  variant?: Variant | 'inherit';
}

export function SettingsInputInlineLabel(
  { variant = 'body2', children = undefined }: SettingsInputInlineLabelProps,
) {
  return (
    <Box alignItems="center" display="flex" width="100%">
      <Typography noWrap variant={variant}>{ children }</Typography>
    </Box>
  );
}

export interface GridItemProps {
  children?: React.ReactNode;
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
