import React from 'react';
import {
  Box,
  Button,
  Dialog, DialogActions, DialogContent, DialogTitle, LinearProgress, Typography,
} from '@mui/material';
import { DescalingProgress, DescalingState } from '../../models/models';
import formatTime from '../../models/time_format';

interface DescalingDialogProps {
  data: DescalingProgress;
  open: boolean;
  onClose: () => void;
}

const descriptions = {
  [DescalingState.IDLE]: 'Waiting. Activate Brew button to begin.',
  [DescalingState.PHASE1]: 'Slowly penetrating that scale.',
  [DescalingState.PHASE2]: 'Softening that scale.',
  [DescalingState.PHASE3]: 'Killing that scale big time.',
  [DescalingState.FINISHED]: 'Finished! Scale exterminated.',
};

function DescalingDialog({ data, open, onClose }:DescalingDialogProps) {
  const { state, time, progress } = data;

  return (
    <Dialog
      open={open}
      onClose={onClose}
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
      <DialogTitle>Descaling Progress</DialogTitle>
      <DialogContent>
        <Typography variant="body1">
          Currently:
          {' '}
          {descriptions[state]}
        </Typography>
        <Typography variant="body2">
          Time:
          {' '}
          {formatTime({ time })}
        </Typography>
        <Box display="flex" alignItems="center" flexDirection="column" sx={{ pt: 2 }}>
          <LinearProgress
            sx={{
              width: '100%',
              height: '20px',
              borderRadius: 5,
            }}
            variant="determinate"
            value={progress}
          />
          <Typography variant="body2">
            {progress}
            %
          </Typography>
        </Box>
      </DialogContent>
      <DialogActions>
        <Button onClick={onClose}>Close</Button>
      </DialogActions>
    </Dialog>
  );
}

export default DescalingDialog;
