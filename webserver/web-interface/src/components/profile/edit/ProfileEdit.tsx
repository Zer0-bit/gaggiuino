import React, {
  useCallback, useEffect, useState,
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
import GlobeIcon from '@mui/icons-material/Public';
import PreviousIcon from '@mui/icons-material/NavigateBefore';
import {
  CurveStyle, Phase, PhaseType, Profile,
} from '../../../models/profile';
import ProfileChart from '../../chart/ProfileChart';
import { GlobalRestrictions } from '../GlobalRestrictions';
import PhaseEdit from './PhaseEdit';
import { getIndexInRange } from '../../../models/utils';
import { ProfileGlobalsEdit } from './ProfileGlobalsEdit';

export interface ProfileEditProps {
  profile: Profile,
  onDone?: (profile: Profile) => void,
  onCancel?: () => void,
}

export function ProfileEdit({ profile, onDone = undefined, onCancel = undefined }: ProfileEditProps) {
  const [editingProfile, setEditingProfile] = useState(profile);
  const [phaseIndexSelected, setPhaseIndexSelected] = useState<number |undefined>(undefined);
  const [globalsSectionVisible, setGlobalsSectionVisible] = useState(true);

  const handlePhaseIndexSelected = useCallback((index: number | undefined) => {
    setPhaseIndexSelected(index);
    if (index !== undefined) {
      setGlobalsSectionVisible(false);
    }
  }, []);

  const handleGlobalsVisibleChange = useCallback((state: boolean) => {
    setGlobalsSectionVisible(state);
    if (state) {
      setPhaseIndexSelected(undefined);
    }
  }, []);

  useEffect(() => {
    setEditingProfile(profile);
    handlePhaseIndexSelected(undefined);
  }, [profile, setEditingProfile, handlePhaseIndexSelected]);

  const deletePhase = useCallback(
    (phaseIndex: number) => {
      const newPhases = [...editingProfile.phases];
      newPhases.splice(phaseIndex, 1);
      setEditingProfile({ ...editingProfile, phases: newPhases });
      handlePhaseIndexSelected(getIndexInRange(phaseIndex - 1, newPhases));
    },
    [setEditingProfile, editingProfile, handlePhaseIndexSelected],
  );

  const createNewPhase = useCallback(
    (creationIndex: number) => {
      const index = Math.min(editingProfile.phases.length, Math.max(0, creationIndex)); // ensure valid range
      const newPhase:Phase = {
        skip: false,
        type: PhaseType.PRESSURE,
        target: { end: 9, curve: CurveStyle.INSTANT, time: 0 },
        stopConditions: { time: 5000 },
      };
      const newPhases = [...editingProfile.phases.slice(0, index), newPhase, ...editingProfile.phases.slice(index)];
      setEditingProfile({ ...editingProfile, phases: newPhases });
      handlePhaseIndexSelected(getIndexInRange(index, newPhases));
    },
    [setEditingProfile, editingProfile, handlePhaseIndexSelected],
  );

  const updatePhase = useCallback((index: number, phase:Phase) => {
    if (index < 0 || index >= editingProfile.phases.length) return;
    const phases = [...editingProfile.phases];
    phases[index] = phase;
    setEditingProfile({ ...editingProfile, phases });
  }, [editingProfile, setEditingProfile]);

  return (
    <>
      <ProfileEditTitle profile={editingProfile} onUpdated={setEditingProfile} onDone={onDone} onCancel={onCancel} />
      <>
        <Box position="relative">
          <ProfileChart
            profile={editingProfile}
            selectedPhaseIndex={phaseIndexSelected}
            onSelectPhase={handlePhaseIndexSelected}
          />
        </Box>
        <GlobalRestrictions profile={editingProfile} />
      </>
      <PhaseManagementBar
        profile={editingProfile}
        phaseIndexSelected={phaseIndexSelected}
        onCreatePhase={createNewPhase}
        onDeletePhase={deletePhase}
        onGlobalClicked={() => handleGlobalsVisibleChange(!globalsSectionVisible)}
        onUpdatePhaseIndexSelected={handlePhaseIndexSelected}
      />
      {globalsSectionVisible && (
        <GlobalsSection
          profile={editingProfile}
          onUpdated={setEditingProfile}
          onClose={() => handleGlobalsVisibleChange(false)}
        />
      )}
      {phaseIndexSelected !== undefined && (
        <PhaseEditingSection
          title={`Phase ${phaseIndexSelected + 1}`}
          phase={editingProfile.phases[phaseIndexSelected]}
          onClose={() => handlePhaseIndexSelected(undefined)}
          onUpdatePhase={(newPhase) => updatePhase(phaseIndexSelected, newPhase)}
        />
      )}
    </>
  );
}

interface PhaseManagementBarProps {
  profile: Profile;
  phaseIndexSelected: number | undefined;
  onUpdatePhaseIndexSelected: (newValue: number | undefined) => void;
  onCreatePhase: (index: number) => void;
  onDeletePhase: (index: number) => void;
  onGlobalClicked: () => void;
}

function PhaseManagementBar(
  {
    profile, phaseIndexSelected, onUpdatePhaseIndexSelected, onCreatePhase, onDeletePhase, onGlobalClicked,
  }: PhaseManagementBarProps,
) {
  const theme = useTheme();

  const nextPhase = useCallback(
    () => onUpdatePhaseIndexSelected(
      getIndexInRange(phaseIndexSelected === undefined ? 0 : phaseIndexSelected + 1, profile.phases),
    ),
    [profile, phaseIndexSelected, onUpdatePhaseIndexSelected],
  );

  const previousPhase = useCallback(
    () => onUpdatePhaseIndexSelected(
      getIndexInRange(phaseIndexSelected === undefined ? 0 : phaseIndexSelected - 1, profile.phases),
    ),
    [profile, phaseIndexSelected, onUpdatePhaseIndexSelected],
  );

  return (
    <Box sx={{
      display: 'flex', gap: theme.spacing(1), alignItems: 'center', justifyContent: 'space-between',
    }}
    >
      {profile.phases.length > 1 && (<Button variant="outlined" onClick={previousPhase}><PreviousIcon /></Button>)}
      <Box display="flex" flexGrow="1" justifyContent="center">
        <IconButton color="info" onClick={() => onGlobalClicked()}><GlobeIcon /></IconButton>
        {phaseIndexSelected !== undefined && (
        <>
          <IconButton color="error" onClick={() => onDeletePhase(phaseIndexSelected)}><DeleteIcon /></IconButton>
          <IconButton color="success" onClick={() => onCreatePhase(phaseIndexSelected + 1)}><CreateIcon /></IconButton>
        </>
        )}
        {((profile.phases.length === 0)) && (
          <IconButton color="success" onClick={() => onCreatePhase(profile.phases.length)}><CreateIcon /></IconButton>
        )}
      </Box>
      {profile.phases.length > 1 && (<Button variant="outlined" onClick={nextPhase}><NextIcon /></Button>)}
    </Box>
  );
}

interface ProfileEditTitleProps extends ProfileEditProps {
  onUpdated: (profile:Profile) => void,
}

function ProfileEditTitle(
  {
    profile,
    onUpdated,
    onDone = undefined,
    onCancel = undefined,
  }: ProfileEditTitleProps,
) {
  const theme = useTheme();
  const updateName = useCallback((name:string) => onUpdated({ ...profile, name }), [profile, onUpdated]);

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
        <TextField
          variant="standard"
          value={profile.name}
          onChange={(e) => updateName(e.target.value)}
          sx={{
            '& .MuiInputBase-root': {
              fontSize: '1.5rem',
            },
          }}
        >
        </TextField>
      </Typography>
      <Box sx={{ display: 'flex', alignItems: 'center', columnGap: theme.spacing(1) }}>
        {onDone && <IconButton color="success" onClick={() => onDone(profile)}><DoneIcon fontSize="inherit" /></IconButton>}
        {onCancel && <IconButton color="error" onClick={onCancel}><CancelIcon fontSize="inherit" /></IconButton>}
      </Box>
    </Box>
  );
}

interface PhaseEditingSectionProps {
  title: string
  phase: Phase;
  onClose: () => void;
  onUpdatePhase: (phase: Phase) => void;
}

function PhaseEditingSection({
  title, phase, onClose, onUpdatePhase,
}: PhaseEditingSectionProps) {
  const theme = useTheme();
  return (
    <Box sx={{
      mt: theme.spacing(2),
      background: alpha(phase.type === PhaseType.FLOW
        ? theme.palette.flow.main : theme.palette.pressure.main, 0.1),
      p: 2,
      borderRadius: theme.spacing(2),
    }}
    >
      <Box sx={{
        mb: 1, display: 'flex', alignItems: 'center', justifyContent: 'space-between',
      }}
      >
        <Typography>{title}</Typography>
        <IconButton size="small" onClick={onClose}><CloseIcon /></IconButton>
      </Box>
      <PhaseEdit phase={phase} onChange={onUpdatePhase} />
    </Box>
  );
}

interface GlobalsSectionProps {
  profile: Profile
  onClose: () => void;
  onUpdated: (profile: Profile) => void;
}

function GlobalsSection({ profile, onUpdated, onClose }: GlobalsSectionProps) {
  const theme = useTheme();
  return (
    <Box sx={{
      mt: theme.spacing(2),
      background: alpha(theme.palette.weightFlow.main, 0.1),
      p: 2,
      borderRadius: theme.spacing(2),
    }}
    >
      <Box sx={{
        mb: 2, display: 'flex', alignItems: 'center', justifyContent: 'space-between',
      }}
      >
        <Typography>Global profile settings</Typography>
        <IconButton size="small" onClick={onClose}><CloseIcon /></IconButton>
      </Box>
      <ProfileGlobalsEdit profile={profile} onChange={onUpdated} />
    </Box>
  );
}
