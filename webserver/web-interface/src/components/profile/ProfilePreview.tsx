import React, { useEffect, useState } from 'react';
import {
  Box, IconButton, Typography, useTheme,
} from '@mui/material';
import ScaleIcon from '@mui/icons-material/Scale';
import TimerIcon from '@mui/icons-material/Timer';
import WaterIcon from '@mui/icons-material/Water';
import SaveIcon from '@mui/icons-material/Save';
import EditIcon from '@mui/icons-material/Edit';
import { Profile } from '../../models/profile';
import ProfileChart from '../chart/ProfileChart';
import { formatTime } from '../../models/api';

interface GlobalRestrictionProps {
  icon: React.ElementType, value: string | undefined, color?: string
}

function GlobalRestriction({ icon: Icon, value, color = undefined }: GlobalRestrictionProps) {
  const theme = useTheme();
  const [finalColor, setFinalColor] = useState(theme.palette.text.primary);

  useEffect(() => {
    if (!value) {
      setFinalColor(theme.palette.text.disabled);
    } else if (!color) {
      setFinalColor(theme.palette.text.primary);
    } else {
      setFinalColor(color);
    }
  }, [color, value, setFinalColor, theme]);

  return (
    <Box sx={{ display: 'flex', alignItems: 'center', columnGap: theme.spacing(0.5) }}>
      <Icon sx={{ color: finalColor }} fontSize="inherit" />
      <Typography variant="caption" color={finalColor}>{value}</Typography>
    </Box>
  );
}

GlobalRestriction.defaultProps = { color: undefined };

interface RestrictionProps {
  value: number | undefined
}

function TimeRestriction({ value }: RestrictionProps) {
  return <GlobalRestriction icon={TimerIcon} value={value ? formatTime({ time: value }) : undefined} />;
}

function WeightRestriction({ value }: RestrictionProps) {
  const theme = useTheme();
  return <GlobalRestriction icon={ScaleIcon} value={value ? `${value.toFixed(0)}g` : undefined} color={theme.palette.weight.main} />;
}

function WaterPumpedRestriction({ value }: RestrictionProps) {
  const theme = useTheme();
  return <GlobalRestriction icon={WaterIcon} value={value ? `${value.toFixed(0)}ml` : undefined} color={theme.palette.water.main} />;
}

export interface ProfileReviewProps {
  profile: Profile,
  onSave?: () => void | undefined,
  onEdit?: () => void | undefined,
}

export function ProfileReview({ profile, onSave, onEdit }: ProfileReviewProps) {
  const theme = useTheme();
  return (
    <>
      <Box sx={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        flexWrap: 'wrap',
        mb: theme.spacing(2),
      }}
      >
        <Typography variant="h6">{profile.name}</Typography>
        {(onSave || onEdit) && (
        <Box sx={{ display: 'flex', alignItems: 'center', columnGap: theme.spacing(1) }}>
          {onSave && <IconButton size="small" color="inherit" onClick={onSave}><SaveIcon fontSize="inherit" /></IconButton>}
          {onEdit && <IconButton size="small" color="inherit" onClick={onEdit}><EditIcon fontSize="inherit" /></IconButton>}
        </Box>
        )}
      </Box>
      <Box position="relative"><ProfileChart profile={profile} /></Box>

      <Box sx={{
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
          <WeightRestriction value={profile.globalStopConditions?.weight} />
          <WaterPumpedRestriction value={profile.globalStopConditions?.waterPumped} />
          <TimeRestriction value={profile.globalStopConditions?.time} />
        </Box>
      </Box>
    </>
  );
}

ProfileReview.defaultProps = {
  onSave: undefined,
  onEdit: undefined,
};
