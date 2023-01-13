import React, { useState } from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import Box from '@mui/material/Box';
import InputLabel from '@mui/material/InputLabel';
import MenuItem from '@mui/material/MenuItem';
import FormControl from '@mui/material/FormControl';
import Select from '@mui/material/Select';
import QrCodeIcon from '@mui/icons-material/QrCode';
import UploadFileIcon from '@mui/icons-material/UploadFile';
import LibraryAddIcon from '@mui/icons-material/LibraryAdd';
import AutoGraphIcon from '@mui/icons-material/AutoGraph';
import TextField from '@mui/material/TextField';
import Grid from '@mui/material/Unstable_Grid2';

export default function Profiles() {
  const theme = useTheme();

  const [inputList, setInputList] = useState([]);

  const [profile, setProfile] = React.useState('');

  const addProfileStep = (event) => {
    setProfile(event.target.value);
  };

  const handleButtonClick = () => {
    setInputList([...inputList,
      <br></br>,
      <Box
        sx={{
          // minWidth: 120,
          mr: theme.spacing(2),
          mt: theme.spacing(2),
        }}
      >
        <FormControl focused size>
          <InputLabel id="demo-simple-select-label">Type</InputLabel>
          <Select labelId="phase-select" key={inputList.length} size="small" id="phase-type-select" value={profile} label="Type" variant="outlined" onChange={addProfileStep}>
            <MenuItem value={1}>Pressure</MenuItem>
            <MenuItem value={2}>Flow</MenuItem>
          </Select>
        </FormControl>
      </Box>,
      <TextField id="outlined-basic" key={inputList.length + 1} size="small" label="Bar" variant="outlined" sx={{ mr: theme.spacing(2), mt: theme.spacing(1) }} />,
      <TextField id="outlined-basic" key={inputList.length + 1} size="small" label="Flow" variant="outlined" sx={{ mr: theme.spacing(2), mt: theme.spacing(1) }} />,
      <TextField id="outlined-basic" key={inputList.length + 1} size="small" label="Time" variant="outlined" sx={{ mr: theme.spacing(2), mt: theme.spacing(1) }} />,
    ]);
  };

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
                  Build Profile
                  <IconButton style={{ float: 'right' }} onClick={handleButtonClick} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <LibraryAddIcon fontSize="large" />
                  </IconButton>
                  <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <AutoGraphIcon fontSize="large" />
                  </IconButton>
                  {inputList}
                </Typography>
              </CardContent>
            </Grid>
          </Grid>
        </Card>
      </Container>
    </div>
  );
}
