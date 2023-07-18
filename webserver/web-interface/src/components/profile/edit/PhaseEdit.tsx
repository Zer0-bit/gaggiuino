import React, {} from 'react';
import {
  Box, MenuItem, TextField, ToggleButton, ToggleButtonGroup, Typography, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2/Grid2';
import {
  CurveStyle, Phase, PhaseStopConditions, PhaseType, Transition,
} from '../../../models/profile';
import { SettingsNumberInput } from '../../inputs/settings_inputs';
import { constrain } from '../../../models/utils';

export interface PhaseEditProps {
  phase: Phase;
  onChange: (phase: Phase) => void;
}

function getNewTargetForTime(target:Transition, time: number): Transition {
  if (time === 0 || target.curve === CurveStyle.INSTANT) {
    return { ...target, time: 0, curve: CurveStyle.INSTANT };
  }
  return { ...target, time };
}

export function PhaseEdit({ phase, onChange }: PhaseEditProps) {
  const theme = useTheme();
  const handleTypeChange = (newType: PhaseType) => onChange({ ...phase, type: newType });
  const handleRestrictionChange = (restriction: number) => onChange({ ...phase, restriction });
  const handleTargetValueChange = (value: number | CurveStyle, key: keyof Transition) => onChange(
    { ...phase, target: { ...phase.target, [key]: value } },
  );
  const handleStopConditionChange = (value: number, key: keyof PhaseStopConditions) => onChange(
    { ...phase, stopConditions: { ...phase.stopConditions, [key]: value } },
  );

  const handleCurveChange = (value: CurveStyle) => {
    const time = value === CurveStyle.INSTANT ? 0 : phase.target.time || phase.stopConditions.time || 0;
    onChange({ ...phase, target: { ...phase.target, time, curve: value } });
  };

  // Managing curve style when transition time changes to/from zero
  const handleTransitionTimeChange = (newTime: number) => {
    onChange({ ...phase, target: getNewTargetForTime(phase.target, newTime * 1000) });
  };

  const handleStopTimeChange = (newTime: number) => {
    const time = newTime * 1000;

    // If there is a stop on time, also update the transition time
    if (time !== 0) {
      onChange({
        ...phase,
        target: getNewTargetForTime(phase.target, time),
        stopConditions: { ...phase.stopConditions, time },
      });
    } else {
      handleStopConditionChange(time, 'time');
    }
  };

  return (
    <Grid container spacing={2}>
      <Grid container xs={12}>
        <Grid xs={6}><PhaseTypeToggle value={phase.type} onChange={handleTypeChange} /></Grid>
      </Grid>
      <Grid container xs={12} sm={6} spacing={2} alignContent="flex-start">
        <Grid xs={12}><Typography variant="body1">Targets</Typography></Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            label={phase.type === PhaseType.FLOW ? 'Flow' : 'Pressure'}
            value={phase.target.end}
            maxDecimals={1}
            onChange={(v) => handleTargetValueChange(constrain(v, 0, 15), 'end')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label={phase.type === PhaseType.FLOW ? 'Pressure limit' : 'Flow limit'}
            value={phase.restriction || 0}
            maxDecimals={1}
            onChange={(v) => handleRestrictionChange(constrain(v, 0, 10))}
          />
        </Grid>
        <Grid xs={12}>
          <SettingsNumberInput
            optional
            label="Time(s)"
            maxDecimals={0}
            value={(phase.stopConditions.time || 0) / 1000}
            onChange={(v) => handleStopTimeChange(constrain(v, 0, 1000))}
          />
        </Grid>
        <Grid xs={12}><Typography variant="body1">Transition</Typography></Grid>
        <Grid xs={phase.target.curve === CurveStyle.INSTANT ? 12 : 6}>
          <TextField
            fullWidth
            size="small"
            label="Curve"
            select
            sx={{ '& > div': { py: theme.spacing(0.6) } }}
            value={phase.target.curve}
            onChange={(e) => handleCurveChange(e.target.value as CurveStyle)}
          >
            {Object.values(CurveStyle).map((value) => (
              <MenuItem key={value} value={value}>
                {value}
              </MenuItem>
            ))}
          </TextField>
        </Grid>
        {phase.target.curve !== CurveStyle.INSTANT && (
        <Grid xs={6}>
          <SettingsNumberInput
            label="Curve time(s)"
            value={(phase.target.time || 0) / 1000}
            maxDecimals={0}
            onChange={(v) => handleTransitionTimeChange(constrain(v, 0, 1000))}
          />
        </Grid>
        )}

      </Grid>
      <Grid container xs={12} sm={6} spacing={2} alignContent="flex-start">
        <Grid xs={12}><Typography variant="body1">Stop conditions</Typography></Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Pressure above"
            maxDecimals={1}
            value={(phase.stopConditions.pressureAbove || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'pressureAbove')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Pressure below"
            maxDecimals={1}
            value={(phase.stopConditions.pressureBelow || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'pressureBelow')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Flow above"
            maxDecimals={1}
            value={(phase.stopConditions.flowAbove || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'flowAbove')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Flow below"
            maxDecimals={1}
            value={(phase.stopConditions.flowBelow || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'flowBelow')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Weight change"
            maxDecimals={1}
            value={(phase.stopConditions.weight || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'weight')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Water pumped"
            maxDecimals={1}
            value={(phase.stopConditions.waterPumpedInPhase || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'waterPumpedInPhase')}
          />
        </Grid>
      </Grid>
    </Grid>
  );
}

export default PhaseEdit;

interface PhaseTypeToggleProps {
  value: PhaseType;
  onChange: (value: PhaseType) => void;
}

function PhaseTypeToggle({ value, onChange }: PhaseTypeToggleProps) {
  return (
    <Box sx={{ width: '100%' }}>
      <ToggleButtonGroup
        color="primary"
        value={value}
        exclusive
        onChange={(e, newValue: PhaseType) => onChange(newValue)}
        fullWidth
      >
        <ToggleButton key={PhaseType.PRESSURE} value={PhaseType.PRESSURE}>PRESSURE</ToggleButton>
        <ToggleButton key={PhaseType.FLOW} value={PhaseType.FLOW}>FLOW</ToggleButton>
      </ToggleButtonGroup>
    </Box>
  );
}
