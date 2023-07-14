import React, { ReactElement } from 'react';
import {
  Paper, Typography, useTheme, Box, Stack, SxProps, Theme,
} from '@mui/material';
import TemperatureIcon from '@mui/icons-material/DeviceThermostat';
import TimerIcon from '@mui/icons-material/Timer';
import ScaleIcon from '@mui/icons-material/Scale';
import CompressIcon from '@mui/icons-material/Compress';
import AirIcon from '@mui/icons-material/Air';
import SportsScoreIcon from '@mui/icons-material/SportsScore';
import formatTime from '../../models/time_format';

function formatNumber(value: number | undefined, decimals = 1) {
  return typeof value === 'number' ? value.toFixed(decimals) : undefined;
}

function formatTarget(value: number | undefined, decimals = 1) {
  return typeof value === 'number' && value > 0 ? value.toFixed(decimals) : undefined;
}
export interface StatBoxProps {
  label: string;
  color: string;
  stat?: string;
  statTarget?: string;
  icon?: ReactElement;
  unit?: string;
  sx?: SxProps<Theme>;
}

export function StatBox({
  label, color, stat = undefined, icon = undefined, statTarget = undefined, unit = '', sx = {},
}: StatBoxProps) {
  const theme = useTheme();
  return (
    <Paper sx={{
      border: `1px solid ${color}`,
      color,
      px: theme.spacing(1),
      py: theme.spacing(0.5),
      fontSize: theme.typography.caption,
      overflow: 'hidden',
      ...sx,
    }}
    >
      <Stack direction="row" alignItems="center" sx={{ height: '100%' }}>
        {icon && (
        <Box display="flex" alignItems="center" color={color}>
          <Typography fontSize="6px">{icon}</Typography>
        </Box>
        )}
        <Box sx={{ flexGrow: 1 }}>
          <Box>
            <Typography fontSize="inherit" color={color} align="right" fontWeight="bold" noWrap>
              {label}
            </Typography>
          </Box>
          <Box display="flex" justifyContent="flex-end" alignItems="center" color={color}>
            {`${stat} ${unit || ''}`}
          </Box>
          {statTarget && statTarget >= '0' && (
            <Box display="flex" justifyContent="flex-end" alignItems="center" color={color}>
              <SportsScoreIcon fontSize="inherit" />
              {`${statTarget} ${unit}`}
            </Box>
          )}
        </Box>
      </Stack>
    </Paper>
  );
}

export function TimeStatBox({ timeInShot, sx = {} }: {timeInShot: number, sx?: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Time"
      icon={<TimerIcon />}
      color={theme.palette.text.primary}
      stat={formatTime({ time: timeInShot })}
      sx={sx}
    />
  );
}

export function WeightStatBox({
  shotWeight, target = undefined, sx = {},
}: {shotWeight: number, target?: number, sx?: SxProps<Theme> }) {
  const theme = useTheme();

  return (
    <StatBox
      label="Weight"
      icon={<ScaleIcon />}
      color={theme.palette.weight.main}
      stat={formatNumber(shotWeight)}
      statTarget={formatTarget(target)}
      unit="g"
      sx={sx}
    />
  );
}

export function TemperatureStatBox({
  temperature, target, sx = {},
}: {temperature: number, target: number, sx?: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Temp"
      icon={<TemperatureIcon />}
      color={theme.palette.temperature.main}
      stat={formatNumber(temperature)}
      statTarget={formatTarget(target)}
      unit="°C"
      sx={sx}
    />
  );
}
export function PumpFlowStatBox({
  pumpFlow, target, sx = undefined,
}: {pumpFlow: number, target: number, sx?: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Pump Flow"
      icon={<AirIcon />}
      color={theme.palette.flow.main}
      stat={formatNumber(pumpFlow)}
      statTarget={formatTarget(target)}
      unit="ml/s"
      sx={sx}
    />
  );
}

export function WeightFlowStatBox({
  flow, target, sx = undefined,
}: {flow: number, target: number, sx?: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Weight Flow"
      icon={<AirIcon />}
      color={theme.palette.weightFlow.main}
      stat={formatNumber(flow)}
      statTarget={formatTarget(target)}
      unit="ml/s"
      sx={sx}
    />
  );
}

export function PressureStatBox({
  pressure, target, sx = undefined,
}: {pressure: number, target: number, sx?: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      icon={<CompressIcon />}
      label="Pressure"
      color={theme.palette.pressure.main}
      stat={formatNumber(pressure)}
      statTarget={formatTarget(target)}
      unit="bar"
      sx={sx}
    />
  );
}
