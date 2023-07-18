import React from 'react';
import { Typography } from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2/Grid2';
import { Profile, GlobalStopConditions } from '../../../models/profile';
import { SettingsNumberInput } from '../../inputs/settings_inputs';
import { constrain } from '../../../models/utils';

export interface ProfileGlobalsEditProps {
  profile: Profile;
  onChange: (profile: Profile) => void;
}

export function ProfileGlobalsEdit({ profile, onChange }: ProfileGlobalsEditProps) {
  const handleStopConditionChange = (value: number, key: keyof GlobalStopConditions) => onChange(
    { ...profile, globalStopConditions: { ...profile.globalStopConditions, [key]: value } },
  );
  const handleWaterTempChange = (value: number) => onChange(
    { ...profile, waterTemperature: value },
  );

  return (
    <Grid container spacing={2}>
      <Grid container xs={12} spacing={2} alignContent="flex-start">
        <Grid xs={12} sm={6}>
          <SettingsNumberInput
            label="Water temp"
            value={profile.waterTemperature}
            maxDecimals={1}
            onChange={(v) => handleWaterTempChange(constrain(v, 0, 105))}
          />
        </Grid>
      </Grid>
      <Grid container xs={12} spacing={2} alignContent="flex-start">
        <Grid xs={12}><Typography variant="body2">STOP ON</Typography></Grid>
        <Grid xs={12} sm={6}>
          <SettingsNumberInput
            optional
            label="Shot Weight"
            value={profile.globalStopConditions?.weight || 0}
            maxDecimals={1}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 300), 'weight')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Total water pumped"
            value={profile.globalStopConditions?.waterPumped || 0}
            maxDecimals={1}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 300), 'waterPumped')}
          />
        </Grid>
        <Grid xs={6}>
          <SettingsNumberInput
            optional
            label="Total time (s)"
            value={(profile.globalStopConditions?.time || 0) / 1000}
            maxDecimals={1}
            onChange={(v) => handleStopConditionChange(constrain(v, 0, 1000) * 1000, 'time')}
          />
        </Grid>
      </Grid>
    </Grid>
  );
}
