import React from 'react';
import {
  Box, Typography, useTheme,
} from '@mui/material';
import ScaleIcon from '@mui/icons-material/Scale';
import TimerIcon from '@mui/icons-material/Timer';
import WaterIcon from '@mui/icons-material/Water';
import { Profile } from '../../models/profile';
import formatTime from '../../models/time_format';

export function GlobalRestrictions({ profile }: { profile: Profile }) {
  const theme = useTheme();
  const { weight, waterPumped, time } = profile.globalStopConditions || {};
  return (
    <Box
      sx={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        flexWrap: 'wrap',
        fontSize: theme.typography.caption.fontSize,
      }}
    >
      <Box sx={{ display: { xs: 'none', sm: 'block' } }}>STOP ON</Box>
      <Box sx={{
        display: 'flex', alignItems: 'center', justifyContent: { xs: 'flex-start', sm: 'flex-end' }, columnGap: theme.spacing(2), flexWrap: 'wrap',
      }}
      >
        <WeightRestriction value={weight} />
        <WaterPumpedRestriction value={waterPumped} />
        <TimeRestriction value={time} />
      </Box>
    </Box>
  );
}

export interface GlobalRestrictionProps {
  icon: React.ElementType, value: string | undefined, color?: string
}

export function GlobalRestriction({ icon: Icon, value, color = undefined }: GlobalRestrictionProps) {
  const theme = useTheme();
  const finalColor = !value ? theme.palette.text.disabled : color || theme.palette.text.primary;

  return (
    <Box sx={{ display: 'flex', alignItems: 'center', columnGap: theme.spacing(0.5) }}>
      <Icon sx={{ color: finalColor }} fontSize="inherit" />
      <Typography variant="caption" color={finalColor}>{value}</Typography>
    </Box>
  );
}

export interface RestrictionProps {
  value: number | undefined
}

export function TimeRestriction({ value }: RestrictionProps) {
  return <GlobalRestriction icon={TimerIcon} value={value ? formatTime({ time: value }) : undefined} />;
}

export function WeightRestriction({ value }: RestrictionProps) {
  const theme = useTheme();
  return <GlobalRestriction icon={ScaleIcon} value={value ? `${value.toFixed(0)}g` : undefined} color={theme.palette.weight.main} />;
}

export function WaterPumpedRestriction({ value }: RestrictionProps) {
  const theme = useTheme();
  return <GlobalRestriction icon={WaterIcon} value={value ? `${value.toFixed(0)}ml` : undefined} color={theme.palette.water.main} />;
}
