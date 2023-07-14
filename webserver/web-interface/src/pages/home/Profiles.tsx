import React, { useState } from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions, Paper, TextareaAutosize, Alert,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import QrCodeIcon from '@mui/icons-material/QrCode';
import UploadFileIcon from '@mui/icons-material/UploadFile';
// import InputAdornment from '@mui/material/InputAdornment';
import Grid from '@mui/material/Grid';
import ProfileChart from '../../components/chart/ProfileChart';
import { Profile } from '../../models/profile';
import { ProfileEdit } from '../../components/profile/ProfileEdit';

export default function Profiles() {
  const theme = useTheme();
  const [selectedPhaseIndex, setSelectedPhaseIndex] = useState<number| undefined>(undefined);

  const [error, setError] = useState<string>();
  const [profile, setProfile] = useState<Profile>({
    name: '',
    phases: [],
    waterTemperature: 93,
  });

  const updateProfile = (value: string) => {
    try {
      const newProfile = JSON.parse(value) as Profile;
      if (!Array.isArray(newProfile.phases)
      || newProfile.name === undefined
      || newProfile.waterTemperature === undefined) {
        throw Error('Invalid syntax');
      }
      setProfile(newProfile);
      setError(undefined);
    } catch (er: unknown) {
      if (er instanceof Error) {
        setError(er.message);
      } else {
        setError('An unknown error occurred.');
      }
    }
  };

  return (
    <div>

      <Container sx={{ mt: theme.spacing(2) }}>
        <Paper sx={{ mt: theme.spacing(2), p: theme.spacing(2) }}>
          <ProfileEdit profile={profile} onDone={console.log} onCancel={console.log} />
        </Paper>
      </Container>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Card sx={{ mt: theme.spacing(2) }}>
          <Grid container columns={{ xs: 1, sm: 2 }}>
            <Grid item xs={1}>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  Load Profile
                </Typography>
              </CardContent>
              <CardActions>
                <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                  <input hidden accept=".json" type="file" onChange={(evt) => updateProfile(evt.target.value)} />
                  <UploadFileIcon fontSize="large" />
                </IconButton>
                <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label">
                  <input hidden accept=".png" type="file" />
                  <QrCodeIcon fontSize="large" />
                </IconButton>
              </CardActions>
            </Grid>
          </Grid>
        </Card>
      </Container>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Paper sx={{ mt: theme.spacing(2), p: theme.spacing(2) }}>
          <Typography variant="h5" sx={{ mb: theme.spacing(2) }}>
            Profile syntax playground
          </Typography>
          <Grid container spacing={2}>
            <Grid item xs={12}>
              <Alert severity={error ? 'error' : 'success'}>
                {error || 'Nice syntax!'}
              </Alert>
            </Grid>
            <Grid item xs={6}>
              <TextareaAutosize
                minRows={15}
                maxRows={15}
                onChange={(evt) => updateProfile(evt.target.value)}
                style={{
                  color: theme.palette.text.secondary,
                  width: '100%',
                  backgroundColor: theme.palette.background.paper,
                }}
              />
            </Grid>
            <Grid item xs={6} position="relative" height="400">
              <ProfileChart
                profile={profile}
                selectedPhaseIndex={selectedPhaseIndex}
                onSelectPhase={setSelectedPhaseIndex}
              />
            </Grid>
          </Grid>
        </Paper>
      </Container>
    </div>
  );
}
