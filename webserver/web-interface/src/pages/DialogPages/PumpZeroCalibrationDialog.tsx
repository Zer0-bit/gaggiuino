import React from 'react';
import {
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  Typography,
} from '@mui/material';

interface CalibrationDialogProps {
  open: boolean;
  onClose: () => void;
}

function PzCalibrationDialog({
  open,
  onClose,
}: CalibrationDialogProps) {
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
      <DialogTitle style={{ textAlign: 'center', fontSize: '1.5rem', fontWeight: 'bold', padding: '20px' }}>
        Pump Zero Calibration
      </DialogTitle>
      <DialogContent>
        <Box display="flex" gap={5} alignItems="center" mt={1}>
          <Typography variant="body1">
            Pump Zero calibration is manual for the time being.
            <br />
            For calibration instructions check
            {' '}
            <a href="https://discord.gg/eJTDJA3xfh" style={{ color: 'Orange' }}>Discord</a>
            .
            <br />
            <br />
            Pump Zero is an important parameter to tune correctly, as it&apos;s directly responsible for
            any and all flow-related calculations, as well as for accurately calculating any predictive scales output.
            <br />
            <br />
          </Typography>
        </Box>
      </DialogContent>
      <DialogActions>
        <Box style={{ display: 'flex', gap: '10px' }}>
          <Button variant="outlined" onClick={onClose}>Close</Button>
        </Box>
      </DialogActions>
    </Dialog>
  );
}

export default PzCalibrationDialog;
