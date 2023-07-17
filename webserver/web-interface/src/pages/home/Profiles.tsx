import Copy from '@mui/icons-material/CopyAll';
import Delete from '@mui/icons-material/Delete';
import Edit from '@mui/icons-material/Edit';
import {
  Box, Button,
  Card,
  CardActions,
  CardContent,
  Container,
  Dialog,
  DialogActions,
  DialogContent, DialogContentText,
  DialogTitle,
  Paper,
  Skeleton,
  Typography,
  useMediaQuery,
  useTheme,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import QrCodeIcon from '@mui/icons-material/QrCode';
import UploadFileIcon from '@mui/icons-material/UploadFile';
import Grid from '@mui/material/Unstable_Grid2';
import React, { useCallback, useState } from 'react';
import {
  createProfile, deleteProfileById, getProfileById, updateProfile,
} from '../../components/client/ProfileClient';
import AvailableProfileSelector from '../../components/profile/AvailableProfileSelector';
import { ProfileReview } from '../../components/profile/ProfilePreview';
import { Profile } from '../../models/profile';
import useProfileStore from '../../state/ProfileStore';
import ProfileEditDialog from '../../components/profile/edit/ProfileEditDialog';

export default function Profiles() {
  const theme = useTheme();
  const smallScreen = useMediaQuery(theme.breakpoints.only('xs'));
  const { activeProfile, fetchAvailableProfiles } = useProfileStore();

  const [selectedProfileId, setSelectedProfileId] = useState(activeProfile?.id);

  const [selectedProfile, setSelectedProfile] = useState<Profile | undefined>(activeProfile || undefined);

  const [showEdit, setShowEdit] = useState(false);

  const handleNewProfileSelected = useCallback(async (id: number) => {
    try {
      setSelectedProfileId(id);
      setSelectedProfile(undefined);
      setSelectedProfile(await getProfileById(id));
    } catch {
      setSelectedProfileId(undefined);
      setSelectedProfile(undefined);
    }
  }, []);

  const startNewProfile = (): Profile => ({
    name: 'New Profile',
    phases: [],
    waterTemperature: 93,
  });

  const handleDelete = useCallback(async () => {
    if (!selectedProfileId) return;

    await deleteProfileById(selectedProfileId);
    setSelectedProfileId(undefined);
    setSelectedProfile(undefined);
    await fetchAvailableProfiles();
  }, [fetchAvailableProfiles, selectedProfileId]);

  const handleDuplicate = useCallback(async () => {
    if (!selectedProfile) return;

    const duplicate = { ...selectedProfile, name: `${selectedProfile.name} (copy)` };
    const profile = await createProfile(duplicate);
    await fetchAvailableProfiles();
    setSelectedProfile(profile);
    setSelectedProfileId(profile.id);
  }, [selectedProfile, fetchAvailableProfiles]);

  const handleEdit = useCallback(async () => {
    if (!selectedProfile) return;
    setShowEdit(true);
  }, [selectedProfile]);

  const handlePersistUpdatedProfile = useCallback(async (profile: Profile) => {
    if (!profile.id) return;
    await updateProfile(profile);
    fetchAvailableProfiles();
    setSelectedProfile(profile);
    setShowEdit(false);
  }, [fetchAvailableProfiles]);

  return (
    <>
      <Container sx={{ mt: theme.spacing(2), height: `calc(100vh - 60px - ${theme.spacing(2)}` }}>
        <Paper
          sx={{
            p: 2,
            maxHeight: { xs: '50vh', sm: '' },
            overflowX: 'hidden',
            overflowY: 'scroll',
            scrollbarWidth: 'thin', // Set the scrollbar width to thin (if supported)
            scrollbarColor: () => `${theme.palette.primary.main} ${theme.palette.background.paper}`, // Customize scrollbar color
            '&::-webkit-scrollbar': {
              width: '10px', // Customize scrollbar width for Webkit browsers (Chrome, Safari, etc.)
            },
            '&::-webkit-scrollbar-thumb': {
              backgroundColor: () => theme.palette.primary.main, // Customize scrollbar thumb color
              borderRadius: '10px', // Customize scrollbar thumb border radius
            },
            '&::-webkit-scrollbar-track': {
              backgroundColor: () => theme.palette.background.paper, // Customize scrollbar track color
              borderRadius: '10px', // Customize scrollbar track border radius
            },
          }}
        >
          <Typography variant="h6">Manage profiles</Typography>
          <Grid container spacing={2}>
            {smallScreen && (
            <Grid xs={12}>
              {selectedProfile && (
              <>
                <ProfileReview profile={selectedProfile} />
                <ActionBar onDeleteConfirmed={handleDelete} onEdit={handleEdit} onDuplicate={handleDuplicate} />
              </>
              )}
              {!selectedProfile && <Skeleton variant="rounded" sx={{ height: '25vh' }} />}
            </Grid>
            )}
            <Grid
              xs={12}
              sm={4}
              md={3}
              sx={{
                overflowX: 'hidden',
                overflowY: 'scroll',
                maxHeight: { sm: '100%' },
                scrollbarWidth: 'thin',
                scrollbarColor: () => `${theme.palette.primary.main} ${theme.palette.background.paper}`, // Customize scrollbar color
                '&::-webkit-scrollbar': {
                  width: '10px', // Customize scrollbar width for Webkit browsers (Chrome, Safari, etc.)
                },
                '&::-webkit-scrollbar-thumb': {
                  backgroundColor: () => theme.palette.primary.main, // Customize scrollbar thumb color
                  borderRadius: '10px', // Customize scrollbar thumb border radius
                },
                '&::-webkit-scrollbar-track': {
                  backgroundColor: () => theme.palette.background.paper, // Customize scrollbar track color
                  borderRadius: '10px', // Customize scrollbar track border radius
                },
              }}
            >
              <AvailableProfileSelector selectedProfileId={selectedProfileId} onSelected={handleNewProfileSelected} />
            </Grid>
            {!smallScreen && (
            <Grid sm={8} md={7}>
              {selectedProfile && (
              <Box>
                <ProfileReview profile={selectedProfile} />
                <ActionBar onDeleteConfirmed={handleDelete} onEdit={handleEdit} onDuplicate={handleDuplicate} />
              </Box>
              )}
              {!selectedProfile && <Skeleton variant="rounded" sx={{ height: '30vh' }} />}
            </Grid>
            )}
          </Grid>
          {selectedProfile && (
          <ProfileEditDialog
            profile={selectedProfile}
            open={showEdit}
            onClose={() => setShowEdit(false)}
            onDone={handlePersistUpdatedProfile}
          />
          )}
        </Paper>
      </Container>
      <Container sx={{ mt: theme.spacing(2) }}>
        <Card sx={{ mt: theme.spacing(2) }}>
          <Grid container columns={{ xs: 1, sm: 2 }}>
            <Grid xs={1}>
              <CardContent>
                <Typography gutterBottom variant="h5">
                  Load Profile
                </Typography>
              </CardContent>
              <CardActions>
                <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                  <input hidden accept=".json" type="file" onChange={(evt) => console.log(evt)} />
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
    </>
  );
}

export interface ActionBarProp {
  onDeleteConfirmed: () => void;
  onEdit: () => void;
  onDuplicate: () => void;
}

function ActionBar({ onDeleteConfirmed, onEdit, onDuplicate }: ActionBarProp) {
  return (
    <Box sx={{
      mt: 2, display: 'flex', justifyContent: 'space-between', alignItems: 'center',
    }}
    >
      <Button
        size="small"
        onClick={onEdit}
        color="info"
        variant="outlined"
        startIcon={<Edit />}
      >
        Edit
      </Button>
      <Button
        size="small"
        onClick={onDuplicate}
        color="info"
        variant="outlined"
        startIcon={<Copy />}
      >
        Duplicate
      </Button>
      <DeleteProfileButton onConfirm={onDeleteConfirmed} />
    </Box>
  );
}

export function DeleteProfileButton({ onConfirm }: { onConfirm: () => void}) {
  const [open, setOpen] = useState(false);

  const handleClose = () => {
    setOpen(false);
  };

  const handleConfirm = () => {
    setOpen(false);
    onConfirm();
  };

  return (
    <div>
      <Button
        onClick={() => setOpen(true)}
        size="small"
        color="error"
        variant="outlined"
        startIcon={<Delete />}
      >
        Delete
      </Button>
      <Dialog
        open={open}
        onClose={handleClose}
        aria-labelledby="alert-dialog-title"
        aria-describedby="alert-dialog-description"
      >
        <DialogTitle id="alert-dialog-title">Are you sure you want to delete the profile?</DialogTitle>
        <DialogContent>
          <DialogContentText id="alert-dialog-description">
            Deleting a profile is irreversible, and all data associated with the profile will be permanently deleted.
          </DialogContentText>
        </DialogContent>
        <DialogActions>
          <Button
            size="small"
            color="info"
            variant="outlined"
            onClick={handleClose}
            autoFocus
          >
            Cancel
          </Button>
          <Button
            size="small"
            color="error"
            variant="outlined"
            onClick={handleConfirm}
            startIcon={<Delete />}
          >
            Delete
          </Button>
        </DialogActions>
      </Dialog>
    </div>
  );
}
