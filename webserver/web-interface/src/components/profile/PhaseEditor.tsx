import React, {} from 'react';
import {
  Box, MenuItem, TextField, ToggleButton, ToggleButtonGroup, Typography, useTheme,
} from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2/Grid2';
import {
  CurveStyle, Phase, PhaseStopConditions, PhaseType, Transition,
} from '../../models/profile';
import { SettingsNumberInput } from '../Tabs/settings_inputs';
import { constrain } from '../../models/utils';

export interface PhaseEditorProps {
  phase: Phase;
  onChange: (phase: Phase) => void;
}

export function PhaseEditor({ phase, onChange }: PhaseEditorProps) {
  const theme = useTheme();
  const handleTypeChange = (newType: PhaseType) => onChange({ ...phase, type: newType });
  const handleRestrictionChange = (restriction: number) => onChange({ ...phase, restriction });
  const handleTargetValueChange = (value: number | CurveStyle, key: keyof Transition) => onChange(
    { ...phase, target: { ...phase.target, [key]: value } },
  );
  const handleStopConditionChange = (value: number, key: keyof PhaseStopConditions) => onChange(
    { ...phase, stopConditions: { ...phase.stopConditions, [key]: value } },
  );

  return (
    <Grid container spacing={2}>
      <Grid container xs={12} sm={6} spacing={2} alignContent="flex-start">
        <Grid xs={12}><Typography variant="body2">Targets</Typography></Grid>
        <Grid xs={12}><PhaseTypeToggle value={phase.type} onChange={handleTypeChange} /></Grid>
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
            label={phase.type === PhaseType.FLOW ? 'Pressure limit' : 'Flow limit'}
            value={phase.restriction || 0}
            maxDecimals={1}
            onChange={(v) => handleRestrictionChange(constrain(v, 0, 10))}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            label="Time(s)"
            value={(phase.target.time || 0) / 1000}
            maxDecimals={0}
            onChange={(v) => handleTargetValueChange(constrain(v, 0, 1000) * 1000, 'time')}
          />
        </Grid>
        <Grid xs={6}>
          <TextField
            fullWidth
            size="small"
            label="Curve"
            select
            sx={{ '& > div': { py: theme.spacing(0.7) } }}
            value={phase.target.curve}
            onChange={(e) => handleTargetValueChange(e.target.value as CurveStyle, 'curve')}
          >
            {Object.values(CurveStyle).map((value) => (
              <MenuItem key={value} value={value}>
                {value}
              </MenuItem>
            ))}
          </TextField>
        </Grid>
      </Grid>
      <Grid container xs={12} sm={6} spacing={2} alignContent="flex-start">
        <Grid xs={12}><Typography variant="body2">Phase stop conditions</Typography></Grid>
        <Grid xs={12}>
          <SettingsNumberInput
            label="Time(s)"
            maxDecimals={0}
            value={(phase.stopConditions.time || 0) / 1000}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 1000) * 1000, 'time')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            label="Pressure above"
            maxDecimals={1}
            value={(phase.stopConditions.pressureAbove || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'pressureAbove')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            label="Pressure below"
            maxDecimals={1}
            value={(phase.stopConditions.pressureBelow || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'pressureBelow')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            label="Flow above"
            maxDecimals={1}
            value={(phase.stopConditions.flowAbove || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'flowAbove')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            label="Flow below"
            maxDecimals={1}
            value={(phase.stopConditions.flowBelow || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'flowBelow')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            label="Weight change"
            maxDecimals={1}
            value={(phase.stopConditions.weight || 0)}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 15), 'weight')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
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

export default PhaseEditor;

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
        {Object.values(PhaseType).map((type) => (
          <ToggleButton key={type} value={type}>
            {type}
          </ToggleButton>
        ))}
      </ToggleButtonGroup>
    </Box>
  );
}
