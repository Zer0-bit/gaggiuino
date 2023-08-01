import React, { ReactNode, useCallback, useState } from 'react';
import {
  Box, ButtonBase, alpha, useTheme,
} from '@mui/material';
import AspectRatioBox from '../layout/AspectRatioBox';

const LED_COLOR_ON = '#ef4e2b';
const GLOW_ON = alpha(LED_COLOR_ON, 0.5);
const LED_COLOR_AUTO = '#00b9ff';
const GLOW_AUTO = alpha(LED_COLOR_AUTO, 0.5);
const LED_COLOR_OFF = '#822714';
const SWITCH_BACKGROUND_ON = 'linear-gradient(to top, rgba(127, 127, 127, 0.1) 0%, rgba(0, 0, 0, 0.1) 100%)';
const SWITCH_BACKGROUND_OFF = 'linear-gradient(to top, rgba(255, 255, 255, 0.1) 0%, rgba(80, 80, 80, 0.1) 100%)';
const SWITCH_SHADOW = '0px 1px 2px -1px rgba(0,0,0,0.2), 0px 3px 3px 0px rgba(0,0,0,0.14), 0px 1px 5px 0px rgba(0,0,0,0.12)';

export enum SwitchLedState {
  ON,
  AUTO,
  OFF
}

export interface SwitchLedButtonProps {
  state: SwitchLedState;
  icon?: ReactNode;
  label?: ReactNode;
  supportsAuto?: boolean;
  longPressDuration?: number;
  onChange?: (state: SwitchLedState) => void;
}

function stateToLedColor(state:SwitchLedState) {
  if (state === SwitchLedState.ON) return LED_COLOR_ON;
  if (state === SwitchLedState.AUTO) return LED_COLOR_AUTO;
  return LED_COLOR_OFF;
}

function stateToGlowColor(state:SwitchLedState) {
  if (state === SwitchLedState.ON) return GLOW_ON;
  if (state === SwitchLedState.AUTO) return GLOW_AUTO;
  return '';
}

export function SwitchLedButton({
  state,
  icon = undefined,
  label = undefined,
  supportsAuto = false,
  longPressDuration = 1500,
  onChange = undefined,
}: SwitchLedButtonProps) {
  const theme = useTheme();
  const background = (state === SwitchLedState.OFF) ? SWITCH_BACKGROUND_OFF : SWITCH_BACKGROUND_ON;

  const [pressTimeoutId, setPressTimeoutId] = useState<NodeJS.Timeout | null>(null);
  const [longPressActive, setLongPressActive] = useState(false);

  const handleLongPress = useCallback(() => {
    if (!supportsAuto) return;
    if (state === SwitchLedState.AUTO) onChange?.(SwitchLedState.OFF);
    else onChange?.(SwitchLedState.AUTO);
  }, [state, onChange, supportsAuto]);

  const handleShortPress = useCallback(() => {
    if (state === SwitchLedState.OFF) onChange?.(SwitchLedState.ON);
    else onChange?.(SwitchLedState.OFF);
  }, [state, onChange]);

  const handlePressStart = useCallback((event: React.MouseEvent | React.TouchEvent) => {
    if (event.type === 'touchstart') {
      event.preventDefault();
    }

    if (!supportsAuto) return;

    const id = setTimeout(() => {
      setLongPressActive(true);
    }, longPressDuration);
    setPressTimeoutId(id);
  }, [longPressDuration, supportsAuto]);

  const handlePressEnd = useCallback((event: React.MouseEvent | React.TouchEvent) => {
    if (event.type === 'touchend') {
      event.preventDefault();
    }

    if (pressTimeoutId) {
      clearTimeout(pressTimeoutId);
    }
    if (longPressActive) {
      handleLongPress();
    } else {
      handleShortPress();
    }
    setLongPressActive(false);
  }, [pressTimeoutId, longPressActive, handleShortPress, handleLongPress]);

  return (
    <Box sx={{
      p: { xs: 0.5, sm: 1 },
      borderRadius: theme.spacing(1),
      background: (theme.palette.mode === 'light') ? 'rgba(0, 0, 0, 0.4)' : 'rgba(25, 25, 25, 0.2)',
      width: '100%',
      maxWidth: '120px',
      border: `1px solid ${alpha(theme.palette.divider, 0.05)}`,
      boxShadow: SWITCH_SHADOW,
    }}
    >
      <AspectRatioBox ratio={1 / 1.4}>
        <ButtonBase
          sx={{
            borderRadius: theme.spacing(1),
            boxShadow: SWITCH_SHADOW,
            background,
            border: `1px solid ${alpha(theme.palette.divider, 0.05)}`,
            ':hover': {
              border: `1px solid ${alpha(theme.palette.divider, 0.2)}`,
            },
          }}
          onMouseDown={handlePressStart}
          onMouseUp={handlePressEnd}
          onTouchStart={handlePressStart}
          onTouchEnd={handlePressEnd}
        >
          <Box sx={{
            height: '100%',
            width: '100%',
            display: 'flex',
            flexDirection: 'column',
            justifyContent: 'space-around',
            alignItems: 'center',
            color: 'rgba(255, 255, 255, 0.8)',
            py: { xs: 1, sm: 2 },
          }}
          >
            <Box sx={{ fontSize: { xs: theme.typography.body2.fontSize, sm: theme.typography.body1.fontSize } }}>
              {icon}
            </Box>
            <Box sx={{ overflowX: 'hidden', textOverflow: 'ellipsis', width: '100%' }}>{label}</Box>
            <Box sx={{
              width: '15px',
              height: '10px',
              backgroundColor: stateToLedColor(state),
              boxShadow: state !== SwitchLedState.OFF ? `0 0 5px 3px ${stateToGlowColor(state)}` : '',
            }}
            />
          </Box>
        </ButtonBase>
      </AspectRatioBox>
    </Box>
  );
}
