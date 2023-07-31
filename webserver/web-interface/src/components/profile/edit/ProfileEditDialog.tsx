import React from 'react';
import { Dialog, Slide } from '@mui/material';
import { Profile } from '../../../models/profile';
import { ProfileEdit } from './ProfileEdit';

interface ProfileEditDialogProps {
  open?: boolean;
  profile: Profile;
  onClose: () => void;
  onDone: (profile: Profile) => void;
}

export default function ProfileEditDialog({
  open = false,
  profile,
  onClose,
  onDone,
}: ProfileEditDialogProps) {
  return (
    <Dialog
      open={open}
      onClose={onClose}
      TransitionComponent={Slide}
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
