import React from 'react';
import {
  Card,
  Container,
  useTheme,
  Typography,
  CardContent,
  CardActions,
  Button,
  Radio,
  RadioGroup,
  FormControlLabel,
  Box,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import AttachFileOutlinedIcon from '@mui/icons-material/AttachFileOutlined';
import Grid from '@mui/material/Unstable_Grid2';
import SaveIcon from '@mui/icons-material/Save';
import WifiSettingsCard from '../../components/wifi/WifiSettingsCard';
import ProgressBar from '../../components/inputs/ProgressBar';
import TabbedSettings from '../../components/settings/tabs_settings';
import useSettingsStore from '../../state/SettingsStore';
import { GaggiaSettings, NotificationType } from '../../models/models';
import useNotificationStore from '../../state/NotificationDataStore';

export default function Settings() {
  const theme = useTheme();
  const { settings, updateSettingsAndSync, persistSettings } = useSettingsStore();
  const { updateLatestNotification } = useNotificationStore();

  async function handlePersistSettings() {
    try {
      await persistSettings();
      updateLatestNotification({ message: 'Successfully persisted settings', type: NotificationType.SUCCESS });
    } catch (e) {
      updateLatestNotification({ message: 'Failed to persisted settings', type: NotificationType.ERROR });
    }
  }

  async function handleUpdateRemoteSettings(newSettings: GaggiaSettings) {
    try {
      await updateSettingsAndSync(newSettings);
      updateLatestNotification({ message: 'Updated running settings.', type: NotificationType.SUCCESS });
    } catch (e) {
      updateLatestNotification({ message: 'Failed to update settings', type: NotificationType.ERROR });
    }
  }

  return (
    <div>
      <Container sx={{ mt: theme.spacing(1) }}>
        <Grid container spacing={2} alignItems="stretch">
          {settings && (
          <Grid xs={12} sm={12}>
            <Box sx={{
              border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '10px', width: '100%', padding: '1px',
            }}
            >
              <Card sx={{ height: '100%' }}>
                <CardContent sx={{ width: '100%' }}>
                  <Box sx={{
                    display: 'flex', alignItems: 'center', justifyContent: 'space-between', mb: theme.spacing(2),
                  }}
                  >
                    <Typography variant="h5">
                      Machine State Management
                    </Typography>
                    <IconButton color="inherit" onClick={handlePersistSettings}><SaveIcon fontSize="inherit" /></IconButton>
                  </Box>
                  <TabbedSettings settings={settings} onChange={handleUpdateRemoteSettings} />
                </CardContent>
              </Card>
            </Box>
          </Grid>
          )}
          <Grid xs={12} sm={6}>
            <WifiSettingsCard />
          </Grid>
          <Grid xs={12} sm={6}>
            <Card sx={{ height: '100%' }}>
              <CardContent>
                <Typography variant="h5" sx={{ mb: theme.spacing(2) }}>
                  OTA Update
                </Typography>
              </CardContent>
              <CardActions><ProgressBar /></CardActions>
              <CardActions>
                <Box sx={{
                  border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '16px', width: '10%', padding: '1px',
                }}
                >
                  <IconButton color="primary" aria-label="download  " component="label" size="large">
                    <input hidden accept=".bin" type="file" />
                    <AttachFileOutlinedIcon />
                  </IconButton>
                </Box>
                <Box sx={{
                  border: `0px solid ${theme.palette.divider}`, position: 'relative', borderRadius: '10px', width: '100%', padding: '1px',
                }}
                >
                  <RadioGroup aria-labelledby="demo-radio-buttons-group-label" defaultValue="blackpill" name="radio-buttons-group">
                    <FormControlLabel value="blackpill" control={<Radio />} label="STM32-Blackpill" />
                    <FormControlLabel value="esp32-flash" control={<Radio />} label="ESP32-Flash" />
                    <FormControlLabel value="esp32-filesystem" control={<Radio />} label="ESP32-Filesystem" />
                  </RadioGroup>
                </Box>
                <Button variant="contained" component="label">Upload</Button>
              </CardActions>
            </Card>
          </Grid>
        </Grid>
      </Container>
    </div>
  );
}
