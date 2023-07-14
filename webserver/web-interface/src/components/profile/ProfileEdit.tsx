import React, {
  useCallback, useEffect, useMemo, useState,
} from 'react';
import {
  Box, Button, IconButton, TextField, Typography, alpha, useTheme,
} from '@mui/material';
import DoneIcon from '@mui/icons-material/Check';
import CancelIcon from '@mui/icons-material/CancelOutlined';
import CloseIcon from '@mui/icons-material/Close';
import NextIcon from '@mui/icons-material/NavigateNext';
import DeleteIcon from '@mui/icons-material/Delete';
import CreateIcon from '@mui/icons-material/AddCircle';
import PreviousIcon from '@mui/icons-material/NavigateBefore';
import {
  CurveStyle, Phase, PhaseType, Profile,
} from '../../models/profile';
import ProfileChart from '../chart/ProfileChart';
import { GlobalRestrictions } from './GlobalRestrictions';
import PhaseEditor from './PhaseEditor';

export interface ProfileEditProps {
  profile: Profile,
  onDone?: (profile: Profile) => void,
  onCancel?: () => void,
}

function getIndexInRange(desiredIndex: number | undefined, array: unknown[]): number | undefined {
  if (array.length === 0 || desiredIndex === undefined) {
    return undefined;
  }
  return Math.max(0, Math.min(array.length - 1, desiredIndex));
}

export function ProfileEdit({ profile, onDone = undefined, onCancel = undefined }: ProfileEditProps) {
  const theme = useTheme();
  const [editingProfile, setEditingProfile] = useState(profile);
  const [phaseIndexSelected, setPhaseIndexSelected] = useState<number |undefined>(undefined);

  useEffect(() => {
    setEditingProfile(profile);
    setPhaseIndexSelected(undefined);
  }, [profile, setEditingProfile, setPhaseIndexSelected]);

  const phaseEditing = useMemo(
    () => (phaseIndexSelected !== undefined ? editingProfile.phases[phaseIndexSelected] : undefined),
    [editingProfile, phaseIndexSelected],
  );

  const nextPhase = useCallback(
    () => setPhaseIndexSelected(
      getIndexInRange(phaseIndexSelected === undefined ? 0 : phaseIndexSelected + 1, editingProfile.phases),
    ),
    [editingProfile, phaseIndexSelected],
  );

  const previousPhase = useCallback(
    () => setPhaseIndexSelected(
      getIndexInRange(phaseIndexSelected === undefined ? 0 : phaseIndexSelected - 1, editingProfile.phases),
    ),
    [editingProfile, phaseIndexSelected],
  );

  const deletePhase = useCallback(
    (phaseIndex: number) => {
      const newPhases = [...editingProfile.phases];
      newPhases.splice(phaseIndex, 1);
      setEditingProfile({ ...editingProfile, phases: newPhases });
      setPhaseIndexSelected(getIndexInRange(phaseIndex - 1, newPhases));
    },
    [setEditingProfile, editingProfile],
  );

  const createNewPhase = useCallback(
    (creationIndex: number) => {
      const index = Math.min(editingProfile.phases.length, Math.max(0, creationIndex)); // ensure valid range
      const newPhase:Phase = {
        skip: false,
        type: PhaseType.PRESSURE,
        target: { end: 9, curve: CurveStyle.INSTANT, time: 5000 },
        stopConditions: { time: 5000 },
      };
      const newPhases = [...editingProfile.phases.slice(0, index), newPhase, ...editingProfile.phases.slice(index)];
      setEditingProfile({ ...editingProfile, phases: newPhases });
      setPhaseIndexSelected(getIndexInRange(index, newPhases));
    },
    [setEditingProfile, editingProfile],
  );

  const updatePhase = useCallback((index: number, phase:Phase) => {
    if (index < 0 || index >= editingProfile.phases.length) return;
    const phases = [...editingProfile.phases];
    phases[index] = phase;
    setEditingProfile({ ...editingProfile, phases });
  }, [editingProfile, setEditingProfile]);

  const updateName = (name:string) => setEditingProfile({ ...editingProfile, name });

  return (
    <>
      <Box>
        {ProfileEditTitle(editingProfile, updateName, onDone, onCancel)}
        <Box position="relative">
          <ProfileChart
            profile={editingProfile}
            selectedPhaseIndex={phaseIndexSelected}
            onSelectPhase={setPhaseIndexSelected}
          />
        </Box>
        <GlobalRestrictions profile={profile} />
      </Box>
      <Box sx={{
        display: 'flex', gap: theme.spacing(1), alignItems: 'center', justifyContent: 'space-between', mt: theme.spacing(2),
      }}
      >
        {editingProfile.phases.length > 1 && (<Button variant="outlined" onClick={previousPhase}><PreviousIcon /></Button>)}
        {phaseIndexSelected !== undefined && (
        <Box display="flex" flexGrow="1" justifyContent="center">
          <IconButton color="success" onClick={() => createNewPhase(phaseIndexSelected)}><CreateIcon /></IconButton>
          <IconButton color="error" onClick={() => deletePhase(phaseIndexSelected)}><DeleteIcon /></IconButton>
          <IconButton color="success" onClick={() => createNewPhase(phaseIndexSelected + 1)}><CreateIcon /></IconButton>
        </Box>
        )}
        {((editingProfile.phases.length === 0) || (phaseIndexSelected === undefined)) && (
        <Box display="flex" flexGrow="1" justifyContent="center">
          <IconButton color="success" onClick={() => createNewPhase(editingProfile.phases.length)}><CreateIcon /></IconButton>
        </Box>
        )}
        {editingProfile.phases.length > 1 && (<Button variant="outlined" onClick={nextPhase}><NextIcon /></Button>)}
      </Box>
      {phaseEditing && phaseIndexSelected !== undefined && (
      <Box sx={{
        mt: theme.spacing(2),
        background: alpha(phaseEditing.type === PhaseType.FLOW
          ? theme.palette.flow.main : theme.palette.pressure.main, 0.1),
        p: 2,
        borderRadius: theme.spacing(2),
      }}
      >
        <Box sx={{
          mb: 1, display: 'flex', alignItems: 'center', justifyContent: 'space-between',
        }}
        >
          <Typography>
            {`Phase ${phaseIndexSelected + 1}`}
          </Typography>
          <IconButton size="small" onClick={() => setPhaseIndexSelected(undefined)}><CloseIcon /></IconButton>
        </Box>
        <PhaseEditor phase={phaseEditing} onChange={(phase) => updatePhase(phaseIndexSelected, phase)} />
      </Box>
      )}
    </>
  );
}

function ProfileEditTitle(
  profile: Profile,
  onNameUpdated: (name:string) => void,
  onDone?: (profile: Profile) => void,
  onCancel?: () => void,
) {
  const theme = useTheme();
  return (
    <Box sx={{
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'space-between',
      flexWrap: 'wrap',
      mb: theme.spacing(2),
    }}
    >
      <Typography variant="h6">
        <TextField variant="standard" value={profile.name || 'New profile'} onChange={(e) => onNameUpdated(e.target.value)}></TextField>
      </Typography>
      <Box sx={{ display: 'flex', alignItems: 'center', columnGap: theme.spacing(1) }}>
        {onDone && <IconButton size="small" color="success" onClick={() => onDone(profile)}><DoneIcon fontSize="inherit" /></IconButton>}
        {onCancel && <IconButton size="small" color="error" onClick={onCancel}><CancelIcon fontSize="inherit" /></IconButton>}
      </Box>
    </Box>
  );
}
