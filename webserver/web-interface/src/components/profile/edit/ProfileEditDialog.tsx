import React from 'react';
import { Dialog } from '@mui/material';
import { Profile } from '../../../models/profile';
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
          borderRadius: { xs: 2, sm: 4 },
          width: '100%',
          maxHeight: '100vh',
          m: { xs: 1, sm: 4 },
          p: 2,
        },
      }}
    >
      <ProfileEdit profile={profile} onDone={onDone} onCancel={onClose} />
    </Dialog>
  );
}
