import React from 'react';
import { Container, useTheme } from '@mui/material';
import Grid from '@mui/material/Unstable_Grid2';
import WifiSettingsCard from '../../components/wifi/WifiSettingsCard';

export default function Settings() {
  const theme = useTheme();

  return (
    <Container sx={{ mt: theme.spacing(2) }}>
      <Grid container columns={{ xs: 1, sm: 2 }}>
        <Grid item xs={1}>
          <WifiSettingsCard />
        </Grid>
      </Grid>
    </Container>
  );
}
