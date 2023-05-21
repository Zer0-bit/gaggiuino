import React from 'react';
import {
  Paper, Typography, useTheme, Box, Stack,
} from '@mui/material';
import TemperatureIcon from '@mui/icons-material/DeviceThermostat';
import TimerIcon from '@mui/icons-material/Timer';
import ScaleIcon from '@mui/icons-material/Scale';
import CompressIcon from '@mui/icons-material/Compress';
import AirIcon from '@mui/icons-material/Air';
import SportsScoreIcon from '@mui/icons-material/SportsScore';
import { formatTimeInShot } from '../../models/api';

function formatNumber(value, decimals = 1) {
  return typeof value === 'number' ? value.toFixed(decimals) : undefined;
}

export function StatBox({
  label, color, stat, icon, statTarget, unit, sx, style,
}) {
  const theme = useTheme();
  return (
    <Paper sx={{ border: `2px solid ${color}`, padding: theme.spacing(1), ...sx }} style={style}>
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
            <Typography color={color} align="right">
              {`${stat} ${unit || ''}`}
            </Typography>
          </Box>
          {statTarget && statTarget >= 0 && (
            <Box display="flex" justifyContent="flex-end" alignItems="center" color={color}>
              <SportsScoreIcon fontSize="6px" />
              {`${statTarget} ${unit}`}
            </Box>
          )}
        </Box>
      </Stack>
    </Paper>
  );
}

export function TimeStatBox({ timeInShot, sx, style }) {
  const theme = useTheme();

  return (
    <StatBox
      label="Time"
      icon={<TimerIcon />}
      color={theme.palette.text.primary}
      stat={formatTimeInShot(timeInShot)}
      sx={sx}
      style={style}
    />
  );
}

export function WeightStatBox({
  shotWeight, target, sx, style,
}) {
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
      style={style}
    />
  );
}

export function TemperatureStatBox({
  temperature, target, sx, style,
}) {
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
      style={style}
    />
  );
}

export function PumpFlowStatBox({
  pumpFlow, target, sx, style,
}) {
  const theme = useTheme();

  return (
    <StatBox
      label="Flow"
      icon={<AirIcon />}
      color={theme.palette.flow.main}
      stat={formatNumber(pumpFlow)}
      statTarget={formatNumber(target)}
      unit="ml/s"
      sx={sx}
      style={style}
    />
  );
}

export function PressureStatBox({
  pressure, target, sx, style,
}) {
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
      style={style}
    />
  );
}
