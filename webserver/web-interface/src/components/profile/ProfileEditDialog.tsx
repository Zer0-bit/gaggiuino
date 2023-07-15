import React from 'react';
import { Dialog } from '@mui/material';
import {
} from '../chart/StatBox';
import { Profile } from '../../models/profile';
import { ProfileEdit } from './ProfileEdit';

interface ProfileEditialogProps {
  open?: boolean;
  profile: Profile;
  onClose: () => void;
  onDone: (profile: Profile) => void;
}

export default function ProfileEditDialog({
  open = false, profile, onClose, onDone,
}: ProfileEditialogProps) {
  return (
    <Dialog
      open={open || false}
      onClose={() => onClose()}
      PaperProps={{
        elevation: 0,
        sx: {
          width: { xs: '100vw', sm: '90vw' },
          p: 2,
        },
      }}
    >
      <ProfileEdit profile={profile} onDone={onDone} onCancel={onClose} />
    </Dialog>
  );
}
