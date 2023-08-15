import React, { useCallback } from 'react';
import {
  Box,
  Button,
  Dialog,
  DialogActions,
  DialogContent,
  DialogTitle,
  debounce,
} from '@mui/material';
import { SettingsNumberInput, SettingsNumber } from '../../components/inputs/settings_inputs';
import useSensorStateStore from '../../state/SensorStateStore';
import { updateTarePending } from '../../components/client/SystemStateClient';
import useSettingsStore from '../../state/SettingsStore';
import useSystemStateStore from '../../state/SystemStateStore';
import useNotificationStore from '../../state/NotificationDataStore';
import { NotificationType } from '../../models/models';

interface CalibrationDialogProps {
  open: boolean;
  onClose: () => void;
  scalesF1: number;
  scalesF2: number;
  onScalesF1Change: (newValue: number) => void;
  onScalesF2Change: (newValue: number) => void;
}

function ScalesCalibrationDialog({
  open,
  onClose,
  scalesF1,
  scalesF2,
  onScalesF1Change,
  onScalesF2Change,
}: CalibrationDialogProps) {
  // const [fakeVal, setValue] = useState(0);
  const currentWeight = useSensorStateStore((state) => state.sensorState.weight);
  const { persistSettings } = useSettingsStore();
  const updateLocalSystemState = useSystemStateStore((state) => state.updateLocalSystemState);
  const { updateLatestNotification } = useNotificationStore();

  function setValue() {
    return currentWeight;
  }

  async function handlePersistSettings() {
    try {
      await persistSettings();
      updateLatestNotification({ message: 'Successfully persisted settings', type: NotificationType.SUCCESS });
      onClose();
    } catch (e) {
      updateLatestNotification({ message: 'Failed to persisted settings', type: NotificationType.ERROR });
    }
  }

  // eslint-disable-next-line react-hooks/exhaustive-deps
  const handleTare = useCallback(debounce(async () => {
    try {
      const systemState = await updateTarePending(true);
      updateLocalSystemState(systemState);
    } catch (e) {
      updateLatestNotification({ message: 'Failed to tare', type: NotificationType.ERROR });
    }
  }, 250), []);

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
        Scales Calibration
      </DialogTitle>
      <DialogContent>
        <Box display="flex" gap={5} alignItems="center" mt={1}>
          <SettingsNumberInput
            label="Loadcell 1 Calibration Factor"
            value={scalesF1}
            maxDecimals={0}
            onChange={(newValue) => onScalesF1Change(newValue)}
          />
          <SettingsNumberInput
            label="Loadcell 2 Calibration Factor"
            value={scalesF2}
            maxDecimals={0}
            onChange={(newValue) => onScalesF2Change(newValue)}
          />
        </Box>
        <Box display="flex" alignItems="center" mt={5}>
          <SettingsNumber
            label="Weight Value"
            value={parseFloat(currentWeight.toFixed(1))}
            maxDecimals={1}
            onChange={setValue}
          />
        </Box>
      </DialogContent>
      <DialogActions>
        <Button variant="contained" onClick={handleTare} style={{ marginRight: 'auto' }}>
          Tare
        </Button>
        <Box style={{ display: 'flex', gap: '10px' }}>
          <Button variant="outlined" onClick={handlePersistSettings}>Save</Button>
          <Button variant="outlined" onClick={onClose}>Close</Button>
        </Box>
      </DialogActions>
    </Dialog>
  );
}

export default ScalesCalibrationDialog;
