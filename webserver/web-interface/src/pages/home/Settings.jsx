import React from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions, Button, Radio, RadioGroup, FormControlLabel
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import AttachFileOutlinedIcon from '@mui/icons-material/AttachFileOutlined';
import Grid from '@mui/material/Grid';
import WifiSettingsCard from '../../components/wifi/WifiSettingsCard';
import ProgressBar from '../../components/inputs/ProgressBar';
import LogContainer from '../../components/log/LogContainer';

export default function Settings() {
  const theme = useTheme();

  return (
    <div>
      <Container sx={{ mt: theme.spacing(1) }}>
        <Grid container columns={12} spacing={2} alignItems="stretch">
          <Grid item xs={4}>
            <WifiSettingsCard />
          </Grid>
          <Grid item xs={8} sm={8}>
            <Card sx={{ height: '100%' }}>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  OTA Update
                </Typography>
              </CardContent>
              <CardActions><ProgressBar /></CardActions>
              <CardActions>
                <IconButton color="primary" aria-label="download  " component="label" size='large'>
                  <input hidden accept=".bin" type="file" />
                  <AttachFileOutlinedIcon />
                </IconButton>
                <RadioGroup aria-labelledby="demo-radio-buttons-group-label" defaultValue="blackpill" name="radio-buttons-group">
                  <FormControlLabel value="blackpill" control={<Radio />} label="STM32-Blackpill" />
                  <FormControlLabel value="esp32" control={<Radio />} label="ESP32-S3" />
                </RadioGroup>
                <Button variant="contained" component="label">
                  Upload
                </Button>
              </CardActions>
            </Card>
          </Grid>
          <Grid item xs={12} sm={12}>
            <LogContainer />
          </Grid>
        </Grid>
      </Container>
    </div>
  );
}
