import React from 'react';
import {
  Box, IconButton, Typography, useTheme,
} from '@mui/material';
import SaveIcon from '@mui/icons-material/Save';
import EditIcon from '@mui/icons-material/Edit';
import { Profile } from '../../models/profile';
import ProfileChart from '../chart/ProfileChart';
import { GlobalRestrictions } from './GlobalRestrictions';

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
      <GlobalRestrictions profile={profile} />
    </>
  );
}

ProfileReview.defaultProps = {
  onSave: undefined,
  onEdit: undefined,
};
