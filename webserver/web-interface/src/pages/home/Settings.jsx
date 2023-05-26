import React from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions, Button,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import AttachFileOutlinedIcon from '@mui/icons-material/AttachFileOutlined';
import Grid from '@mui/material/Unstable_Grid2';
import { useWebSocket } from 'react-use-websocket/dist/lib/use-websocket';
import WifiSettingsCard from '../../components/wifi/WifiSettingsCard';
import ProgressBar from '../../components/inputs/ProgressBar';
import LogContainer from '../../components/log/LogContainer';
import {
  MSG_TYPE_LOG, apiHost, filterSocketMessage,
} from '../../models/api';

export default function Settings() {
  const theme = useTheme();
  const { lastJsonMessage } = useWebSocket(`ws://${apiHost}/ws`, {
    share: true,
    retryOnError: true,
    shouldReconnect: () => true,
    reconnectAttempts: 1000,
    filter: (message) => filterSocketMessage(message, MSG_TYPE_LOG),
  });

  return (
    <div>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Grid container columns={{ xs: 1, sm: 2 }} spacing={2}>
          <Grid item xs={1}>
            <WifiSettingsCard />
          </Grid>
          <Grid item xs={1}>
            <Card>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  OTA Update
                </Typography>
              </CardContent>
              <CardActions><ProgressBar /></CardActions>
              <CardActions>
                <IconButton color="primary" aria-label="download  " component="label">
                  <input hidden accept=".bin" type="file" />
                  <AttachFileOutlinedIcon />
                </IconButton>
                <Button variant="contained" component="label">
                  Upload
                </Button>
              </CardActions>
            </Card>
          </Grid>
          <Grid item xs={1} sm={2}>
            <LogContainer logLine={lastJsonMessage && lastJsonMessage.data} />
          </Grid>
        </Grid>
      </Container>
    </div>
  );
}
