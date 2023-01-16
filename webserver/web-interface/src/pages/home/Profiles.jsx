import React, { useState } from 'react';
import {
  Card, Container, useTheme, Typography, CardContent, CardActions,
} from '@mui/material';
import IconButton from '@mui/material/IconButton';
import QrCodeIcon from '@mui/icons-material/QrCode';
import UploadFileIcon from '@mui/icons-material/UploadFile';
import AddIcon from '@mui/icons-material/Add';
import RemoveIcon from '@mui/icons-material/Remove';
import AutoGraphIcon from '@mui/icons-material/AutoGraph';
import DeleteIcon from '@mui/icons-material/Delete';
import TextField from '@mui/material/TextField';
import Select from '@mui/material/Select';
// import InputAdornment from '@mui/material/InputAdornment';
import Grid from '@mui/material/Unstable_Grid2';

export default function Profiles() {
  const theme = useTheme();
  // here
  const [elements, setElements] = useState([
    { id: 1, type: 'select', value: '' },
    { id: 2, type: 'select', value: '' },
    { id: 3, type: 'text', value: '' },
    { id: 4, type: 'text', value: '' },
    { id: 5, type: 'text', value: '' },
    { id: 6, type: 'text', value: '' },
  ]);
  const [nextId, setNextId] = useState(7);

  const handleAddRow = () => {
    const newElements = [
      ...elements,
      { id: nextId, type: 'select', value: '' },
      { id: nextId + 1, type: 'select', value: '' },
      { id: nextId + 2, type: 'text', value: '' },
      { id: nextId + 3, type: 'text', value: '' },
      { id: nextId + 4, type: 'text', value: '' },
      { id: nextId + 5, type: 'text', value: '' },
    ];
    setElements(newElements);
    setNextId(nextId + 6);
  };

  const handleRemoveRow = () => {
    const newElements = [...elements];
    for (let i = 0; i < 6; i++) {
      newElements.pop();
    }
    setElements(newElements);
  };

  const handleRemoveAll = () => {
    setElements([
      { id: 1, type: 'select', value: '' },
      { id: 2, type: 'select', value: '' },
      { id: 3, type: 'text', value: '' },
      { id: 4, type: 'text', value: '' },
      { id: 5, type: 'text', value: '' },
      { id: 6, type: 'text', value: '' },
    ]);
    setNextId(7);
  };

  const handleSelectChange = (event, id) => {
    const updatedElements = elements.map((element) => {
      if (element.id === id) {
        return { ...element, value: event.target.value };
      }
      return element;
    });
    setElements(updatedElements);
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
                  <IconButton style={{ float: 'right' }} onClick={handleRemoveAll} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <DeleteIcon fontSize="large" />
                  </IconButton>
                  <IconButton style={{ float: 'right' }} onClick={handleRemoveRow} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <RemoveIcon fontSize="large" />
                  </IconButton>
                  <IconButton style={{ float: 'right' }} onClick={handleAddRow} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <AddIcon fontSize="large" />
                  </IconButton>
                  <IconButton style={{ float: 'right' }} color="primary" aria-label="upload picture" component="label" sx={{ ml: theme.spacing(3) }}>
                    <AutoGraphIcon fontSize="large" />
                  </IconButton>
                  <div>
                    <Grid container spacing={2}>
                      {elements.map((element) => {
                        if (element.type === 'select') {
                          return (
                            <Grid item xs={6} key={element.id}>
                              <Select
                                value={element.value}
                                onChange={(event) => handleSelectChange(event, element.id)}
                              >
                                <option value="1">Option1</option>
                                <option value="2">Option2</option>
                                <option value="3">Option3</option>
                              </Select>
                            </Grid>
                          );
                        }
                        if (element.type === 'text') {
                          return (
                            <Grid item xs={3} key={element.id}>
                              <TextField value={element.value} />
                            </Grid>
                          );
                        }
                        return null;
                      })}
                    </Grid>
                  </div>
                  {/* {inputList} */}
                </Typography>
              </CardContent>
            </Grid>
          </Grid>
        </Card>
      </Container>
    </div>
  );
}
