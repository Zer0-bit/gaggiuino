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
import { formatTime } from '../../models/api';

function formatNumber(value: number | undefined, decimals = 1) {
  return typeof value === 'number' ? value.toFixed(decimals) : undefined;
}

export interface StatBoxProps {
  label: string;
  color: string;
  stat?: string;
  statTarget?: string;
  icon?: ReactElement;
  unit?: string;
  sx: SxProps<Theme>;
}

export function StatBox({
  label, color, stat, icon, statTarget, unit, sx,
}: StatBoxProps) {
  const theme = useTheme();
  return (
    <Paper sx={{ border: `2px solid ${color}`, padding: theme.spacing(1), ...sx }}>
      <Stack direction="row" alignContent="stretch">
        {icon && (
        <Box display="flex" alignItems="center" color={color}>
          {icon}
        </Box>
        )}
        <Box sx={{ flexGrow: 1 }}>
          <Box>
            <Typography color={color} align="right" sx={{ fontWeight: 'bold' }}>
              {label}
            </Typography>
          </Box>
          <Box>
            <Typography sx={{ color }} align="right">
              {`${stat} ${unit || ''}`}
            </Typography>
          </Box>
          {statTarget && statTarget >= '0' && (
            <Box display="flex" justifyContent="flex-end" alignItems="center" color={color}>
              <SportsScoreIcon sx={{ fontSize: '6px' }} />
              {`${statTarget} ${unit}`}
            </Box>
          )}
        </Box>
      </Stack>
    </Paper>
  );
}

StatBox.defaultProps = {
  stat: undefined,
  icon: undefined,
  statTarget: undefined,
  unit: '',
};

export function TimeStatBox({ timeInShot, sx }: {timeInShot: number, sx: SxProps<Theme>}) {
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
  shotWeight, target, sx,
}: {shotWeight: number, target?: number, sx: SxProps<Theme> }) {
  const theme = useTheme();

  return (
    <StatBox
      label="Weight"
      icon={<ScaleIcon />}
      color={theme.palette.weight.main}
      stat={formatNumber(shotWeight)}
      statTarget={formatNumber(target)}
      unit="g"
      sx={sx}
    />
  );
}
WeightStatBox.defaultProps = { target: undefined };

export function TemperatureStatBox({
  temperature, target, sx,
}: {temperature: number, target: number, sx: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Temp"
      icon={<TemperatureIcon />}
      color={theme.palette.temperature.main}
      stat={formatNumber(temperature)}
      statTarget={formatNumber(target)}
      unit="°C"
      sx={sx}
    />
  );
}

export function PumpFlowStatBox({
  pumpFlow, target, sx,
}: {pumpFlow: number, target: number, sx: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Pump Flow"
      icon={<AirIcon />}
      color={theme.palette.flow.main}
      stat={formatNumber(pumpFlow)}
      statTarget={formatNumber(target)}
      unit="ml/s"
      sx={sx}
    />
  );
}

export function WeightFlowStatBox({
  flow, target, sx,
}: {flow: number, target: number, sx: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Weight Flow"
      icon={<AirIcon />}
      color={theme.palette.weightFlow.main}
      stat={formatNumber(flow)}
      statTarget={formatNumber(target)}
      unit="ml/s"
      sx={sx}
    />
  );
}

export function PressureStatBox({
  pressure, target, sx,
}: {pressure: number, target: number, sx: SxProps<Theme>}) {
  const theme = useTheme();

  return (
    <StatBox
      icon={<CompressIcon />}
      label="Pressure"
      color={theme.palette.pressure.main}
      stat={formatNumber(pressure)}
      statTarget={formatNumber(target)}
      unit="bar"
      sx={sx}
    />
  );
}
