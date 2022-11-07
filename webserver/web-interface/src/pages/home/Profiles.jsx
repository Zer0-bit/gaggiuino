import React from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import UploadFileIcon from '@mui/icons-material/UploadFile';
import Grid from '@mui/material/Unstable_Grid2';
import RangeSlider from '../../components/inputs/RangeSlider';

export default function Settings() {
  const theme = useTheme();

  return (
    <div>
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
                <IconButton color="primary" aria-label="upload picture" component="label">
                  <input hidden accept="file/*.bin" type="file" />
                  <UploadFileIcon />
                </IconButton>
              </CardActions>
            </Grid>
          </Grid>
        </Card>
      </Container>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Card sx={{ mt: theme.spacing(2) }}>
          <Grid container columns={{ xs: 1, sm: 2 }}>
            <Grid item xs={1}>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  Create Profile
                </Typography>
              </CardContent>
              <CardActions>
                <Grid container columns={{ xs: 1, sm: 3 }}>
                  <Grid item xs={2}>
                    <Typography gutterBottom variant="h8">
                      Preinfusion
                    </Typography>
                    <RangeSlider />
                  </Grid>
                  <Grid item xs={3}>
                    <Typography gutterBottom variant="h8">
                      Blooming
                    </Typography>
                    <RangeSlider />
                  </Grid>
                  <Grid item xs={4}>
                    <Typography gutterBottom variant="h8">
                      Raise
                    </Typography>
                    <RangeSlider />
                  </Grid>
                </Grid>
              </CardActions>
            </Grid>
          </Grid>
        </Card>
      </Container>
    </div>
  );
}
