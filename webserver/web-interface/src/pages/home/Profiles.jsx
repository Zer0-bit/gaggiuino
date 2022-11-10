import React, { useEffect, useState } from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions,
} from '@mui/material';
import useWebSocket from 'react-use-websocket';
import IconButton from '@mui/material/IconButton';
import QrCodeIcon from '@mui/icons-material/QrCode';
import UploadFileIcon from '@mui/icons-material/UploadFile';
import TextField from '@mui/material/TextField';
import Grid from '@mui/material/Unstable_Grid2';
import Chart from '../../components/chart/ShotChart';
import AddPhaseButton from '../../components/inputs/AddPhase';

export default function Settings() {
  const theme = useTheme();
  const { lastJsonMessage } = useWebSocket(`ws://${window.location.host}/ws`, {
    share: true,
  });

  const [sensorData, setSensorData] = useState([]);

  useEffect(() => {
    if (lastJsonMessage !== null && lastJsonMessage.action === 'sensor_data_update') {
      setSensorData((prev) => {
        if (prev.length >= 400) {
          prev.shift();
        }
        return prev.concat(lastJsonMessage.data);
      });
    }
  }, [lastJsonMessage]);

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
                <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                  <input hidden accept=".bin" type="file" />
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
        <Card sx={{ mt: theme.spacing(2) }}>
          <Grid container columns={{ xs: 1, sm: 1 }}>
            <Grid item xs={1}>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  Profile preview
                  <div style={{ float: 'right' }}>
                    <TextField id="outlined-basic" size="small" label="Time" variant="outlined" sx={{ mr: theme.spacing(4) }} />
                    <TextField id="outlined-basic" size="small" label="Value" variant="outlined" sx={{ mr: theme.spacing(4) }} />
                    <AddPhaseButton style={{ float: 'right' }} sx={{ ml: theme.spacing(2) }} />
                  </div>
                </Typography>
              </CardContent>
              <CardActions>
                <Grid container columns={{ xs: 1, sm: 1 }} position="relative" width="1550px" height={400}>
                  <Chart data={sensorData} />
                </Grid>
              </CardActions>
            </Grid>
          </Grid>
        </Card>
      </Container>
    </div>
  );
}
