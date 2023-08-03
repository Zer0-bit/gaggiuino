import React, { useCallback, useState } from 'react';
import {
  Box, IconButton, Typography, useTheme,
} from '@mui/material';
import SaveIcon from '@mui/icons-material/Save';
import EditIcon from '@mui/icons-material/Edit';
import { Profile } from '../../models/profile';
import ProfileChart from '../chart/ProfileChart';
import { GlobalRestrictions } from './GlobalRestrictions';
import ProfileEditDialog from './edit/ProfileEditDialog';

export interface ProfileReviewProps {
  profile: Profile;
  onSave?: () => void;
  onChange?: (profile: Profile) => void;
}

export function ProfileReview({ profile, onSave = undefined, onChange = undefined }: ProfileReviewProps) {
  const theme = useTheme();
  const [editOpen, setEditOpen] = useState(false);

  const handleDone = useCallback((newProfile: Profile) => {
    setEditOpen(false);
    onChange?.(newProfile);
  }, [onChange]);

  return (
    <Box sx={{
      width: '100%',
    }}
    >
      <Box sx={{
        width: '100%',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'space-between',
        flexWrap: 'wrap',
        mb: theme.spacing(2),
      }}
      >
        <Typography variant="h6">{profile.name}</Typography>
        {(onSave || onChange) && (
          <Box sx={{ display: 'flex', alignItems: 'center', columnGap: theme.spacing(1) }}>
            {onSave && <IconButton size="small" color="inherit" onClick={onSave}><SaveIcon fontSize="inherit" /></IconButton>}
            {onChange && <IconButton size="small" color="inherit" onClick={() => setEditOpen(true)}><EditIcon fontSize="inherit" /></IconButton>}
          </Box>
        )}
      </Box>
      <Box position="relative" width="100%"><ProfileChart profile={profile} /></Box>
      <GlobalRestrictions profile={profile} />
      {onChange && (
        <ProfileEditDialog open={editOpen} onClose={() => setEditOpen(false)} profile={profile} onDone={handleDone} />
      )}
    </Box>
  );
}
